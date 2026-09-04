set_source_files_properties(
    Theme.qml
    UiMetrics.qml
    PROPERTIES
        QT_QML_SINGLETON_TYPE TRUE
)

qt_add_qml_module(NaadaLabUi
    STATIC
    URI NaadaLab.Ui
    VERSION 1.0
    RESOURCE_PREFIX /qt/qml
    NO_PLUGIN
    QML_FILES
        Theme.qml
        UiMetrics.qml
        LabeledControl.qml
        ActionButton.qml
        MidiChannelSelector.qml
        ModalPanel.qml
        LabeledComboBox.qml
        SearchableComboField.qml
        FormSection.qml
        ManualView.qml
        DragValueField.qml
        MidiKnob.qml
        SearchableSelectionPopup.qml
        TintedSvg.qml
        ResponsivePageHost.qml
)

add_library(NaadaLab::Ui ALIAS NaadaLabUi)

target_link_libraries(NaadaLabUi
    PUBLIC
        Qt6::Qml
        Qt6::Quick
)