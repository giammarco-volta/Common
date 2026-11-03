qt_add_library(NaadaLabLicense STATIC LicenseAccess.h LicenseAccess.cpp)
add_library(NaadaLab::License ALIAS NaadaLabLicense)
target_include_directories(NaadaLabLicense PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(NaadaLabLicense PUBLIC Qt6::Core Qt6::Gui)
