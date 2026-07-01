#pragma once
#include <JuceHeader.h>

#include "MmlData/MmlData.h"
#include "MmlData/MmlLogic.h"
#include "AudioEngine/AudioEngine.h"
#include "CustomUI/CustomUI.h"
#include "ProjectFile/ProjectFileIO.h"
#include "PianoRoll/PianoRollComponent.h"

class MainComponent : public juce::AudioAppComponent, public juce::Timer, public juce::ScrollBar::Listener
{
public:
    MainComponent();
    ~MainComponent() override;

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;
    void sanitizeNewLines(juce::TextEditor& ed);

    void paint(juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;

    bool keyPressed(const juce::KeyPress& key) override;

    void exportToWavFile();
    void exportToMidiFile();
    void saveMmiFile();
    void importMidiFile();

    void requestCloseWithSavePrompt();
    bool isProjectDirty() const;
    void markProjectDirty();
    void markProjectClean();

private:
    class BlankSubScreenComponent : public juce::Component
    {
    public:
        void paint(juce::Graphics& g) override
        {
            g.fillAll(juce::Colours::white);
        }
    };

    struct MabbiicoUndoSnapshot
    {
        int bankIndex = -1;
        juce::String partMml[4];
        int activePartIdx = 0;
    };

    class TrackTabButton : public juce::TextButton
    {
    public:
        TrackTabButton() = default;

        void setTrackIndex(int newIndex) { trackIndex = newIndex; }
        std::function<bool(int, const juce::MouseEvent&)> onTabMouseDown;

        void mouseDown(const juce::MouseEvent& event) override
        {
            if (onTabMouseDown != nullptr && onTabMouseDown(trackIndex, event))
                return;

            juce::TextButton::mouseDown(event);
        }

    private:
        int trackIndex = 0;
    };

    AudioEngine audioEngine;
    PianoRollComponent pianoRoll;
    BlankSubScreenComponent blankSubScreen;

    juce::TextButton playButton;
    juce::TextButton stopButton;
    juce::TextButton trackNameFromPresetButton;
    juce::TextButton rewindButton;
    juce::TextButton exportButton;
    juce::TextButton openProjectButton;
    juce::TextButton saveProjectButton;
    juce::TextButton themeButton;
    juce::TextButton licenceButton;
    juce::TextButton screenSwitchButton;
    juce::TextButton copyMabi3PartButton;
    juce::TextButton helperButton;
    juce::TextButton meterButton;
    juce::TextButton optimizeButton;

    juce::ComboBox timeSignatureCombo;
    juce::ComboBox helperCombo;
    juce::Label noteLengthLabel;
    juce::ComboBox noteLengthCombo;

    juce::ComboBox autoBassScaleCombo;
    juce::Label autoBassScaleLabel;
    juce::Label scaleSignatureLabel;
    juce::TextButton detectScaleButton;

    juce::ComboBox languageCombo;

    juce::TextButton loadSampleBtn;
    juce::AudioFormatManager formatManager;

    juce::ComboBox sf2FileCombo;
    juce::Label sf2FileLabel;
    juce::ComboBox dlsPresetCombo;
    juce::Label dlsPresetLabel;
    juce::ToggleButton pcExcludeSongPartLimitToggle;
    juce::Label compositionRankGuideLabel;

    static constexpr int MAX_BANKS = 16;
    TrackTabButton tabButtons[MAX_BANKS];

    juce::TextButton addTrackButton{ "+" };
    juce::TextButton removeTrackButton{ "-" };
    int numActiveTracks = 3;

    juce::ComboBox trackInstrumentCombo;
    juce::Label trackInstrumentLabel;

    // ★ 트랙 1~4 UI 변수들이 하나의 배열로 예쁘게 통합되었습니다!
    juce::Label trackLabels[4];
    juce::Label trackCountLabels[4];
    juce::TextEditor trackEditors[4];
    juce::TextButton muteBtns[4];
    juce::TextButton soloBtns[4];
    juce::ToggleButton subPartCheckBoxes[3];

    juce::TextButton importButton;

    juce::ScrollBar verticalScrollBar{ true };
    juce::ScrollBar horizontalScrollBar{ false };

    std::unique_ptr<juce::FileChooser> fileChooser;
    std::unique_ptr<CustomUI::WorkLoadingOverlayComponent> workLoadingOverlay;
    std::unique_ptr<juce::DocumentWindow> helperPopupWindow;
    std::unique_ptr<juce::DocumentWindow> meterPopupWindow;
    std::unique_ptr<juce::DocumentWindow> licencePopupWindow;

    InstrumentBank banks[MAX_BANKS];
    juce::String customTrackNames[MAX_BANKS];
    bool trackNamesUsePresetInstruments = false;
    int currentBankIndex = 0;
    juce::File currentDmmfProjectFile;
    juce::String currentProjectTitle = "Atelier de Derstin";
    int currentThemeId = 3;
    int lastEditorTextThemeId = -1;

    int playbackEditorHighlightStart[4] = { -1, -1, -1, -1 };
    int playbackEditorHighlightEnd[4] = { -1, -1, -1, -1 };
    bool playbackEditorHighlightActive = false;

    bool hasUnsavedChanges = false;
    bool closePromptVisible = false;
    bool pendingCloseAfterSave = false;
    bool suppressDirtyTracking = false;
    bool suppressPresetCallbacks = false;
    bool suppressNoteLengthComboCallback = false;
    bool suppressSubPartCheckBoxCallback = false;
    bool isSubScreenVisible = false;
    int activeMabbiicoPartIdx = 0;
    int selectedPianoRollPartIdx = -1;
    int selectedPianoRollNoteIdx = -1;
    std::vector<std::pair<int, int>> selectedPianoRollNotes;
    std::vector<MmlNote> copiedPianoRollNotes;
    double copiedPianoRollBaseBeat = 0.0;
    std::vector<MabbiicoUndoSnapshot> mabbiicoUndoStack;
    bool mabbiicoUndoTransactionActive = false;
    static constexpr int MAX_MABBIICO_UNDO_HISTORY = 5;

    // Piano-roll drag movement is based on a fixed snapshot from mouse-down.
    // This prevents cumulative delta/index-sort bugs when notes are moved left.
    bool mabbiicoMoveSnapshotActive = false;
    int mabbiicoMoveSnapshotBankIndex = -1;
    int mabbiicoMoveSnapshotPartIdx = -1;
    std::vector<MmlNote> mabbiicoMoveSnapshotSequence;
    std::vector<std::pair<int, int>> mabbiicoMoveSnapshotSelection;

    int pianoRollPreviewSf2Index = -1;
    int pianoRollPreviewPresetIndex = -1;
    int pianoRollPreviewInstrumentWave = 0;
    int pianoRollPreviewMidi = -1;

    bool isStartupLoading = true;
    juce::Image startupLoadingLogo;
    juce::String startupLoadingText = L"마비노기 프리셋을 불러오는 중...";

    bool isWorkLoading = false;
    juce::Image workLoadingImage;
    juce::String workLoadingText = L"작업 중...";

    bool isPlaying = false;
    int64_t globalSampleCount = 0;
    std::vector<TempoChange> tempoMap;
    std::vector<EventItem> cachedEventList;
    std::vector<MeterChange> meterChanges;

    double scrollY = 600.0;
    double scrollX = 0.0;
    float fixedRowHeight = 12.0f;
    double timelinePixelsPerBeat = 50.0; // Mabiicco-only Ctrl+wheel timeline zoom state. 3MLE uses fixed default 50.0.

    juce::String T(const juce::String& en, const juce::String& ko) const;
    void updateUITexts();
    juce::String getWindowTitleText() const;
    juce::String getSafeProjectTitleForFile() const;
    void updateWindowTitle();
    void setCurrentProjectTitle(const juce::String& title);
    void setCurrentProjectTitleFromFile(const juce::File& file);

    juce::File getUserSettingsFile() const;
    void loadUserSettings();
    void saveUserSettings() const;

    void showThemeDialog();
    void showThemedMessageBoxAsync(juce::AlertWindow::AlertIconType iconType, const juce::String& title, const juce::String& message);
    void quitApplicationNow();
    void applyTheme(int themeId);

    void refreshEditorTextColours();
    void updatePlaybackEditorHighlights();
    void clearPlaybackEditorHighlights();
    juce::Colour getEditorPlaybackHighlightColour(int trackIdx) const;
    void updateMmlCharCountLabels();
    void copyMabinogi3PartsToClipboard();
    void toggleTrackNamesFromPresets();
    juce::String getPresetInstrumentDisplayNameForBank(int bankIndex) const;
    juce::String getTrackDisplayName(int trackIndex) const;
    void refreshTrackTabTexts();
    void refreshTrackNameFromPresetButtonText();
    void optimizeCurrentBankMml();
    void refreshPianoRollModel();
    void zoomMabbiicoTimeline(float localMouseX, float wheelDeltaY);
    void seekToBeat(double beat);
    void handlePianoRollNoteDoubleClick(int partIdx, int noteIdx, int textStart, int textEnd);
    void beginMabbiicoUndoTransaction();
    void endMabbiicoUndoTransaction();
    void beginMabbiicoMoveSnapshot();
    void clearMabbiicoMoveSnapshot();
    void previewPianoRollMidi(int midiNote);
    void stopPianoRollPreviewNote();
    void pushMabbiicoUndoState();
    void ensureMabbiicoUndoState();
    void undoMabbiicoEdit();
    void clearMabbiicoUndoHistory();
    void showPianoRollNoteVolumeDialog(int partIdx, int noteIdx);
    void applyPianoRollNoteVolume(int partIdx, int noteIdx, int volumeValue);
    void showPianoRollTempoDialog(double beatPosition);
    void applyPianoRollTempoAtBeat(double beatPosition, int bpm);
    void removePianoRollTempoAtBeat(double beatPosition);
    int getTempoAtBeat(double beatPosition) const;
    void handlePianoRollNoteSelected(int partIdx, int noteIdx);
    void handlePianoRollNotesSelected(const std::vector<std::pair<int, int>>& selectedNotes);
    void copySelectedPianoRollNotes();
    void pasteCopiedPianoRollNotes();
    void deleteSelectedPianoRollNotes();
    void moveSelectedPianoRollNotesPitch(int semitoneDelta);
    bool moveSelectedPianoRollNotes(int semitoneDelta, double beatDelta);
    double getCurrentPlayheadBeatForPaste() const;
    void handlePianoRollNoteEdit(int partIdx, int noteIdx, int midiNote, double lengthBeats);
    void handlePianoRollNoteInsert(int partIdx, int midiNote, double startBeat, double lengthBeats);
    void deleteSelectedPianoRollNote();
    double getSelectedNoteLengthBeats() const;
    void applySelectedNoteLengthToPianoRollNote();
    void selectMabbiicoPart(int partIdx);
    int getActiveMabbiicoPartIndex() const;
    bool canEditMabbiicoPart(int partIdx) const;
    juce::String makeMmlTokenForNote(int midiNote, double lengthBeats) const;

    bool isMabinogiPresetInstrument(int instrumentId) const;
    bool isMobilePresetInstrument(int instrumentId) const;
    bool isMobileSf2Engine(int sf2Index) const;
    bool isSongPresetSelected() const;
    bool isSongPresetForBank(int bankIdx) const;
    bool computeSongPresetModeForBank(int bankIdx) const;
    bool isXylophonePresetSelected() const;
    bool isXylophonePresetForBank(int bankIdx) const;
    bool computeXylophonePresetModeForBank(int bankIdx) const;
    void refreshSongPresetModeCache();
    bool isPartActiveForCurrentPreset(int trackIdx) const;
    bool isPartActiveForBank(int bankIdx, int trackIdx) const;
    void updatePartEditorVisibility();

    void setSubScreenMode(bool shouldShowSubScreen);
    void refreshScreenSwitchButtonText();
    void setMainUiVisible(bool shouldBeVisible);
    void updateMabbiicoEditorLineMode();
    void layoutSubScreen(juce::Rectangle<int> area, juce::Rectangle<int> tabArea);
    void drawStartupLoadingScreen(juce::Graphics& g);
    void loadWorkLoadingImage();
    void showWorkLoadingOverlay(const juce::String& message);
    void hideWorkLoadingOverlay();
    void refreshWorkLoadingOverlayTheme();

    juce::String getScaleSignatureText(int scaleId) const;
    void updateScaleSignatureLabel();
    void detectScaleFromMelodyAndApply();

    void switchBank(int index);
    void saveCurrentBank();
    void loadBank(int index);
    void addNewTrack();
    void showTrackContextMenu(int trackIndex);
    void deleteTrackAtIndex(int trackIndex);
    bool shouldUseProjectTrack1TempoMap() const;
    bool shouldUseMabbiicoTrack1TempoMap() const;
    bool shouldUseThreeMleTrack1TempoMap() const;
    int getMabbiicoTrack1TempoMasterBankIndex() const;
    juce::String getMabbiicoTrack1TempoSourceMml() const;
    void syncMabbiicoTempoEventsFromTrack1();
    void syncThreeMleTempoEventsFromTrack1();
    void updateAllSequences();
    void updatePresetCombo();

    void applyHelperModeState(int mode);
    void populateTimeSignatureComboItems();
    void setTimeSignatureId(int id, juce::NotificationType notificationType = juce::sendNotification);
    int getTimeSignatureBeatsPerMeasure() const;
    int getTimeSignatureBeatsPerMeasureForId(int id) const;
    juce::String getTimeSignatureDisplayText(int id = -1) const;
    double getCurrentPlayheadBeat() const;
    double getMeasureStartForBeat(double beat) const;
    void applyTimeSignatureAtPlayheadMeasure(int id);
    void rebuildMeterChangesForPianoRoll();
    void scrollBarMoved(juce::ScrollBar* scrollBar, double newRangeStart) override;
    void mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel) override;

    void showLoadMenu();
    void showSaveMenu();
    void showHelperMenu();
    void showTimeSignatureWindow();
    void importMmiFile();
    void saveDmmfProject();
    void saveDmmfProjectAs();
    void writeDmmfProjectToFile(const juce::File& file, bool showSuccessMessage);
    void loadDmmfProject();

    void buildTempoMap();

    juce::Label bottomPanelSeparator;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};