add_library(NaadaLabManual STATIC
    ManualDocumentParser.h
    ManualDocumentParser.cpp
)

add_library(NaadaLab::Manual ALIAS NaadaLabManual)

target_include_directories(NaadaLabManual
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(NaadaLabManual
    PUBLIC
        Qt6::Core
        Qt6::Gui
        Qt6::Svg
)