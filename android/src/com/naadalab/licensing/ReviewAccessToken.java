package com.naadalab.licensing;

import java.nio.charset.StandardCharsets;
import java.security.KeyFactory;
import java.security.Signature;
import java.security.spec.X509EncodedKeySpec;
import java.util.Base64;

/** App-specific, reusable review credential. No clock, account, device or network dependency. */
public final class ReviewAccessToken {
    private ReviewAccessToken() {}
    public static boolean verify(String token, String app, String publicKeyPem) {
        try {
            if (token == null || token.length() > 1024 || app == null ||
                !(app.equals("com.naadalab.morphora") || app.equals("com.naadalab.intona"))) return false;
            token = token.trim();
            if (!token.matches("NLR1\\.[A-Za-z0-9_-]+")) return false;
            byte[] signature = Base64.getUrlDecoder().decode(token.substring(5));
            String key = publicKeyPem.replace("-----BEGIN PUBLIC KEY-----", "")
                .replace("-----END PUBLIC KEY-----", "").replaceAll("\\s", "");
            Signature verifier = Signature.getInstance("SHA256withECDSA");
            verifier.initVerify(KeyFactory.getInstance("EC").generatePublic(
                new X509EncodedKeySpec(Base64.getDecoder().decode(key))));
            // Separate signing domain: ordinary licence responses cannot become review credentials.
            verifier.update(("NaadaLab review access v1\n" + app).getBytes(StandardCharsets.US_ASCII));
            return verifier.verify(signature);
        } catch (Exception e) { return false; }
    }
}
