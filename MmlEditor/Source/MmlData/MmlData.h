#pragma once
#include <JuceHeader.h>
#include <vector>

/**
 * @brief 재생 및 렌더링에 사용되는 개별 음표 정보
 */
struct MmlNote {
    double frequency;
    int64_t startSample;
    int64_t endSample;
    bool isTie;
    float volume;
    double startBeat = 0.0;
    double endBeat = 0.0;
    int textStart = -1; // 재생 중 TextEditor에서 강조할 원본 MML 시작 위치
    int textEnd = -1;   // 재생 중 TextEditor에서 강조할 원본 MML 끝 위치
};

/**
 * @brief 템포 변경 이벤트 정보
 */
struct TempoChange {
    double beatPosition;
    double bpm;
    int64_t samplePosition = 0;
};

/**
 * @brief 피아노롤 마디선/변박 표시용 박자 변경 정보
 */
struct MeterChange {
    double beatPosition = 0.0;
    int timeSignatureId = 1;
    int beatsPerMeasure = 4;
    juce::String displayText = "4/4";
};

/**
 * @brief 실시간 이벤트 리스트 렌더링용 캐시 데이터
 */
struct EventItem {
    double startBeat;
    double endBeat;
    double frequency;
    juce::String timeStr;
    juce::String stepStr;
    juce::String eventType;
    int trackIdx;
};

/**
 * @brief 개별 MML 파트(멜로디, 화음 등)의 상태 및 연주 시퀀스
 */
struct TrackState {
    juce::String mml;
    bool mute = false;
    bool solo = false;
    std::vector<MmlNote> sequence;
    size_t noteIndex = 0;
    double currentAngle = 0.0;
};

/**
 * @brief 악기 탭(뱅크) 하나가 가지는 설정 및 하위 4개 파트의 상태
 */
struct InstrumentBank {
    int instrumentWave = 1;
    int helperMode = 1;
    int sf2FileIndex = 0;
    int dlsPreset = 0;
    int autoBassScale = 1;
    bool songPresetMode = false;         // 오디오 콜백 최적화용 캐시 플래그
    bool xylophonePresetMode = false;    // 마비노기 실로폰: 멜로디 파트만 연주
    bool mmiSongPartWithProgram = false; // MMI 하이브리드 트랙 연주용 플래그
    bool pcPresetExcludeSongPartLimit = false; // PC 마비노기 프리셋: 노래 파트 제외 글자수 제한 표시
    TrackState tracks[4];                // 0: 멜로디, 1: 화음1, 2: 화음2, 3: 노래
};