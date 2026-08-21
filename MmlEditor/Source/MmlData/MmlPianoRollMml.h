#pragma once
#include <JuceHeader.h>
#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

#include "MmlData.h"
#include "MmlLogic.h"

// Piano-roll MML serialization + byte-golf optimizer helpers.
// Header-only on purpose: the source package does not include a JUCE project file,
// so adding a .cpp would require manual project regeneration.
namespace MmlPianoRoll
{
    inline double midiToFrequencyForMml(int midiNote)
    {
        return 440.0 * std::pow(2.0, (static_cast<double>(midiNote) - 69.0) / 12.0);
    }

    inline int frequencyToMidiForMml(double frequency)
    {
        if (frequency <= 0.0)
            return 60;
        return juce::jlimit(12, 119, static_cast<int>(std::round(69.0 + 12.0 * std::log2(frequency / 440.0))));
    }

    inline double quantizeBeat64(double beats)
    {
        return std::round(beats * 64.0) / 64.0;
    }

    inline int volumeFloatToMml(float volume)
    {
        return juce::jlimit(0, 15, static_cast<int>(std::round(juce::jlimit(0.0f, 1.0f, volume) * 15.0f)));
    }

    inline float volumeMmlToFloat(int volumeValue)
    {
        return static_cast<float>(juce::jlimit(0, 15, volumeValue)) / 15.0f;
    }

    inline juce::String makeShortestOctaveCommand(int currentOctave, int targetOctave);
    inline juce::String cleanupRedundantOctaveCommandsInMml(const juce::String& source);

    inline float getInheritedVolumeAtBeat(const std::vector<MmlNote>& sequence, double beat)
    {
        float inherited = 8.0f / 15.0f;
        std::vector<MmlNote> events = sequence;
        std::stable_sort(events.begin(), events.end(), [](const MmlNote& a, const MmlNote& b)
        {
            if (std::abs(a.startBeat - b.startBeat) > 0.0001)
                return a.startBeat < b.startBeat;
            return a.endBeat < b.endBeat;
        });

        for (const auto& ev : events)
        {
            if (ev.startBeat > beat + 0.0001)
                break;
            inherited = juce::jlimit(0.0f, 1.0f, ev.volume);
        }
        return inherited;
    }

    inline juce::String makeMmlDurationTokens(const juce::String& head, double beats, bool tieSegments)
    {
        struct Candidate { const char* suffix; double beats; };
        static const Candidate candidates[] = {
            { "1.", 6.0 }, { "1", 4.0 },
            { "2.", 3.0 }, { "2", 2.0 },
            { "4.", 1.5 }, { "4", 1.0 },
            { "8.", 0.75 }, { "8", 0.5 },
            { "16.", 0.375 }, { "16", 0.25 },
            { "32.", 0.1875 }, { "32", 0.125 },
            { "64.", 0.09375 }, { "64", 0.0625 }
        };

        beats = juce::jmax(0.0625, quantizeBeat64(beats));
        juce::String out;
        int safety = 0;

        while (beats > 0.0001 && safety++ < 128)
        {
            const Candidate* chosen = nullptr;
            for (const auto& c : candidates)
            {
                if (c.beats <= beats + 0.0001)
                {
                    chosen = &c;
                    break;
                }
            }

            if (chosen == nullptr)
                chosen = &candidates[sizeof(candidates) / sizeof(candidates[0]) - 1];

            if (tieSegments && out.isNotEmpty())
                out += "&";

            out += head + juce::String(chosen->suffix);
            beats = quantizeBeat64(beats - chosen->beats);
        }

        return out;
    }

    inline juce::String makeMmlNoteTokensFromMidi(int midiNote, double beats)
    {
        midiNote = juce::jlimit(12, 119, midiNote);
        const int octave = juce::jlimit(0, 9, (midiNote / 12) - 1);
        const int noteIndex = ((midiNote % 12) + 12) % 12;
        static const char* names[12] = { "c", "c#", "d", "d#", "e", "f", "f#", "g", "g#", "a", "a#", "b" };
        const juce::String head = juce::String("o") + juce::String(octave) + juce::String(names[noteIndex]);
        return makeMmlDurationTokens(head, beats, true);
    }

    inline juce::String makeMmlRestTokens(double beats)
    {
        return makeMmlDurationTokens("r", beats, false);
    }

    inline juce::String extractFirstMmlCommand(const juce::String& source, juce::juce_wchar command)
    {
        const auto lowerCommand = juce::CharacterFunctions::toLowerCase(command);
        for (int i = 0; i < source.length(); ++i)
        {
            if (juce::CharacterFunctions::toLowerCase(source[i]) != lowerCommand)
                continue;

            juce::String digits;
            int j = i + 1;
            while (j < source.length() && juce::CharacterFunctions::isDigit(source[j]))
            {
                digits += source[j];
                ++j;
            }

            if (digits.isNotEmpty())
                return juce::String::charToString(lowerCommand) + digits;
        }
        return {};
    }

    struct PianoRollTempoEvent
    {
        double beat = 0.0;
        int bpm = 120;
    };

    inline std::vector<PianoRollTempoEvent> extractTempoEventsFromMml(const juce::String& text)
    {
        std::vector<PianoRollTempoEvent> result;
        double currentBeats = 0.0;
        int defaultLength = 4;
        bool isDefaultDotted = false;

        auto consumeLengthAndAdvanceBeat = [&](int& i)
        {
            int noteLength = defaultLength > 0 ? defaultLength : 4;
            bool isDotted = isDefaultDotted;

            juce::String numStr;
            while (i + 1 < text.length() && juce::CharacterFunctions::isDigit(text[i + 1]))
            {
                numStr += text[i + 1];
                ++i;
            }

            if (numStr.isNotEmpty())
            {
                const int parsed = numStr.getIntValue();
                if (parsed > 0)
                    noteLength = parsed;
                isDotted = false;
            }

            if (i + 1 < text.length() && text[i + 1] == '.')
            {
                isDotted = true;
                ++i;
            }

            if (noteLength <= 0)
                noteLength = 4;

            double beats = 4.0 / static_cast<double>(noteLength);
            if (isDotted)
                beats *= 1.5;
            currentBeats += beats;
        };

        for (int i = 0; i < text.length(); ++i)
        {
            const auto c = juce::CharacterFunctions::toLowerCase(text[i]);

            if (c == 't')
            {
                juce::String digits;
                while (i + 1 < text.length() && juce::CharacterFunctions::isDigit(text[i + 1]))
                {
                    digits += text[i + 1];
                    ++i;
                }

                if (digits.isNotEmpty())
                {
                    const int bpm = juce::jlimit(1, 999, digits.getIntValue());
                    result.push_back({ quantizeBeat64(currentBeats), bpm });
                }
                continue;
            }

            if (c == 'v' || c == 'o')
            {
                while (i + 1 < text.length() && juce::CharacterFunctions::isDigit(text[i + 1]))
                    ++i;
                continue;
            }

            if (c == 'l')
            {
                juce::String digits;
                while (i + 1 < text.length() && juce::CharacterFunctions::isDigit(text[i + 1]))
                {
                    digits += text[i + 1];
                    ++i;
                }

                if (digits.isNotEmpty())
                {
                    const int parsed = digits.getIntValue();
                    if (parsed > 0)
                        defaultLength = parsed;
                }

                isDefaultDotted = false;
                if (i + 1 < text.length() && text[i + 1] == '.')
                {
                    isDefaultDotted = true;
                    ++i;
                }
                continue;
            }

            if (c == '<' || c == '>' || c == '&')
                continue;

            if (c == 'n')
            {
                while (i + 1 < text.length() && juce::CharacterFunctions::isDigit(text[i + 1]))
                    ++i;
                int noteLength = defaultLength > 0 ? defaultLength : 4;
                double beats = 4.0 / static_cast<double>(noteLength);
                if (isDefaultDotted)
                    beats *= 1.5;
                currentBeats += beats;
                continue;
            }

            const bool isRest = (c == 'r');
            const bool isNote = MmlLogic::getFrequencyFromNote(c) > 0.0;
            if (isRest || isNote)
            {
                if (i + 1 < text.length() && (text[i + 1] == '+' || text[i + 1] == '#' || text[i + 1] == '-'))
                    ++i;
                consumeLengthAndAdvanceBeat(i);
            }
        }

        std::stable_sort(result.begin(), result.end(), [](const PianoRollTempoEvent& a, const PianoRollTempoEvent& b)
        {
            return a.beat < b.beat;
        });

        std::vector<PianoRollTempoEvent> unique;
        for (const auto& ev : result)
        {
            if (!unique.empty() && std::abs(unique.back().beat - ev.beat) <= 0.0001)
                unique.back() = ev;
            else
                unique.push_back(ev);
        }
        return unique;
    }

    inline std::vector<PianoRollTempoEvent> normaliseTempoEvents(std::vector<PianoRollTempoEvent> events)
    {
        for (auto& ev : events)
        {
            ev.beat = juce::jmax(0.0, quantizeBeat64(ev.beat));
            ev.bpm = juce::jlimit(1, 999, ev.bpm);
        }

        std::stable_sort(events.begin(), events.end(), [](const PianoRollTempoEvent& a, const PianoRollTempoEvent& b)
        {
            return a.beat < b.beat;
        });

        std::vector<PianoRollTempoEvent> unique;
        for (const auto& ev : events)
        {
            if (!unique.empty() && std::abs(unique.back().beat - ev.beat) <= 0.0001)
                unique.back() = ev;
            else
                unique.push_back(ev);
        }
        return unique;
    }

    inline juce::String buildMmlFromPianoRollSequenceWithTempoEvents(const std::vector<MmlNote>& sequence,
                                                              const juce::String& originalMml,
                                                              std::vector<PianoRollTempoEvent> tempoEvents,
                                                              double minimumEndBeat = 0.0)
    {
        std::vector<MmlNote> events = sequence;
        std::stable_sort(events.begin(), events.end(), [](const MmlNote& a, const MmlNote& b)
        {
            if (std::abs(a.startBeat - b.startBeat) > 0.0001)
                return a.startBeat < b.startBeat;
            return a.endBeat < b.endBeat;
        });

        tempoEvents = normaliseTempoEvents(std::move(tempoEvents));

        juce::String result;
        int currentVolumeCommand = -1;
        int currentTempoCommand = -1;
        double currentBeat = 0.0;
        size_t tempoIndex = 0;

        auto emitTempoEventsAtCurrentBeat = [&]()
        {
            while (tempoIndex < tempoEvents.size() && tempoEvents[tempoIndex].beat <= currentBeat + 0.0001)
            {
                const int bpm = juce::jlimit(1, 999, tempoEvents[tempoIndex].bpm);
                if (bpm != currentTempoCommand)
                {
                    result += juce::String("t") + juce::String(bpm);
                    currentTempoCommand = bpm;
                }
                ++tempoIndex;
            }
        };

        auto advanceWithRestsUntil = [&](double targetBeat)
        {
            targetBeat = juce::jmax(0.0, quantizeBeat64(targetBeat));
            while (tempoIndex < tempoEvents.size() && tempoEvents[tempoIndex].beat < targetBeat - 0.0001)
            {
                const double tempoBeat = juce::jmax(currentBeat, tempoEvents[tempoIndex].beat);
                if (tempoBeat > currentBeat + 0.0001)
                {
                    result += makeMmlRestTokens(tempoBeat - currentBeat);
                    currentBeat = tempoBeat;
                }
                emitTempoEventsAtCurrentBeat();
            }

            if (targetBeat > currentBeat + 0.0001)
            {
                result += makeMmlRestTokens(targetBeat - currentBeat);
                currentBeat = targetBeat;
            }
            emitTempoEventsAtCurrentBeat();
        };

        emitTempoEventsAtCurrentBeat();

        for (const auto& ev : events)
        {
            const double eventStart = juce::jmax(0.0, quantizeBeat64(ev.startBeat));
            const double eventEnd = juce::jmax(eventStart, quantizeBeat64(ev.endBeat));
            if (eventEnd <= currentBeat + 0.0001)
                continue;

            advanceWithRestsUntil(eventStart);

            const double effectiveDuration = eventEnd - currentBeat;
            if (effectiveDuration <= 0.0001)
                continue;

            if (ev.frequency <= 0.0)
            {
                result += makeMmlRestTokens(effectiveDuration);
            }
            else
            {
                const int noteVolume = volumeFloatToMml(ev.volume);
                if (noteVolume != currentVolumeCommand)
                {
                    result += juce::String("v") + juce::String(noteVolume);
                    currentVolumeCommand = noteVolume;
                }
                result += makeMmlNoteTokensFromMidi(frequencyToMidiForMml(ev.frequency), effectiveDuration);
            }

            currentBeat = eventEnd;
            emitTempoEventsAtCurrentBeat();
        }

        // If the user places a tempo marker beyond the last note, keep it by
        // advancing with rests. This preserves the visual/event position instead
        // of silently dropping the marker.
        while (tempoIndex < tempoEvents.size())
            advanceWithRestsUntil(tempoEvents[tempoIndex].beat);

        // Mabiicco Track-1 tempo sync needs empty/new tracks to show the same
        // visible song length as the Track-1 master lane.  When minimumEndBeat
        // is supplied, extend the generated MML with rests after the final tempo
        // marker as well.  Example: Track1 t120c1d2e4t130c4d4c4 -> other tracks
        // t120r1r2.t130r2.
        if (minimumEndBeat > currentBeat + 0.0001)
            advanceWithRestsUntil(minimumEndBeat);

        return cleanupRedundantOctaveCommandsInMml(result);
    }

    inline juce::String buildMmlFromPianoRollSequence(const std::vector<MmlNote>& sequence, const juce::String& originalMml)
    {
        return buildMmlFromPianoRollSequenceWithTempoEvents(sequence, originalMml, extractTempoEventsFromMml(originalMml));
    }

    struct OptimizedMmlItem
    {
        enum Type { Tempo, Note, Rest } type = Rest;
        double beat = 0.0;
        int bpm = 120;
        int midi = 60;
        int suffixId = 5;
        int volume = 8;
        bool tieContinuation = false;
    };

    struct OptimizerSuffix
    {
        const char* suffix;
        double beats;
    };

    static constexpr int numOptimizerSuffixes = 14;
    static const OptimizerSuffix optimizerSuffixes[numOptimizerSuffixes] = {
        { "1.", 6.0 }, { "1", 4.0 },
        { "2.", 3.0 }, { "2", 2.0 },
        { "4.", 1.5 }, { "4", 1.0 },
        { "8.", 0.75 }, { "8", 0.5 },
        { "16.", 0.375 }, { "16", 0.25 },
        { "32.", 0.1875 }, { "32", 0.125 },
        { "64.", 0.09375 }, { "64", 0.0625 }
    };

    inline int defaultOptimizerLengthId()
    {
        return 5; // l4
    }

    inline int findClosestOptimizerSuffixId(double beats)
    {
        beats = juce::jmax(0.0625, quantizeBeat64(beats));
        int best = defaultOptimizerLengthId();
        double bestDistance = std::abs(beats - optimizerSuffixes[best].beats);
        for (int i = 0; i < numOptimizerSuffixes; ++i)
        {
            const double distance = std::abs(beats - optimizerSuffixes[i].beats);
            if (distance < bestDistance)
            {
                bestDistance = distance;
                best = i;
            }
        }
        return best;
    }

    inline juce::String makeShortestOctaveCommand(int currentOctave, int targetOctave)
    {
        currentOctave = juce::jlimit(0, 9, currentOctave);
        targetOctave = juce::jlimit(0, 9, targetOctave);
        if (currentOctave == targetOctave)
            return {};

        juce::String relative;
        const int delta = targetOctave - currentOctave;
        const juce::String step = delta > 0 ? ">" : "<";
        for (int i = 0; i < std::abs(delta); ++i)
            relative += step;

        const juce::String absolute = juce::String("o") + juce::String(targetOctave);
        return relative.length() <= absolute.length() ? relative : absolute;
    }


    inline juce::String cleanupRedundantOctaveCommandsInMml(const juce::String& source)
    {
        juce::String out;
        int currentOctave = 4; // MML default octave is 4, so o4 is redundant until the octave really changes.

        for (int i = 0; i < source.length(); ++i)
        {
            const auto c = source[i];
            const auto lower = juce::CharacterFunctions::toLowerCase(c);

            if (lower == 'o' && i + 1 < source.length() && juce::CharacterFunctions::isDigit(source[i + 1]))
            {
                int j = i + 1;
                juce::String digits;
                while (j < source.length() && juce::CharacterFunctions::isDigit(source[j]))
                {
                    digits += source[j];
                    ++j;
                }

                const int targetOctave = juce::jlimit(0, 9, digits.getIntValue());
                if (targetOctave != currentOctave)
                {
                    out += makeShortestOctaveCommand(currentOctave, targetOctave);
                    currentOctave = targetOctave;
                }

                i = j - 1;
                continue;
            }

            if (c == '>' || c == '<')
            {
                const int targetOctave = juce::jlimit(0, 9, currentOctave + (c == '>' ? 1 : -1));
                if (targetOctave != currentOctave)
                {
                    out += c;
                    currentOctave = targetOctave;
                }
                continue;
            }

            out += c;
        }

        return out;
    }

    inline void appendOptimizedDurationSegments(std::vector<OptimizedMmlItem>& items,
                                         OptimizedMmlItem::Type type,
                                         int midi,
                                         int volume,
                                         double startBeat,
                                         double endBeat,
                                         bool firstNoteSegmentIsTie)
    {
        startBeat = juce::jmax(0.0, quantizeBeat64(startBeat));
        endBeat = juce::jmax(startBeat, quantizeBeat64(endBeat));
        double remaining = endBeat - startBeat;
        if (remaining <= 0.0001)
            return;

        bool tie = firstNoteSegmentIsTie;
        int safety = 0;
        double cursor = startBeat;
        while (remaining > 0.0001 && safety++ < 512)
        {
            int chosen = -1;
            for (int i = 0; i < numOptimizerSuffixes; ++i)
            {
                if (optimizerSuffixes[i].beats <= remaining + 0.0001)
                {
                    chosen = i;
                    break;
                }
            }

            if (chosen < 0)
                chosen = numOptimizerSuffixes - 1;

            OptimizedMmlItem item;
            item.type = type;
            item.beat = cursor;
            item.midi = midi;
            item.volume = volume;
            item.suffixId = chosen;
            item.tieContinuation = (type == OptimizedMmlItem::Note) && tie;
            items.push_back(item);

            cursor = quantizeBeat64(cursor + optimizerSuffixes[chosen].beats);
            remaining = quantizeBeat64(endBeat - cursor);
            if (type == OptimizedMmlItem::Note)
                tie = true;
        }
    }

    inline void appendOptimizedSpanSplitByTempo(std::vector<OptimizedMmlItem>& items,
                                         const std::vector<PianoRollTempoEvent>& tempoEvents,
                                         OptimizedMmlItem::Type type,
                                         int midi,
                                         int volume,
                                         double startBeat,
                                         double endBeat,
                                         bool forceTieAtStart)
    {
        startBeat = juce::jmax(0.0, quantizeBeat64(startBeat));
        endBeat = juce::jmax(startBeat, quantizeBeat64(endBeat));
        if (endBeat <= startBeat + 0.0001)
            return;

        double segmentStart = startBeat;
        bool tie = forceTieAtStart;
        for (const auto& tempo : tempoEvents)
        {
            const double tempoBeat = juce::jmax(0.0, quantizeBeat64(tempo.beat));
            if (tempoBeat <= segmentStart + 0.0001)
                continue;
            if (tempoBeat >= endBeat - 0.0001)
                break;

            appendOptimizedDurationSegments(items, type, midi, volume, segmentStart, tempoBeat, tie);
            segmentStart = tempoBeat;
            if (type == OptimizedMmlItem::Note)
                tie = true;
        }

        appendOptimizedDurationSegments(items, type, midi, volume, segmentStart, endBeat, tie);
    }

    inline juce::String buildOptimizedMmlFromPianoRollSequenceWithTempoEvents(const std::vector<MmlNote>& sequence,
                                                                        const juce::String& originalMml,
                                                                        std::vector<PianoRollTempoEvent> tempoEvents,
                                                                        bool padToFutureTempoEvents = true,
                                                                        bool forceInitialTempo = false)
    {
        std::vector<MmlNote> events = sequence;
        std::stable_sort(events.begin(), events.end(), [](const MmlNote& a, const MmlNote& b)
        {
            if (std::abs(a.startBeat - b.startBeat) > 0.0001)
                return a.startBeat < b.startBeat;
            return a.endBeat < b.endBeat;
        });

        tempoEvents = normaliseTempoEvents(std::move(tempoEvents));

        std::vector<OptimizedMmlItem> items;
        double currentBeat = 0.0;

        for (const auto& ev : events)
        {
            const double eventStart = juce::jmax(0.0, quantizeBeat64(ev.startBeat));
            const double eventEnd = juce::jmax(eventStart, quantizeBeat64(ev.endBeat));
            if (eventEnd <= currentBeat + 0.0001)
                continue;

            if (eventStart > currentBeat + 0.0001)
                appendOptimizedSpanSplitByTempo(items, tempoEvents, OptimizedMmlItem::Rest, 60, 8, currentBeat, eventStart, false);

            const double effectiveStart = juce::jmax(currentBeat, eventStart);
            const int volume = volumeFloatToMml(ev.volume);
            if (ev.frequency <= 0.0)
            {
                appendOptimizedSpanSplitByTempo(items, tempoEvents, OptimizedMmlItem::Rest, 60, volume, effectiveStart, eventEnd, false);
            }
            else
            {
                appendOptimizedSpanSplitByTempo(items, tempoEvents, OptimizedMmlItem::Note, frequencyToMidiForMml(ev.frequency), volume, effectiveStart, eventEnd, false);
            }

            currentBeat = juce::jmax(currentBeat, eventEnd);
        }

        for (const auto& tempo : tempoEvents)
        {
            const double tempoBeat = juce::jmax(0.0, quantizeBeat64(tempo.beat));
            if (tempoBeat > currentBeat + 0.0001)
            {
                if (!padToFutureTempoEvents)
                    continue;

                appendOptimizedSpanSplitByTempo(items, tempoEvents, OptimizedMmlItem::Rest, 60, 8, currentBeat, tempoBeat, false);
                currentBeat = tempoBeat;
            }

            const int bpm = juce::jlimit(1, 999, tempo.bpm);
            if (tempoBeat <= 0.0001 && bpm == 120 && !forceInitialTempo && !originalMml.trim().startsWithIgnoreCase("t120"))
                continue;

            OptimizedMmlItem item;
            item.type = OptimizedMmlItem::Tempo;
            item.beat = tempoBeat;
            item.bpm = bpm;
            items.push_back(item);
        }

        std::stable_sort(items.begin(), items.end(), [](const OptimizedMmlItem& a, const OptimizedMmlItem& b)
        {
            if (std::abs(a.beat - b.beat) > 0.0001)
                return a.beat < b.beat;
            if (a.type != b.type)
                return a.type == OptimizedMmlItem::Tempo;
            return a.tieContinuation < b.tieContinuation;
        });

        if (items.empty())
            return {};

        struct OptCell
        {
            int cost = std::numeric_limits<int>::max() / 4;
            juce::String text;
        };

        constexpr int octaveCount = 10;
        constexpr int volumeCount = 16;
        constexpr int stateCount = numOptimizerSuffixes * octaveCount * volumeCount;

        auto stateIndex = [](int lengthId, int octave, int volume) -> int
        {
            lengthId = juce::jlimit(0, numOptimizerSuffixes - 1, lengthId);
            octave = juce::jlimit(0, 9, octave);
            volume = juce::jlimit(0, 15, volume);
            return (lengthId * 10 + octave) * 16 + volume;
        };

        auto decodeState = [](int index, int& lengthId, int& octave, int& volume)
        {
            volume = index % 16;
            index /= 16;
            octave = index % 10;
            lengthId = index / 10;
        };

        std::vector<OptCell> current(stateCount), next(stateCount);
        current[stateIndex(defaultOptimizerLengthId(), 4, 8)].cost = 0;

        auto tryUpdate = [](std::vector<OptCell>& states, int index, const OptCell& base, const juce::String& append)
        {
            const int newCost = base.cost + append.length();
            if (newCost < states[index].cost)
            {
                states[index].cost = newCost;
                states[index].text = base.text + append;
            }
        };

        static const char* noteNames[12] = { "c", "c#", "d", "d#", "e", "f", "f#", "g", "g#", "a", "a#", "b" };

        for (const auto& item : items)
        {
            for (auto& cell : next)
            {
                cell.cost = std::numeric_limits<int>::max() / 4;
                cell.text.clear();
            }

            for (int state = 0; state < stateCount; ++state)
            {
                const auto& base = current[state];
                if (base.cost >= std::numeric_limits<int>::max() / 8)
                    continue;

                int lengthId = defaultOptimizerLengthId();
                int octave = 4;
                int volume = 8;
                decodeState(state, lengthId, octave, volume);

                if (item.type == OptimizedMmlItem::Tempo)
                {
                    tryUpdate(next, state, base, juce::String("t") + juce::String(item.bpm));
                    continue;
                }

                if (item.type == OptimizedMmlItem::Rest)
                {
                    const juce::String explicitSuffix = (item.suffixId == lengthId) ? juce::String() : juce::String(optimizerSuffixes[item.suffixId].suffix);
                    tryUpdate(next, state, base, juce::String("r") + explicitSuffix);

                    if (item.suffixId != lengthId)
                    {
                        const int newState = stateIndex(item.suffixId, octave, volume);
                        tryUpdate(next, newState, base, juce::String("l") + optimizerSuffixes[item.suffixId].suffix + "r");
                    }
                    continue;
                }

                const int midi = juce::jlimit(12, 119, item.midi);
                const int targetOctave = juce::jlimit(0, 9, (midi / 12) - 1);
                const int noteIndex = ((midi % 12) + 12) % 12;
                const juce::String octaveCommand = makeShortestOctaveCommand(octave, targetOctave);
                const juce::String volumeCommand = (item.volume == volume) ? juce::String() : (juce::String("v") + juce::String(item.volume));
                const juce::String tiePrefix = item.tieContinuation ? juce::String("&") : juce::String();
                const juce::String noteHead = juce::String(noteNames[noteIndex]);

                const juce::String explicitSuffix = (item.suffixId == lengthId) ? juce::String() : juce::String(optimizerSuffixes[item.suffixId].suffix);
                const int noLengthChangeState = stateIndex(lengthId, targetOctave, item.volume);
                tryUpdate(next, noLengthChangeState, base, volumeCommand + octaveCommand + tiePrefix + noteHead + explicitSuffix);

                if (!item.tieContinuation && item.suffixId != lengthId)
                {
                    const int newState = stateIndex(item.suffixId, targetOctave, item.volume);
                    tryUpdate(next, newState, base, volumeCommand + octaveCommand + juce::String("l") + optimizerSuffixes[item.suffixId].suffix + tiePrefix + noteHead);
                }
            }

            current.swap(next);
        }

        int bestState = -1;
        int bestCost = std::numeric_limits<int>::max() / 4;
        for (int i = 0; i < stateCount; ++i)
        {
            if (current[i].cost < bestCost)
            {
                bestCost = current[i].cost;
                bestState = i;
            }
        }

        if (bestState < 0)
            return cleanupRedundantOctaveCommandsInMml(originalMml.removeCharacters("\r\n"));

        return cleanupRedundantOctaveCommandsInMml(current[bestState].text);
    }

    inline juce::String buildOptimizedMmlFromPianoRollSequenceWithTempoEventsClampedToSequenceEnd(const std::vector<MmlNote>& sequence,
                                                                                             const juce::String& originalMml,
                                                                                             std::vector<PianoRollTempoEvent> tempoEvents,
                                                                                             bool forceInitialTempo = true)
    {
        // 3MLE Track-1 master tempo sync: insert only the tempo markers that the
        // target lane actually reaches.  Unlike Mabiicco's scaffold builder, this
        // never pads the target text with rests just to reach a future tempo.
        return buildOptimizedMmlFromPianoRollSequenceWithTempoEvents(sequence,
                                                                     originalMml,
                                                                     std::move(tempoEvents),
                                                                     false,
                                                                     forceInitialTempo);
    }

    inline juce::String buildOptimizedMmlFromPianoRollSequence(const std::vector<MmlNote>& sequence, const juce::String& originalMml)
    {
        return buildOptimizedMmlFromPianoRollSequenceWithTempoEvents(sequence, originalMml, extractTempoEventsFromMml(originalMml));
    }

    inline void sortAndDeduplicateMeterChanges(std::vector<MeterChange>& changes)
    {
        std::stable_sort(changes.begin(), changes.end(), [](const MeterChange& a, const MeterChange& b)
        {
            return a.beatPosition < b.beatPosition;
        });

        std::vector<MeterChange> unique;
        for (const auto& change : changes)
        {
            if (!unique.empty() && std::abs(unique.back().beatPosition - change.beatPosition) <= 0.0001)
                unique.back() = change;
            else
                unique.push_back(change);
        }
        changes.swap(unique);
    }


}
