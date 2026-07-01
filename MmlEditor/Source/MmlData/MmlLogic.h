#pragma once
#include <JuceHeader.h>
#include "MmlData.h"
#include <vector>

class MmlLogic
{
public:
    // --------------------------------------------------------
    // 음표 주파수 및 MIDI 유틸리티
    // --------------------------------------------------------
    static double getFrequencyFromNote(juce::juce_wchar note);
    static juce::String getMMLStringFromMidi(int midiNote);
    static void getChordIntervals(int midiNote, int& third, int& fifth);

    // --------------------------------------------------------
    // 스케일(조표) 추정 및 변환
    // --------------------------------------------------------
    static int detectScaleIdFromMelody(const juce::String& input);

    // 도우미 (자동 화음/베이스 생성)
    // type: 1=Auto Chord(1), 2=Auto Chord(2), 3=Auto Arpeggio, 4=Auto Bass
    // scaleId: Auto Bass에서 사용할 스케일 ID (1~31)
    // beatsPerMeasure: 박자 기호 분자 (예: 3/4박자면 3, 4/4박자면 4)
    static juce::String transformMML(const juce::String& input, int type, int scaleId, int beatsPerMeasure);

    // --------------------------------------------------------
    // 템포 맵 및 시퀀스 파싱
    // --------------------------------------------------------
    // mmlTracks: 현재 뱅크의 활성화된 4개 트랙 MML 텍스트들
    static std::vector<TempoChange> buildTempoMap(const std::vector<juce::String>& mmlTracks, double sampleRateHz);
    
    // 단일 트랙 MML 파싱
    static std::vector<MmlNote> parseMMLWithTempoMap(const juce::String& text, const std::vector<TempoChange>& tempoMap, double sampleRateHz);

    // --------------------------------------------------------
    // 시간 / 샘플 / 비트 변환 유틸리티
    // --------------------------------------------------------
    static int64_t getSamplePositionFromTime(double timeInSeconds, double sampleRateHz);
    static double getBeatFromSample(int64_t sample, const std::vector<TempoChange>& tempoMap, double sampleRateHz);
    static int64_t getSampleFromBeat(double beat, const std::vector<TempoChange>& tempoMap, double sampleRateHz);
};