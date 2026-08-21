#pragma once
#include <JuceHeader.h>
#include <functional>
#include "../MmlData/MmlData.h"

class ProjectFileIO
{
public:
    // --------------------------------------------------------
    // 문자열 파싱 및 데이터 유틸리티 
    // --------------------------------------------------------
    static juce::String dmmfEscapeLine(juce::String value);
    static juce::String dmmfUnescapeLine(const juce::String& value);
    static bool dmmfReadBool(const juce::String& value);

    // --------------------------------------------------------
    // ★ 복구된 번역 및 파일 로드 유틸리티 ★
    // --------------------------------------------------------
    static juce::String makeDefaultPresetKoText();
    static juce::String makeDefaultPresetEnText();
    static juce::String makeDefaultPresetJaText();
    static juce::Array<juce::File> getLanguageFoldersForPresetText();
    static bool folderHasReferenceSf2(const juce::File& folder);
    static juce::File getPresetTranslationFile(bool useKorean);
    static juce::File getPresetTranslationFileForLanguage(int languageId);

    // 이 함수가 실제로 번역된 이름을 반환해 줍니다.
    static juce::String getLocalizedSf2PresetName(const juce::String& rawName, const juce::String& sf2FileName, int presetIdx, int languageId);

    // --------------------------------------------------------
    // DMMF 프로젝트 저장/불러오기
    // --------------------------------------------------------
    struct DmmfSaveOptions
    {
        juce::String title;
        int currentTrackIndex = 0;
        int timeSignatureId = 1;
        int themeId = 3;
        int languageId = 2;

        std::function<juce::String(int)> getSf2Name;
        std::function<juce::String(int, int)> getPresetName;
        std::function<juce::String(int)> getTrackName;
    };

    struct DmmfLoadResult
    {
        bool ok = false;
        juce::String errorMessage;

        int numActiveTracks = 0;
        int currentTrackIndex = 0;
        int timeSignatureId = 1;
        int themeId = 0;
        int languageId = 0;
        int loadedTrackCount = 0;
    };

    static bool saveDmmfProject(const juce::File& file,
                                const InstrumentBank* banks,
                                int numActiveTracks,
                                const DmmfSaveOptions& options);

    static DmmfLoadResult loadDmmfProject(const juce::File& file,
                                          InstrumentBank* banks,
                                          int maxBanks,
                                          int maxSf2Index,
                                          std::function<int(const juce::String&)> findSf2IndexByName);

    // --------------------------------------------------------
    // MabiIcco MMI 저장/불러오기
    // --------------------------------------------------------
    struct MmiSaveOptions
    {
        juce::String title;
        int timeSignatureId = 1;
        int fallbackTempo = 120;
        int languageId = 2;

        std::function<juce::String(int)> getSf2Name;
        std::function<juce::String(int)> getTrackName;
    };

    struct MmiLoadOptions
    {
        int maxBanks = 16;
        int maxSf2Files = 0;

        std::function<int(const juce::String&)> findSf2IndexByStem;
        std::function<int(int)> getPresetCount;
        std::function<int(int, int, int)> findPresetIndexByBankAndProgram;
        std::function<bool(int)> refreshSongPresetModeForBank;
    };

    struct MmiLoadResult
    {
        bool ok = false;
        juce::String errorMessage;
        juce::String title;

        int numActiveTracks = 0;
        int loadedTrackCount = 0;
    };

    static bool saveMmiFile(const juce::File& file,
                            const InstrumentBank* banks,
                            int numActiveTracks,
                            const MmiSaveOptions& options);

    static MmiLoadResult importMmiFile(const juce::File& file,
                                       InstrumentBank* banks,
                                       const MmiLoadOptions& options);


    // --------------------------------------------------------
    // MIDI 저장/불러오기
    // --------------------------------------------------------
    struct MidiSaveOptions
    {
        int timeSignatureId = 1;
        double bpm = 120.0;

        std::function<bool(int, int)> isPartActiveForBank;
    };

    struct MidiLoadOptions
    {
        int maxBanks = 16;

        std::function<int(const juce::String&)> findSf2IndexByStem;
        std::function<int(int)> getPresetCount;
    };

    struct MidiLoadResult
    {
        bool ok = false;
        juce::String errorMessage;

        int numActiveTracks = 0;
        int loadedTrackCount = 0;
        int numerator = 4;
        int denominator = 4;
        int roundedBpm = 120;
    };

    static bool saveMidiFile(const juce::File& file,
                             const InstrumentBank* banks,
                             int numActiveTracks,
                             const MidiSaveOptions& options);

    static MidiLoadResult importMidiFile(const juce::File& file,
                                         InstrumentBank* banks,
                                         const MidiLoadOptions& options);

    // --------------------------------------------------------
    // WAV 내보내기
    // --------------------------------------------------------
    struct WavExportOptions
    {
        double sampleRate = 44100.0;
        int numChannels = 2;
        int bitsPerSample = 16;
        int blockSize = 512;

        std::function<bool(int, int)> isPartActiveForBank;
        std::function<void()> stopAllNotes;
        std::function<void(juce::AudioBuffer<float>&, int64_t, bool&)> renderAudioBlock;
    };

    struct WavExportResult
    {
        bool ok = false;
        bool hasAnyNotes = false;
        juce::String errorMessage;
        int64_t maxLengthInSamples = 0;
    };

    static WavExportResult exportWavFile(const juce::File& file,
                                         InstrumentBank* banks,
                                         int numActiveTracks,
                                         const WavExportOptions& options);

    // 구버전 호출부 보호용 래퍼입니다. 새 코드는 위 overload를 사용하세요.
    static bool saveDmmfProject(const juce::File& file, const InstrumentBank* banks, int numActiveTracks);
    static bool loadDmmfProject(const juce::File& file, InstrumentBank* banks, int& outNumActiveTracks);
    static bool importMmiFile(const juce::File& file, InstrumentBank* banks, int& outNumActiveTracks);
};