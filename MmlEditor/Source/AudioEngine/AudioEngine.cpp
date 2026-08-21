#include "AudioEngine.h"
#include <cstdlib>

namespace
{
    constexpr float kDefaultDlsMasterGain = 0.36f;

    bool shouldWriteDlsDebugMaps()
    {
        // Debug maps are huge and slow down startup/reload noticeably.
        // Keep them opt-in for development:
        // 1) create "enable_dls_debug_dump.txt" next to the exe, or
        // 2) set environment variable MML_EDITOR_WRITE_DLS_DEBUG=1
        const juce::File exeFlag = juce::File::getSpecialLocation(juce::File::currentExecutableFile)
            .getParentDirectory()
            .getChildFile("enable_dls_debug_dump.txt");

        if (exeFlag.existsAsFile())
            return true;

        if (const char* env = std::getenv("MML_EDITOR_WRITE_DLS_DEBUG"))
        {
            const juce::String v(env);
            return v == "1" || v.equalsIgnoreCase("true") || v.equalsIgnoreCase("yes");
        }

        return false;
    }

    int getKnownDlsFileRank(const juce::File& file)
    {
        auto stem = file.getFileNameWithoutExtension().trim().toLowerCase();
        if (stem.containsChar('('))
            stem = stem.upToFirstOccurrenceOf("(", false, false).trim();
        stem = stem.removeCharacters(" _-");

        if (stem == "msxspirit01") return 1;
        if (stem == "msxspirit02") return 2;
        if (stem == "msxspirit03") return 3;
        if (stem == "msxspirit04") return 4;
        if (stem == "msxspirit05") return 5;
        return 1000;
    }

    bool isDlsInstrumentWave(int instrumentWave)
    {
        // 5 = Mabinogi Preset, 6 = Mabinogi Mobile Preset.
        // Both are rendered from user-selected original DLS files.
        return instrumentWave == 5 || instrumentWave == 6;
    }

    bool isBuiltInPreviewInstrument(int instrumentWave)
    {
        // 1 = Acoustic Piano, 2 = Square, 3 = Sawtooth, 4 = Sampler.
        return instrumentWave >= 1 && instrumentWave <= 4;
    }

    double midiNoteToFrequency(int midiNote)
    {
        const int safeMidi = juce::jlimit(0, 127, midiNote);
        return 440.0 * std::pow(2.0, (static_cast<double>(safeMidi) - 69.0) / 12.0);
    }
}

AudioEngine::AudioEngine() {}

AudioEngine::~AudioEngine()
{
    clearSf2Engines();
}

void AudioEngine::prepareToPlay(int /*samplesPerBlockExpected*/, double sampleRate)
{
    currentSampleRate = sampleRate;
    const juce::ScopedLock sl(audioLock);
    for (auto& engine : dlsEngines)
        if (engine != nullptr)
            engine->prepareToPlay(currentSampleRate);
}

void AudioEngine::releaseResources()
{
    // 리소스 해제 필요 시 구현
}

void AudioEngine::clearSf2Engines()
{
    const juce::ScopedLock sl(audioLock);
    dlsEngines.clear();
    sf2Names.clear();
    sf2Files.clear();
    sf2MasterGains.clear();
}

void AudioEngine::stopAllNotes()
{
    const juce::ScopedLock sl(audioLock);
    for (auto& engine : dlsEngines)
        if (engine != nullptr)
            engine->allNotesOff();
    previewSynthVoice.active = false;
}

void AudioEngine::previewNoteOn(int sf2Index, int presetIndex, int midiNote, float velocity)
{
    const juce::ScopedLock sl(audioLock);
    if (sf2Index < 0 || sf2Index >= static_cast<int>(dlsEngines.size()))
        return;

    auto* engine = dlsEngines[static_cast<size_t>(sf2Index)].get();
    if (engine == nullptr)
        return;

    const int safePreset = juce::jmax(0, presetIndex);
    const int safeMidi = juce::jlimit(0, 127, midiNote);
    const float safeVelocity = juce::jlimit(0.0f, 1.0f, velocity);

    // Drag-preview notes are retriggered very quickly while the mouse moves.
    // Reset the selected DLS engine before preview so dragged notes do not echo-stack.
    engine->reset();
    engine->noteOn(safePreset, safeMidi, safeVelocity);
}

void AudioEngine::previewNoteOff(int sf2Index, int presetIndex, int midiNote)
{
    const juce::ScopedLock sl(audioLock);
    if (sf2Index < 0 || sf2Index >= static_cast<int>(dlsEngines.size()))
        return;

    auto* engine = dlsEngines[static_cast<size_t>(sf2Index)].get();
    if (engine == nullptr)
        return;

    const int safePreset = juce::jmax(0, presetIndex);
    const int safeMidi = juce::jlimit(0, 127, midiNote);

    engine->noteOff(safePreset, safeMidi);
}

void AudioEngine::previewSynthNoteOn(int instrumentWave, int midiNote, float velocity)
{
    const juce::ScopedLock sl(audioLock);

    const int safeWave = isBuiltInPreviewInstrument(instrumentWave) ? instrumentWave : 1;
    const int safeMidi = juce::jlimit(0, 127, midiNote);

    previewSynthVoice.active = true;
    previewSynthVoice.instrumentWave = safeWave;
    previewSynthVoice.midiNote = safeMidi;
    previewSynthVoice.frequency = midiNoteToFrequency(safeMidi);
    previewSynthVoice.velocity = juce::jlimit(0.0f, 1.0f, velocity);
    previewSynthVoice.angle = 0.0;
    previewSynthVoice.samplePos = 0;
}

void AudioEngine::previewSynthNoteOff()
{
    const juce::ScopedLock sl(audioLock);
    previewSynthVoice.active = false;
    previewSynthVoice.samplePos = 0;
    previewSynthVoice.angle = 0.0;
}

float AudioEngine::getDlsGainForFile(const juce::File& dlsFile) const
{
    const juce::String dlsStem = dlsFile.getFileNameWithoutExtension();

    const juce::File exeDlsFolder = juce::File::getSpecialLocation(juce::File::currentExecutableFile)
        .getParentDirectory()
        .getChildFile("MabinogiDls");
    exeDlsFolder.createDirectory();

    const juce::File dlsFolder = dlsFile.getParentDirectory();
    const juce::File gainFileNextToDls = dlsFolder.getChildFile("dls_gain.txt");
    const juce::File gainFileNextToExe = exeDlsFolder.getChildFile("dls_gain.txt");

    auto makeDefaultGainText = []()
    {
        juce::String defaultText;
        defaultText << "# DLS file volume correction\n";
        defaultText << "# 1.0 = raw DLS sampler output, 0.36 = safe default\n";
        defaultText << "# Change values and press DLS Reload to apply.\n";
        defaultText << "default=0.36\n";
        return defaultText;
    };

    if (!gainFileNextToDls.existsAsFile() && !gainFileNextToExe.existsAsFile())
        gainFileNextToDls.replaceWithText(makeDefaultGainText());

    const juce::File gainFile = gainFileNextToDls.existsAsFile() ? gainFileNextToDls : gainFileNextToExe;

    float defaultGain = kDefaultDlsMasterGain;
    float matchedGain = -1.0f;

    if (gainFile.existsAsFile())
    {
        juce::StringArray lines;
        lines.addLines(gainFile.loadFileAsString());

        for (auto line : lines)
        {
            line = line.trim();
            if (line.isEmpty() || line.startsWithChar('#') || line.startsWithChar(';'))
                continue;

            const int equalsPos = line.indexOfChar('=');
            if (equalsPos <= 0)
                continue;

            const juce::String key = line.substring(0, equalsPos).trim();
            const juce::String value = line.substring(equalsPos + 1).trim();

            float parsedGain = static_cast<float>(value.getDoubleValue());
            parsedGain = juce::jlimit(0.0f, 10.0f, parsedGain);

            if (key.equalsIgnoreCase("default"))
                defaultGain = parsedGain;

            if (key.equalsIgnoreCase(dlsStem) || key.equalsIgnoreCase(dlsStem + ".dls"))
                matchedGain = parsedGain;
        }
    }

    return matchedGain >= 0.0f ? matchedGain : defaultGain;
}

int AudioEngine::loadDlsFiles(const juce::Array<juce::File>& files, bool useSafetyLimit, juce::String& outLoadedGainInfo, int& outSkippedCount)
{
    clearSf2Engines();
    outLoadedGainInfo.clear();
    outSkippedCount = 0;

    int successCount = 0;
    const int maxAutoFiles = 16;
    const int64_t maxAutoFileBytes = 700LL * 1024LL * 1024LL;
    const double engineSampleRate = currentSampleRate > 0.0 ? currentSampleRate : 44100.0;

    std::vector<juce::File> sortedFiles;
    sortedFiles.reserve(static_cast<size_t>(files.size()));
    for (int i = 0; i < files.size(); ++i)
        sortedFiles.push_back(files.getReference(i));

    std::stable_sort(sortedFiles.begin(), sortedFiles.end(), [](const juce::File& a, const juce::File& b)
    {
        const int ra = getKnownDlsFileRank(a);
        const int rb = getKnownDlsFileRank(b);
        if (ra != rb) return ra < rb;
        return a.getFileName().compareIgnoreCase(b.getFileName()) < 0;
    });

    for (int fileIndex = 0; fileIndex < static_cast<int>(sortedFiles.size()) && (!useSafetyLimit || successCount < maxAutoFiles); ++fileIndex)
    {
        const auto file = sortedFiles[static_cast<size_t>(fileIndex)];
        if (!file.existsAsFile() || !file.hasFileExtension(".dls") || (useSafetyLimit && file.getSize() > maxAutoFileBytes))
        {
            ++outSkippedCount;
            continue;
        }

        auto newEngine = std::make_unique<SimpleDlsSynth>();
        juce::String error;
        if (newEngine->loadFromFile(file, error))
        {
            newEngine->prepareToPlay(engineSampleRate);
            const juce::String dlsStem = file.getFileNameWithoutExtension();
            const float dlsGain = getDlsGainForFile(file);

            juce::String debugDumpPath;
            if (shouldWriteDlsDebugMaps())
            {
                const juce::File exeDebugFolder = juce::File::getSpecialLocation(juce::File::currentExecutableFile)
                    .getParentDirectory()
                    .getChildFile("MabinogiDlsDebug");
                const juce::File debugDump = exeDebugFolder.getChildFile(dlsStem + "_map.txt");
                newEngine->writeDebugDump(debugDump);
                debugDumpPath = debugDump.getFullPathName();
            }

            const juce::ScopedLock sl(audioLock);
            dlsEngines.push_back(std::move(newEngine));
            sf2Names.push_back(dlsStem);
            sf2Files.push_back(file);
            sf2MasterGains.push_back(dlsGain);

            outLoadedGainInfo += "\n" + dlsStem + "=" + juce::String(dlsGain, 2);
            if (debugDumpPath.isNotEmpty())
                outLoadedGainInfo += "\nDEBUG " + debugDumpPath;
            ++successCount;
        }
        else
        {
            ++outSkippedCount;
        }
    }

    if (useSafetyLimit && files.size() > maxAutoFiles)
        outSkippedCount += files.size() - maxAutoFiles;

    return successCount;
}

int AudioEngine::loadSf2Files(const juce::Array<juce::File>& files, bool useSafetyLimit, juce::String& outLoadedGainInfo, int& outSkippedCount)
{
    // Backward-compatible API name.  SF2/tsf.h path is no longer used.
    return loadDlsFiles(files, useSafetyLimit, outLoadedGainInfo, outSkippedCount);
}

bool AudioEngine::isPartActiveForBank(const InstrumentBank& bank, int trackIdx) const
{
    if (trackIdx < 0 || trackIdx >= 4) return false;
    if (bank.songPresetMode) return trackIdx == 3;
    if (bank.xylophonePresetMode) return trackIdx == 0;
    if (trackIdx == 3) return bank.mmiSongPartWithProgram && !bank.tracks[3].mml.isEmpty();
    return trackIdx < 3;
}

void AudioEngine::renderAudioBlock(juce::AudioBuffer<float>& buffer,
                                   InstrumentBank* banks,
                                   int numActiveTracks,
                                   int64_t& globalSampleCount,
                                   bool& isPlaying)
{
    const juce::ScopedLock sl(audioLock);

    const int numSamples = buffer.getNumSamples();
    auto* leftBuffer = buffer.getWritePointer(0);
    auto* rightBuffer = buffer.getNumChannels() > 1 ? buffer.getWritePointer(1) : leftBuffer;

    if (!isPlaying)
    {
        for (int sample = 0; sample < numSamples; ++sample)
        {
            float previewOut = 0.0f;

            if (previewSynthVoice.active)
            {
                int trackWave = previewSynthVoice.instrumentWave;
                const double frequency = previewSynthVoice.frequency;
                const int64_t playedS = previewSynthVoice.samplePos;
                const double tSec = static_cast<double>(playedS) / currentSampleRate;
                const double delta = (frequency / currentSampleRate) * juce::MathConstants<double>::twoPi;
                float tSample = 0.0f;
                float env = 1.0f;

                if (trackWave == 4)
                {
                    if (samplerBuffer.getNumSamples() > 0)
                    {
                        const double speedRatio = (frequency / 261.625565) * (loadedSampleRate / currentSampleRate);
                        const double pos = static_cast<double>(playedS) * speedRatio;
                        const int idx1 = static_cast<int>(pos);
                        const int idx2 = idx1 + 1;
                        const float frac = static_cast<float>(pos - idx1);

                        if (idx1 >= 0 && idx2 < samplerBuffer.getNumSamples())
                        {
                            const float s1 = samplerBuffer.getSample(0, idx1);
                            const float s2 = samplerBuffer.getSample(0, idx2);
                            tSample = s1 + frac * (s2 - s1);

                            const int64_t fadeS = static_cast<int64_t>(currentSampleRate * 0.005);
                            if (fadeS > 0 && playedS < fadeS)
                                env = static_cast<float>(playedS) / static_cast<float>(fadeS);
                        }
                        else
                        {
                            previewSynthVoice.active = false;
                        }
                    }
                    else
                    {
                        trackWave = 1;
                    }
                }

                if (previewSynthVoice.active && trackWave == 1)
                {
                    const float wave = static_cast<float>(std::sin(previewSynthVoice.angle) * 0.5
                                      + std::sin(previewSynthVoice.angle * 2.001) * 0.25
                                      + std::sin(previewSynthVoice.angle * 3.003) * 0.125
                                      + std::sin(previewSynthVoice.angle * 4.006) * 0.0625);
                    const float strike = (tSec < 0.015)
                        ? static_cast<float>(((rand() / static_cast<double>(RAND_MAX)) - 0.5) * 0.15 * std::exp(-200.0 * tSec))
                        : 0.0f;
                    tSample = wave + strike;
                    env = static_cast<float>(std::exp(-3.0 * tSec));

                    if (tSec > 5.0 || env < 0.0004f)
                        previewSynthVoice.active = false;
                }
                else if (previewSynthVoice.active && (trackWave == 2 || trackWave == 3))
                {
                    if (trackWave == 2)
                        tSample = std::sin(previewSynthVoice.angle) >= 0.0 ? 1.0f : -1.0f;
                    else
                        tSample = static_cast<float>((previewSynthVoice.angle / juce::MathConstants<double>::pi) - 1.0);

                    const int64_t fadeS = static_cast<int64_t>(currentSampleRate * 0.005);
                    if (fadeS > 0 && playedS < fadeS)
                        env = static_cast<float>(playedS) / static_cast<float>(fadeS);
                }

                if (previewSynthVoice.active)
                {
                    previewOut = tSample * env * previewSynthVoice.velocity * 0.25f;
                    previewSynthVoice.angle += delta;
                    if (previewSynthVoice.angle >= juce::MathConstants<double>::twoPi)
                        previewSynthVoice.angle -= juce::MathConstants<double>::twoPi;
                    ++previewSynthVoice.samplePos;
                }
            }

            float dlsOut[2] = { 0.0f, 0.0f };
            for (int engineIndex = 0; engineIndex < static_cast<int>(dlsEngines.size()); ++engineIndex)
            {
                auto* engine = dlsEngines[static_cast<size_t>(engineIndex)].get();
                if (engine != nullptr && engine->hasActiveVoices())
                {
                    float tmp[2] = { 0.0f, 0.0f };
                    engine->render(tmp, 1);
                    const float gain = (engineIndex < static_cast<int>(sf2MasterGains.size())) ? sf2MasterGains[static_cast<size_t>(engineIndex)] : kDefaultDlsMasterGain;
                    dlsOut[0] += tmp[0] * gain;
                    dlsOut[1] += tmp[1] * gain;
                }
            }

            leftBuffer[sample] = juce::jlimit(-1.0f, 1.0f, previewOut + dlsOut[0]);
            if (buffer.getNumChannels() > 1)
                rightBuffer[sample] = juce::jlimit(-1.0f, 1.0f, previewOut + dlsOut[1]);
        }
        return;
    }

    bool anySolo = false;
    for (int i = 0; i < numActiveTracks; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            if (isPartActiveForBank(banks[i], j) && banks[i].tracks[j].solo)
                anySolo = true;
        }
    }

    for (int sample = 0; sample < numSamples; ++sample)
    {
        float mixedSample = 0.0f;
        bool allFinished = true;

        for (int i = 0; i < numActiveTracks; ++i)
        {
            int trackWave = banks[i].instrumentWave;
            const int fileIdx = banks[i].sf2FileIndex;
            const int preset = banks[i].dlsPreset;
            SimpleDlsSynth* currentEngine = (fileIdx >= 0 && fileIdx < static_cast<int>(dlsEngines.size()))
                ? dlsEngines[static_cast<size_t>(fileIdx)].get()
                : nullptr;

            for (int j = 0; j < 4; ++j)
            {
                if (!isPartActiveForBank(banks[i], j)) continue;

                auto& track = banks[i].tracks[j];
                if (track.sequence.empty()) continue;

                const bool playTrack = anySolo ? track.solo : !track.mute;

                while (track.noteIndex < track.sequence.size() && globalSampleCount >= track.sequence[track.noteIndex].endSample)
                {
                    if (playTrack && isDlsInstrumentWave(trackWave) && currentEngine != nullptr && track.sequence[track.noteIndex].frequency > 0.0)
                    {
                        const int midiNote = static_cast<int>(std::round(69.0 + 12.0 * std::log2(track.sequence[track.noteIndex].frequency / 440.0)));
                        currentEngine->noteOff(preset, midiNote);
                    }
                    track.noteIndex++;
                }

                if (track.noteIndex < track.sequence.size())
                {
                    allFinished = false;
                    if (playTrack)
                    {
                        auto& note = track.sequence[track.noteIndex];
                        if (globalSampleCount == note.startSample && note.frequency > 0.0)
                        {
                            if (isDlsInstrumentWave(trackWave) && currentEngine != nullptr)
                            {
                                const int midiNote = static_cast<int>(std::round(69.0 + 12.0 * std::log2(note.frequency / 440.0)));
                                currentEngine->noteOn(preset, midiNote, note.volume);
                            }
                        }

                        if (globalSampleCount >= note.startSample && globalSampleCount < note.endSample && note.frequency > 0.0 && !isDlsInstrumentWave(trackWave))
                        {
                            const double delta = (note.frequency / currentSampleRate) * juce::MathConstants<double>::twoPi;
                            float tSample = 0.0f;
                            float env = 1.0f;
                            const int64_t totalS = note.endSample - note.startSample;
                            const int64_t playedS = globalSampleCount - note.startSample;
                            const double tSec = static_cast<double>(playedS) / currentSampleRate;

                            if (trackWave == 4)
                            {
                                if (samplerBuffer.getNumSamples() > 0)
                                {
                                    const double speedRatio = (note.frequency / 261.625565) * (loadedSampleRate / currentSampleRate);
                                    const double pos = static_cast<double>(playedS) * speedRatio;
                                    const int idx1 = static_cast<int>(pos);
                                    const int idx2 = idx1 + 1;
                                    const float frac = static_cast<float>(pos - idx1);
                                    if (idx2 < samplerBuffer.getNumSamples())
                                    {
                                        const float s1 = samplerBuffer.getSample(0, idx1);
                                        const float s2 = samplerBuffer.getSample(0, idx2);
                                        tSample = s1 + frac * (s2 - s1);
                                    }
                                    int64_t fadeS = static_cast<int64_t>(currentSampleRate * 0.01);
                                    if (fadeS * 2 > totalS) fadeS = totalS / 2;
                                    if (fadeS > 0 && playedS < fadeS && (track.noteIndex == 0 || !track.sequence[track.noteIndex - 1].isTie))
                                        env = static_cast<float>(playedS) / static_cast<float>(fadeS);
                                    if (fadeS > 0 && playedS > totalS - fadeS && !note.isTie)
                                        env = static_cast<float>(totalS - playedS) / static_cast<float>(fadeS);
                                }
                                else
                                {
                                    trackWave = 1;
                                }
                            }

                            if (trackWave == 1)
                            {
                                const float wave = static_cast<float>(std::sin(track.currentAngle) * 0.5 + std::sin(track.currentAngle * 2.001) * 0.25 + std::sin(track.currentAngle * 3.003) * 0.125 + std::sin(track.currentAngle * 4.006) * 0.0625);
                                const float strike = (tSec < 0.015) ? static_cast<float>(((rand() / static_cast<double>(RAND_MAX)) - 0.5) * 0.15 * std::exp(-200.0 * tSec)) : 0.0f;
                                tSample = wave + strike;
                                env = static_cast<float>(std::exp(-3.0 * tSec));
                                const int64_t fadeS = static_cast<int64_t>(currentSampleRate * 0.01);
                                if (fadeS > 0 && playedS > totalS - fadeS)
                                    env *= static_cast<float>(totalS - playedS) / static_cast<float>(fadeS);
                            }
                            else if (trackWave == 2 || trackWave == 3)
                            {
                                if (trackWave == 2)
                                    tSample = std::sin(track.currentAngle) >= 0.0 ? 1.0f : -1.0f;
                                else
                                    tSample = static_cast<float>((track.currentAngle / juce::MathConstants<double>::pi) - 1.0);
                                int64_t fadeS = static_cast<int64_t>(currentSampleRate * 0.005);
                                if (fadeS * 2 > totalS) fadeS = totalS / 2;
                                if (fadeS > 0 && playedS < fadeS && (track.noteIndex == 0 || !track.sequence[track.noteIndex - 1].isTie))
                                    env = static_cast<float>(playedS) / static_cast<float>(fadeS);
                                if (fadeS > 0 && playedS > totalS - fadeS && !note.isTie)
                                    env = static_cast<float>(totalS - playedS) / static_cast<float>(fadeS);
                            }

                            tSample *= env * note.volume;
                            track.currentAngle += delta;
                            if (track.currentAngle >= juce::MathConstants<double>::twoPi)
                                track.currentAngle -= juce::MathConstants<double>::twoPi;
                            mixedSample += tSample;
                        }
                    }
                }
            }
        }

        mixedSample *= 0.25f;
        mixedSample = juce::jlimit(-1.0f, 1.0f, mixedSample);

        float dlsOut[2] = { 0.0f, 0.0f };
        for (int engineIndex = 0; engineIndex < static_cast<int>(dlsEngines.size()); ++engineIndex)
        {
            auto* engine = dlsEngines[static_cast<size_t>(engineIndex)].get();
            if (engine != nullptr && engine->hasActiveVoices())
            {
                float tmp[2] = { 0.0f, 0.0f };
                engine->render(tmp, 1);
                const float gain = (engineIndex < static_cast<int>(sf2MasterGains.size())) ? sf2MasterGains[static_cast<size_t>(engineIndex)] : kDefaultDlsMasterGain;
                dlsOut[0] += tmp[0] * gain;
                dlsOut[1] += tmp[1] * gain;
            }
        }

        leftBuffer[sample] = juce::jlimit(-1.0f, 1.0f, mixedSample + dlsOut[0]);
        if (buffer.getNumChannels() > 1)
            rightBuffer[sample] = juce::jlimit(-1.0f, 1.0f, mixedSample + dlsOut[1]);

        globalSampleCount++;
        if (allFinished)
        {
            isPlaying = false;
            stopAllNotes();
            break;
        }
    }
}


juce::String AudioEngine::getSf2Name(int index) const
{
    if (index >= 0 && index < static_cast<int>(sf2Names.size())) return sf2Names[static_cast<size_t>(index)];
    return {};
}

juce::File AudioEngine::getSf2File(int index) const
{
    if (index >= 0 && index < static_cast<int>(sf2Files.size())) return sf2Files[static_cast<size_t>(index)];
    return {};
}

int AudioEngine::getNumEngines() const
{
    return static_cast<int>(dlsEngines.size());
}

const char* AudioEngine::getPresetName(int sf2Index, int presetIndex) const
{
    if (sf2Index >= 0 && sf2Index < static_cast<int>(dlsEngines.size()))
        if (auto* engine = dlsEngines[static_cast<size_t>(sf2Index)].get())
            return engine->getPresetName(presetIndex);
    return nullptr;
}

int AudioEngine::getPresetCount(int sf2Index) const
{
    if (sf2Index >= 0 && sf2Index < static_cast<int>(dlsEngines.size()))
        if (auto* engine = dlsEngines[static_cast<size_t>(sf2Index)].get())
            return engine->getPresetCount();
    return 0;
}

int AudioEngine::getPresetIndex(int sf2Index, int bank, int program) const
{
    if (sf2Index >= 0 && sf2Index < static_cast<int>(dlsEngines.size()))
        if (auto* engine = dlsEngines[static_cast<size_t>(sf2Index)].get())
            return engine->getPresetIndex(bank, program);
    return -1;
}
