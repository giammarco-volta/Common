package com.naadalab.licensing;

import android.app.Activity;
import android.os.Handler;
import android.os.Looper;
import android.os.SystemClock;
import android.provider.Settings;
import android.util.Base64;
import com.android.billingclient.api.*;
import com.google.android.play.core.integrity.IntegrityManagerFactory;
import com.google.android.play.core.integrity.StandardIntegrityManager;
import com.google.android.play.core.integrity.StandardIntegrityManager.PrepareIntegrityTokenRequest;
import com.google.android.play.core.integrity.StandardIntegrityManager.StandardIntegrityTokenRequest;
import org.json.JSONObject;
import java.io.*;
import java.net.URL;
import javax.net.ssl.HttpsURLConnection;
import java.nio.charset.StandardCharsets;
import java.security.*;
import java.security.spec.X509EncodedKeySpec;
import java.util.*;
import java.util.concurrent.Executors;
import java.util.concurrent.ExecutorService;

/** All mutable state belongs to the main thread; Qt reads a volatile JSON snapshot. */
public final class PlayLicenseBridge {
    private static final Handler UI = new Handler(Looper.getMainLooper());
    private static final ExecutorService IO = Executors.newSingleThreadExecutor();
    private static PlayLicenseBridge instance;
    private static volatile String snapshot = "{\"allowed\":false,\"busy\":true,\"state\":\"checking\",\"message\":\"Checking access…\"}";
    private final Activity activity;
    private final String app;
    private JSONObject config;
    private JSONObject lease;
    private String device = "", state = "not_started", message = "", price = "";
    private boolean busy, allowed, configured, sessionGranted, checkoutOpen;
    private long serverAnchor, elapsedAnchor;
    private BillingClient billing;
    private ProductDetails product;
    private String offerToken;
    private String productMessage = "";
    private boolean productLoading;
    private int productRequest;
    private StandardIntegrityManager integrity;
    private StandardIntegrityManager.StandardIntegrityTokenProvider integrityProvider;
    private String purchaseToken = "";
    private boolean purchasePending;
    private boolean reviewGranted;
    private int operation;

    private PlayLicenseBridge(Activity a) { activity=a; app=a.getPackageName(); }
    public static void initialize(Activity a) {
        UI.post(() -> { if(instance==null) { instance=new PlayLicenseBridge(a); instance.initialize(); } });
    }
    public static String status() { return snapshot; }
    public static void activate() { UI.post(() -> { if(instance!=null && !instance.busy) instance.refresh(true); }); }
    public static void restore() { UI.post(() -> { if(instance!=null && !instance.busy) instance.refresh(false); }); }
    public static void review(String code) { UI.post(() -> { if(instance!=null) instance.unlockReview(code); }); }
    public static void buy() { UI.post(() -> { if(instance!=null) instance.purchase(); }); }
    public static void resume() { UI.post(() -> {
        // Checkout can close without delivering PurchasesUpdated (for example after disconnection).
        // Query Play again on return, even while the checkout operation was marked busy.
        if(instance!=null && (!instance.busy || instance.checkoutOpen)) instance.refresh(false);
    }); }

    private void initialize() {
        try (InputStream input=activity.getAssets().open("naadalab-license.json")) {
            config=new JSONObject(read(input, 32768));
            URL url=new URL(config.getString("endpoint"));
            if(!"https".equals(url.getProtocol()) || url.getUserInfo()!=null || config.getLong("cloudProjectNumber")<=0 ||
                config.getString("publicKey").isEmpty() || !config.getString("productId").equals("full_version"))
                throw new IllegalArgumentException();
            String id=Settings.Secure.getString(activity.getContentResolver(),Settings.Secure.ANDROID_ID);
            if(id==null || id.isEmpty()) throw new IllegalStateException();
            device=hex(MessageDigest.getInstance("SHA-256").digest((app+":"+id).getBytes(StandardCharsets.UTF_8)));
            configured=true;
            if (loadReview()) { showReview(); return; }
            loadCache();
            integrity=IntegrityManagerFactory.createStandard(activity);
            billing=BillingClient.newBuilder(activity).setListener(this::onPurchasesUpdated)
                .enablePendingPurchases(PendingPurchasesParams.newBuilder().enableOneTimeProducts().build())
                .enableAutoServiceReconnection().build();
            busy=true; message="Connecting to Google Play…"; publish();
            final int connecting=++operation;
            UI.postDelayed(()->{if(connecting==operation && busy)fail("Google Play did not respond. Please retry online.");},90000);
            billing.startConnection(new BillingClientStateListener() {
                @Override public void onBillingSetupFinished(BillingResult result) {
                    if(reviewGranted) return;
                    if(result.getResponseCode()==BillingClient.BillingResponseCode.OK) { loadProduct(); refresh(false); }
                    else fail("Google Play is unavailable. Check your account and connection, then retry.");
                }
                @Override public void onBillingServiceDisconnected() {
                    // Automatic reconnection is enabled; explicit user actions can retry.
                    fail("Google Play disconnected. Please retry.");
                }
            });
        } catch(Exception e) {
            state="not_configured"; message="Purchases are not configured in this build."; busy=false; publish();
        }
    }
    private File reviewFile() { return new File(activity.getNoBackupFilesDir(), "naadalab-review.txt"); }
    private boolean loadReview() {
        try (InputStream input = new FileInputStream(reviewFile())) {
            reviewGranted = ReviewAccessToken.verify(read(input, 1024), app, config.getString("reviewPublicKey"));
        } catch (Exception e) { reviewGranted = false; }
        return reviewGranted;
    }
    private void showReview() {
        ++operation; busy=false; checkoutOpen=false; purchasePending=false;
        reviewGranted=true; allowed=true; sessionGranted=true; state="review";
        message="Review access: all features unlocked. No purchase, trial or expiry.";
        publish();
    }
    private void unlockReview(String code) {
        if (!configured || !ReviewAccessToken.verify(code, app, config.optString("reviewPublicKey"))) {
            fail("Invalid review code. Paste the complete code supplied for this app."); return;
        }
        // Validate first, then persist only the signed credential in Android's no-backup area.
        boolean saved=false;
        File temp=new File(activity.getNoBackupFilesDir(), "naadalab-review.tmp");
        try {
            try (FileOutputStream output=new FileOutputStream(temp)) {
                output.write(code.trim().getBytes(StandardCharsets.US_ASCII)); output.getFD().sync();
            }
            saved=temp.renameTo(reviewFile());
        } catch (Exception ignored) {} finally { temp.delete(); }
        showReview();
        if (!saved) { message+=" Storage unavailable: re-enter the code after restarting."; publish(); }
    }
    private void loadProduct() {
        if(billing==null || productLoading) return;
        productLoading=true;
        final int request=++productRequest;
        productMessage="Loading purchase price from Google Play…";
        publish();
        UI.postDelayed(()->{
            if(request==productRequest && productLoading) {
                ++productRequest; productLoading=false;
                productMessage="Google Play did not return a purchase price. Use Restore / check access to retry. [catalog-timeout]";
                android.util.Log.i("NaadaLabBilling", "catalog-timeout");
                publish();
            }
        },30000);
        QueryProductDetailsParams.Product p=QueryProductDetailsParams.Product.newBuilder()
            .setProductId(config.optString("productId")).setProductType(BillingClient.ProductType.INAPP).build();
        billing.queryProductDetailsAsync(QueryProductDetailsParams.newBuilder().setProductList(Collections.singletonList(p)).build(),
            (result,details)->UI.post(()->{
                if(request!=productRequest) return;
                productLoading=false;
                product=null; offerToken=null; price="";
                String diagnostic="response="+result.getResponseCode();
                boolean foundProduct=false;
                if(result.getResponseCode()==BillingClient.BillingResponseCode.OK) {
                    for(ProductDetails d:details.getProductDetailsList()) {
                        if(!d.getProductId().equals(config.optString("productId"))) continue;
                        foundProduct=true;
                        List<ProductDetails.OneTimePurchaseOfferDetails> offers=d.getOneTimePurchaseOfferDetailsList();
                        if(offers==null) continue;
                        for(ProductDetails.OneTimePurchaseOfferDetails offer:offers) {
                            diagnostic+="; option="+offer.getPurchaseOptionId()+", offer="+offer.getOfferId();
                            if("buy".equals(offer.getPurchaseOptionId()) && offer.getOfferId()==null
                                && offer.getOfferToken()!=null && !offer.getOfferToken().isEmpty()) {
                                product=d;offerToken=offer.getOfferToken();price=offer.getFormattedPrice();break;
                            }
                        }
                    }
                    for(UnfetchedProduct missing:details.getUnfetchedProductList()) {
                        if(config.optString("productId").equals(missing.getProductId()))
                            diagnostic+="; unfetched="+missing.getStatusCode();
                    }
                }
                if(product!=null) productMessage="";
                else if(result.getResponseCode()!=BillingClient.BillingResponseCode.OK)
                    productMessage="Google Play could not load the purchase price. Use Restore / check access to retry.";
                else if(!foundProduct)
                    productMessage="Google Play has not made this product available for this account yet.";
                else
                    productMessage="Google Play returned the product without the required buy option.";
                if(product==null) productMessage+=" ["+diagnostic+"]";
                // Log catalog metadata only: never purchase tokens, account details or Integrity tokens.
                android.util.Log.i("NaadaLabBilling", diagnostic+"; selected="+(product!=null));
                publish();
            }));
    }
    private void refresh(boolean activate) {
        if(reviewGranted) { showReview(); return; }
        if(!configured || billing==null) return;
        if(product==null) loadProduct();
        checkoutOpen=false;
        final int current=++operation;
        UI.postDelayed(()->{if(current==operation && busy)fail("The access check timed out. Please retry online.");},90000);
        busy=true;message="Checking access…";publish();
        billing.queryPurchasesAsync(QueryPurchasesParams.newBuilder().setProductType(BillingClient.ProductType.INAPP).build(),
            (result,purchases)->UI.post(()->{
                if(current!=operation)return;
                if(result.getResponseCode()!=BillingClient.BillingResponseCode.OK) { fail("Unable to check purchases. Please retry when connected.");return; }
                selectPurchase(purchases);
                requestLease(activate?"activate":"status",current);
            }));
    }
    private void selectPurchase(List<Purchase> purchases) {
        purchaseToken="";purchasePending=false;
        for(Purchase p:purchases) if(p.getProducts().contains(config.optString("productId"))) {
            if(p.getPurchaseState()==Purchase.PurchaseState.PURCHASED) purchaseToken=p.getPurchaseToken();
            if(p.getPurchaseState()==Purchase.PurchaseState.PENDING) purchasePending=true;
        }
    }
    private void onPurchasesUpdated(BillingResult result,List<Purchase> purchases) {
        UI.post(()->{
            if(reviewGranted) return;
            int code=result.getResponseCode();
            if(code==BillingClient.BillingResponseCode.OK || code==BillingClient.BillingResponseCode.ITEM_ALREADY_OWNED) refresh(false);
            else if(code==BillingClient.BillingResponseCode.USER_CANCELED) {
                // A resume-triggered verification may already be running. Let it finish.
                if(checkoutOpen) fail("Purchase cancelled. No unlock was applied.");
            }
            else fail("The purchase could not be completed. Restore purchases to check its status before trying again.");
        });
    }
    private void purchase() {
        if(reviewGranted) { showReview(); return; }
        if(!configured || busy) return;
        if(product==null || offerToken==null) {loadProduct();fail("The product is not available yet. Check Google Play and retry.");return;}
        if(purchasePending) {message="Payment is pending. Access will unlock after Google confirms it.";publish();return;}
        final int checkout=++operation;
        checkoutOpen=true;busy=true;message="Complete the purchase in Google Play.";publish();
        UI.postDelayed(()->{
            if(checkout==operation && checkoutOpen)
                fail("Google Play has not returned a result. Restore purchases to check the payment status.");
        },90000);
        BillingFlowParams.ProductDetailsParams item=BillingFlowParams.ProductDetailsParams.newBuilder()
            .setProductDetails(product).setOfferToken(offerToken).build();
        BillingResult result=billing.launchBillingFlow(activity,BillingFlowParams.newBuilder()
            .setProductDetailsParamsList(Collections.singletonList(item)).build());
        if(result.getResponseCode()!=BillingClient.BillingResponseCode.OK) {
            if(result.getResponseCode()==BillingClient.BillingResponseCode.ITEM_ALREADY_OWNED) refresh(false);
            else {product=null;offerToken=null;fail("Unable to open Google Play checkout. Please retry.");loadProduct();}
        }
    }
    private void requestLease(String action,int current) {
        try {
            JSONObject request=new JSONObject().put("app",app).put("device",device).put("action",action)
                .put("purchaseToken",purchaseToken).put("requestId",UUID.randomUUID().toString().replace("-",""));
            String canonical=String.join("\n","1",app,device,action,purchaseToken,request.getString("requestId"));
            String hash=b64(MessageDigest.getInstance("SHA-256").digest(canonical.getBytes(StandardCharsets.UTF_8)));
            if(integrityProvider==null) {
                integrity.prepareIntegrityToken(PrepareIntegrityTokenRequest.builder().setCloudProjectNumber(config.getLong("cloudProjectNumber")).build())
                    .addOnSuccessListener(provider->{if(current!=operation)return;integrityProvider=provider;attest(request,hash,current);})
                    .addOnFailureListener(e->{if(current==operation)fail("Unable to verify this installation. Install from Google Play and retry online.");});
            } else attest(request,hash,current);
        } catch(Exception e) {fail("Unable to prepare the access check.");}
    }
    private void attest(JSONObject request,String hash,int current) {
        integrityProvider.request(StandardIntegrityTokenRequest.builder().setRequestHash(hash).build())
            .addOnSuccessListener(token->{
                if(current!=operation)return;
                try {request.put("integrityToken",token.token());}catch(Exception e){fail("Unable to verify access.");return;}
                IO.execute(()->{
                    try {
                        HttpsURLConnection connection=(HttpsURLConnection)new URL(config.getString("endpoint")).openConnection();
                        connection.setInstanceFollowRedirects(false);connection.setConnectTimeout(15000);connection.setReadTimeout(45000);
                        connection.setRequestMethod("POST");connection.setDoOutput(true);connection.setRequestProperty("Content-Type","application/json");
                        byte[] body=request.toString().getBytes(StandardCharsets.UTF_8);connection.setFixedLengthStreamingMode(body.length);
                        try(OutputStream output=connection.getOutputStream()){output.write(body);}
                        int code=connection.getResponseCode();String text;
                        try(InputStream input=code==200?connection.getInputStream():connection.getErrorStream()){text=input==null?"":read(input,32768);}
                        finally {connection.disconnect();}
                        if(code!=200) throw new IOException("Access service unavailable");
                        JSONObject envelope=new JSONObject(text);
                        JSONObject verified=verify(envelope,request.getString("requestId"));
                        UI.post(()->{if(current==operation)accept(envelope,verified);});
                    }catch(Exception e){UI.post(()->{if(current==operation)fail("Access could not be verified. Check your connection and retry. Existing valid access is preserved.");});}
                });
            }).addOnFailureListener(e->{if(current!=operation)return;integrityProvider=null;fail("Google Play verification failed. Please retry online.");});
    }
    private JSONObject verify(JSONObject envelope,String requestId) throws Exception {
        String payload=envelope.getString("payload");
        String key=config.getString("publicKey").replace("-----BEGIN PUBLIC KEY-----","").replace("-----END PUBLIC KEY-----","").replaceAll("\\s","");
        PublicKey publicKey=KeyFactory.getInstance("RSA").generatePublic(new X509EncodedKeySpec(Base64.decode(key,Base64.DEFAULT)));
        Signature verifier=Signature.getInstance("SHA256withRSA");verifier.initVerify(publicKey);verifier.update(payload.getBytes(StandardCharsets.US_ASCII));
        if(!verifier.verify(unb64(envelope.getString("signature")))) throw new SecurityException();
        JSONObject p=new JSONObject(new String(unb64(payload),StandardCharsets.UTF_8));
        if(p.getInt("v")!=1 || !app.equals(p.getString("app")) || !device.equals(p.getString("device")) ||
            (requestId!=null && !requestId.equals(p.getString("requestId"))) ||
            !Arrays.asList("not_started","trial","expired","purchased").contains(p.getString("state")) ||
            p.getLong("issuedAt")<=0 || p.getLong("validUntil")<p.getLong("issuedAt")) throw new SecurityException();
        return p;
    }
    private void accept(JSONObject envelope,JSONObject verified) {
        lease=verified;serverAnchor=verified.optLong("issuedAt");elapsedAnchor=SystemClock.elapsedRealtime();
        state=verified.optString("state");allowed=sessionGranted || isValidAt(serverAnchor);
        // An active musical session is not interrupted by a failed network check or clock expiry.
        sessionGranted=allowed;busy=false;++operation;
        message=purchasePending?"Payment is pending. Google Play has not confirmed the purchase yet.":
            state.equals("purchased")?"Full version unlocked.":state.equals("trial")?"Your 7-day trial is active.":
            state.equals("expired")?"Your trial has ended. Your presets are kept.":"Start your 7-day trial when you are ready.";
        try {
            JSONObject cache=new JSONObject().put("envelope",envelope).put("wall",System.currentTimeMillis())
                .put("elapsed",elapsedAnchor).put("boot",bootCount());
            File file=new File(activity.getNoBackupFilesDir(),"naadalab-license.json");
            File temp=new File(activity.getNoBackupFilesDir(),"naadalab-license.tmp");
            try(FileOutputStream output=new FileOutputStream(temp)){output.write(cache.toString().getBytes(StandardCharsets.UTF_8));output.getFD().sync();}
            if(!temp.renameTo(file)) {temp.delete();throw new IOException("Cannot save verified access");}
        }catch(Exception e){message+=" Connect again after restarting if access cannot be restored.";}
        publish();
    }
    private int bootCount(){return Settings.Global.getInt(activity.getContentResolver(),Settings.Global.BOOT_COUNT,-1);}
    private void loadCache() {
        try(InputStream input=new FileInputStream(new File(activity.getNoBackupFilesDir(),"naadalab-license.json"))){
            JSONObject c=new JSONObject(read(input,32768));lease=verify(c.getJSONObject("envelope"),null);
            if(lease.optString("state").equals("purchased") && lease.optBoolean("paidOfflineForever")) {
                allowed=true;sessionGranted=true;state="purchased";return;
            }
            long elapsed=SystemClock.elapsedRealtime(), saved=c.getLong("elapsed");
            if(bootCount()<0 || bootCount()!=c.getInt("boot") || elapsed<saved) {lease=null;return;}
            // Trust monotonic time only. After reboot an online check is required.
            serverAnchor=lease.getLong("issuedAt")+(elapsed-saved)/1000;elapsedAnchor=elapsed;
            allowed=isValidAt(serverAnchor);sessionGranted=allowed;state=lease.getString("state");
            if(!allowed && state.equals("trial"))state="expired";
        }catch(Exception e){lease=null;allowed=false;}
    }
    private boolean isValidAt(long now){return lease!=null &&
        ((lease.optString("state").equals("purchased") && lease.optBoolean("paidOfflineForever")) ||
        ((lease.optString("state").equals("trial")||lease.optString("state").equals("purchased")) && now<lease.optLong("validUntil")));}
    private void fail(String text){if(reviewGranted){showReview();return;}++operation;checkoutOpen=false;busy=false;allowed=sessionGranted;message=text;publish();}
    private void publish(){
        try{snapshot=new JSONObject().put("allowed",allowed).put("busy",busy).put("state",state)
            .put("message",message).put("price",price).put("canBuy",configured&&!reviewGranted&&product!=null&&!purchasePending)
            .put("productMessage",productMessage)
            .put("configured",configured).put("purchasePending",purchasePending).put("trialEndsAt",lease==null?0:lease.optLong("trialEndsAt"))
            .put("trialAvailableAt",lease==null?0:lease.optLong("trialAvailableAt"))
            .put("supportReference",device.isEmpty()?"":"NL1:"+app+":"+device)
            .put("privacyUrl",config==null?"":config.optString("privacyUrl")).toString();}catch(Exception ignored){}
    }
    private static String read(InputStream input,int limit)throws IOException{
        ByteArrayOutputStream output=new ByteArrayOutputStream();byte[] b=new byte[4096];int n;
        while((n=input.read(b))!=-1){if(output.size()+n>limit)throw new IOException("Response too large");output.write(b,0,n);}
        return output.toString("UTF-8");
    }
    private static String hex(byte[] bytes){StringBuilder s=new StringBuilder();for(byte b:bytes)s.append(String.format(Locale.ROOT,"%02x",b&255));return s.toString();}
    private static String b64(byte[] b){return Base64.encodeToString(b,Base64.URL_SAFE|Base64.NO_WRAP|Base64.NO_PADDING);}
    private static byte[] unb64(String s){return Base64.decode(s,Base64.URL_SAFE|Base64.NO_WRAP|Base64.NO_PADDING);}
}
