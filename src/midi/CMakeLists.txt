set(NAADALAB_MIDI_SOURCES
    IMidiIn.h
    IMidiOut.h
    MidiMessage.h
    MidiMonoIn.h
    MidiInFactory.h
    MidiInFactory.cpp
    MidiOutFactory.h
    MidiOutFactory.cpp
)

if(WIN32)
    list(APPEND NAADALAB_MIDI_SOURCES
        MidiIn_WinMM.h
        MidiIn_WinMM.cpp
        MidiOut_WinMM.h
        MidiOut_WinMM.cpp
    )
elseif(ANDROID)
    list(APPEND NAADALAB_MIDI_SOURCES
        MidiAndroidBridge.h
        MidiAndroidBridge.cpp
        MidiIn_Android.h
        MidiIn_Android.cpp
        MidiOut_Android.h
        MidiOut_Android.cpp
    )
elseif(IOS)
    list(APPEND NAADALAB_MIDI_SOURCES
        MidiIn_Stub.h
        MidiIn_Stub.cpp
        MidiOut_Stub.h
        MidiOut_Stub.cpp
    )
else()
    list(APPEND NAADALAB_MIDI_SOURCES
        MidiIn_Stub.h
        MidiIn_Stub.cpp
        MidiOut_Stub.h
        MidiOut_Stub.cpp
    )
endif()

qt_add_library(NaadaLabMidi STATIC
    ${NAADALAB_MIDI_SOURCES}
)

add_library(NaadaLab::Midi ALIAS NaadaLabMidi)

target_include_directories(NaadaLabMidi
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(NaadaLabMidi
    PUBLIC
        Qt6::Core
)

if(WIN32)
    target_link_libraries(NaadaLabMidi
        PRIVATE
            winmm
    )
endif()