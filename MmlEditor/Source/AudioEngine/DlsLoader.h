#pragma once
#include <JuceHeader.h>
#include <vector>
#include <memory>
#include <cmath>
#include <algorithm>
#include <utility>
#include <limits>
#include <string>
#include <cstdio>

#if JUCE_WINDOWS
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
#endif


namespace dlsriff
{
    inline bool probeRiffDlsWithMmio(const juce::File& file)
    {
#if JUCE_WINDOWS
        if (!file.existsAsFile())
            return false;

        const juce::String path = file.getFullPathName();
        std::wstring widePath;

        auto chars = path.toWideCharPointer();
        while (*chars != 0)
        {
            widePath.push_back(static_cast<wchar_t>(*chars));
            ++chars;
        }

        if (widePath.empty())
            return false;

        HMMIO handle = mmioOpenW(const_cast<LPWSTR>(widePath.c_str()), nullptr, MMIO_READ | MMIO_DENYWRITE);
        if (handle == nullptr)
            return false;

        MMCKINFO riffInfo = {};
        riffInfo.fccType = mmioFOURCC('D', 'L', 'S', ' ');
        const bool ok = (mmioDescend(handle, &riffInfo, nullptr, MMIO_FINDRIFF) == MMSYSERR_NOERROR);

        mmioClose(handle, 0);
        return ok;
#else
        return file.hasFileExtension("dls");
#endif
    }
}

/**
    DLS Level 1/2 RIFF parser and lightweight JUCE sampler.

    목적:
    - tsf.h/SF2 경로를 제거하고 사용자가 보유한 원본 .dls를 직접 읽는다.
    - MML 에디터 재생에 필요한 악기명, bank/program, key region, PCM wave, loop만 우선 지원한다.

    지원 범위:
    - RIFF DLS / lins / ins / lrgn / rgn/rgn2 / rgnh / wlnk / wsmp
    - wvpl / wave / fmt / data / wsmp
    - PCM 8-bit unsigned, PCM 16-bit signed mono/stereo

    비고:
    - DLS 파일은 프로그램에 포함하지 않는다. 사용자가 직접 경로를 지정한 파일만 런타임에 로드한다.
    - 압축 wave나 제조사 확장 articulation은 일단 안전하게 무시한다.
*/
class SimpleDlsSynth
{
public:
    struct ArtConnection
    {
        juce::String chunkId;
        uint16_t source = 0;
        uint16_t control = 0;
        uint16_t destination = 0;
        uint16_t transform = 0;
        int32_t scale = 0;
        bool applied = false;
        juce::String sourceName;
        juce::String controlName;
        juce::String destinationName;
        juce::String transformName;
        juce::String applyNote;
    };

    struct Region
    {
        int keyLow = 0;
        int keyHigh = 127;
        int velocityLow = 0;
        int velocityHigh = 127;
        int waveIndex = -1;
        bool hasWsmp = false;
        int unityNote = 60;
        int fineTuneCents = 0;
        int attenuation = 0;
        int loopStart = -1;
        int loopLength = 0;

        // DLS articulation / TSF-style envelope compatibility.
        // These values come from lart/lar2 art1/art2 EG1 destinations.
        // They replace the old fixed note-off fade, which made DLS playback feel much drier than SF2+tsf.h.
        bool hasAttackSeconds = false;
        bool hasDecaySeconds = false;
        bool hasSustainLevel = false;
        bool hasReleaseSeconds = false;
        double attackSeconds = 0.0;
        double decaySeconds = 0.0;
        float sustainLevel = 1.0f;
        double releaseSeconds = 0.01;

        std::vector<ArtConnection> artConnections;

        // DLS articulation connection block extras.
        // These are raw DLS-derived controls, not name-based corrections.
        bool hasArtAttenuation = false;
        int32_t artAttenuation = 0;
        bool hasVelocityToAttenuation = false;
        int32_t velocityToAttenuation = 0;
        bool hasPan = false;
        float pan = 0.0f; // -1 left, 0 center, +1 right
        bool hasLfoFrequencyHz = false;
        double lfoFrequencyHz = 5.0;
        bool hasLfoDelaySeconds = false;
        double lfoDelaySeconds = 0.0;
        bool hasLfoToPitchCents = false;
        double lfoToPitchCents = 0.0;
        bool hasLfoToPitchCentsModWheel = false;
        double lfoToPitchCentsModWheel = 0.0;
        bool hasLfoToGainDb = false;
        double lfoToGainDb = 0.0;
        bool hasLfoToGainDbModWheel = false;
        double lfoToGainDbModWheel = 0.0;
        bool hasKeyToDecayTimecents = false;
        double keyToDecayTimecents = 0.0;
        bool hasVelocityToAttackTimecents = false;
        double velocityToAttackTimecents = 0.0;
        bool hasPitchOffsetCents = false;
        double pitchOffsetCents = 0.0;
        bool hasFilterCutoffHz = false;
        double filterCutoffHz = 20000.0;
        bool hasFilterQ = false;
        float filterQ = 0.707f;

        bool hasModAttackSeconds = false;
        bool hasModDecaySeconds = false;
        bool hasModSustainLevel = false;
        bool hasModReleaseSeconds = false;
        double modAttackSeconds = 0.0;
        double modDecaySeconds = 0.0;
        float modSustainLevel = 1.0f;
        double modReleaseSeconds = 0.01;
        bool hasModToPitchCents = false;
        double modToPitchCents = 0.0;
        bool hasModToFilterCents = false;
        double modToFilterCents = 0.0;
    };

    struct Instrument
    {
        juce::String name;
        int bank = 0;
        int program = 0;
        bool drum = false;

        bool hasAttackSeconds = false;
        bool hasDecaySeconds = false;
        bool hasSustainLevel = false;
        bool hasReleaseSeconds = false;
        double attackSeconds = 0.0;
        double decaySeconds = 0.0;
        float sustainLevel = 1.0f;
        double releaseSeconds = 0.01;

        std::vector<ArtConnection> artConnections;

        // DLS articulation connection block extras.
        // These are raw DLS-derived controls, not name-based corrections.
        bool hasArtAttenuation = false;
        int32_t artAttenuation = 0;
        bool hasVelocityToAttenuation = false;
        int32_t velocityToAttenuation = 0;
        bool hasPan = false;
        float pan = 0.0f; // -1 left, 0 center, +1 right
        bool hasLfoFrequencyHz = false;
        double lfoFrequencyHz = 5.0;
        bool hasLfoDelaySeconds = false;
        double lfoDelaySeconds = 0.0;
        bool hasLfoToPitchCents = false;
        double lfoToPitchCents = 0.0;
        bool hasLfoToPitchCentsModWheel = false;
        double lfoToPitchCentsModWheel = 0.0;
        bool hasLfoToGainDb = false;
        double lfoToGainDb = 0.0;
        bool hasLfoToGainDbModWheel = false;
        double lfoToGainDbModWheel = 0.0;
        bool hasKeyToDecayTimecents = false;
        double keyToDecayTimecents = 0.0;
        bool hasVelocityToAttackTimecents = false;
        double velocityToAttackTimecents = 0.0;
        bool hasPitchOffsetCents = false;
        double pitchOffsetCents = 0.0;
        bool hasFilterCutoffHz = false;
        double filterCutoffHz = 20000.0;
        bool hasFilterQ = false;
        float filterQ = 0.707f;

        bool hasModAttackSeconds = false;
        bool hasModDecaySeconds = false;
        bool hasModSustainLevel = false;
        bool hasModReleaseSeconds = false;
        double modAttackSeconds = 0.0;
        double modDecaySeconds = 0.0;
        float modSustainLevel = 1.0f;
        double modReleaseSeconds = 0.01;
        bool hasModToPitchCents = false;
        double modToPitchCents = 0.0;
        bool hasModToFilterCents = false;
        double modToFilterCents = 0.0;

        std::vector<Region> regions;
    };

    struct Wave
    {
        juce::String name;
        int sampleRate = 44100;
        int channels = 0;
        int bitsPerSample = 0;
        int formatTag = 0;
        bool hasWsmp = false;
        int unityNote = 60;
        int fineTuneCents = 0;
        int attenuation = 0;
        int loopStart = -1;
        int loopLength = 0;

        // Lazy decode:
        // 이전 버전은 DLS 로딩 시점에 모든 PCM을 float samples로 풀어 메모리에 올렸다.
        // 이 버전은 원본 PCM bytes만 보관하고, 실제로 해당 wave가 처음 연주될 때만 float로 변환한다.
        juce::MemoryBlock pcmData;
        std::vector<float> samples;
    };

    SimpleDlsSynth() = default;

    bool loadFromFile(const juce::File& fileToLoad, juce::String& errorMessage)
    {
        sourceFile = fileToLoad;
        displayName = fileToLoad.getFileNameWithoutExtension();
        instruments.clear();
        waves.clear();
        poolTableOffsets.clear();
        wavePoolOffsets.clear();
        poolTableToWaveIndex.clear();
        activeVoices.clear();
        globalArticulation = Instrument();
        hasGlobalArticulation = false;

        // Win32 MMIO로 RIFF/DLS 컨테이너 여부를 먼저 탐색한다.
        // 실제 청크 데이터는 아래 직접 RIFF 파서가 SampleBank 구조로 변환한다.
        (void)dlsriff::probeRiffDlsWithMmio(fileToLoad);

        juce::MemoryBlock block;
        if (!fileToLoad.existsAsFile() || !fileToLoad.loadFileAsData(block))
        {
            errorMessage = "DLS file could not be read.";
            return false;
        }

        const auto* data = static_cast<const uint8_t*>(block.getData());
        const size_t size = static_cast<size_t>(block.getSize());
        if (size < 12 || fourCC(data, 0) != "RIFF" || fourCC(data, 8) != "DLS ")
        {
            errorMessage = "Not a RIFF DLS file.";
            return false;
        }

        const uint32_t riffSize = readU32(data, size, 4);
        const size_t end = juce::jmin<size_t>(size, static_cast<size_t>(riffSize) + 8u);
        parseTopLevel(data, size, 12, end);
        inheritGlobalArticulationToInstruments();
        buildWavePoolIndexMap();
        reorderMabinogiInstrumentsForPresetCompatibility();

        if (instruments.empty())
        {
            errorMessage = "No DLS instruments found.";
            return false;
        }

        for (size_t i = 0; i < instruments.size(); ++i)
        {
            if (instruments[i].name.isEmpty())
                instruments[i].name = "DLS Preset " + juce::String(static_cast<int>(i));
        }

        errorMessage.clear();
        return true;
    }

    void prepareToPlay(double newSampleRate)
    {
        outputSampleRate = newSampleRate > 0.0 ? newSampleRate : 44100.0;
    }

    void reset()
    {
        activeVoices.clear();
    }

    bool hasActiveVoices() const
    {
        return !activeVoices.empty();
    }

    void allNotesOff()
    {
        for (auto& v : activeVoices)
            startVoiceRelease(v);
    }

    int getPresetCount() const
    {
        return static_cast<int>(instruments.size());
    }

    const char* getPresetName(int presetIndex) const
    {
        if (presetIndex < 0 || presetIndex >= static_cast<int>(instruments.size()))
            return nullptr;
        return instruments[static_cast<size_t>(presetIndex)].name.toRawUTF8();
    }

    int getPresetIndex(int bank, int program) const
    {
        const int wantedBank = bank & 0x7fff;
        const int wantedProgram = program & 0x7f;

        for (int i = 0; i < static_cast<int>(instruments.size()); ++i)
        {
            const auto& inst = instruments[static_cast<size_t>(i)];
            if ((inst.bank & 0x7fff) == wantedBank && (inst.program & 0x7f) == wantedProgram)
                return i;
        }

        // Some DLS collections store a flattened order only.  Fall back to program number.
        if (wantedProgram >= 0 && wantedProgram < static_cast<int>(instruments.size()))
            return wantedProgram;

        return -1;
    }

    const juce::File& getSourceFile() const { return sourceFile; }
    const juce::String& getDisplayName() const { return displayName; }

    static juce::String hex16(uint16_t value)
    {
        char buffer[16] = {};
        std::snprintf(buffer, sizeof(buffer), "0x%04X", static_cast<unsigned int>(value));
        return juce::String(buffer);
    }

    static juce::String hex32(int32_t value)
    {
        char buffer[32] = {};
        std::snprintf(buffer, sizeof(buffer), "0x%08X", static_cast<unsigned int>(value));
        return juce::String(buffer);
    }

    static juce::String dlsSourceName(uint16_t source)
    {
        switch (source)
        {
            case 0x0000: return "none/constant";
            case 0x0001: return "LFO";
            case 0x0002: return "key velocity";
            case 0x0003: return "key number";
            case 0x0004: return "EG1";
            case 0x0005: return "EG2";
            case 0x0006: return "pitch wheel";
            case 0x0081: return "CC1 modulation";
            case 0x0087: return "CC7 volume";
            case 0x008A: return "CC10 pan";
            case 0x008B: return "CC11 expression";
            case 0x0093: return "CC91 reverb send";
            case 0x0094: return "CC93 chorus send";
            default: return "unknown source";
        }
    }

    static juce::String dlsDestinationName(uint16_t destination)
    {
        switch (destination)
        {
            case 0x0001: return "gain/attenuation";
            case 0x0003: return "pitch";
            case 0x0004: return "pan";
            case 0x0104: return "LFO frequency";
            case 0x0105: return "LFO start delay";
            case 0x0206: return "EG1 attack";
            case 0x0207: return "EG1 decay";
            case 0x0208: return "EG1 reserved/hold";
            case 0x0209: return "EG1 release";
            case 0x020A: return "EG1 sustain";
            case 0x0306: return "EG2 attack";
            case 0x0307: return "EG2 decay";
            case 0x0308: return "EG2 reserved/hold";
            case 0x0309: return "EG2 release";
            case 0x030A: return "EG2 sustain";
            case 0x030B: return "EG2 decay/legacy";
            case 0x030D: return "EG2 attack/legacy";
            case 0x030E: return "EG2 release/legacy";
            case 0x0500: return "filter cutoff";
            case 0x0501: return "filter resonance/Q";
            default: return "unknown destination";
        }
    }

    static juce::String dlsTransformName(uint16_t transform)
    {
        switch (transform)
        {
            case 0x0000: return "none";
            case 0x0001: return "concave";
            case 0x0002: return "convex";
            case 0x0003: return "switch";
            default: return "unknown transform";
        }
    }

    static bool isArtConnectionApplied(uint16_t source, uint16_t destination)
    {
        if (source == 0x0000)
        {
            switch (destination)
            {
                case 0x0001:
                case 0x0003:
                case 0x0004:
                case 0x0104:
                case 0x0105:
                case 0x0206:
                case 0x0207:
                case 0x0209:
                case 0x020A:
                case 0x0306:
                case 0x0307:
                case 0x0309:
                case 0x030A:
                case 0x030B:
                case 0x030D:
                case 0x030E:
                case 0x0500:
                case 0x0501:
                    return true;
                default:
                    return false;
            }
        }

        if (source == 0x0001) // LFO
            return destination == 0x0001 || destination == 0x0003;

        if (source == 0x0002) // velocity
            return destination == 0x0001 || destination == 0x0206;

        if (source == 0x0003) // key number
            return destination == 0x0207;

        if (source == 0x0004 || source == 0x0005) // envelope sources
            return destination == 0x0003 || destination == 0x0500;

        return false;
    }

    static juce::String artApplyNote(uint16_t source, uint16_t destination, uint16_t control, uint16_t transform)
    {
        juce::String note;
        if (isArtConnectionApplied(source, destination))
            note << "APPLIED";
        else
            note << "UNIMPLEMENTED";

        if (control != 0)
        {
            note << ", has control=" << hex16(control) << "(" << dlsSourceName(control) << ")";
            if (source == 0x0001 && control == 0x0081 && (destination == 0x0003 || destination == 0x0001))
                note << ", CC1-gated(default 0)";
        }
        if (transform != 0)
            note << ", transform=" << hex16(transform) << "(" << dlsTransformName(transform) << ")";
        return note;
    }

    static ArtConnection makeArtConnection(const juce::String& chunkId,
                                           uint16_t source,
                                           uint16_t control,
                                           uint16_t destination,
                                           uint16_t transform,
                                           int32_t scale)
    {
        ArtConnection c;
        c.chunkId = chunkId;
        c.source = source;
        c.control = control;
        c.destination = destination;
        c.transform = transform;
        c.scale = scale;
        c.applied = isArtConnectionApplied(source, destination);
        c.sourceName = dlsSourceName(source);
        c.controlName = dlsSourceName(control);
        c.destinationName = dlsDestinationName(destination);
        c.transformName = dlsTransformName(transform);
        c.applyNote = artApplyNote(source, destination, control, transform);
        return c;
    }

    static void appendArtConnectionDump(juce::String& text, const ArtConnection& c, const juce::String& indent)
    {
        text << indent
             << c.chunkId
             << " Src=" << hex16(c.source) << " " << c.sourceName
             << " Ctl=" << hex16(c.control) << " " << c.controlName
             << " Dst=" << hex16(c.destination) << " " << c.destinationName
             << " Xform=" << hex16(c.transform) << " " << c.transformName
             << " Scale=" << c.scale << "(" << hex32(c.scale) << ")"
             << " => " << c.applyNote << "\n";
    }

    void writeDebugDump(const juce::File& dumpFile) const
    {
        juce::String text;
        text << "DLS Debug Map\n";
        text << "DLS_OPTIMIZED_FLUTE_LFO_OFF_NO_ECHO_BUILD: 2026-06-30\n";
        text << "Expected markers: RuntimeOptimization=enabled, FluteLfoOffFix=enabled, LfoFrequencyFix=enabled, MmiLoadCrashSafe=enabled, CC1-gated(default 0), EchoBus=removed, NaturalOneShotBehavior=removed\n";
        text << "Source: " << sourceFile.getFullPathName() << "\n";
        text << "DisplayName: " << displayName << "\n";
        text << "PresetCount: " << static_cast<int>(instruments.size()) << "\n";
        text << "WaveCount: " << static_cast<int>(waves.size()) << "\n";
        text << "PoolTableCount: " << static_cast<int>(poolTableOffsets.size()) << "\n\n";

        int totalArtConnections = 0;
        int totalUnsupportedArtConnections = 0;
        for (const auto& inst : instruments)
        {
            totalArtConnections += static_cast<int>(inst.artConnections.size());
            for (const auto& c : inst.artConnections)
                if (!c.applied) ++totalUnsupportedArtConnections;

            for (const auto& region : inst.regions)
            {
                totalArtConnections += static_cast<int>(region.artConnections.size());
                for (const auto& c : region.artConnections)
                    if (!c.applied) ++totalUnsupportedArtConnections;
            }
        }

        text << "ArtConnectionCount: " << totalArtConnections << "\n";
        text << "ArtConnectionUnsupportedCount: " << totalUnsupportedArtConnections << "\n";
        text << "MmiLoadCrashSafe=enabled\n";
        text << "LfoFrequencyFix=enabled\n";
        text << "FluteLfoOffFix=enabled\n";
        text << "RuntimeOptimization=enabled\n\n";

        text << "[Articulation Matrix: unsupported summary]\n";
        for (int i = 0; i < static_cast<int>(instruments.size()); ++i)
        {
            const auto& inst = instruments[static_cast<size_t>(i)];
            for (const auto& c : inst.artConnections)
            {
                if (!c.applied)
                {
                    text << "PresetIndex=" << i << " Name='" << inst.name << "' InstrumentArt ";
                    appendArtConnectionDump(text, c, "");
                }
            }

            int r = 0;
            for (const auto& region : inst.regions)
            {
                for (const auto& c : region.artConnections)
                {
                    if (!c.applied)
                    {
                        text << "PresetIndex=" << i << " Name='" << inst.name << "' Region=" << r << " ";
                        appendArtConnectionDump(text, c, "");
                    }
                }
                ++r;
            }
        }
        text << "\n";

        text << "[Preset order after compatibility reorder]\n";
        for (int i = 0; i < static_cast<int>(instruments.size()); ++i)
        {
            const auto& inst = instruments[static_cast<size_t>(i)];
            text << "PresetIndex=" << i
                 << " Program=" << inst.program
                 << " Bank=" << inst.bank
                 << " Drum=" << (inst.drum ? "Y" : "N")
                 << " Name='" << inst.name << "'"
                 << " RegionCount=" << static_cast<int>(inst.regions.size()) << "\n";
        }

        text << "\n[Regions]\n";
        for (int i = 0; i < static_cast<int>(instruments.size()); ++i)
        {
            const auto& inst = instruments[static_cast<size_t>(i)];
            text << "\n--- PresetIndex=" << i << " Program=" << inst.program << " Name='" << inst.name << "'"
                 << " InstEnvA=" << (inst.hasAttackSeconds ? juce::String(inst.attackSeconds, 3) : juce::String("default"))
                 << " InstEnvD=" << (inst.hasDecaySeconds ? juce::String(inst.decaySeconds, 3) : juce::String("default"))
                 << " InstEnvS=" << (inst.hasSustainLevel ? juce::String(inst.sustainLevel, 3) : juce::String("default"))
                 << " InstEnvR=" << (inst.hasReleaseSeconds ? juce::String(inst.releaseSeconds, 3) : juce::String("default"))
                 << " ArtConnections=" << static_cast<int>(inst.artConnections.size())
                 << " ---\n";

            for (const auto& c : inst.artConnections)
                appendArtConnectionDump(text, c, "  IART ");

            int regionIndex = 0;
            for (const auto& region : inst.regions)
            {
                const int resolvedWaveIndex = resolveWaveIndex(region.waveIndex);
                juce::String waveName;
                int waveRoot = -1;
                int waveFine = 0;
                int waveSampleRate = 0;
                int waveLoopStart = -1;
                int waveLoopLength = 0;

                if (resolvedWaveIndex >= 0 && resolvedWaveIndex < static_cast<int>(waves.size()))
                {
                    const auto& wave = waves[static_cast<size_t>(resolvedWaveIndex)];
                    waveName = wave.name;
                    waveRoot = wave.unityNote;
                    waveFine = wave.fineTuneCents;
                    waveSampleRate = wave.sampleRate;
                    waveLoopStart = wave.loopStart;
                    waveLoopLength = wave.loopLength;
                }

                const int effectiveRoot = region.hasWsmp ? region.unityNote : waveRoot;
                const int effectiveFine = region.hasWsmp ? region.fineTuneCents : waveFine;
                const int effectiveLoopStart = region.loopStart >= 0 ? region.loopStart : waveLoopStart;
                const int effectiveLoopLength = region.loopLength > 0 ? region.loopLength : waveLoopLength;

                text << "  R" << regionIndex
                     << " Key=" << region.keyLow << "-" << region.keyHigh
                     << " Vel=" << region.velocityLow << "-" << region.velocityHigh
                     << " WlnkTable=" << region.waveIndex
                     << " ResolvedWave=" << resolvedWaveIndex
                     << " WaveName='" << waveName << "'"
                     << " Root=" << effectiveRoot
                     << " Fine=" << effectiveFine
                     << " RegionWsmp=" << (region.hasWsmp ? "Y" : "N")
                     << " SampleRate=" << waveSampleRate
                     << " Loop=" << effectiveLoopStart << "+" << effectiveLoopLength
                     << " EnvA=" << (region.hasAttackSeconds ? juce::String(region.attackSeconds, 3) : juce::String("default"))
                     << " EnvD=" << (region.hasDecaySeconds ? juce::String(region.decaySeconds, 3) : juce::String("default"))
                     << " EnvS=" << (region.hasSustainLevel ? juce::String(region.sustainLevel, 3) : juce::String("default"))
                     << " EnvR=" << (region.hasReleaseSeconds ? juce::String(region.releaseSeconds, 3) : juce::String("default"))
                     << " NaturalOneShotBehavior=removed"
                     << " EchoBus=removed"
                     << " LfoHz=" << juce::String(region.hasLfoFrequencyHz ? region.lfoFrequencyHz : (inst.hasLfoFrequencyHz ? inst.lfoFrequencyHz : 0.0), 3)
                     << " FluteLfoOffFix=" << ((inst.name.toLowerCase().contains("flute") || waveName.toLowerCase().contains("flute")) ? "Y" : "N")
                     << " ArtConnections=" << static_cast<int>(region.artConnections.size())
                     << "\n";

                for (const auto& c : region.artConnections)
                    appendArtConnectionDump(text, c, "    RART ");

                ++regionIndex;
            }
        }

        text << "\n[Wave list]\n";
        for (int i = 0; i < static_cast<int>(waves.size()); ++i)
        {
            const auto& wave = waves[static_cast<size_t>(i)];
            text << "Wave=" << i
                 << " Name='" << wave.name << "'"
                 << " Root=" << wave.unityNote
                 << " Fine=" << wave.fineTuneCents
                 << " Wsmp=" << (wave.hasWsmp ? "Y" : "N")
                 << " SampleRate=" << wave.sampleRate
                 << " Channels=" << wave.channels
                 << " Bits=" << wave.bitsPerSample
                 << " Loop=" << wave.loopStart << "+" << wave.loopLength
                 << "\n";
        }

        dumpFile.getParentDirectory().createDirectory();
        dumpFile.replaceWithText(text);
    }

    void noteOn(int presetIndex, int midiNote, float velocity)
    {
        if (instruments.empty() || waves.empty())
            return;

        const int safePreset = juce::jlimit(0, static_cast<int>(instruments.size()) - 1, presetIndex);
        const int safeMidi = juce::jlimit(0, 127, midiNote);
        const float safeVelocity = juce::jlimit(0.0f, 1.0f, velocity);
        const int velocityValue = juce::jlimit(0, 127, static_cast<int>(std::round(safeVelocity * 127.0f)));

        // MML files often use v0 notes only as timing/tempo anchors.
        // Do not allocate a silent DLS voice for those notes.
        if (velocityValue <= 0)
            return;

        const auto& inst = instruments[static_cast<size_t>(safePreset)];
        const Region* bestRegion = nullptr;
        int bestScore = std::numeric_limits<int>::max();

        for (const auto& region : inst.regions)
        {
            if (safeMidi >= region.keyLow && safeMidi <= region.keyHigh
                && velocityValue >= region.velocityLow && velocityValue <= region.velocityHigh
                && resolveWaveIndex(region.waveIndex) >= 0)
            {
                const int score = regionSpecificityScore(region);
                if (bestRegion == nullptr || score < bestScore)
                {
                    bestRegion = &region;
                    bestScore = score;
                }
            }
        }

        if (bestRegion == nullptr)
        {
            for (const auto& region : inst.regions)
            {
                if (resolveWaveIndex(region.waveIndex) >= 0)
                {
                    bestRegion = &region;
                    break;
                }
            }
        }

        if (bestRegion == nullptr)
            return;

        const int resolvedWaveIndex = resolveWaveIndex(bestRegion->waveIndex);
        if (resolvedWaveIndex < 0 || !ensureWaveDecoded(resolvedWaveIndex))
            return;

        const auto& wave = waves[static_cast<size_t>(resolvedWaveIndex)];
        if (wave.samples.empty())
            return;

        Voice voice;
        voice.instrumentIndex = safePreset;
        voice.midiNote = safeMidi;
        voice.waveIndex = resolvedWaveIndex;
        voice.position = 0.0;

        auto useRegionFloat = [](bool regionHas, float regionValue, bool instHas, float instValue, float fallback)
        {
            return regionHas ? regionValue : (instHas ? instValue : fallback);
        };
        auto useRegionDouble = [](bool regionHas, double regionValue, bool instHas, double instValue, double fallback)
        {
            return regionHas ? regionValue : (instHas ? instValue : fallback);
        };
        auto useRegionInt = [](bool regionHas, int32_t regionValue, bool instHas, int32_t instValue, int32_t fallback)
        {
            return regionHas ? regionValue : (instHas ? instValue : fallback);
        };

        const float attenuationGain = dlsAttenuationToGain(bestRegion->hasWsmp ? bestRegion->attenuation : wave.attenuation);
        const int32_t artAttenuation = useRegionInt(bestRegion->hasArtAttenuation, bestRegion->artAttenuation,
                                                    inst.hasArtAttenuation, inst.artAttenuation, 0);
        const int32_t velocityAttenuation = useRegionInt(bestRegion->hasVelocityToAttenuation, bestRegion->velocityToAttenuation,
                                                         inst.hasVelocityToAttenuation, inst.velocityToAttenuation, 0);
        const float artGain = dlsArtAttenuationToGain(artAttenuation)
                            * dlsVelocityToAttenuationGain(velocityAttenuation, safeVelocity);
        voice.gain = safeVelocity * 0.72f * attenuationGain * artGain;

        voice.attackSeconds = bestRegion->hasAttackSeconds ? bestRegion->attackSeconds
                                                           : (inst.hasAttackSeconds ? inst.attackSeconds : 0.0);
        voice.decaySeconds = bestRegion->hasDecaySeconds ? bestRegion->decaySeconds
                                                          : (inst.hasDecaySeconds ? inst.decaySeconds : 0.0);
        voice.sustainLevel = bestRegion->hasSustainLevel ? bestRegion->sustainLevel
                                                          : (inst.hasSustainLevel ? inst.sustainLevel : 1.0f);
        voice.releaseSeconds = bestRegion->hasReleaseSeconds ? bestRegion->releaseSeconds
                                                              : (inst.hasReleaseSeconds ? inst.releaseSeconds : 0.01);

        const double velocityAttack = useRegionDouble(bestRegion->hasVelocityToAttackTimecents, bestRegion->velocityToAttackTimecents,
                                                      inst.hasVelocityToAttackTimecents, inst.velocityToAttackTimecents, 0.0);
        if (velocityAttack != 0.0)
            voice.attackSeconds = applyTimecentsModulationToSeconds(voice.attackSeconds, velocityAttack * (1.0 - static_cast<double>(safeVelocity)));

        const double keyToDecay = useRegionDouble(bestRegion->hasKeyToDecayTimecents, bestRegion->keyToDecayTimecents,
                                                  inst.hasKeyToDecayTimecents, inst.keyToDecayTimecents, 0.0);
        if (keyToDecay != 0.0)
            voice.decaySeconds = applyTimecentsModulationToSeconds(voice.decaySeconds, keyToDecay * (60.0 - static_cast<double>(safeMidi)));

        voice.lfoFrequencyHz = useRegionDouble(bestRegion->hasLfoFrequencyHz, bestRegion->lfoFrequencyHz,
                                               inst.hasLfoFrequencyHz, inst.lfoFrequencyHz, 0.0);
        voice.lfoDelaySeconds = useRegionDouble(bestRegion->hasLfoDelaySeconds, bestRegion->lfoDelaySeconds,
                                                inst.hasLfoDelaySeconds, inst.lfoDelaySeconds, 0.0);

        // DLS matrix nuance:
        // Src=LFO, Ctl=CC1(modulation), Dst=pitch/gain is not always-on vibrato/tremolo.
        // It is multiplied by the current modulation wheel. MML playback normally sends no CC1,
        // so default CC1 depth must be 0. Keep always-on LFO and CC1-controlled LFO separate.
        const double baseLfoPitch = useRegionDouble(bestRegion->hasLfoToPitchCents, bestRegion->lfoToPitchCents,
                                                    inst.hasLfoToPitchCents, inst.lfoToPitchCents, 0.0);
        const double cc1LfoPitch = useRegionDouble(bestRegion->hasLfoToPitchCentsModWheel, bestRegion->lfoToPitchCentsModWheel,
                                                   inst.hasLfoToPitchCentsModWheel, inst.lfoToPitchCentsModWheel, 0.0);
        const double baseLfoGain = useRegionDouble(bestRegion->hasLfoToGainDb, bestRegion->lfoToGainDb,
                                                   inst.hasLfoToGainDb, inst.lfoToGainDb, 0.0);
        const double cc1LfoGain = useRegionDouble(bestRegion->hasLfoToGainDbModWheel, bestRegion->lfoToGainDbModWheel,
                                                  inst.hasLfoToGainDbModWheel, inst.lfoToGainDbModWheel, 0.0);
        constexpr double defaultModWheel01 = 0.0;
        voice.lfoToPitchCents = baseLfoPitch + cc1LfoPitch * defaultModWheel01;
        voice.lfoToGainDb = baseLfoGain + cc1LfoGain * defaultModWheel01;

        // Flute "후후후후" fix:
        // Mabinogi DLS flute presets contain always-on LFO-to-gain/pitch connections.
        // Our simple DLS renderer applies them as realtime tremolo/vibrato, which makes
        // a sustained flute note pulse as "후후후후" instead of a single smooth "후~".
        // Mabiicco-style preview does not expose that pulsing this strongly, so for
        // flute/FLUTE only, disable the realtime LFO depth.  This does not touch release,
        // echo, reverb, loop points, or other instruments.
        const juce::String instNameLowerForLfo = inst.name.toLowerCase();
        const juce::String waveNameLowerForLfo = wave.name.toLowerCase();
        const bool disableRealtimeLfoForFlute =
            instNameLowerForLfo.contains("flute") || waveNameLowerForLfo.contains("flute");
        if (disableRealtimeLfoForFlute)
        {
            voice.lfoToPitchCents = 0.0;
            voice.lfoToGainDb = 0.0;
        }

        voice.lfoPhaseDelta = (voice.lfoFrequencyHz > 0.0 && (voice.lfoToPitchCents != 0.0 || voice.lfoToGainDb != 0.0))
            ? (juce::MathConstants<double>::twoPi * voice.lfoFrequencyHz) / outputSampleRate
            : 0.0;

        const float pan = useRegionFloat(bestRegion->hasPan, bestRegion->pan, inst.hasPan, inst.pan, 0.0f);
        applyPanToVoice(pan, voice.panLeft, voice.panRight);

        voice.modAttackSeconds = useRegionDouble(bestRegion->hasModAttackSeconds, bestRegion->modAttackSeconds,
                                                 inst.hasModAttackSeconds, inst.modAttackSeconds, 0.0);
        voice.modDecaySeconds = useRegionDouble(bestRegion->hasModDecaySeconds, bestRegion->modDecaySeconds,
                                                inst.hasModDecaySeconds, inst.modDecaySeconds, 0.0);
        voice.modSustainLevel = useRegionFloat(bestRegion->hasModSustainLevel, bestRegion->modSustainLevel,
                                               inst.hasModSustainLevel, inst.modSustainLevel, 1.0f);
        voice.modReleaseSeconds = useRegionDouble(bestRegion->hasModReleaseSeconds, bestRegion->modReleaseSeconds,
                                                  inst.hasModReleaseSeconds, inst.modReleaseSeconds, 0.01);
        voice.modToPitchCents = useRegionDouble(bestRegion->hasModToPitchCents, bestRegion->modToPitchCents,
                                                inst.hasModToPitchCents, inst.modToPitchCents, 0.0);
        voice.modToFilterCents = useRegionDouble(bestRegion->hasModToFilterCents, bestRegion->modToFilterCents,
                                                 inst.hasModToFilterCents, inst.modToFilterCents, 0.0);

        const double cutoff = useRegionDouble(bestRegion->hasFilterCutoffHz, bestRegion->filterCutoffHz,
                                              inst.hasFilterCutoffHz, inst.filterCutoffHz, 20000.0);
        voice.filterCutoffHz = cutoff;
        voice.filterEnabled = cutoff < 19000.0;

        initialiseVoiceEnvelope(voice);
        initialiseModEnvelope(voice);
        voice.age = ++voiceAgeCounter;

        const int unity = bestRegion->hasWsmp ? bestRegion->unityNote : (wave.hasWsmp ? wave.unityNote : 60);
        const int fine = bestRegion->hasWsmp ? bestRegion->fineTuneCents : (wave.hasWsmp ? wave.fineTuneCents : 0);
        const double artPitchCents = useRegionDouble(bestRegion->hasPitchOffsetCents, bestRegion->pitchOffsetCents,
                                                     inst.hasPitchOffsetCents, inst.pitchOffsetCents, 0.0);
        // DLS/SF2 conversion tools treat fineTune as an additive cent offset.
        const double pitchRatio = std::pow(2.0, (static_cast<double>(safeMidi - unity) * 100.0
                                                + static_cast<double>(fine) + artPitchCents) / 1200.0);
        voice.increment = (static_cast<double>(wave.sampleRate) / outputSampleRate) * pitchRatio;

        voice.loopStart = bestRegion->loopStart >= 0 ? bestRegion->loopStart : wave.loopStart;
        voice.loopLength = bestRegion->loopLength > 0 ? bestRegion->loopLength : wave.loopLength;

        // No artificial echo/tail behavior:
        // Even Loop=-1+0 long-release samples must still obey normal noteOff.
        // Mabiicco-like echo/reverb experiments are intentionally removed.
        voice.naturalOneShot = false;
        voice.noteOffIgnoredForOneShot = false;

        if (activeVoices.size() >= maxVoices)
        {
            auto victim = activeVoices.end();

            // Prefer stealing voices that are already releasing or nearly inaudible.
            for (auto it = activeVoices.begin(); it != activeVoices.end(); ++it)
            {
                if (it->envStage == Voice::EnvStage::Release || it->env < 0.002f)
                {
                    if (victim == activeVoices.end() || it->age < victim->age)
                        victim = it;
                }
            }

            if (victim == activeVoices.end())
            {
                victim = activeVoices.begin();
                for (auto it = activeVoices.begin(); it != activeVoices.end(); ++it)
                    if (it->age < victim->age)
                        victim = it;
            }

            if (victim != activeVoices.end())
                activeVoices.erase(victim);
        }

        activeVoices.push_back(voice);
    }

    void noteOff(int presetIndex, int midiNote)
    {
        const int safeMidi = juce::jlimit(0, 127, midiNote);

        // Important for dense MMI files:
        // There can be overlapping notes with the same preset + MIDI key across melody/chord/song parts.
        // Releasing every matching voice makes one part's note-off cut another still-sounding note,
        // which is especially obvious on song / flute / harp style presets.
        //
        // Release one matching non-releasing voice per noteOff, preferring the oldest one.
        auto victim = activeVoices.end();
        for (auto it = activeVoices.begin(); it != activeVoices.end(); ++it)
        {
            if (!it->active || it->envStage == Voice::EnvStage::Release || it->envStage == Voice::EnvStage::Done)
                continue;

            if (it->midiNote == safeMidi && (presetIndex < 0 || it->instrumentIndex == presetIndex))
            {
                if (victim == activeVoices.end() || it->age < victim->age)
                    victim = it;
            }
        }

        if (victim != activeVoices.end())
        {
            victim->naturalOneShot = false;
            victim->noteOffIgnoredForOneShot = false;
            startVoiceRelease(*victim);
        }
    }

    void render(float* interleavedStereo, int numFrames)
    {
        if (interleavedStereo == nullptr || numFrames <= 0)
            return;

        for (int frame = 0; frame < numFrames; ++frame)
        {
            float mixedLeft = 0.0f;
            float mixedRight = 0.0f;

            for (auto& voice : activeVoices)
            {
                if (!voice.active || voice.waveIndex < 0 || voice.waveIndex >= static_cast<int>(waves.size()))
                    continue;

                const auto& wave = waves[static_cast<size_t>(voice.waveIndex)];
                const int sampleCount = static_cast<int>(wave.samples.size());
                const int i0 = static_cast<int>(voice.position);
                const int i1 = i0 + 1;

                if (i0 < 0 || i0 >= sampleCount)
                {
                    voice.active = false;
                    continue;
                }

                const float frac = static_cast<float>(voice.position - static_cast<double>(i0));
                const float s0 = wave.samples[static_cast<size_t>(i0)];
                const float s1 = (i1 >= 0 && i1 < sampleCount) ? wave.samples[static_cast<size_t>(i1)] : s0;

                float sample = s0 + (s1 - s0) * frac;

                double lfoValue = 0.0;
                if (voice.lfoPhaseDelta != 0.0)
                {
                    const double secondsSinceStart = static_cast<double>(voice.samplesRendered) / outputSampleRate;
                    if (secondsSinceStart >= voice.lfoDelaySeconds)
                    {
                        lfoValue = std::sin(voice.lfoPhase);
                        voice.lfoPhase += voice.lfoPhaseDelta;
                        if (voice.lfoPhase > juce::MathConstants<double>::twoPi)
                            voice.lfoPhase -= juce::MathConstants<double>::twoPi;
                    }
                }

                const double pitchModCents = lfoValue * voice.lfoToPitchCents
                                           + static_cast<double>(voice.modEnv) * voice.modToPitchCents;
                const double currentIncrement = (pitchModCents != 0.0)
                    ? voice.increment * std::pow(2.0, pitchModCents / 1200.0)
                    : voice.increment;

                if (voice.filterEnabled || voice.modToFilterCents != 0.0)
                {
                    double cutoff = voice.filterCutoffHz;
                    if (voice.modToFilterCents != 0.0)
                        cutoff *= std::pow(2.0, (static_cast<double>(voice.modEnv) * voice.modToFilterCents) / 1200.0);
                    cutoff = juce::jlimit(20.0, 20000.0, cutoff);

                    if (cutoff < 19000.0)
                    {
                        const double alpha = 1.0 - std::exp((-juce::MathConstants<double>::twoPi * cutoff) / outputSampleRate);
                        voice.filterState += static_cast<float>(alpha) * (sample - voice.filterState);
                        sample = voice.filterState;
                    }
                }

                float tremoloGain = 1.0f;
                if (voice.lfoToGainDb != 0.0)
                    tremoloGain = juce::jlimit(0.02f, 2.0f, static_cast<float>(std::pow(10.0, (lfoValue * voice.lfoToGainDb) / 20.0)));

                const float out = sample * voice.gain * voice.env * tremoloGain;
                mixedLeft += out * voice.panLeft;
                mixedRight += out * voice.panRight;

                voice.position += currentIncrement;
                ++voice.samplesRendered;

                const bool hasLoop = voice.loopStart >= 0 && voice.loopLength > 8;
                if (hasLoop && voice.envStage != Voice::EnvStage::Done)
                {
                    const double loopStart = static_cast<double>(voice.loopStart);
                    const double loopEnd = static_cast<double>(voice.loopStart + voice.loopLength);
                    if (voice.position >= loopEnd)
                    {
                        const double loopLen = juce::jmax(1.0, loopEnd - loopStart);
                        voice.position = loopStart + std::fmod(voice.position - loopStart, loopLen);
                    }
                }
                else if (voice.position >= static_cast<double>(sampleCount - 1))
                {
                    voice.active = false;
                }

                advanceVoiceEnvelope(voice);
                advanceModEnvelope(voice);
            }

            interleavedStereo[frame * 2] += mixedLeft;
            interleavedStereo[frame * 2 + 1] += mixedRight;
        }

        activeVoices.erase(std::remove_if(activeVoices.begin(), activeVoices.end(), [](const Voice& v) { return !v.active; }), activeVoices.end());
    }

private:
    struct Voice
    {
        enum class EnvStage { Attack, Decay, Sustain, Release, Done };

        bool active = true;
        int instrumentIndex = 0;
        int midiNote = 60;
        int waveIndex = -1;
        int loopStart = -1;
        int loopLength = 0;
        double position = 0.0;
        double increment = 1.0;
        float gain = 0.6f;
        float env = 1.0f;
        EnvStage envStage = EnvStage::Sustain;
        double envPosition = 0.0;
        double attackSeconds = 0.0;
        double decaySeconds = 0.0;
        float sustainLevel = 1.0f;
        double releaseSeconds = 0.01;
        float releaseStartEnv = 1.0f;

        float panLeft = 1.0f;
        float panRight = 1.0f;
        double lfoFrequencyHz = 0.0;
        double lfoDelaySeconds = 0.0;
        double lfoToPitchCents = 0.0;
        double lfoToGainDb = 0.0;
        double lfoPhase = 0.0;
        double lfoPhaseDelta = 0.0;
        bool naturalOneShot = false;
        bool noteOffIgnoredForOneShot = false;
        uint64_t samplesRendered = 0;
        bool filterEnabled = false;
        double filterCutoffHz = 20000.0;
        float filterState = 0.0f;

        float modEnv = 0.0f;
        EnvStage modEnvStage = EnvStage::Done;
        double modEnvPosition = 0.0;
        double modAttackSeconds = 0.0;
        double modDecaySeconds = 0.0;
        float modSustainLevel = 1.0f;
        double modReleaseSeconds = 0.01;
        double modToPitchCents = 0.0;
        double modToFilterCents = 0.0;
        uint64_t age = 0;
    };

    void initialiseVoiceEnvelope(Voice& voice) const
    {
        voice.attackSeconds = juce::jlimit(0.0, 30.0, voice.attackSeconds);
        voice.decaySeconds = juce::jlimit(0.0, 30.0, voice.decaySeconds);
        voice.releaseSeconds = juce::jlimit(0.001, 30.0, voice.releaseSeconds);
        voice.sustainLevel = juce::jlimit(0.0f, 1.0f, voice.sustainLevel);
        voice.envPosition = 0.0;
        voice.releaseStartEnv = 1.0f;

        if (voice.attackSeconds > 0.0005)
        {
            voice.env = 0.0f;
            voice.envStage = Voice::EnvStage::Attack;
        }
        else if (voice.decaySeconds > 0.0005 && voice.sustainLevel < 0.999f)
        {
            voice.env = 1.0f;
            voice.envStage = Voice::EnvStage::Decay;
        }
        else
        {
            voice.env = voice.sustainLevel;
            voice.envStage = Voice::EnvStage::Sustain;
        }
    }

    void advanceVoiceEnvelope(Voice& voice) const
    {
        if (!voice.active)
            return;

        switch (voice.envStage)
        {
            case Voice::EnvStage::Attack:
            {
                const double attackSamples = juce::jmax(1.0, voice.attackSeconds * outputSampleRate);
                voice.envPosition += 1.0;
                const double phase = juce::jlimit(0.0, 1.0, voice.envPosition / attackSamples);
                voice.env = static_cast<float>(phase);
                if (phase >= 1.0)
                {
                    voice.envPosition = 0.0;
                    voice.env = 1.0f;
                    voice.envStage = (voice.decaySeconds > 0.0005 && voice.sustainLevel < 0.999f)
                                       ? Voice::EnvStage::Decay
                                       : Voice::EnvStage::Sustain;
                    if (voice.envStage == Voice::EnvStage::Sustain)
                        voice.env = voice.sustainLevel;
                }
                break;
            }

            case Voice::EnvStage::Decay:
            {
                const double decaySamples = juce::jmax(1.0, voice.decaySeconds * outputSampleRate);
                voice.envPosition += 1.0;
                const double phase = juce::jlimit(0.0, 1.0, voice.envPosition / decaySamples);

                // TSF uses an exponential-style amp envelope.  This normalized curve reaches the
                // requested sustain level at the end of the decay stage without the abrupt linear drop.
                constexpr double curve = 6.0;
                const double expEnd = std::exp(-curve);
                const double factor = (std::exp(-curve * phase) - expEnd) / (1.0 - expEnd);
                voice.env = static_cast<float>(static_cast<double>(voice.sustainLevel)
                                              + (1.0 - static_cast<double>(voice.sustainLevel)) * factor);
                if (phase >= 1.0)
                {
                    voice.envPosition = 0.0;
                    voice.env = voice.sustainLevel;
                    voice.envStage = Voice::EnvStage::Sustain;
                }
                break;
            }

            case Voice::EnvStage::Sustain:
                voice.env = voice.sustainLevel;
                break;

            case Voice::EnvStage::Release:
            {
                const double releaseSamples = juce::jmax(1.0, voice.releaseSeconds * outputSampleRate);
                voice.envPosition += 1.0;
                const double phase = juce::jlimit(0.0, 1.0, voice.envPosition / releaseSamples);

                // Same idea as TSF's amp release: a smooth exponential tail rather than fixed dry fade.
                voice.env = voice.releaseStartEnv * static_cast<float>(std::exp(-6.0 * phase));
                if (voice.env < 0.0005f || phase >= 1.0)
                {
                    voice.env = 0.0f;
                    voice.envStage = Voice::EnvStage::Done;
                    voice.active = false;
                }
                break;
            }

            case Voice::EnvStage::Done:
            default:
                voice.env = 0.0f;
                voice.active = false;
                break;
        }
    }

    void initialiseModEnvelope(Voice& voice) const
    {
        voice.modAttackSeconds = juce::jlimit(0.0, 30.0, voice.modAttackSeconds);
        voice.modDecaySeconds = juce::jlimit(0.0, 30.0, voice.modDecaySeconds);
        voice.modReleaseSeconds = juce::jlimit(0.001, 30.0, voice.modReleaseSeconds);
        voice.modSustainLevel = juce::jlimit(0.0f, 1.0f, voice.modSustainLevel);
        voice.modEnvPosition = 0.0;

        if (voice.modToPitchCents == 0.0 && voice.modToFilterCents == 0.0)
        {
            voice.modEnv = 0.0f;
            voice.modEnvStage = Voice::EnvStage::Done;
            return;
        }

        if (voice.modAttackSeconds > 0.0005)
        {
            voice.modEnv = 0.0f;
            voice.modEnvStage = Voice::EnvStage::Attack;
        }
        else if (voice.modDecaySeconds > 0.0005 && voice.modSustainLevel < 0.999f)
        {
            voice.modEnv = 1.0f;
            voice.modEnvStage = Voice::EnvStage::Decay;
        }
        else
        {
            voice.modEnv = voice.modSustainLevel;
            voice.modEnvStage = Voice::EnvStage::Sustain;
        }
    }

    void advanceModEnvelope(Voice& voice) const
    {
        if (!voice.active || voice.modEnvStage == Voice::EnvStage::Done)
            return;

        switch (voice.modEnvStage)
        {
            case Voice::EnvStage::Attack:
            {
                const double samples = juce::jmax(1.0, voice.modAttackSeconds * outputSampleRate);
                voice.modEnvPosition += 1.0;
                const double phase = juce::jlimit(0.0, 1.0, voice.modEnvPosition / samples);
                voice.modEnv = static_cast<float>(phase);
                if (phase >= 1.0)
                {
                    voice.modEnvPosition = 0.0;
                    voice.modEnv = 1.0f;
                    voice.modEnvStage = (voice.modDecaySeconds > 0.0005 && voice.modSustainLevel < 0.999f)
                                      ? Voice::EnvStage::Decay
                                      : Voice::EnvStage::Sustain;
                    if (voice.modEnvStage == Voice::EnvStage::Sustain)
                        voice.modEnv = voice.modSustainLevel;
                }
                break;
            }

            case Voice::EnvStage::Decay:
            {
                const double samples = juce::jmax(1.0, voice.modDecaySeconds * outputSampleRate);
                voice.modEnvPosition += 1.0;
                const double phase = juce::jlimit(0.0, 1.0, voice.modEnvPosition / samples);
                voice.modEnv = static_cast<float>(static_cast<double>(voice.modSustainLevel)
                                                + (1.0 - static_cast<double>(voice.modSustainLevel)) * (1.0 - phase));
                if (phase >= 1.0)
                {
                    voice.modEnvPosition = 0.0;
                    voice.modEnv = voice.modSustainLevel;
                    voice.modEnvStage = Voice::EnvStage::Sustain;
                }
                break;
            }

            case Voice::EnvStage::Sustain:
                voice.modEnv = voice.modSustainLevel;
                break;

            case Voice::EnvStage::Release:
            {
                const double samples = juce::jmax(1.0, voice.modReleaseSeconds * outputSampleRate);
                voice.modEnvPosition += 1.0;
                const double phase = juce::jlimit(0.0, 1.0, voice.modEnvPosition / samples);
                voice.modEnv *= static_cast<float>(std::exp(-6.0 / samples));
                if (voice.modEnv < 0.0005f || phase >= 1.0)
                {
                    voice.modEnv = 0.0f;
                    voice.modEnvStage = Voice::EnvStage::Done;
                }
                break;
            }

            case Voice::EnvStage::Done:
            default:
                voice.modEnv = 0.0f;
                voice.modEnvStage = Voice::EnvStage::Done;
                break;
        }
    }

    static constexpr size_t maxVoices = 192;

    static juce::String fourCC(const uint8_t* data, size_t pos)
    {
        return juce::String::charToString(static_cast<juce::juce_wchar>(data[pos]))
             + juce::String::charToString(static_cast<juce::juce_wchar>(data[pos + 1]))
             + juce::String::charToString(static_cast<juce::juce_wchar>(data[pos + 2]))
             + juce::String::charToString(static_cast<juce::juce_wchar>(data[pos + 3]));
    }

    static uint16_t readU16(const uint8_t* data, size_t size, size_t pos)
    {
        if (pos + 2 > size) return 0;
        return static_cast<uint16_t>(data[pos] | (data[pos + 1] << 8));
    }

    static int16_t readS16(const uint8_t* data, size_t size, size_t pos)
    {
        return static_cast<int16_t>(readU16(data, size, pos));
    }

    static int16_t readS16Raw(const uint8_t* data, size_t pos)
    {
        return static_cast<int16_t>(static_cast<uint16_t>(data[pos] | (data[pos + 1] << 8)));
    }

    static int32_t readS32(const uint8_t* data, size_t size, size_t pos)
    {
        return static_cast<int32_t>(readU32(data, size, pos));
    }

    static uint32_t readU32(const uint8_t* data, size_t size, size_t pos)
    {
        if (pos + 4 > size) return 0;
        return static_cast<uint32_t>(data[pos])
             | (static_cast<uint32_t>(data[pos + 1]) << 8)
             | (static_cast<uint32_t>(data[pos + 2]) << 16)
             | (static_cast<uint32_t>(data[pos + 3]) << 24);
    }

    static size_t alignedNext(size_t dataPos, uint32_t chunkSize)
    {
        return dataPos + static_cast<size_t>(chunkSize) + (chunkSize & 1u);
    }

    static float dlsAttenuationToGain(int attenuation)
    {
        // DLS wsmp lAttenuation is a signed LONG using 65536 units per dB.
        // Positive values attenuate.  Some uploaded original DLS regions contain negative
        // values; treating those as huge unsigned numbers made certain notes almost silent.
        if (attenuation <= 0)
            return 1.0f;

        const double dB = static_cast<double>(attenuation) / 65536.0;
        const double gain = std::pow(10.0, -dB / 20.0);
        return juce::jlimit(0.02f, 1.0f, static_cast<float>(gain));
    }


    static juce::String readTextChunk(const uint8_t* data, size_t size, size_t pos, size_t len)
    {
        if (pos >= size || len == 0)
            return {};

        size_t safeLen = juce::jmin(len, size - pos);
        while (safeLen > 0 && data[pos + safeLen - 1] == 0)
            --safeLen;
        return juce::String::fromUTF8(reinterpret_cast<const char*>(data + pos), static_cast<int>(safeLen)).trim();
    }


    static juce::String normalisedDlsStem(juce::String stem)
    {
        stem = stem.upToFirstOccurrenceOf(".", false, false).trim().toLowerCase();

        // Windows에서 파일을 복사하면 MSXspirit03(1).dls, MSXspirit03 - Copy.dls처럼
        // 이름이 변할 수 있다.  그래도 MSXspirit03으로 인식해 preset order가 깨지지 않게 한다.
        if (stem.containsChar('('))
            stem = stem.upToFirstOccurrenceOf("(", false, false).trim();
        if (stem.containsIgnoreCase("copy"))
            stem = stem.upToFirstOccurrenceOf("copy", false, false).trim();

        stem = stem.removeCharacters(" _-");
        return stem;
    }

    static std::vector<int> getMabiiccoProgramOrderForStem(const juce::String& rawStem)
    {
        const auto stem = normalisedDlsStem(rawStem);
        std::vector<int> order;

        if (stem == "msxspirit01")
        {
            for (int p = 0; p <= 23; ++p) order.push_back(p);
            order.push_back(25);
            order.push_back(26);
            for (int p = 65; p <= 77; ++p) order.push_back(p);
            return order;
        }

        if (stem == "msxspirit02")
            return { 80, 81, 82, 83, 84, 90, 91, 92, 93, 94, 100, 110, 120, 121 };

        if (stem == "msxspirit03")
            return { 50, 51, 52, 53, 54 };

        if (stem == "msxspirit04")
            return { 24, 27, 28, 55, 56 };

        if (stem == "msxspirit05")
            return { 29 };

        return order;
    }

    int findInstrumentByProgramForCompatibility(int program, const std::vector<bool>& alreadyUsed) const
    {
        const int wantedProgram = program & 0x7f;

        // First prefer normal melodic banks.  Some DLS files include drum flags or extra banks,
        // but Mabiicco instrument selection expects the melodic MSXspirit program set.
        for (int i = 0; i < static_cast<int>(instruments.size()); ++i)
        {
            const auto& inst = instruments[static_cast<size_t>(i)];
            if (!alreadyUsed[static_cast<size_t>(i)] && !inst.drum && ((inst.program & 0x7f) == wantedProgram))
                return i;
        }

        for (int i = 0; i < static_cast<int>(instruments.size()); ++i)
        {
            const auto& inst = instruments[static_cast<size_t>(i)];
            if (!alreadyUsed[static_cast<size_t>(i)] && ((inst.program & 0x7f) == wantedProgram))
                return i;
        }

        return -1;
    }

    void reorderMabinogiInstrumentsForPresetCompatibility()
    {
        const auto order = getMabiiccoProgramOrderForStem(sourceFile.getFileNameWithoutExtension());
        if (order.empty() || instruments.empty())
            return;

        std::vector<Instrument> reordered;
        std::vector<bool> used(instruments.size(), false);
        reordered.reserve(instruments.size());

        for (const int program : order)
        {
            const int index = findInstrumentByProgramForCompatibility(program, used);
            if (index >= 0)
            {
                used[static_cast<size_t>(index)] = true;
                reordered.push_back(instruments[static_cast<size_t>(index)]);
            }
        }

        // If the DLS file did not expose the expected program map, keep the original order.
        // This prevents a partly parsed or custom DLS from becoming more confusing.
        if (reordered.size() < juce::jmin<size_t>(order.size(), instruments.size()))
            return;

        for (size_t i = 0; i < instruments.size(); ++i)
        {
            if (!used[i])
                reordered.push_back(instruments[i]);
        }

        instruments = std::move(reordered);
    }

    static int regionSpecificityScore(const Region& region)
    {
        const int keySpan = juce::jlimit(0, 127, region.keyHigh - region.keyLow);
        const int velSpan = juce::jlimit(0, 127, region.velocityHigh - region.velocityLow);
        return keySpan * 256 + velSpan;
    }


    static double dlsTimecents16ToSeconds(int32_t fixedTimecents)
    {
        // DLS art1 time destinations use 16.16 fixed-point timecents.
        // Example: 3600 timecents = 2^(3600/1200) = 8 seconds.
        if (fixedTimecents <= -2147000000)
            return 0.0;

        const double timecents = static_cast<double>(fixedTimecents) / 65536.0;
        const double seconds = std::pow(2.0, timecents / 1200.0);
        return juce::jlimit(0.0, 30.0, seconds);
    }

    static float dlsSustainScaleToGain(int32_t scale)
    {
        // DLS EG1 sustain is an attenuation-style value in 16.16 units.
        // Convert centibels to a linear gain, matching the SF2/TSF sustain interpretation closely.
        if (scale <= -2147000000)
            return 1.0f;

        const double centibels = juce::jlimit(0.0, 1440.0, static_cast<double>(scale) / 65536.0);
        const double gain = std::pow(10.0, -centibels / 200.0);
        return juce::jlimit(0.0f, 1.0f, static_cast<float>(gain));
    }

    static double dlsFixed16ToUnits(int32_t scale)
    {
        if (scale <= -2147000000)
            return 0.0;
        if (scale >= 2147000000)
            return 2147000000.0 / 65536.0;
        return static_cast<double>(scale) / 65536.0;
    }

    static double dlsCents16ToHz(int32_t fixedCents)
    {
        // For audio-rate/filter destinations, keep the usual audible cutoff range.
        if (fixedCents <= -2147000000)
            return 20.0;
        if (fixedCents >= 2147000000)
            return 20000.0;

        const double cents = dlsFixed16ToUnits(fixedCents);
        const double hz = 8.176 * std::pow(2.0, cents / 1200.0);
        return juce::jlimit(20.0, 20000.0, hz);
    }

    static double dlsLfoCents16ToHz(int32_t fixedCents)
    {
        // DLS LFO frequency uses the same cents representation, but it is a low-frequency
        // oscillator.  Do NOT clamp it to 20 Hz.  The previous shared conversion forced
        // Mabinogi flute LFOs around 4~6 Hz up to 20 Hz, creating the audible
        // "후후후후" pulsing on sustained flute notes.
        if (fixedCents <= -2147000000)
            return 0.0;
        if (fixedCents >= 2147000000)
            return 20.0;

        const double cents = dlsFixed16ToUnits(fixedCents);
        const double hz = 8.176 * std::pow(2.0, cents / 1200.0);
        return juce::jlimit(0.05, 20.0, hz);
    }

    static float dlsArtAttenuationToGain(int32_t scale)
    {
        if (scale == 0 || scale <= -2147000000)
            return 1.0f;

        // DLS attenuation connection values are 16.16 dB-like values.
        // Positive attenuates, negative boosts.  Clamp to avoid runaway gain.
        const double db = dlsFixed16ToUnits(scale);
        const double gain = std::pow(10.0, -db / 20.0);
        return juce::jlimit(0.02f, 2.0f, static_cast<float>(gain));
    }

    static float dlsVelocityToAttenuationGain(int32_t scale, float velocity01)
    {
        if (scale == 0)
            return 1.0f;

        // Low velocity should receive more of the attenuation curve.
        const double amount = 1.0 - juce::jlimit(0.0, 1.0, static_cast<double>(velocity01));
        const double db = dlsFixed16ToUnits(scale) * amount;
        const double gain = std::pow(10.0, -db / 20.0);
        return juce::jlimit(0.02f, 2.0f, static_cast<float>(gain));
    }

    static float dlsPanScaleToPan(int32_t scale)
    {
        if (scale <= -2147000000 || scale >= 2147000000)
            return 0.0f;

        // DLS pan uses a signed 16.16 style scale around center.
        // Use a conservative mapping so malformed data cannot hard-pan everything.
        const double pan = dlsFixed16ToUnits(scale) / 500.0;
        return juce::jlimit(-1.0f, 1.0f, static_cast<float>(pan));
    }

    static double secondsToDlsTimecents(double seconds)
    {
        seconds = juce::jlimit(0.000001, 30.0, seconds);
        return 1200.0 * std::log(seconds) / std::log(2.0);
    }

    static double applyTimecentsModulationToSeconds(double baseSeconds, double timecentsOffset)
    {
        const double baseTc = secondsToDlsTimecents(baseSeconds <= 0.0 ? 0.000001 : baseSeconds);
        const double seconds = std::pow(2.0, (baseTc + timecentsOffset) / 1200.0);
        return juce::jlimit(0.0, 30.0, seconds);
    }

    static void applyPanToVoice(float pan, float& left, float& right)
    {
        const double p = juce::jlimit(-1.0, 1.0, static_cast<double>(pan));
        const double angle = (p + 1.0) * 0.25 * 3.14159265358979323846;
        left = static_cast<float>(std::cos(angle));
        right = static_cast<float>(std::sin(angle));
    }

    template <typename Target>
    static void applyArtConnectionToEnvelope(Target& target, uint16_t source, uint16_t control, uint16_t destination, uint16_t transform, int32_t scale)
    {
        (void)control;
        (void)transform;

        // DLS connection block support.
        // Absolute source (0) sets base region/instrument parameters.
        // Source 1 = LFO, 2 = key velocity, 3 = key number, 4/5 = EG/mod sources in common DLS files.
        if (source == 0x0000)
        {
            switch (destination)
            {
                case 0x0001: // CONN_DST_GAIN / attenuation
                    target.hasArtAttenuation = true;
                    target.artAttenuation = scale;
                    break;

                case 0x0003: // CONN_DST_PITCH absolute pitch offset
                    target.hasPitchOffsetCents = true;
                    target.pitchOffsetCents = dlsFixed16ToUnits(scale);
                    break;

                case 0x0004: // CONN_DST_PAN
                    target.hasPan = true;
                    target.pan = dlsPanScaleToPan(scale);
                    break;

                case 0x0104: // CONN_DST_LFO_FREQUENCY
                    target.hasLfoFrequencyHz = true;
                    target.lfoFrequencyHz = dlsLfoCents16ToHz(scale);
                    break;

                case 0x0105: // CONN_DST_LFO_STARTDELAY
                    target.hasLfoDelaySeconds = true;
                    target.lfoDelaySeconds = dlsTimecents16ToSeconds(scale);
                    break;

                case 0x0206: // CONN_DST_EG1_ATTACKTIME
                    target.hasAttackSeconds = true;
                    target.attackSeconds = dlsTimecents16ToSeconds(scale);
                    break;

                case 0x0207: // CONN_DST_EG1_DECAYTIME
                    target.hasDecaySeconds = true;
                    target.decaySeconds = dlsTimecents16ToSeconds(scale);
                    break;

                case 0x0209: // CONN_DST_EG1_RELEASETIME
                    target.hasReleaseSeconds = true;
                    target.releaseSeconds = dlsTimecents16ToSeconds(scale);
                    break;

                case 0x020A: // CONN_DST_EG1_SUSTAINLEVEL
                    target.hasSustainLevel = true;
                    target.sustainLevel = dlsSustainScaleToGain(scale);
                    break;

                case 0x0306: // CONN_DST_EG2_ATTACKTIME, when present
                case 0x030D: // Some DLS files use this EG2 time slot
                    target.hasModAttackSeconds = true;
                    target.modAttackSeconds = dlsTimecents16ToSeconds(scale);
                    break;

                case 0x0307: // CONN_DST_EG2_DECAYTIME, when present
                case 0x030B: // Common EG2 time slot in older DLS files
                    target.hasModDecaySeconds = true;
                    target.modDecaySeconds = dlsTimecents16ToSeconds(scale);
                    break;

                case 0x0309: // CONN_DST_EG2_RELEASETIME, when present
                case 0x030E: // Common EG2 release/hold slot in older DLS files
                    target.hasModReleaseSeconds = true;
                    target.modReleaseSeconds = dlsTimecents16ToSeconds(scale);
                    break;

                case 0x030A: // CONN_DST_EG2_SUSTAINLEVEL
                    target.hasModSustainLevel = true;
                    target.modSustainLevel = dlsSustainScaleToGain(scale);
                    break;

                case 0x0500: // CONN_DST_FILTER_CUTOFF
                    if (scale < 2147000000)
                    {
                        target.hasFilterCutoffHz = true;
                        target.filterCutoffHz = dlsCents16ToHz(scale);
                    }
                    break;

                case 0x0501: // CONN_DST_FILTER_Q / resonance, approximate
                    target.hasFilterQ = true;
                    target.filterQ = juce::jlimit(0.2f, 8.0f, static_cast<float>(dlsFixed16ToUnits(scale) / 100.0));
                    break;

                default:
                    break;
            }
            return;
        }

        if (source == 0x0001) // LFO source
        {
            const bool controlledByModWheel = (control == 0x0081);

            if (destination == 0x0003)
            {
                if (controlledByModWheel)
                {
                    target.hasLfoToPitchCentsModWheel = true;
                    target.lfoToPitchCentsModWheel = dlsFixed16ToUnits(scale);
                }
                else
                {
                    target.hasLfoToPitchCents = true;
                    target.lfoToPitchCents = dlsFixed16ToUnits(scale);
                }
            }
            else if (destination == 0x0001)
            {
                if (controlledByModWheel)
                {
                    target.hasLfoToGainDbModWheel = true;
                    target.lfoToGainDbModWheel = dlsFixed16ToUnits(scale);
                }
                else
                {
                    target.hasLfoToGainDb = true;
                    target.lfoToGainDb = dlsFixed16ToUnits(scale);
                }
            }
            return;
        }

        if (source == 0x0002) // key velocity source
        {
            if (destination == 0x0001)
            {
                target.hasVelocityToAttenuation = true;
                target.velocityToAttenuation = scale;
            }
            else if (destination == 0x0206)
            {
                target.hasVelocityToAttackTimecents = true;
                target.velocityToAttackTimecents = dlsFixed16ToUnits(scale);
            }
            return;
        }

        if (source == 0x0003) // key number source
        {
            if (destination == 0x0207)
            {
                target.hasKeyToDecayTimecents = true;
                target.keyToDecayTimecents = dlsFixed16ToUnits(scale);
            }
            return;
        }

        // EG2/mod envelope routed to pitch or filter.
        if (source == 0x0004 || source == 0x0005)
        {
            if (destination == 0x0003)
            {
                target.hasModToPitchCents = true;
                target.modToPitchCents = dlsFixed16ToUnits(scale);
            }
            else if (destination == 0x0500)
            {
                target.hasModToFilterCents = true;
                target.modToFilterCents = dlsFixed16ToUnits(scale);
            }
        }
    }

    static void applyArtConnectionToRegion(Region& region, uint16_t source, uint16_t control, uint16_t destination, uint16_t transform, int32_t scale)
    {
        applyArtConnectionToEnvelope(region, source, control, destination, transform, scale);
    }

    static void applyArtConnectionToInstrument(Instrument& instrument, uint16_t source, uint16_t control, uint16_t destination, uint16_t transform, int32_t scale)
    {
        applyArtConnectionToEnvelope(instrument, source, control, destination, transform, scale);
    }

    void parseArtListForRegion(const uint8_t* data, size_t size, size_t pos, size_t end, Region& region)
    {
        while (pos + 8 <= end && pos + 8 <= size)
        {
            const auto id = fourCC(data, pos);
            const uint32_t chunkSize = readU32(data, size, pos + 4);
            const size_t dataPos = pos + 8;
            const size_t chunkEnd = juce::jmin(end, dataPos + static_cast<size_t>(chunkSize));

            if ((id == "art1" || id == "art2") && chunkSize >= 8)
            {
                const uint32_t connectionSize = juce::jmax<uint32_t>(12u, readU32(data, size, dataPos));
                const uint32_t connectionCount = readU32(data, size, dataPos + 4);
                size_t connectionPos = dataPos + 8;
                for (uint32_t i = 0; i < connectionCount && connectionPos + 12 <= chunkEnd; ++i, connectionPos += connectionSize)
                {
                    const uint16_t source = readU16(data, size, connectionPos);
                    const uint16_t control = readU16(data, size, connectionPos + 2);
                    const uint16_t destination = readU16(data, size, connectionPos + 4);
                    const uint16_t transform = readU16(data, size, connectionPos + 6);
                    const int32_t scale = readS32(data, size, connectionPos + 8);
                    region.artConnections.push_back(makeArtConnection(id, source, control, destination, transform, scale));
                    applyArtConnectionToRegion(region, source, control, destination, transform, scale);
                }
            }
            else if (id == "LIST" && dataPos + 4 <= chunkEnd)
            {
                const auto listType = fourCC(data, dataPos);
                if (listType == "lart" || listType == "lar2")
                    parseArtListForRegion(data, size, dataPos + 4, chunkEnd, region);
            }

            pos = alignedNext(dataPos, chunkSize);
        }
    }

    void parseArtListForInstrument(const uint8_t* data, size_t size, size_t pos, size_t end, Instrument& instrument)
    {
        while (pos + 8 <= end && pos + 8 <= size)
        {
            const auto id = fourCC(data, pos);
            const uint32_t chunkSize = readU32(data, size, pos + 4);
            const size_t dataPos = pos + 8;
            const size_t chunkEnd = juce::jmin(end, dataPos + static_cast<size_t>(chunkSize));

            if ((id == "art1" || id == "art2") && chunkSize >= 8)
            {
                const uint32_t connectionSize = juce::jmax<uint32_t>(12u, readU32(data, size, dataPos));
                const uint32_t connectionCount = readU32(data, size, dataPos + 4);
                size_t connectionPos = dataPos + 8;
                for (uint32_t i = 0; i < connectionCount && connectionPos + 12 <= chunkEnd; ++i, connectionPos += connectionSize)
                {
                    const uint16_t source = readU16(data, size, connectionPos);
                    const uint16_t control = readU16(data, size, connectionPos + 2);
                    const uint16_t destination = readU16(data, size, connectionPos + 4);
                    const uint16_t transform = readU16(data, size, connectionPos + 6);
                    const int32_t scale = readS32(data, size, connectionPos + 8);
                    instrument.artConnections.push_back(makeArtConnection(id, source, control, destination, transform, scale));
                    applyArtConnectionToInstrument(instrument, source, control, destination, transform, scale);
                }
            }
            else if (id == "LIST" && dataPos + 4 <= chunkEnd)
            {
                const auto listType = fourCC(data, dataPos);
                if (listType == "lart" || listType == "lar2")
                    parseArtListForInstrument(data, size, dataPos + 4, chunkEnd, instrument);
            }

            pos = alignedNext(dataPos, chunkSize);
        }
    }

    void startVoiceRelease(Voice& voice)
    {
        if (!voice.active || voice.envStage == Voice::EnvStage::Release || voice.envStage == Voice::EnvStage::Done)
            return;

        voice.envStage = Voice::EnvStage::Release;
        voice.releaseStartEnv = voice.env;
        voice.envPosition = 0.0;

        if (voice.modEnvStage != Voice::EnvStage::Done)
        {
            voice.modEnvStage = Voice::EnvStage::Release;
            voice.modEnvPosition = 0.0;
        }
    }

    void sortInstrumentRegionsForStablePlayback(Instrument& instrument)
    {
        // 원본 DLS는 region 순서가 파일마다 제각각이다.
        // SF2 변환본과 비교한 결과 key/root/sample 연결은 맞지만, 재생 시에는
        // 겹치는 region이 있을 때 더 좁은 region을 우선해야 음색이 안정적이다.
        std::stable_sort(instrument.regions.begin(), instrument.regions.end(), [](const Region& a, const Region& b)
        {
            if (a.keyLow != b.keyLow) return a.keyLow < b.keyLow;
            if (a.keyHigh != b.keyHigh) return a.keyHigh < b.keyHigh;
            if (a.velocityLow != b.velocityLow) return a.velocityLow < b.velocityLow;
            if (a.velocityHigh != b.velocityHigh) return a.velocityHigh < b.velocityHigh;
            return a.waveIndex < b.waveIndex;
        });
    }

    void parseTopLevel(const uint8_t* data, size_t size, size_t pos, size_t end)
    {
        while (pos + 8 <= end && pos + 8 <= size)
        {
            const auto id = fourCC(data, pos);
            const uint32_t chunkSize = readU32(data, size, pos + 4);
            const size_t dataPos = pos + 8;
            const size_t chunkEnd = juce::jmin(end, dataPos + static_cast<size_t>(chunkSize));

            if (id == "LIST" && dataPos + 4 <= chunkEnd)
            {
                const auto listType = fourCC(data, dataPos);
                if (listType == "lins")
                    parseLins(data, size, dataPos + 4, chunkEnd);
                else if (listType == "wvpl")
                    parseWvpl(data, size, dataPos + 4, chunkEnd, dataPos + 4);
                else if (listType == "INFO")
                    parseTopInfo(data, size, dataPos + 4, chunkEnd);
                else if (listType == "lart" || listType == "lar2")
                {
                    parseArtListForInstrument(data, size, dataPos + 4, chunkEnd, globalArticulation);
                    hasGlobalArticulation = true;
                }
            }
            else if (id == "ptbl")
            {
                parsePtbl(data, size, dataPos, chunkEnd);
            }

            pos = alignedNext(dataPos, chunkSize);
        }
    }

    void parseTopInfo(const uint8_t* data, size_t size, size_t pos, size_t end)
    {
        while (pos + 8 <= end && pos + 8 <= size)
        {
            const auto id = fourCC(data, pos);
            const uint32_t chunkSize = readU32(data, size, pos + 4);
            const size_t dataPos = pos + 8;
            if (id == "INAM")
            {
                const auto name = readTextChunk(data, size, dataPos, chunkSize);
                if (name.isNotEmpty()) displayName = name;
            }
            pos = alignedNext(dataPos, chunkSize);
        }
    }

    juce::String parseInfoName(const uint8_t* data, size_t size, size_t pos, size_t end)
    {
        juce::String name;
        while (pos + 8 <= end && pos + 8 <= size)
        {
            const auto id = fourCC(data, pos);
            const uint32_t chunkSize = readU32(data, size, pos + 4);
            const size_t dataPos = pos + 8;
            if (id == "INAM")
                name = readTextChunk(data, size, dataPos, chunkSize);
            pos = alignedNext(dataPos, chunkSize);
        }
        return name;
    }

    void parseLins(const uint8_t* data, size_t size, size_t pos, size_t end)
    {
        while (pos + 8 <= end && pos + 8 <= size)
        {
            const auto id = fourCC(data, pos);
            const uint32_t chunkSize = readU32(data, size, pos + 4);
            const size_t dataPos = pos + 8;
            const size_t chunkEnd = juce::jmin(end, dataPos + static_cast<size_t>(chunkSize));

            if (id == "LIST" && dataPos + 4 <= chunkEnd && fourCC(data, dataPos) == "ins ")
                parseInstrument(data, size, dataPos + 4, chunkEnd);

            pos = alignedNext(dataPos, chunkSize);
        }
    }

    static void inheritMissingArtFieldsFromInstrument(Instrument& target, const Instrument& source)
    {
#define DLS_INHERIT_INST_FIELD(flag, value) \
        if (!target.flag && source.flag) { target.flag = true; target.value = source.value; }

        DLS_INHERIT_INST_FIELD(hasAttackSeconds, attackSeconds)
        DLS_INHERIT_INST_FIELD(hasDecaySeconds, decaySeconds)
        DLS_INHERIT_INST_FIELD(hasSustainLevel, sustainLevel)
        DLS_INHERIT_INST_FIELD(hasReleaseSeconds, releaseSeconds)
        DLS_INHERIT_INST_FIELD(hasArtAttenuation, artAttenuation)
        DLS_INHERIT_INST_FIELD(hasVelocityToAttenuation, velocityToAttenuation)
        DLS_INHERIT_INST_FIELD(hasPan, pan)
        DLS_INHERIT_INST_FIELD(hasLfoFrequencyHz, lfoFrequencyHz)
        DLS_INHERIT_INST_FIELD(hasLfoDelaySeconds, lfoDelaySeconds)
        DLS_INHERIT_INST_FIELD(hasLfoToPitchCents, lfoToPitchCents)
        DLS_INHERIT_INST_FIELD(hasLfoToGainDb, lfoToGainDb)
        DLS_INHERIT_INST_FIELD(hasKeyToDecayTimecents, keyToDecayTimecents)
        DLS_INHERIT_INST_FIELD(hasVelocityToAttackTimecents, velocityToAttackTimecents)
        DLS_INHERIT_INST_FIELD(hasPitchOffsetCents, pitchOffsetCents)
        DLS_INHERIT_INST_FIELD(hasFilterCutoffHz, filterCutoffHz)
        DLS_INHERIT_INST_FIELD(hasFilterQ, filterQ)
        DLS_INHERIT_INST_FIELD(hasModAttackSeconds, modAttackSeconds)
        DLS_INHERIT_INST_FIELD(hasModDecaySeconds, modDecaySeconds)
        DLS_INHERIT_INST_FIELD(hasModSustainLevel, modSustainLevel)
        DLS_INHERIT_INST_FIELD(hasModReleaseSeconds, modReleaseSeconds)
        DLS_INHERIT_INST_FIELD(hasModToPitchCents, modToPitchCents)
        DLS_INHERIT_INST_FIELD(hasModToFilterCents, modToFilterCents)

#undef DLS_INHERIT_INST_FIELD
    }

    void inheritGlobalArticulationToInstruments()
    {
        if (!hasGlobalArticulation)
            return;

        for (auto& inst : instruments)
        {
            inheritMissingArtFieldsFromInstrument(inst, globalArticulation);
            inheritInstrumentArticulationToRegions(inst);
        }
    }

    static void inheritInstrumentArticulationToRegions(Instrument& instrument)
    {
        // Some DLS files, including the MSXspirit song presets, store lrgn before lart.
        // Apply the full instrument-level connection block after the whole instrument is parsed.
        for (auto& region : instrument.regions)
        {
#define DLS_INHERIT_FIELD(flag, value) \
            if (!region.flag && instrument.flag) { region.flag = true; region.value = instrument.value; }

            DLS_INHERIT_FIELD(hasAttackSeconds, attackSeconds)
            DLS_INHERIT_FIELD(hasDecaySeconds, decaySeconds)
            DLS_INHERIT_FIELD(hasSustainLevel, sustainLevel)
            DLS_INHERIT_FIELD(hasReleaseSeconds, releaseSeconds)
            DLS_INHERIT_FIELD(hasArtAttenuation, artAttenuation)
            DLS_INHERIT_FIELD(hasVelocityToAttenuation, velocityToAttenuation)
            DLS_INHERIT_FIELD(hasPan, pan)
            DLS_INHERIT_FIELD(hasLfoFrequencyHz, lfoFrequencyHz)
            DLS_INHERIT_FIELD(hasLfoDelaySeconds, lfoDelaySeconds)
            DLS_INHERIT_FIELD(hasLfoToPitchCents, lfoToPitchCents)
            DLS_INHERIT_FIELD(hasLfoToPitchCentsModWheel, lfoToPitchCentsModWheel)
            DLS_INHERIT_FIELD(hasLfoToGainDb, lfoToGainDb)
            DLS_INHERIT_FIELD(hasLfoToGainDbModWheel, lfoToGainDbModWheel)
            DLS_INHERIT_FIELD(hasKeyToDecayTimecents, keyToDecayTimecents)
            DLS_INHERIT_FIELD(hasVelocityToAttackTimecents, velocityToAttackTimecents)
            DLS_INHERIT_FIELD(hasPitchOffsetCents, pitchOffsetCents)
            DLS_INHERIT_FIELD(hasFilterCutoffHz, filterCutoffHz)
            DLS_INHERIT_FIELD(hasFilterQ, filterQ)
            DLS_INHERIT_FIELD(hasModAttackSeconds, modAttackSeconds)
            DLS_INHERIT_FIELD(hasModDecaySeconds, modDecaySeconds)
            DLS_INHERIT_FIELD(hasModSustainLevel, modSustainLevel)
            DLS_INHERIT_FIELD(hasModReleaseSeconds, modReleaseSeconds)
            DLS_INHERIT_FIELD(hasModToPitchCents, modToPitchCents)
            DLS_INHERIT_FIELD(hasModToFilterCents, modToFilterCents)

#undef DLS_INHERIT_FIELD
        }
    }

    void parseInstrument(const uint8_t* data, size_t size, size_t pos, size_t end)
    {
        Instrument instrument;

        while (pos + 8 <= end && pos + 8 <= size)
        {
            const auto id = fourCC(data, pos);
            const uint32_t chunkSize = readU32(data, size, pos + 4);
            const size_t dataPos = pos + 8;
            const size_t chunkEnd = juce::jmin(end, dataPos + static_cast<size_t>(chunkSize));

            if (id == "insh" && chunkSize >= 12)
            {
                const uint32_t bankRaw = readU32(data, size, dataPos + 4);
                const uint32_t programRaw = readU32(data, size, dataPos + 8);
                instrument.drum = (bankRaw & 0x80000000u) != 0;
                instrument.bank = static_cast<int>(bankRaw & 0x7fffu);
                instrument.program = static_cast<int>(programRaw & 0x7fu);
            }
            else if (id == "LIST" && dataPos + 4 <= chunkEnd)
            {
                const auto listType = fourCC(data, dataPos);
                if (listType == "INFO")
                {
                    auto name = parseInfoName(data, size, dataPos + 4, chunkEnd);
                    if (name.isNotEmpty()) instrument.name = name;
                }
                else if (listType == "lrgn")
                {
                    parseLrgn(data, size, dataPos + 4, chunkEnd, instrument);
                }
                else if (listType == "lart" || listType == "lar2")
                {
                    parseArtListForInstrument(data, size, dataPos + 4, chunkEnd, instrument);
                }
            }

            pos = alignedNext(dataPos, chunkSize);
        }

        if (!instrument.regions.empty())
        {
            inheritInstrumentArticulationToRegions(instrument);
            sortInstrumentRegionsForStablePlayback(instrument);
            instruments.push_back(instrument);
        }
    }

    void parseLrgn(const uint8_t* data, size_t size, size_t pos, size_t end, Instrument& instrument)
    {
        while (pos + 8 <= end && pos + 8 <= size)
        {
            const auto id = fourCC(data, pos);
            const uint32_t chunkSize = readU32(data, size, pos + 4);
            const size_t dataPos = pos + 8;
            const size_t chunkEnd = juce::jmin(end, dataPos + static_cast<size_t>(chunkSize));

            if (id == "LIST" && dataPos + 4 <= chunkEnd)
            {
                const auto listType = fourCC(data, dataPos);
                if (listType == "rgn " || listType == "rgn2")
                    parseRegion(data, size, dataPos + 4, chunkEnd, instrument);
            }

            pos = alignedNext(dataPos, chunkSize);
        }
    }

    void parseRegion(const uint8_t* data, size_t size, size_t pos, size_t end, Instrument& instrument)
    {
        Region region;

        while (pos + 8 <= end && pos + 8 <= size)
        {
            const auto id = fourCC(data, pos);
            const uint32_t chunkSize = readU32(data, size, pos + 4);
            const size_t dataPos = pos + 8;
            const size_t chunkEnd = juce::jmin(end, dataPos + static_cast<size_t>(chunkSize));

            if (id == "rgnh" && chunkSize >= 8)
            {
                region.keyLow = juce::jlimit(0, 127, static_cast<int>(readU16(data, size, dataPos)));
                region.keyHigh = juce::jlimit(0, 127, static_cast<int>(readU16(data, size, dataPos + 2)));
                region.velocityLow = juce::jlimit(0, 127, static_cast<int>(readU16(data, size, dataPos + 4)));
                region.velocityHigh = juce::jlimit(0, 127, static_cast<int>(readU16(data, size, dataPos + 6)));
            }
            else if (id == "wlnk" && chunkSize >= 12)
            {
                region.waveIndex = static_cast<int>(readU32(data, size, dataPos + 8));
            }
            else if (id == "wsmp" && chunkSize >= 20)
            {
                region.hasWsmp = true;
                region.unityNote = juce::jlimit(0, 127, static_cast<int>(readU16(data, size, dataPos + 4)));
                region.fineTuneCents = static_cast<int>(readS16(data, size, dataPos + 6));
                region.attenuation = static_cast<int>(readS32(data, size, dataPos + 8));
                const uint32_t loopCount = readU32(data, size, dataPos + 16);
                if (loopCount > 0 && dataPos + 36 <= chunkEnd)
                {
                    region.loopStart = static_cast<int>(readU32(data, size, dataPos + 28));
                    region.loopLength = static_cast<int>(readU32(data, size, dataPos + 32));
                }
            }
            else if (id == "LIST" && dataPos + 4 <= chunkEnd)
            {
                const auto listType = fourCC(data, dataPos);
                if (listType == "lart" || listType == "lar2")
                    parseArtListForRegion(data, size, dataPos + 4, chunkEnd, region);
            }

            pos = alignedNext(dataPos, chunkSize);
        }

        if (region.keyHigh < region.keyLow)
            std::swap(region.keyLow, region.keyHigh);
        if (region.velocityHigh < region.velocityLow)
            std::swap(region.velocityLow, region.velocityHigh);

        if (!region.hasAttackSeconds && instrument.hasAttackSeconds)
        {
            region.hasAttackSeconds = true;
            region.attackSeconds = instrument.attackSeconds;
        }
        if (!region.hasDecaySeconds && instrument.hasDecaySeconds)
        {
            region.hasDecaySeconds = true;
            region.decaySeconds = instrument.decaySeconds;
        }
        if (!region.hasSustainLevel && instrument.hasSustainLevel)
        {
            region.hasSustainLevel = true;
            region.sustainLevel = instrument.sustainLevel;
        }
        if (!region.hasReleaseSeconds && instrument.hasReleaseSeconds)
        {
            region.hasReleaseSeconds = true;
            region.releaseSeconds = instrument.releaseSeconds;
        }

        instrument.regions.push_back(region);
    }

    void parsePtbl(const uint8_t* data, size_t size, size_t pos, size_t end)
    {
        poolTableOffsets.clear();
        if (pos + 8 > end || pos + 8 > size)
            return;

        const uint32_t cueCount = readU32(data, size, pos + 4);
        size_t cuePos = pos + 8;
        for (uint32_t i = 0; i < cueCount && cuePos + 4 <= end && cuePos + 4 <= size; ++i, cuePos += 4)
            poolTableOffsets.push_back(readU32(data, size, cuePos));
    }

    void buildWavePoolIndexMap()
    {
        poolTableToWaveIndex.clear();
        if (poolTableOffsets.empty())
            return;

        poolTableToWaveIndex.resize(poolTableOffsets.size(), -1);
        for (size_t i = 0; i < poolTableOffsets.size(); ++i)
        {
            for (size_t w = 0; w < wavePoolOffsets.size(); ++w)
            {
                if (poolTableOffsets[i] == wavePoolOffsets[w])
                {
                    poolTableToWaveIndex[i] = static_cast<int>(w);
                    break;
                }
            }

            // Some collections align offsets slightly differently.  Fall back to sequential order.
            if (poolTableToWaveIndex[i] < 0 && i < waves.size())
                poolTableToWaveIndex[i] = static_cast<int>(i);
        }
    }

    int resolveWaveIndex(int tableIndex) const
    {
        if (tableIndex < 0)
            return -1;

        if (!poolTableToWaveIndex.empty() && tableIndex < static_cast<int>(poolTableToWaveIndex.size()))
            return poolTableToWaveIndex[static_cast<size_t>(tableIndex)];

        if (tableIndex < static_cast<int>(waves.size()))
            return tableIndex;

        return -1;
    }

    void parseWvpl(const uint8_t* data, size_t size, size_t pos, size_t end, size_t wavePoolBase)
    {
        while (pos + 8 <= end && pos + 8 <= size)
        {
            const auto id = fourCC(data, pos);
            const uint32_t chunkSize = readU32(data, size, pos + 4);
            const size_t dataPos = pos + 8;
            const size_t chunkEnd = juce::jmin(end, dataPos + static_cast<size_t>(chunkSize));

            if (id == "LIST" && dataPos + 4 <= chunkEnd && fourCC(data, dataPos) == "wave")
            {
                wavePoolOffsets.push_back(static_cast<uint32_t>(pos - wavePoolBase));
                parseWave(data, size, dataPos + 4, chunkEnd);
            }

            pos = alignedNext(dataPos, chunkSize);
        }
    }

    bool ensureWaveDecoded(int waveIndex)
    {
        if (waveIndex < 0 || waveIndex >= static_cast<int>(waves.size()))
            return false;

        auto& wave = waves[static_cast<size_t>(waveIndex)];
        if (!wave.samples.empty())
            return true;

        if (wave.formatTag != 1 || wave.channels <= 0 || wave.sampleRate <= 0 || wave.pcmData.isEmpty())
            return false;

        const int bytesPerSample = wave.bitsPerSample / 8;
        if (!(wave.bitsPerSample == 8 || wave.bitsPerSample == 16) || bytesPerSample <= 0)
            return false;

        const int frameSize = wave.channels * bytesPerSample;
        if (frameSize <= 0)
            return false;

        const auto* pcm = static_cast<const uint8_t*>(wave.pcmData.getData());
        const auto pcmSize = static_cast<size_t>(wave.pcmData.getSize());
        const int numFrames = static_cast<int>(pcmSize / static_cast<size_t>(frameSize));
        if (numFrames <= 0)
            return false;

        wave.samples.clear();
        wave.samples.reserve(static_cast<size_t>(numFrames));

        for (int frame = 0; frame < numFrames; ++frame)
        {
            float sum = 0.0f;
            for (int ch = 0; ch < wave.channels; ++ch)
            {
                const size_t samplePos = static_cast<size_t>(frame * frameSize + ch * bytesPerSample);
                if (wave.bitsPerSample == 8)
                    sum += (static_cast<int>(pcm[samplePos]) - 128) / 128.0f;
                else
                    sum += static_cast<float>(readS16Raw(pcm, samplePos)) / 32768.0f;
            }
            wave.samples.push_back(sum / static_cast<float>(wave.channels));
        }

        // float samples로 변환된 뒤에는 원본 PCM bytes를 버려 중복 메모리를 막는다.
        wave.pcmData.reset();
        return !wave.samples.empty();
    }

    void parseWave(const uint8_t* data, size_t size, size_t pos, size_t end)
    {
        Wave wave;
        int channels = 0;
        int bitsPerSample = 0;
        int formatTag = 0;
        size_t waveDataPos = 0;
        uint32_t waveDataSize = 0;

        while (pos + 8 <= end && pos + 8 <= size)
        {
            const auto id = fourCC(data, pos);
            const uint32_t chunkSize = readU32(data, size, pos + 4);
            const size_t dataPos = pos + 8;
            const size_t chunkEnd = juce::jmin(end, dataPos + static_cast<size_t>(chunkSize));

            if (id == "fmt " && chunkSize >= 16)
            {
                formatTag = static_cast<int>(readU16(data, size, dataPos));
                channels = static_cast<int>(readU16(data, size, dataPos + 2));
                wave.sampleRate = static_cast<int>(readU32(data, size, dataPos + 4));
                bitsPerSample = static_cast<int>(readU16(data, size, dataPos + 14));
            }
            else if (id == "data")
            {
                waveDataPos = dataPos;
                waveDataSize = static_cast<uint32_t>(chunkEnd - dataPos);
            }
            else if (id == "wsmp" && chunkSize >= 20)
            {
                wave.hasWsmp = true;
                wave.unityNote = juce::jlimit(0, 127, static_cast<int>(readU16(data, size, dataPos + 4)));
                wave.fineTuneCents = static_cast<int>(readS16(data, size, dataPos + 6));
                wave.attenuation = static_cast<int>(readS32(data, size, dataPos + 8));
                const uint32_t loopCount = readU32(data, size, dataPos + 16);
                if (loopCount > 0 && dataPos + 36 <= chunkEnd)
                {
                    wave.loopStart = static_cast<int>(readU32(data, size, dataPos + 28));
                    wave.loopLength = static_cast<int>(readU32(data, size, dataPos + 32));
                }
            }
            else if (id == "LIST" && dataPos + 4 <= chunkEnd && fourCC(data, dataPos) == "INFO")
            {
                auto name = parseInfoName(data, size, dataPos + 4, chunkEnd);
                if (name.isNotEmpty()) wave.name = name;
            }

            pos = alignedNext(dataPos, chunkSize);
        }

        if (formatTag != 1 || channels <= 0 || wave.sampleRate <= 0 || waveDataPos == 0 || waveDataSize == 0)
        {
            waves.push_back(wave);
            return;
        }

        const int bytesPerSample = bitsPerSample / 8;
        if (!(bitsPerSample == 8 || bitsPerSample == 16) || bytesPerSample <= 0)
        {
            waves.push_back(wave);
            return;
        }

        const int frameSize = channels * bytesPerSample;
        if (frameSize <= 0)
        {
            waves.push_back(wave);
            return;
        }

        // Lightweight mode:
        // DLS 로딩 시 모든 wave를 float로 풀지 않고, 원본 PCM bytes만 저장한다.
        // 실제 첫 noteOn 때 ensureWaveDecoded()에서 필요한 wave만 변환한다.
        wave.formatTag = formatTag;
        wave.channels = channels;
        wave.bitsPerSample = bitsPerSample;
        wave.pcmData.append(data + waveDataPos, static_cast<size_t>(waveDataSize));

        waves.push_back(std::move(wave));
    }

    juce::File sourceFile;
    juce::String displayName;
    double outputSampleRate = 44100.0;
    uint64_t voiceAgeCounter = 0;
    Instrument globalArticulation;
    bool hasGlobalArticulation = false;
    std::vector<Instrument> instruments;
    std::vector<Wave> waves;
    std::vector<uint32_t> poolTableOffsets;
    std::vector<uint32_t> wavePoolOffsets;
    std::vector<int> poolTableToWaveIndex;
    std::vector<Voice> activeVoices;
};
