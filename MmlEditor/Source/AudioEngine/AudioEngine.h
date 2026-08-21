#pragma once
#include <JuceHeader.h>
#include "../MmlData/MmlData.h"
#include "DlsLoader.h"

class AudioEngine
{
public:
    AudioEngine();
    ~AudioEngine();

    // 오디오 시스템 준비 및 해제
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate);
    void releaseResources();

    // 실시간 재생 및 WAV 내보내기에서 공통으로 사용할 핵심 오디오 렌더링 함수
    void renderAudioBlock(juce::AudioBuffer<float>& buffer,
                          InstrumentBank* banks,
                          int numActiveTracks,
                          int64_t& globalSampleCount,
                          bool& isPlaying);

    // DLS 관리
    int loadDlsFiles(const juce::Array<juce::File>& files, bool useSafetyLimit, juce::String& outLoadedGainInfo, int& outSkippedCount);
    // 기존 호출부 호환용 이름. 내부적으로 loadDlsFiles()를 호출한다.
    int loadSf2Files(const juce::Array<juce::File>& files, bool useSafetyLimit, juce::String& outLoadedGainInfo, int& outSkippedCount);
    void clearSf2Engines();
    float getDlsGainForFile(const juce::File& dlsFile) const;

    // 엔진 상태 제어 및 조회
    void stopAllNotes();

    // 피아노롤 드래그 미리듣기용 단음 재생
    void previewNoteOn(int sf2Index, int presetIndex, int midiNote, float velocity);
    void previewNoteOff(int sf2Index, int presetIndex, int midiNote);
    void previewSynthNoteOn(int instrumentWave, int midiNote, float velocity);
    void previewSynthNoteOff();
    juce::String getSf2Name(int index) const;
    juce::File getSf2File(int index) const;
    int getNumEngines() const;
    double getSampleRate() const { return currentSampleRate; }
    
    // DLS 프리셋 조회 유틸리티
    const char* getPresetName(int sf2Index, int presetIndex) const;
    int getPresetCount(int sf2Index) const;
    int getPresetIndex(int sf2Index, int bank, int program) const;

    // 스레드 안전성을 위한 Lock 노출
    juce::CriticalSection& getLock() { return audioLock; }

private:
    bool isPartActiveForBank(const InstrumentBank& bank, int trackIdx) const;

    struct PreviewSynthVoice
    {
        bool active = false;
        int instrumentWave = 1;
        int midiNote = 60;
        double frequency = 261.625565;
        float velocity = 0.7f;
        double angle = 0.0;
        int64_t samplePos = 0;
    };

    double currentSampleRate = 44100.0;
    juce::CriticalSection audioLock;
    PreviewSynthVoice previewSynthVoice;

    std::vector<std::unique_ptr<SimpleDlsSynth>> dlsEngines;
    // 내부 변수명은 프로젝트/저장 파일 호환 때문에 sf2 이름을 유지하지만 실제 내용은 DLS다.
    std::vector<juce::String> sf2Names;
    std::vector<juce::File> sf2Files;
    std::vector<float> sf2MasterGains;

    juce::AudioBuffer<float> samplerBuffer;
    double loadedSampleRate = 44100.0;
};