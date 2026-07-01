#include "MmlLogic.h"

double MmlLogic::getFrequencyFromNote(juce::juce_wchar note) {
    if (note == 'c') return 261.63; else if (note == 'd') return 293.66; else if (note == 'e') return 329.63; else if (note == 'f') return 349.23;
    else if (note == 'g') return 392.00; else if (note == 'a') return 440.00; else if (note == 'b') return 493.88; return -1.0;
}

juce::String MmlLogic::getMMLStringFromMidi(int midiNote) {
    int octave = (midiNote / 12) - 1; if (octave < 0) octave = 0;
    int noteIndex = ((midiNote % 12) + 12) % 12;
    juce::String noteNames[] = { "c", "c#", "d", "d#", "e", "f", "f#", "g", "g#", "a", "a#", "b" }; 
    return "o" + juce::String(octave) + noteNames[noteIndex];
}

void MmlLogic::getChordIntervals(int midiNote, int& third, int& fifth) {
    int note = ((midiNote % 12) + 12) % 12;
    if (note == 0 || note == 5 || note == 7 || note == 8 || note == 10) { third = 4; fifth = 7; }
    else if (note == 2 || note == 4 || note == 9 || note == 1 || note == 3 || note == 6) { third = 3; fifth = 7; }
    else { third = 3; fifth = 6; }
}

int MmlLogic::detectScaleIdFromMelody(const juce::String& input)
{
    double pitchWeights[12] = { 0.0 };
    double totalWeight = 0.0;
    int firstPitchClass = -1;
    int lastPitchClass = -1;

    int currentOctave = 4;
    int defaultLength = 4;
    bool defaultDotted = false;

    auto wrapPc = [](int value) -> int { return ((value % 12) + 12) % 12; };

    auto getNoteBasePc = [](juce::juce_wchar noteChar) -> int {
        switch (juce::CharacterFunctions::toLowerCase(noteChar)) {
        case 'c': return 0; case 'd': return 2; case 'e': return 4;
        case 'f': return 5; case 'g': return 7; case 'a': return 9; case 'b': return 11;
        default:  return -1;
        }
    };

    auto getBeatWeight = [](int lengthValue, bool dotted) -> double {
        if (lengthValue <= 0) lengthValue = 4;
        double beats = 4.0 / static_cast<double>(lengthValue);
        if (dotted) beats *= 1.5;
        return juce::jlimit(0.0625, 8.0, beats);
    };

    auto addPitch = [&](int pitchClass, double weight) {
        pitchClass = wrapPc(pitchClass);
        pitchWeights[pitchClass] += weight;
        totalWeight += weight;
        if (firstPitchClass < 0) firstPitchClass = pitchClass;
        lastPitchClass = pitchClass;
    };

    for (int i = 0; i < input.length(); ++i) {
        auto c = juce::CharacterFunctions::toLowerCase(input[i]);

        if (c == 'm' && i + 2 < input.length() && juce::CharacterFunctions::toLowerCase(input[i + 1]) == 'm' && juce::CharacterFunctions::toLowerCase(input[i + 2]) == 'l') { i += 2; continue; }
        if (c == '@' || c == ',' || c == ';') continue;
        if (c == '<') { --currentOctave; continue; }
        if (c == '>') { ++currentOctave; continue; }

        if (c == 'o') {
            juce::String numStr;
            while (i + 1 < input.length() && juce::CharacterFunctions::isDigit(input[i + 1])) { numStr += input[i + 1]; ++i; }
            if (numStr.isNotEmpty()) currentOctave = numStr.getIntValue();
            continue;
        }

        if (c == 'l') {
            juce::String numStr;
            while (i + 1 < input.length() && juce::CharacterFunctions::isDigit(input[i + 1])) { numStr += input[i + 1]; ++i; }
            if (numStr.isNotEmpty()) { int parsed = numStr.getIntValue(); if (parsed > 0) defaultLength = parsed; }
            defaultDotted = false;
            if (i + 1 < input.length() && input[i + 1] == '.') { defaultDotted = true; ++i; }
            continue;
        }

        if (c == 'v' || c == 't' || c == 'q' || c == 'p') {
            while (i + 1 < input.length() && juce::CharacterFunctions::isDigit(input[i + 1])) ++i;
            continue;
        }

        if (c == 'r') {
            while (i + 1 < input.length() && juce::CharacterFunctions::isDigit(input[i + 1])) ++i;
            if (i + 1 < input.length() && input[i + 1] == '.') ++i;
            continue;
        }

        if (c == 'n') {
            juce::String numStr;
            while (i + 1 < input.length() && juce::CharacterFunctions::isDigit(input[i + 1])) { numStr += input[i + 1]; ++i; }
            if (numStr.isNotEmpty()) { int nVal = numStr.getIntValue(); addPitch(nVal + 12, getBeatWeight(defaultLength, defaultDotted)); }
            continue;
        }

        const int basePc = getNoteBasePc(c);
        if (basePc >= 0) {
            int pitchClass = basePc;
            if (i + 1 < input.length() && (input[i + 1] == '+' || input[i + 1] == '#')) { ++pitchClass; ++i; }
            else if (i + 1 < input.length() && input[i + 1] == '-') { --pitchClass; ++i; }

            int noteLength = defaultLength;
            bool dotted = defaultDotted;
            juce::String numStr;
            while (i + 1 < input.length() && juce::CharacterFunctions::isDigit(input[i + 1])) { numStr += input[i + 1]; ++i; }
            if (numStr.isNotEmpty()) { int parsed = numStr.getIntValue(); if (parsed > 0) noteLength = parsed; dotted = false; }
            if (i + 1 < input.length() && input[i + 1] == '.') { dotted = true; ++i; }

            addPitch(pitchClass, getBeatWeight(noteLength, dotted));
        }
    }

    if (totalWeight <= 0.0) return 1;

    const double majorProfile[12] = { 6.35, 2.23, 3.48, 2.33, 4.38, 4.09, 2.52, 5.19, 2.39, 3.66, 2.29, 2.88 };
    const double minorProfile[12] = { 6.33, 2.68, 3.52, 5.38, 2.60, 3.53, 2.54, 4.75, 3.98, 2.69, 3.34, 3.17 };

    int bestRoot = 0; bool bestIsMinor = false; double bestScore = -1.0e18;

    for (int root = 0; root < 12; ++root) {
        double majorScore = 0.0, minorScore = 0.0;
        for (int pc = 0; pc < 12; ++pc) {
            const int rel = wrapPc(pc - root);
            majorScore += pitchWeights[pc] * majorProfile[rel];
            minorScore += pitchWeights[pc] * minorProfile[rel];
        }

        if (lastPitchClass == root) { majorScore += totalWeight * 1.20; minorScore += totalWeight * 1.20; }
        if (firstPitchClass == root) { majorScore += totalWeight * 0.45; minorScore += totalWeight * 0.45; }
        majorScore += pitchWeights[wrapPc(root + 4)] * 0.35;
        minorScore += pitchWeights[wrapPc(root + 3)] * 0.35;

        if (majorScore > bestScore) { bestScore = majorScore; bestRoot = root; bestIsMinor = false; }
        if (minorScore > bestScore) { bestScore = minorScore; bestRoot = root; bestIsMinor = true; }
    }

    static const int majorScaleIdByRoot[12] = { 2, 14, 4, 12, 6, 10, 8, 3, 13, 5, 11, 7 };
    static const int minorScaleIdByRoot[12] = { 27, 21, 25, 30, 18, 28, 20, 26, 22, 17, 29, 19 };

    return bestIsMinor ? minorScaleIdByRoot[bestRoot] : majorScaleIdByRoot[bestRoot];
}

juce::String MmlLogic::transformMML(const juce::String& input, int type, int scaleId, int beatsPerMeasure) {
    // type 3(아르페지오), 5~9(새로운 아르페지오 패턴들), 4(자동 베이스) 처리
    bool isArp = (type == 3 || (type >= 5 && type <= 9));
    if (type == 4 || isArp) {
        struct BassNoteInfo { juce::String noteName = ""; int octave = 4; bool hasNote = false; };
        std::vector<BassNoteInfo> measureNotes;
        double currentBeats = 0.0; int currentOctave = 4; int defaultLength = 4; bool isDefaultDotted = false;

        // 1단계: 마디별 첫 멜로디 노트 추출
        for (int i = 0; i < input.length(); ++i) {
            juce::juce_wchar c = juce::CharacterFunctions::toLowerCase(input[i]);
            if (c == 'v' || c == 't') { while (i + 1 < input.length() && juce::CharacterFunctions::isDigit(input[i + 1])) { i++; } continue; }
            if (c == 'o') { juce::String numStr = ""; while (i + 1 < input.length() && juce::CharacterFunctions::isDigit(input[i + 1])) { numStr += input[i + 1]; i++; } if (numStr.isNotEmpty()) currentOctave = numStr.getIntValue(); continue; }
            if (c == '<') { currentOctave--; continue; } if (c == '>') { currentOctave++; continue; }
            if (c == 'l') {
                juce::String numStr = ""; while (i + 1 < input.length() && juce::CharacterFunctions::isDigit(input[i + 1])) { numStr += input[i + 1]; i++; }
                if (numStr.isNotEmpty()) { int parsed = numStr.getIntValue(); if (parsed > 0) defaultLength = parsed; }
                isDefaultDotted = false; if (i + 1 < input.length() && input[i + 1] == '.') { isDefaultDotted = true; i++; } continue;
            }
            if (c == '&') { continue; }

            if (c == 'n') {
                juce::String numStr = ""; while (i + 1 < input.length() && juce::CharacterFunctions::isDigit(input[i + 1])) { numStr += input[i + 1]; i++; }
                int noteLength = defaultLength > 0 ? defaultLength : 4;
                double beats = 4.0 / static_cast<double>(noteLength); if (isDefaultDotted) beats *= 1.5;
                int measureIndex = static_cast<int>(std::floor(currentBeats / beatsPerMeasure));
                if (measureIndex >= (int)measureNotes.size()) measureNotes.resize(measureIndex + 1);
                if (numStr.isNotEmpty()) {
                    int midi = numStr.getIntValue() + 12; int noteOct = midi / 12 - 1;
                    juce::String noteNames[] = { "c", "c#", "d", "d#", "e", "f", "f#", "g", "g#", "a", "a#", "b" };
                    if (!measureNotes[measureIndex].hasNote) { measureNotes[measureIndex].noteName = noteNames[((midi % 12) + 12) % 12]; measureNotes[measureIndex].octave = noteOct; measureNotes[measureIndex].hasNote = true; }
                }
                currentBeats += beats; continue;
            }

            bool isNote = (c >= 'a' && c <= 'g'); bool isRest = (c == 'r');
            if (isNote || isRest) {
                juce::String fullNoteName = juce::String::charToString(c);
                if (i + 1 < input.length() && (input[i + 1] == '+' || input[i + 1] == '#')) { fullNoteName += input[i + 1]; i++; }
                else if (i + 1 < input.length() && input[i + 1] == '-') { fullNoteName += input[i + 1]; i++; }

                int noteLength = defaultLength; bool isDotted = isDefaultDotted;
                juce::String numStr = ""; while (i + 1 < input.length() && juce::CharacterFunctions::isDigit(input[i + 1])) { numStr += input[i + 1]; i++; }
                if (numStr.isNotEmpty()) { int parsed = numStr.getIntValue(); if (parsed > 0) noteLength = parsed; isDotted = false; }
                if (i + 1 < input.length() && input[i + 1] == '.') { isDotted = true; i++; }
                if (noteLength <= 0) noteLength = 4;
                double beats = 4.0 / static_cast<double>(noteLength); if (isDotted) beats *= 1.5;
                int measureIndex = static_cast<int>(std::floor(currentBeats / beatsPerMeasure));
                if (measureIndex >= (int)measureNotes.size()) measureNotes.resize(measureIndex + 1);
                if (isNote && !measureNotes[measureIndex].hasNote) { measureNotes[measureIndex].noteName = fullNoteName; measureNotes[measureIndex].octave = currentOctave; measureNotes[measureIndex].hasNote = true; }
                currentBeats += beats;
            }
        }

        juce::String resultBfs;
        int lastOutputOctave = 4;
        juce::String restLengthStr = (beatsPerMeasure == 3) ? "2." : "1";

        // 조표(스케일)에 포함된 7개의 음표를 미리 계산해 둡니다.
        int root = 0;
        switch (scaleId) {
        case 2: root = 0; break; case 3: root = 7; break; case 4: root = 2; break; case 5: root = 9; break; case 6: root = 4; break; case 7: root = 11; break; case 8: root = 6; break; case 9: root = 1; break;
        case 10: root = 5; break; case 11: root = 10; break; case 12: root = 3; break; case 13: root = 8; break; case 14: root = 1; break; case 15: root = 6; break; case 16: root = 11; break;
        case 17: root = 9; break; case 18: root = 4; break; case 19: root = 11; break; case 20: root = 6; break; case 21: root = 1; break; case 22: root = 8; break; case 23: root = 3; break; case 24: root = 10; break;
        case 25: root = 2; break; case 26: root = 7; break; case 27: root = 0; break; case 28: root = 5; break; case 29: root = 10; break; case 30: root = 3; break; case 31: root = 8; break;
        }
        bool isMajor = (scaleId >= 2 && scaleId <= 16);
        bool isMinor = (scaleId >= 17 && scaleId <= 31);
        int scalePcs[7] = { 0, 2, 4, 5, 7, 9, 11 };
        if (scaleId > 1) {
            int majorInt[7] = { 0, 2, 4, 5, 7, 9, 11 };
            int minorInt[7] = { 0, 2, 3, 5, 7, 8, 10 };
            for (int i = 0; i < 7; ++i) scalePcs[i] = (root + (isMajor ? majorInt[i] : minorInt[i])) % 12;
        }

        auto wrapPc = [](int value) -> int { return ((value % 12) + 12) % 12; };

        // 2단계: 각 마디별로 코드를 분석하고 아르페지오 생성
        for (size_t m = 0; m < measureNotes.size(); ++m) {
            if (measureNotes[m].hasNote) {
                juce::String noteName = measureNotes[m].noteName;
                int pitchClass = 0; juce::juce_wchar baseC = noteName[0];
                if (baseC == 'c') pitchClass = 0; else if (baseC == 'd') pitchClass = 2; else if (baseC == 'e') pitchClass = 4; else if (baseC == 'f') pitchClass = 5; else if (baseC == 'g') pitchClass = 7; else if (baseC == 'a') pitchClass = 9; else if (baseC == 'b') pitchClass = 11;
                if (noteName.length() > 1) { if (noteName[1] == '#' || noteName[1] == '+') pitchClass += 1; else if (noteName[1] == '-') pitchClass -= 1; }
                pitchClass = wrapPc(pitchClass);

                // 현재 마디의 코드 근음(Root) 계산
                int bassPitchClass = pitchClass;
                if (scaleId > 1) {
                    const int relativePitch = wrapPc(pitchClass - root);
                    if (isMajor) {
                        switch (relativePitch) {
                        case 0:  bassPitchClass = root; break; case 2:  bassPitchClass = wrapPc(root + 7); break;
                        case 4:  bassPitchClass = root; break; case 5:  bassPitchClass = wrapPc(root + 5); break;
                        case 7:  bassPitchClass = wrapPc(root + 7); break; case 9:  bassPitchClass = wrapPc(root + 9); break;
                        case 11: bassPitchClass = wrapPc(root + 7); break; case 1:  bassPitchClass = wrapPc(root + 1); break;
                        case 3:  bassPitchClass = wrapPc(root + 3); break; case 6:  bassPitchClass = wrapPc(root + 7); break;
                        case 8:  bassPitchClass = wrapPc(root + 8); break; case 10: bassPitchClass = wrapPc(root + 10); break;
                        default: bassPitchClass = pitchClass; break;
                        }
                    }
                    else if (isMinor) {
                        switch (relativePitch) {
                        case 0:  bassPitchClass = root; break; case 2:  bassPitchClass = wrapPc(root + 7); break;
                        case 3:  bassPitchClass = root; break; case 5:  bassPitchClass = wrapPc(root + 5); break;
                        case 7:  bassPitchClass = root; break; case 8:  bassPitchClass = wrapPc(root + 8); break;
                        case 10: bassPitchClass = wrapPc(root + 10); break; case 11: bassPitchClass = wrapPc(root + 7); break;
                        case 1:  bassPitchClass = wrapPc(root + 1); break; case 4:  bassPitchClass = wrapPc(root + 3); break;
                        case 6:  bassPitchClass = wrapPc(root + 7); break; case 9:  bassPitchClass = wrapPc(root + 5); break;
                        default: bassPitchClass = pitchClass; break;
                        }
                    }
                }

                // ========================================================
                // [도우미 4번] 자동 베이스 모드
                // ========================================================
                if (type == 4) {
                    int bassOffset = 2; // 베이스도 2옥타브 내립니다.
                    int targetOctave = std::max(1, measureNotes[m].octave - bassOffset);
                    juce::String outNoteNames[] = { "c", "c#", "d", "d#", "e", "f", "f#", "g", "g#", "a", "a#", "b" };
                    juce::String finalBassStr = outNoteNames[wrapPc(bassPitchClass)];

                    if (std::abs(targetOctave - lastOutputOctave) <= 2) {
                        while (lastOutputOctave < targetOctave) { resultBfs += ">"; lastOutputOctave++; }
                        while (lastOutputOctave > targetOctave) { resultBfs += "<"; lastOutputOctave--; }
                    }
                    else {
                        resultBfs += "o" + juce::String(targetOctave); lastOutputOctave = targetOctave;
                    }
                    resultBfs += finalBassStr + restLengthStr;
                }
                // ========================================================
                // [도우미 3, 5~9번] 자동 아르페지오 모드
                // ========================================================
                else if (isArp) {
                    // ★ 멜로디로부터 아르페지오를 정확히 2옥타브 내립니다.
                    int arpOffset = 2;
                    int targetOctave = std::max(1, measureNotes[m].octave - arpOffset);

                    int rootIdx = -1;
                    for (int i = 0; i < 7; ++i) { if (scalePcs[i] == bassPitchClass) { rootIdx = i; break; } }

                    int pc1 = bassPitchClass;
                    int pc2 = (rootIdx >= 0) ? scalePcs[(rootIdx + 1) % 7] : wrapPc(pc1 + 2);
                    int pc3 = (rootIdx >= 0) ? scalePcs[(rootIdx + 2) % 7] : wrapPc(pc1 + 4);
                    int pc5 = (rootIdx >= 0) ? scalePcs[(rootIdx + 4) % 7] : wrapPc(pc1 + 7);

                    int midi1 = (targetOctave + 1) * 12 + pc1;
                    auto getAbsMidi = [](int base, int tgtPc) {
                        int curr = base % 12; int diff = tgtPc - curr; if (diff < 0) diff += 12; return base + diff;
                    };

                    int midi2 = getAbsMidi(midi1, pc2);
                    int midi3 = getAbsMidi(midi1, pc3);
                    int midi5 = getAbsMidi(midi1, pc5);
                    int midiH1 = midi1 + 12; // 높은음 1
                    int midiH2 = midi2 + 12; // 높은음 2
                    int midiH3 = midi3 + 12; // 높은음 3

                    struct ArpNote { int midi; int length; };
                    std::vector<ArpNote> pattern;

                    if (beatsPerMeasure == 4) {
                        if (type == 3) pattern = { {midi1, 4}, {midi3, 4}, {midi5, 2} };
                        else if (type == 5) pattern = { {midi1, 2}, {midi5, 2} };
                        else if (type == 6) pattern = { {midi1, 4}, {midi5, 4}, {midiH1, 2} };
                        else if (type == 7) pattern = { {midi1, 4}, {midi5, 4}, {midiH3, 2} };
                        else if (type == 8) pattern = { {midi1, 4}, {midi5, 4}, {midiH1, 4}, {midiH3, 4} };
                        else if (type == 9) pattern = { {midi1, 4}, {midi5, 4}, {midiH2, 4}, {midiH3, 4} };
                    }
                    else { // 3/4 박자
                        if (type == 3) pattern = { {midi1, 4}, {midi3, 4}, {midi5, 4} };
                        else if (type == 5) pattern = { {midi1, 4}, {midi5, 2} };
                        else if (type == 6 || type == 8) pattern = { {midi1, 4}, {midi5, 4}, {midiH1, 4} };
                        else if (type == 7 || type == 9) pattern = { {midi1, 4}, {midi5, 4}, {midiH3, 4} };
                    }

                    juce::String outNoteNames[] = { "c", "c#", "d", "d#", "e", "f", "f#", "g", "g#", "a", "a#", "b" };
                    for (const auto& pn : pattern) {
                        int nOct = (pn.midi / 12) - 1;
                        juce::String nName = outNoteNames[pn.midi % 12];
                        juce::String lenStr = juce::String(pn.length);

                        if (std::abs(nOct - lastOutputOctave) <= 2) {
                            while (lastOutputOctave < nOct) { resultBfs += ">"; lastOutputOctave++; }
                            while (lastOutputOctave > nOct) { resultBfs += "<"; lastOutputOctave--; }
                        }
                        else {
                            resultBfs += "o" + juce::String(nOct); lastOutputOctave = nOct;
                        }
                        resultBfs += nName + lenStr;
                    }
                }
            }
            else {
                resultBfs += "r" + restLengthStr; // 쉼표 마디
            }
        }
        return resultBfs;
    }

    // ========================================================
    // [도우미 1 & 2번] 자동 화음 모드 (기존 유지)
    // ========================================================
    juce::String result; int currentOctave = 4; int lastOutputOctave = 4;
    juce::String noteNames[] = { "c", "c#", "d", "d#", "e", "f", "f#", "g", "g#", "a", "a#", "b" };
    for (int i = 0; i < input.length(); ++i) {
        juce::juce_wchar c = juce::CharacterFunctions::toLowerCase(input[i]);
        if (c == 'o') { juce::String numStr = ""; result += input[i]; while (i + 1 < input.length() && juce::CharacterFunctions::isDigit(input[i + 1])) { numStr += input[i + 1]; i++; } if (numStr.isNotEmpty()) currentOctave = numStr.getIntValue(); result += numStr; lastOutputOctave = currentOctave; }
        else if (c == '<') { currentOctave--; result += "<"; lastOutputOctave--; }
        else if (c == '>') { currentOctave++; result += ">"; lastOutputOctave++; }
        else if (c == 'l') {
            result += c; juce::String numStr = ""; while (i + 1 < input.length() && juce::CharacterFunctions::isDigit(input[i + 1])) { numStr += input[i + 1]; i++; }
            result += numStr; if (i + 1 < input.length() && input[i + 1] == '.') { result += input[i + 1]; i++; }
        }
        else if (c == 'r' || c == 'v' || c == 't') { result += input[i]; }
        else if (c == 'n') {
            juce::String numStr = ""; while (i + 1 < input.length() && juce::CharacterFunctions::isDigit(input[i + 1])) { numStr += input[i + 1]; i++; }
            if (numStr.isNotEmpty()) {
                int midi = numStr.getIntValue() + 12;
                if (type == 1 || type == 2) {
                    int third, fifth; getChordIntervals(((midi % 12) + 12) % 12, third, fifth);
                    int newMidi = midi + (type == 1 ? fifth - 12 : third - 12);
                    int targetOctave = newMidi / 12 - 1;
                    while (lastOutputOctave < targetOctave) { result += ">"; lastOutputOctave++; }
                    while (lastOutputOctave > targetOctave) { result += "<"; lastOutputOctave--; }
                    result += noteNames[((newMidi % 12) + 12) % 12];
                }
            }
        }
        else if (c >= 'a' && c <= 'g') {
            int semitoneShift = 0;
            if (i + 1 < input.length() && (input[i + 1] == '+' || input[i + 1] == '#')) { semitoneShift = 1; i++; }
            else if (i + 1 < input.length() && input[i + 1] == '-') { semitoneShift = -1; i++; }
            juce::String lengthStr = ""; while (i + 1 < input.length() && (juce::CharacterFunctions::isDigit(input[i + 1]) || input[i + 1] == '.')) { lengthStr += input[i + 1]; i++; }
            int baseMidi = 0; if (c == 'c') baseMidi = 0; else if (c == 'd') baseMidi = 2; else if (c == 'e') baseMidi = 4; else if (c == 'f') baseMidi = 5; else if (c == 'g') baseMidi = 7; else if (c == 'a') baseMidi = 9; else if (c == 'b') baseMidi = 11;
            int midi = (currentOctave + 1) * 12 + baseMidi + semitoneShift;

            if (type == 1 || type == 2) {
                int third, fifth; getChordIntervals(((midi % 12) + 12) % 12, third, fifth);
                int newMidi = midi + (type == 1 ? fifth - 12 : third - 12);
                int targetOctave = newMidi / 12 - 1;
                while (lastOutputOctave < targetOctave) { result += ">"; lastOutputOctave++; }
                while (lastOutputOctave > targetOctave) { result += "<"; lastOutputOctave--; }
                result += noteNames[((newMidi % 12) + 12) % 12] + lengthStr;
            }
        }
        else { result += input[i]; }
    }
    return result;
}

std::vector<TempoChange> MmlLogic::buildTempoMap(const std::vector<juce::String>& mmlTracks, double sampleRateHz) {
    std::vector<TempoChange> tempoMap;
    std::vector<TempoChange> tempMap;
    double safeSampleRateHz = sampleRateHz > 0 ? sampleRateHz : 44100.0;

    for (size_t tIdx = 0; tIdx < mmlTracks.size(); ++tIdx) {
        juce::String text = mmlTracks[tIdx];
        double currentBpm = 120.0; double currentBeats = 0.0; double currentTime = 0.0; int defaultLength = 4; bool isDefaultDotted = false;

        // The initial tempo is 120 unless there is an actual tempo command at beat 0.
        // Do not scan ahead for the first t-command: a later t150 after leading rests
        // must not become the beat-0 tempo.
        if (tIdx == 0) tempMap.push_back({ 0.0, 120.0, 0 });

        for (int i = 0; i < text.length(); ++i) {
            juce::juce_wchar c = juce::CharacterFunctions::toLowerCase(text[i]);
            if (c == 'v') { while (i + 1 < text.length() && juce::CharacterFunctions::isDigit(text[i + 1])) i++; continue; }
            if (c == 't') {
                juce::String numStr = "";
                while (i + 1 < text.length() && juce::CharacterFunctions::isDigit(text[i + 1])) { numStr += text[i + 1]; i++; }
                if (numStr.isNotEmpty()) {
                    double parsed = numStr.getDoubleValue();
                    if (parsed > 0.0) currentBpm = parsed;
                    int64_t samplePos = static_cast<int64_t>(currentTime * safeSampleRateHz);
                    tempMap.push_back({ currentBeats, currentBpm, samplePos });
                } continue;
            }
            else if (c == 'o') { while (i + 1 < text.length() && juce::CharacterFunctions::isDigit(text[i + 1])) i++; continue; }
            else if (c == 'l') {
                juce::String numStr = ""; while (i + 1 < text.length() && juce::CharacterFunctions::isDigit(text[i + 1])) { numStr += text[i + 1]; i++; }
                if (numStr.isNotEmpty()) { int parsed = numStr.getIntValue(); if (parsed > 0) defaultLength = parsed; }
                isDefaultDotted = false; if (i + 1 < text.length() && text[i + 1] == '.') { isDefaultDotted = true; i++; } continue;
            }
            else if (c == '<' || c == '>' || c == '&') { continue; }
            else if (c == 'n') {
                while (i + 1 < text.length() && juce::CharacterFunctions::isDigit(text[i + 1])) i++;
                int noteLength = defaultLength > 0 ? defaultLength : 4;
                double beats = 4.0 / static_cast<double>(noteLength); if (isDefaultDotted) beats *= 1.5;
                currentTime += beats * (60.0 / currentBpm); currentBeats += beats; continue;
            }

            double baseFreq = getFrequencyFromNote(c); bool isRest = (c == 'r');
            if (baseFreq > 0.0 || isRest) {
                if (i + 1 < text.length() && (text[i + 1] == '+' || text[i + 1] == '#' || text[i + 1] == '-')) i++;
                int noteLength = defaultLength; bool isDotted = isDefaultDotted;
                juce::String numStr = ""; while (i + 1 < text.length() && juce::CharacterFunctions::isDigit(text[i + 1])) { numStr += text[i + 1]; i++; }
                if (numStr.isNotEmpty()) { int parsed = numStr.getIntValue(); if (parsed > 0) noteLength = parsed; isDotted = false; }
                if (i + 1 < text.length() && text[i + 1] == '.') { isDotted = true; i++; }
                if (noteLength <= 0) noteLength = 4;
                double beats = 4.0 / static_cast<double>(noteLength); if (isDotted) beats *= 1.5;
                currentTime += beats * (60.0 / currentBpm); currentBeats += beats;
            }
        }
    }

    std::sort(tempMap.begin(), tempMap.end(), [](const TempoChange& a, const TempoChange& b) { return a.beatPosition < b.beatPosition; });
    for (const auto& tc : tempMap) {
        if (tempoMap.empty() || std::abs(tempoMap.back().beatPosition - tc.beatPosition) > 0.001) { tempoMap.push_back(tc); }
        else { tempoMap.back().bpm = tc.bpm; tempoMap.back().samplePosition = tc.samplePosition; }
    }
    if (tempoMap.empty()) tempoMap.push_back({ 0.0, 120.0, 0 });

    return tempoMap;
}

std::vector<MmlNote> MmlLogic::parseMMLWithTempoMap(const juce::String& text, const std::vector<TempoChange>& tempoMap, double sampleRateHz) {
    std::vector<MmlNote> result; double currentTime = 0.0; double currentBeats = 0.0; int currentOctave = 4; int defaultLength = 4; bool isDefaultDotted = false;
    float currentVolume = 8.0f / 15.0f; bool pendingTie = false;

    if (tempoMap.empty()) return result;

    for (int i = 0; i < text.length(); ++i) {
        juce::juce_wchar c = juce::CharacterFunctions::toLowerCase(text[i]);
        if (c == 'v') {
            juce::String numStr = ""; while (i + 1 < text.length() && juce::CharacterFunctions::isDigit(text[i + 1])) { numStr += text[i + 1]; i++; }
            if (numStr.isNotEmpty()) { int vVal = numStr.getIntValue(); if (vVal > 15) vVal = 15; if (vVal < 0) vVal = 0; currentVolume = static_cast<float>(vVal) / 15.0f; } continue;
        }
        if (c == 't') { while (i + 1 < text.length() && juce::CharacterFunctions::isDigit(text[i + 1])) i++; continue; }
        if (c == 'o') { juce::String numStr = ""; while (i + 1 < text.length() && juce::CharacterFunctions::isDigit(text[i + 1])) { numStr += text[i + 1]; i++; } if (numStr.isNotEmpty()) currentOctave = numStr.getIntValue(); continue; }
        if (c == 'l') {
            juce::String numStr = ""; while (i + 1 < text.length() && juce::CharacterFunctions::isDigit(text[i + 1])) { numStr += text[i + 1]; i++; }
            if (numStr.isNotEmpty()) { int parsed = numStr.getIntValue(); if (parsed > 0) defaultLength = parsed; }
            isDefaultDotted = false; if (i + 1 < text.length() && text[i + 1] == '.') { isDefaultDotted = true; i++; } continue;
        }
        if (c == '<') { currentOctave--; continue; } if (c == '>') { currentOctave++; continue; }
        if (c == '&') { pendingTie = true; continue; }
        if (c == 'n') {
            const int tokenStart = i;
            juce::String numStr = ""; while (i + 1 < text.length() && juce::CharacterFunctions::isDigit(text[i + 1])) { numStr += text[i + 1]; i++; }
            const int tokenEnd = i + 1;
            if (numStr.isNotEmpty()) {
                int nVal = numStr.getIntValue(); double freq = 440.0 * std::pow(2.0, (nVal + 12 - 69) / 12.0);
                double activeBpm = tempoMap[0].bpm; for (const auto& t : tempoMap) { if (currentBeats >= t.beatPosition - 0.001) activeBpm = t.bpm; }
                int noteLength = defaultLength > 0 ? defaultLength : 4;
                double beats = 4.0 / static_cast<double>(noteLength); if (isDefaultDotted) beats *= 1.5; double duration = beats * (60.0 / activeBpm);
                int64_t startS = getSamplePositionFromTime(currentTime, sampleRateHz); currentTime += duration; int64_t endS = getSamplePositionFromTime(currentTime, sampleRateHz);

                if (pendingTie && !result.empty() && std::abs(result.back().frequency - freq) < 1.0) { result.back().endSample = endS; result.back().endBeat = currentBeats + beats; result.back().textEnd = tokenEnd; }
                else { if (pendingTie && !result.empty()) result.back().isTie = true; result.push_back({ freq, startS, endS, false, currentVolume, currentBeats, currentBeats + beats, tokenStart, tokenEnd }); }
                currentBeats += beats; pendingTie = false;
            } continue;
        }

        double baseFreq = getFrequencyFromNote(c); bool isRest = (c == 'r');
        if (baseFreq > 0.0 || isRest) {
            const int tokenStart = i;
            int semitoneShift = 0; if (i + 1 < text.length()) { juce::juce_wchar nextC = text[i + 1]; if (nextC == '+' || nextC == '#' || nextC == '-') { semitoneShift = (nextC == '-') ? -1 : 1; i++; } }
            int noteLength = defaultLength; bool isDotted = isDefaultDotted;
            juce::String numStr = ""; while (i + 1 < text.length() && juce::CharacterFunctions::isDigit(text[i + 1])) { numStr += text[i + 1]; i++; }
            if (numStr.isNotEmpty()) { int parsed = numStr.getIntValue(); if (parsed > 0) noteLength = parsed; isDotted = false; }
            if (i + 1 < text.length() && text[i + 1] == '.') { isDotted = true; i++; }
            if (noteLength <= 0) noteLength = 4; const int tokenEnd = i + 1;

            double activeBpm = tempoMap[0].bpm; for (const auto& t : tempoMap) { if (currentBeats >= t.beatPosition - 0.001) activeBpm = t.bpm; }
            double beats = 4.0 / static_cast<double>(noteLength); if (isDotted) beats *= 1.5; double duration = beats * (60.0 / activeBpm);
            int64_t startS = getSamplePositionFromTime(currentTime, sampleRateHz); currentTime += duration; int64_t endS = getSamplePositionFromTime(currentTime, sampleRateHz);
            double finalFreq = 0.0; if (!isRest) { finalFreq = baseFreq * std::pow(2.0, currentOctave - 4) * std::pow(1.059463094359, semitoneShift); }

            if (isRest) { result.push_back({ 0.0, startS, endS, false, currentVolume, currentBeats, currentBeats + beats, tokenStart, tokenEnd }); pendingTie = false; }
            else {
                if (pendingTie && !result.empty() && std::abs(result.back().frequency - finalFreq) < 1.0) { result.back().endSample = endS; result.back().endBeat = currentBeats + beats; result.back().textEnd = tokenEnd; }
                else { if (pendingTie && !result.empty()) result.back().isTie = true; result.push_back({ finalFreq, startS, endS, false, currentVolume, currentBeats, currentBeats + beats, tokenStart, tokenEnd }); }
                pendingTie = false;
            }
            currentBeats += beats;
        }
    }
    return result;
}

int64_t MmlLogic::getSamplePositionFromTime(double timeInSeconds, double sampleRateHz) {
    return static_cast<int64_t>(timeInSeconds * sampleRateHz);
}

double MmlLogic::getBeatFromSample(int64_t sample, const std::vector<TempoChange>& tempoMap, double sampleRateHz) {
    if (tempoMap.empty()) return 0.0;
    size_t idx = 0;
    for (size_t i = 0; i < tempoMap.size(); ++i) {
        if (sample >= tempoMap[i].samplePosition) idx = i;
        else break;
    }
    const auto& active = tempoMap[idx];
    int64_t elapsedSamples = sample - active.samplePosition;
    double elapsedSeconds = static_cast<double>(elapsedSamples) / (sampleRateHz > 0 ? sampleRateHz : 44100.0);
    double elapsedBeats = elapsedSeconds * (active.bpm / 60.0);
    return active.beatPosition + elapsedBeats;
}

int64_t MmlLogic::getSampleFromBeat(double beat, const std::vector<TempoChange>& tempoMap, double sampleRateHz) {
    if (tempoMap.empty()) return 0;
    size_t idx = 0;
    for (size_t i = 0; i < tempoMap.size(); ++i) {
        if (beat >= tempoMap[i].beatPosition) idx = i;
        else break;
    }
    const auto& active = tempoMap[idx];
    double elapsedBeats = beat - active.beatPosition;
    double elapsedSeconds = elapsedBeats * (60.0 / active.bpm);
    return active.samplePosition + static_cast<int64_t>(elapsedSeconds * (sampleRateHz > 0 ? sampleRateHz : 44100.0));
}