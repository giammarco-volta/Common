function(naadalab_configure_android_midi_package target app_android_dir)
    if(NOT ANDROID)
        return()
    endif()

    if(NOT TARGET ${target})
        message(FATAL_ERROR
            "Unknown target passed to "
            "naadalab_configure_android_midi_package: ${target}"
        )
    endif()

    if(NOT IS_DIRECTORY "${app_android_dir}")
        message(FATAL_ERROR
            "Android package directory does not exist: "
            "${app_android_dir}"
        )
    endif()

    set(package_dir
        "${CMAKE_CURRENT_BINARY_DIR}/android-package-${target}"
    )

    file(MAKE_DIRECTORY "${package_dir}")
    file(COPY "${app_android_dir}/"
         DESTINATION "${package_dir}")

    set(java_package_dir
        "${package_dir}/src/org/qtproject/qt/android"
    )

    file(MAKE_DIRECTORY "${java_package_dir}")

    configure_file(
        "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../android/src/org/qtproject/qt/android/MidiAndroidBridge.java"
        "${java_package_dir}/MidiAndroidBridge.java"
        COPYONLY
    )

    set_property(
        TARGET ${target}
        PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
        "${package_dir}"
    )
endfunction()
