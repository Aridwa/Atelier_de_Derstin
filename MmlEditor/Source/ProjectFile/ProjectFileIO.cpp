#include "ProjectFileIO.h"
#include <algorithm>
#include <cmath>
#include <map>


juce::String ProjectFileIO::dmmfEscapeLine(juce::String value) {
    value = value.replace("\\", "\\\\"); value = value.replace("\r", ""); value = value.replace("\n", "\\n"); return value;
}
juce::String ProjectFileIO::dmmfUnescapeLine(const juce::String& value) {
    juce::String out;
    for (int i = 0; i < value.length(); ++i) {
        const auto ch = value[i];
        if (ch == '\\' && i + 1 < value.length()) { const auto next = value[i + 1]; if (next == 'n') { out << "\n"; ++i; continue; } if (next == '\\') { out << "\\"; ++i; continue; } }
        out << juce::String::charToString(ch);
    } return out;
}
bool ProjectFileIO::dmmfReadBool(const juce::String& value) {
    return value.trim().equalsIgnoreCase("true") || value.trim() == "1" || value.trim().equalsIgnoreCase("yes");
}

// =========================================================================
// ★ 여기서부터 선생님의 피땀 눈물이 담긴 번역 코드 원본입니다! ★
// =========================================================================

juce::String ProjectFileIO::makeDefaultPresetKoText() {
    juce::String text;
    text << juce::String(L"# SF2 preset Korean translation\n");
    text << juce::String(L"# Put this file in Language/mabinogi_preset_ko.txt\n");
    text << juce::String(L"# Format A: MSXspirit01:0=류트   ; exact file/index match, highest priority\n");
    text << juce::String(L"# Format B: Lute=류트            ; raw preset name fallback\n");
    text << juce::String(L"# Change values and press SF2 Reload or switch language to refresh.\n\n");

    text << juce::String(L"# Exact MSXspirit01 presets\n");
    text << juce::String(L"MSXspirit01:0=류트\nMSXspirit01:1=우쿨렐레\nMSXspirit01:2=만돌린\nMSXspirit01:3=휘슬\nMSXspirit01:4=론카도라\nMSXspirit01:5=플루트\nMSXspirit01:6=샬루모\nMSXspirit01:7='도'음 빈 병\nMSXspirit01:8='레'음 빈 병\nMSXspirit01:9='미'음 빈 병\nMSXspirit01:10='파'음 빈 병\nMSXspirit01:11='솔'음 빈 병\nMSXspirit01:12='시'음 빈 병\nMSXspirit01:13='라'음 빈 병\nMSXspirit01:14=방울새\nMSXspirit01:15=팔색조\nMSXspirit01:16=물총새\nMSXspirit01:17=방울새 짧은음\nMSXspirit01:18=튜바\nMSXspirit01:19=리라\nMSXspirit01:20=일렉기타\nMSXspirit01:21=피아노\nMSXspirit01:22=바이올린 \nMSXspirit01:23=첼로\nMSXspirit01:24=튜닝 바이올린\nMSXspirit01:25=튜닝 첼로\nMSXspirit01:26=스네어\nMSXspirit01:27=큰 북\nMSXspirit01:28=작은 북\nMSXspirit01:29=심벌즈즈\nMSXspirit01:30='도'음 핸드벨\nMSXspirit01:31='레'음 핸드벨\nMSXspirit01:32='미'음 핸드벨\nMSXspirit01:33='파'음 핸드벨\nMSXspirit01:34='솔'음 핸드벨\nMSXspirit01:35='라'음 핸드벨\nMSXspirit01:36='시'음 핸드벨\nMSXspirit01:37=높은 '도' 음 핸드벨\nMSXspirit01:38=실로폰\n\n");

    text << juce::String(L"# Exact MSXspirit02 vocal presets\n");
    text << juce::String(L"MSXspirit02:0=마이크(남) 1\nMSXspirit02:1=마이크(남) 2\nMSXspirit02:2=마이크(남) 3\nMSXspirit02:3=마이크(남) 4\nMSXspirit02:4=마이크(남) 5\nMSXspirit02:5=마이크(여) 6\nMSXspirit02:6=마이크(여) 7\nMSXspirit02:7=마이크(여) 8\nMSXspirit02:8=마이크(여) 9\nMSXspirit02:9=마이크(여) 10\nMSXspirit02:10=환상의 코러스(남)\nMSXspirit02:11=환상의 코러스(여)\nMSXspirit02:12=밀키웨이 마이크(남)\nMSXspirit02:13=밀키웨이 마이크(여)\n\n");

    text << juce::String(L"# Exact MSXspirit03 presets\n");
    text << juce::String(L"MSXspirit03:0=축제 류트\nMSXspirit03:1=축제 우쿨렐레\nMSXspirit03:2=축제 만돌린\nMSXspirit03:3=축제 휘슬\nMSXspirit03:4=축제 플루트\n\n");

    text << juce::String(L"# Exact MSXspirit04 presets\n");
    text << juce::String(L"MSXspirit04:0=하프\nMSXspirit04:1=드럼\nMSXspirit04:2=가야금\nMSXspirit04:3=튜닝 플루트\nMSXspirit04:4=튜닝 휘슬\n\n");

    text << juce::String(L"# Exact MSXspirit05 presets\n");
    text << juce::String(L"MSXspirit05:0=베이스\n\n");

    text << juce::String(L"# Raw preset name fallback translations\n");
    text << juce::String(L"Lute=류트\nUkulele=우쿨렐레\nMandolin=만돌린\nRecorder=리코더\nPipe=파이프\nFlute=플루트\nOboe=오보에\nWhistle=휘슬\nBottle_C=병 C\nBottle_D=병 D\nBottle_E=병 E\nBottle_F=병 F\nBottle_G=병 G\nBottle_A=병 A\nBottle_B=병 B\nGreenFinch=방울새\nFairyPitta=팔색조\nKingFisher=물총새\nGreenFinch_Short=방울새 짧은음\nTuba=튜바\nLyra=리라\nelectricguitar=일렉기타\nPiano=피아노\nViollin_vt=바이올린 비브라토\nViollin_long=바이올린 롱톤\nCello=첼로\nCello_Marcato=첼로 마르카토\nBass=베이스\nsnare_c3=스네어 C3\nsnare_1=스네어 1\nbassdrum=베이스드럼\ncymbal=심벌\nChime=차임\nHandbell_C3=핸드벨 C3\nHandbell_D3=핸드벨 D3\nHandbell_E3=핸드벨 E3\nHandbell_F3=핸드벨 F3\nHandbell_G3=핸드벨 G3\nHandbell_A3=핸드벨 A3\nHandbell_B3=핸드벨 B3\nHandbell_C4=핸드벨 C4\nsong=노래\nHarp=하프\nDrum=드럼\nGayageum=가야금\nFlute_Nonvib=플루트 논비브라토\nWhistle_Nonvib=휘슬 논비브라토\n");
    return text;
}

juce::String ProjectFileIO::makeDefaultPresetEnText() {
    juce::String text;
    text << "# SF2 preset English display names\n";
    text << "# Put this file in Language/mabinogi_preset_en.txt\n";
    text << "# Format A: MSXspirit01:0=Lute       ; exact file/index match, highest priority\n";
    text << "# Format B: Viollin_vt=Violin Vibrato ; raw preset name fallback\n";
    text << "# Change values and press SF2 Reload or switch language to refresh.\n\n";

    text << "# Exact MSXspirit01 presets\n";
    text << "MSXspirit01:0=Lute\nMSXspirit01:1=Ukulele\nMSXspirit01:2=Mandolin\nMSXspirit01:3=Whistle\nMSXspirit01:4=Roncadora\nMSXspirit01:5=Flute\nMSXspirit01:6=Chalumeau\nMSXspirit01:7=Bottle C\nMSXspirit01:8=Bottle D\nMSXspirit01:9=Bottle E\nMSXspirit01:10=Bottle F\nMSXspirit01:11=Bottle G\nMSXspirit01:12=Bottle B\nMSXspirit01:13=Bottle A\nMSXspirit01:14=Greenfinch\nMSXspirit01:15=Fairy Pitta\nMSXspirit01:16=Kingfisher\nMSXspirit01:17=Greenfinch Short\nMSXspirit01:18=Tuba\nMSXspirit01:19=Lyre\nMSXspirit01:20=Electric Guitar\nMSXspirit01:21=Piano\nMSXspirit01:22=Violin Vibrato\nMSXspirit01:23=Cello\nMSXspirit01:24=Violin Long Tone\nMSXspirit01:25=Cello Marcato\nMSXspirit01:26=Snare C3\nMSXspirit01:27=Bass Drum\nMSXspirit01:28=Snare 1\nMSXspirit01:29=Cymbal\nMSXspirit01:30=Handbell C3\nMSXspirit01:31=Handbell D3\nMSXspirit01:32=Handbell E3\nMSXspirit01:33=Handbell F3\nMSXspirit01:34=Handbell G3\nMSXspirit01:35=Handbell A3\nMSXspirit01:36=Handbell B3\nMSXspirit01:37=Handbell C4\nMSXspirit01:38=Chime\n\n";

    text << "# Exact MSXspirit02 vocal presets\n";
    for (int i = 0; i <= 13; ++i) text << "MSXspirit02:" << juce::String(i) << "=Voice " << juce::String(i + 1) << "\n";
    text << "\n";

    text << "# Exact MSXspirit03 presets\n";
    text << "MSXspirit03:0=Lute\nMSXspirit03:1=Ukulele\nMSXspirit03:2=Mandolin\nMSXspirit03:3=Whistle\nMSXspirit03:4=Flute\n\n";

    text << "# Exact MSXspirit04 presets\n";
    text << "MSXspirit04:0=Harp\nMSXspirit04:1=Drum\nMSXspirit04:2=Gayageum\nMSXspirit04:3=Flute Non-vibrato\nMSXspirit04:4=Whistle Non-vibrato\n\n";

    text << "# Exact MSXspirit05 presets\n";
    text << "MSXspirit05:0=Bass\n\n";

    text << "# Raw preset name fallback translations\n";
    text << "Lute=Lute\nUkulele=Ukulele\nMandolin=Mandolin\nRecorder=Recorder\nPipe=Pipe\nFlute=Flute\nOboe=Oboe\nWhistle=Whistle\nBottle_C=Bottle C\nBottle_D=Bottle D\nBottle_E=Bottle E\nBottle_F=Bottle F\nBottle_G=Bottle G\nBottle_A=Bottle A\nBottle_B=Bottle B\nGreenFinch=Greenfinch\nFairyPitta=Fairy Pitta\nKingFisher=Kingfisher\nGreenFinch_Short=Greenfinch Short\nTuba=Tuba\nLyra=Lyre\nelectricguitar=Electric Guitar\nPiano=Piano\nViollin_vt=Violin Vibrato\nViolin_vt=Violin Vibrato\nViollin_long=Violin Long Tone\nViolin_long=Violin Long Tone\nCello=Cello\nCello_Marcato=Cello Marcato\nBass=Bass\nsnare_c3=Snare C3\nsnare_1=Snare 1\nbassdrum=Bass Drum\ncymbal=Cymbal\nChime=Chime\nHandbell_C3=Handbell C3\nHandbell_D3=Handbell D3\nHandbell_E3=Handbell E3\nHandbell_F3=Handbell F3\nHandbell_G3=Handbell G3\nHandbell_A3=Handbell A3\nHandbell_B3=Handbell B3\nHandbell_C4=Handbell C4\nsong=Voice\nHarp=Harp\nDrum=Drum\nGayageum=Gayageum\nFlute_Nonvib=Flute Non-vibrato\nWhistle_Nonvib=Whistle Non-vibrato\n";
    return text;
}


juce::String ProjectFileIO::makeDefaultPresetJaText() {
    juce::String text;
    text << juce::String(L"# SF2 preset Japanese display names\n");
    text << juce::String(L"# Put this file in Language/mabinogi_preset_ja.txt\n");
    text << juce::String(L"# Format A: MSXspirit01:0=リュート   ; exact file/index match, highest priority\n");
    text << juce::String(L"# Format B: Lute=リュート            ; raw preset name fallback\n");
    text << juce::String(L"# Change values and press SF2 Reload or switch language to refresh.\n\n");

    text << juce::String(L"# Exact MSXspirit01 presets\n");
    text << juce::String(L"MSXspirit01:0=リュート\nMSXspirit01:1=ウクレレ\nMSXspirit01:2=マンドリン\nMSXspirit01:3=ホイッスル\nMSXspirit01:4=ロンカドーラ\nMSXspirit01:5=フルート\nMSXspirit01:6=シャリュモー\nMSXspirit01:7=空き瓶（ド）\nMSXspirit01:8=空き瓶（レ）\nMSXspirit01:9=空き瓶（ミ）\nMSXspirit01:10=空き瓶（ファ）\nMSXspirit01:11=空き瓶（ソ）\nMSXspirit01:12=空き瓶（シ）\nMSXspirit01:13=空き瓶（ラ）\nMSXspirit01:14=カワラヒワ\nMSXspirit01:15=ヤイロチョウ\nMSXspirit01:16=カワセミ\nMSXspirit01:17=カワラヒワ短音\nMSXspirit01:18=チューバ\nMSXspirit01:19=リラ\nMSXspirit01:20=エレキギター\nMSXspirit01:21=ピアノ\nMSXspirit01:22=バイオリン・ビブラート\nMSXspirit01:23=チェロ\nMSXspirit01:24=バイオリン・ロングトーン\nMSXspirit01:25=チェロ・マルカート\nMSXspirit01:26=スネア C3\nMSXspirit01:27=バスドラム\nMSXspirit01:28=スネア 1\nMSXspirit01:29=シンバル\nMSXspirit01:30=ハンドベル C3\nMSXspirit01:31=ハンドベル D3\nMSXspirit01:32=ハンドベル E3\nMSXspirit01:33=ハンドベル F3\nMSXspirit01:34=ハンドベル G3\nMSXspirit01:35=ハンドベル A3\nMSXspirit01:36=ハンドベル B3\nMSXspirit01:37=ハンドベル C4\nMSXspirit01:38=シロフォン\n\n");

    text << juce::String(L"# Exact MSXspirit02 vocal presets\n");
    text << juce::String(L"MSXspirit02:0=マイク(男) 1\nMSXspirit02:1=マイク(男) 2\nMSXspirit02:2=マイク(男) 3\nMSXspirit02:3=マイク(男) 4\nMSXspirit02:4=マイク(男) 5\nMSXspirit02:5=マイク(女) 6\nMSXspirit02:6=マイク(女) 7\nMSXspirit02:7=マイク(女) 8\nMSXspirit02:8=マイク(女) 9\nMSXspirit02:9=マイク(女) 10\nMSXspirit02:10=幻想のコーラス(男)\nMSXspirit02:11=幻想のコーラス(女)\nMSXspirit02:12=ミルキーウェイマイク(男)\nMSXspirit02:13=ミルキーウェイマイク(女)\n\n");

    text << juce::String(L"# Exact MSXspirit03 presets\n");
    text << juce::String(L"MSXspirit03:0=祭りのリュート\nMSXspirit03:1=祭りのウクレレ\nMSXspirit03:2=祭りのマンドリン\nMSXspirit03:3=祭りのホイッスル\nMSXspirit03:4=祭りのフルート\n\n");

    text << juce::String(L"# Exact MSXspirit04 presets\n");
    text << juce::String(L"MSXspirit04:0=ハープ\nMSXspirit04:1=ドラム\nMSXspirit04:2=カヤグム\nMSXspirit04:3=チューニングフルート\nMSXspirit04:4=チューニングホイッスル\n\n");

    text << juce::String(L"# Exact MSXspirit05 presets\n");
    text << juce::String(L"MSXspirit05:0=ベース\n\n");

    text << juce::String(L"# Raw preset name fallback translations\n");
    text << juce::String(L"Lute=リュート\nUkulele=ウクレレ\nMandolin=マンドリン\nRecorder=リコーダー\nPipe=パイプ\nFlute=フルート\nOboe=オーボエ\nWhistle=ホイッスル\nBottle_C=空き瓶（ド）\nBottle_D=空き瓶（レ）\nBottle_E=空き瓶（ミ）\nBottle_F=空き瓶（ファ）\nBottle_G=空き瓶（ソ）\nBottle_A=空き瓶（ラ）\nBottle_B=空き瓶（シ）\nGreenFinch=カワラヒワ\nFairyPitta=ヤイロチョウ\nKingFisher=カワセミ\nGreenFinch_Short=カワラヒワ短音\nTuba=チューバ\nLyra=リラ\nelectricguitar=エレキギター\nPiano=ピアノ\nViollin_vt=バイオリン・ビブラート\nViolin_vt=バイオリン・ビブラート\nViollin_long=バイオリン・ロングトーン\nViolin_long=バイオリン・ロングトーン\nCello=チェロ\nCello_Marcato=チェロ・マルカート\nBass=ベース\nsnare_c3=スネア C3\nsnare_1=スネア 1\nbassdrum=バスドラム\ncymbal=シンバル\nChime=シロフォン\nHandbell_C3=ハンドベル C3\nHandbell_D3=ハンドベル D3\nHandbell_E3=ハンドベル E3\nHandbell_F3=ハンドベル F3\nHandbell_G3=ハンドベル G3\nHandbell_A3=ハンドベル A3\nHandbell_B3=ハンドベル B3\nHandbell_C4=ハンドベル C4\nsong=歌\nHarp=ハープ\nDrum=ドラム\nGayageum=カヤグム\nFlute_Nonvib=チューニングフルート\nWhistle_Nonvib=チューニングホイッスル\n");
    return text;
}

juce::Array<juce::File> ProjectFileIO::getLanguageFoldersForPresetText() {
    juce::Array<juce::File> folders;
    const auto exeFolder = juce::File::getSpecialLocation(juce::File::currentExecutableFile).getParentDirectory().getChildFile("Language");
    const auto cwdFolder = juce::File::getCurrentWorkingDirectory().getChildFile("Language");
    const juce::File devMmlEditorFolder(R"(C:\Users\User\Documents\MabinogiMMLEditor\MmlEditor)");
    const juce::File devFolder = devMmlEditorFolder.getChildFile("Language");

    auto addUnique = [&folders](const juce::File& folder) {
        for (const auto& existing : folders) if (existing.getFullPathName().equalsIgnoreCase(folder.getFullPathName())) return;
        folders.add(folder);
    };
    addUnique(exeFolder); addUnique(cwdFolder); addUnique(devFolder);
    return folders;
}

bool ProjectFileIO::folderHasReferenceSf2(const juce::File& folder) {
    if (!folder.isDirectory()) return false;
    for (int i = 1; i <= 5; ++i) {
        const auto fileName = "MSXspirit" + juce::String(i).paddedLeft('0', 2) + ".sf2";
        if (folder.getChildFile(fileName).existsAsFile()) return true;
    } return false;
}

juce::File ProjectFileIO::getPresetTranslationFile(bool useKorean) {
    return getPresetTranslationFileForLanguage(useKorean ? 2 : 1);
}

juce::File ProjectFileIO::getPresetTranslationFileForLanguage(int languageId) {
    const auto folders = getLanguageFoldersForPresetText();

    juce::String fileName = "mabinogi_preset_en.txt";
    juce::String defaultText = makeDefaultPresetEnText();

    if (languageId == 2) {
        fileName = "mabinogi_preset_ko.txt";
        defaultText = makeDefaultPresetKoText();
    }
    else if (languageId == 3) {
        fileName = "mabinogi_preset_ja.txt";
        defaultText = makeDefaultPresetJaText();
    }

    for (const auto& folder : folders) {
        const auto file = folder.getChildFile(fileName);
        if (folderHasReferenceSf2(folder) && file.existsAsFile()) return file;
    }
    for (const auto& folder : folders) {
        const auto file = folder.getChildFile(fileName);
        if (file.existsAsFile()) return file;
    }

    for (const auto& folder : folders) {
        if (folderHasReferenceSf2(folder)) {
            const auto file = folder.getChildFile(fileName);
            file.replaceWithText(defaultText, false, false, "\n");
            return file;
        }
    }

    auto fallbackFolder = folders[0];
    fallbackFolder.createDirectory();
    const auto file = fallbackFolder.getChildFile(fileName);
    file.replaceWithText(defaultText, false, false, "\n");
    return file;
}

juce::String ProjectFileIO::getLocalizedSf2PresetName(const juce::String& rawName, const juce::String& sf2FileName, int presetIdx, int languageId) {
    const bool useKorean = (languageId == 2);
    const bool useJapanese = (languageId == 3);
    const bool useEnglish = (languageId == 1);
    const juce::String name = rawName.trim();

    if (name.isEmpty()) {
        if (useKorean) return juce::String(L"프리셋 ") + juce::String(presetIdx);
        if (useJapanese) return juce::String(L"プリセット ") + juce::String(presetIdx);
        return juce::String("Preset ") + juce::String(presetIdx);
    }
    if (!useKorean && !useEnglish && !useJapanese) return name;

    // Mabinogi Mobile Fury Sound Pack built-in names.
    // This keeps the mobile preset labels stable even when the original SF2
    // preset names are abbreviated, mixed-case, or localized differently.
    const juce::String lowerFileName = sf2FileName.toLowerCase();
    const bool isFuryMobilePack = lowerFileName.contains("fury sound pack")
                              || lowerFileName.contains("mabinogi mobile instrument set")
                              || lowerFileName.contains("mabinogimobile");
    if (isFuryMobilePack)
    {
        struct MobileName { int preset; const char* en; const wchar_t* ko; const wchar_t* ja; };
        static const MobileName mobileNames[] = {
            { 0,  "Lute",      L"류트",     L"リュート" },
            { 2,  "Mandollin", L"만돌린",   L"マンドリン" },
            { 5,  "Flute",     L"플루트",   L"フルート" },
            { 6,  "Chalumeau", L"샬루모",   L"シャリュモー" },
            { 21, "Piano",     L"피아노",   L"ピアノ" },
            { 22, "Violin",    L"바이올린", L"バイオリン" },
            { 24, "Harp",      L"하프",     L"ハープ" },
            { 29, "Music Box", L"오르골",   L"オルゴール" },
            { 66, "Bass Drum", L"큰 북",    L"バスドラム" },
            { 68, "Cymbals",   L"심벌즈",   L"シンバル" },
            { 77, "Xylophone", L"실로폰",   L"シロフォン" }
        };

        for (const auto& entry : mobileNames)
        {
            if (entry.preset == presetIdx) {
                if (useKorean) return juce::String(entry.ko);
                if (useJapanese) return juce::String(entry.ja);
                return juce::String(entry.en);
            }
        }
    }

        // Fury_Sound_Pack_v150 built-in names.
    // External files are in Language/mabinogi_preset_*.txt; this is a fallback.
    {
        const juce::String lowerFileName = sf2FileName.toLowerCase();
        const bool isFuryMobilePack = lowerFileName.contains("fury_sound_pack_v150")
                                  || lowerFileName.contains("fury_sound_pack")
                                  || lowerFileName.contains("fury sound pack")
                                  || lowerFileName.contains("mabinogi mobile instrument set")
                                  || lowerFileName.contains("mabinogimobile")
                                  || lowerFileName.contains("mabinogi_mobile")
                                  || lowerFileName.contains("mobile instrument");
        if (isFuryMobilePack)
        {
            struct FuryName { int preset; const char* en; const wchar_t* ko; const wchar_t* ja; };
            static const FuryName furyNames[] = {
                { 18,  "Piano",      L"피아노",   L"ピアノ" },
                { 20,  "Harp",       L"하프",     L"ハープ" },
                { 25,  "Music Box",  L"오르골",   L"オルゴール" },
                { 62,  "Bass Drum",  L"큰 북",    L"バスドラム" },
                { 63,  "Cymbals",    L"심벌즈",   L"シンバル" },
                { 122, "Mandolin",   L"만돌린",   L"マンドリン" },
                { 123, "Lute",       L"류트",     L"リュート" },
                { 124, "Chalumeau",  L"샬루모",   L"シャリュモー" },
                { 125, "Flute",      L"플루트",   L"フルート" },
                { 126, "Xylophone",  L"실로폰",   L"シロフォン" },
                { 127, "Violin",     L"바이올린", L"バイオリン" }
            };

            for (const auto& e : furyNames)
            {
                if (e.preset == presetIdx)
                {
                    if (useKorean) return juce::String(e.ko);
                    if (useJapanese) return juce::String(e.ja);
                    return juce::String(e.en);
                }
            }
        }
    }

const auto translationFile = getPresetTranslationFileForLanguage(languageId);
    if (!translationFile.existsAsFile()) return name;

    const juce::String exactKey = sf2FileName.trim() + ":" + juce::String(presetIdx);
    const juce::String exactKeySf2 = sf2FileName.trim() + ".sf2:" + juce::String(presetIdx);
    const juce::String exactKeyDls = sf2FileName.trim() + ".dls:" + juce::String(presetIdx);

    juce::String exactResult; juce::String rawResult;
    juce::StringArray lines; lines.addLines(translationFile.loadFileAsString());

    for (auto line : lines) {
        line = line.trim();
        if (line.isNotEmpty() && line[0] == 0xfeff) line = line.substring(1).trim();
        if (line.isEmpty() || line.startsWithChar('#') || line.startsWithChar(';')) continue;
        const int equalsPos = line.indexOfChar('='); if (equalsPos <= 0) continue;

        const juce::String key = line.substring(0, equalsPos).trim();
        const juce::String value = line.substring(equalsPos + 1).trim();
        if (value.isEmpty()) continue;

        if (key.equalsIgnoreCase(exactKey) || key.equalsIgnoreCase(exactKeySf2) || key.equalsIgnoreCase(exactKeyDls)) exactResult = value;
        else if (key.equalsIgnoreCase(name)) rawResult = value;
    }
    if (exactResult.isNotEmpty()) return exactResult;
    if (rawResult.isNotEmpty()) return rawResult;

    // If the user already has an older/empty mabinogi_preset_ja.txt, still provide
    // the built-in Japanese Mabinogi preset names as a safe fallback.
    if (useJapanese)
    {
        exactResult.clear();
        rawResult.clear();
        lines.clear();
        lines.addLines(makeDefaultPresetJaText());

        for (auto line : lines) {
            line = line.trim();
            if (line.isNotEmpty() && line[0] == 0xfeff) line = line.substring(1).trim();
            if (line.isEmpty() || line.startsWithChar('#') || line.startsWithChar(';')) continue;
            const int equalsPos = line.indexOfChar('='); if (equalsPos <= 0) continue;

            const juce::String key = line.substring(0, equalsPos).trim();
            const juce::String value = line.substring(equalsPos + 1).trim();
            if (value.isEmpty()) continue;

            if (key.equalsIgnoreCase(exactKey) || key.equalsIgnoreCase(exactKeySf2) || key.equalsIgnoreCase(exactKeyDls)) exactResult = value;
            else if (key.equalsIgnoreCase(name)) rawResult = value;
        }

        if (exactResult.isNotEmpty()) return exactResult;
        if (rawResult.isNotEmpty()) return rawResult;
    }

    return name;
}

namespace
{
    void resetDmmfBank(InstrumentBank& bank)
    {
        bank.instrumentWave = 5;
        bank.helperMode = 1;
        bank.autoBassScale = 1;
        bank.sf2FileIndex = 0;
        bank.dlsPreset = 0;
        bank.songPresetMode = false;
        bank.mmiSongPartWithProgram = false;
        bank.pcPresetExcludeSongPartLimit = false;

        for (auto& track : bank.tracks)
        {
            track.mml.clear();
            track.mute = false;
            track.solo = false;
            track.sequence.clear();
            track.noteIndex = 0;
            track.currentAngle = 0.0;
        }
    }

    juce::String getFallbackTrackName(int trackIndex)
    {
        return juce::String("Track ") + juce::String(trackIndex + 1);
    }
}

bool ProjectFileIO::saveDmmfProject(const juce::File& file,
                                    const InstrumentBank* banks,
                                    int numActiveTracks,
                                    const DmmfSaveOptions& options)
{
    if (file == juce::File() || banks == nullptr || numActiveTracks <= 0)
        return false;

    juce::String title = options.title.trim();
    if (title.isEmpty())
        title = "Atelier de Derstin";

    juce::String text;
    text << "# Derstin Mabinogi Music File\n"
            "# Extension=.dmmf\n"
            "FormatVersion=1\n\n"
            "[Meta]\n"
         << "Title=" << dmmfEscapeLine(title) << "\n"
         << "CreatedBy=Atelier de Derstin\n"
         << "NumTracks=" << numActiveTracks << "\n"
         << "CurrentTrack=" << (options.currentTrackIndex + 1) << "\n"
         << "TimeSignatureId=" << options.timeSignatureId << "\n"
         << "ThemeId=" << options.themeId << "\n"
         << "LanguageId=" << options.languageId << "\n\n";

    text << "[InstrumentList]\n";
    for (int i = 0; i < numActiveTracks; ++i)
    {
        const auto& bank = banks[i];

        juce::String presetName;
        if (options.getPresetName)
            presetName = options.getPresetName(bank.sf2FileIndex, bank.dlsPreset);

        if (presetName.isEmpty())
            presetName = options.getTrackName ? options.getTrackName(i) : getFallbackTrackName(i);

        text << dmmfEscapeLine(presetName) << juce::String(i + 1) << "\n";
    }

    text << "\n";

    for (int i = 0; i < numActiveTracks; ++i)
    {
        const auto& bank = banks[i];

        juce::String sf2Name;
        if (options.getSf2Name)
            sf2Name = options.getSf2Name(bank.sf2FileIndex);

        juce::String presetName;
        if (options.getPresetName)
            presetName = options.getPresetName(bank.sf2FileIndex, bank.dlsPreset);

        const juce::String trackName = options.getTrackName ? options.getTrackName(i) : getFallbackTrackName(i);

        text << "[Track " << (i + 1) << "]\n"
             << "Name=" << dmmfEscapeLine(trackName) << "\n"
             << "InstrumentWave=" << bank.instrumentWave << "\n"
             << "HelperMode=" << bank.helperMode << "\n"
             << "AutoBassScale=" << bank.autoBassScale << "\n"
             << "Sf2FileIndex=" << bank.sf2FileIndex << "\n"
             << "Sf2FileName=" << dmmfEscapeLine(sf2Name) << "\n"
             << "PresetIndex=" << bank.dlsPreset << "\n"
             << "PresetName=" << dmmfEscapeLine(presetName) << "\n"
             << "PartMode=" << (bank.songPresetMode ? "Song" : "Normal") << "\n"
             << "MmiSongPartWithProgram=" << (bank.mmiSongPartWithProgram ? 1 : 0) << "\n"
             << "PcPresetExcludeSongPartLimit=" << (bank.pcPresetExcludeSongPartLimit ? 1 : 0) << "\n"
             << "MuteMelody=" << (bank.tracks[0].mute ? 1 : 0) << "\n"
             << "SoloMelody=" << (bank.tracks[0].solo ? 1 : 0) << "\n"
             << "MuteChord1=" << (bank.tracks[1].mute ? 1 : 0) << "\n"
             << "SoloChord1=" << (bank.tracks[1].solo ? 1 : 0) << "\n"
             << "MuteChord2=" << (bank.tracks[2].mute ? 1 : 0) << "\n"
             << "SoloChord2=" << (bank.tracks[2].solo ? 1 : 0) << "\n"
             << "MuteSong=" << (bank.tracks[3].mute ? 1 : 0) << "\n"
             << "SoloSong=" << (bank.tracks[3].solo ? 1 : 0) << "\n"
             << "Melody=" << dmmfEscapeLine(bank.tracks[0].mml) << "\n"
             << "Chord1=" << dmmfEscapeLine(bank.tracks[1].mml) << "\n"
             << "Chord2=" << dmmfEscapeLine(bank.tracks[2].mml) << "\n"
             << "Song=" << dmmfEscapeLine(bank.tracks[3].mml) << "\n\n";
    }

    text << "[End]\n";
    return file.replaceWithText(text);
}

ProjectFileIO::DmmfLoadResult ProjectFileIO::loadDmmfProject(const juce::File& file,
                                                             InstrumentBank* banks,
                                                             int maxBanks,
                                                             int maxSf2Index,
                                                             std::function<int(const juce::String& sf2FileName)> findSf2IndexByName)
{
    DmmfLoadResult result;

    if (file == juce::File() || banks == nullptr || maxBanks <= 0)
    {
        result.errorMessage = "Invalid DMMF load target.";
        return result;
    }

    juce::StringArray lines;
    file.readLines(lines);

    if (lines.size() == 0)
    {
        result.errorMessage = "The selected file is empty.";
        return result;
    }

    for (int i = 0; i < maxBanks; ++i)
        resetDmmfBank(banks[i]);

    int loadedTrackCount = 0;
    int requestedTrackCount = 0;
    int requestedCurrentTrack = 0;
    int currentTrackToFill = -1;
    bool sawDmmfSignature = false;

    const int safeMaxSf2Index = std::max(0, maxSf2Index);

    for (auto rawLine : lines)
    {
        juce::String line = rawLine.trim();
        if (line.isEmpty())
            continue;

        if (line.startsWith("#"))
        {
            if (line.containsIgnoreCase("Derstin Mabinogi Music File"))
                sawDmmfSignature = true;
            continue;
        }

        if (line.equalsIgnoreCase("[End]"))
            break;

        if (line.startsWithChar('[') && line.endsWithChar(']'))
        {
            const juce::String section = line.substring(1, line.length() - 1).trim();
            if (section.startsWithIgnoreCase("Track"))
            {
                int parsedIndex = section.fromFirstOccurrenceOf("Track", false, true).trim().getIntValue();
                if (parsedIndex <= 0)
                    parsedIndex = loadedTrackCount + 1;

                currentTrackToFill = juce::jlimit(0, maxBanks - 1, parsedIndex - 1);
                loadedTrackCount = std::max(loadedTrackCount, currentTrackToFill + 1);
            }
            else
            {
                currentTrackToFill = -1;
            }
            continue;
        }

        const int equals = line.indexOfChar('=');
        if (equals <= 0)
            continue;

        const juce::String key = line.substring(0, equals).trim();
        const juce::String value = dmmfUnescapeLine(line.substring(equals + 1).trim());

        if (currentTrackToFill < 0)
        {
            if (key.equalsIgnoreCase("FormatVersion"))
                sawDmmfSignature = true;
            else if (key.equalsIgnoreCase("NumTracks"))
                requestedTrackCount = value.getIntValue();
            else if (key.equalsIgnoreCase("CurrentTrack"))
                requestedCurrentTrack = value.getIntValue() - 1;
            else if (key.equalsIgnoreCase("TimeSignatureId"))
                result.timeSignatureId = juce::jlimit(1, 110, value.getIntValue());
            else if (key.equalsIgnoreCase("ThemeId"))
                result.themeId = juce::jlimit(1, 12, value.getIntValue());
            else if (key.equalsIgnoreCase("LanguageId"))
            {
                const int lang = value.getIntValue();
                if (lang == 1 || lang == 2)
                    result.languageId = lang;
            }

            continue;
        }

        auto& bank = banks[currentTrackToFill];

        if (key.equalsIgnoreCase("InstrumentWave"))
            bank.instrumentWave = juce::jlimit(1, 6, value.getIntValue());
        else if (key.equalsIgnoreCase("HelperMode"))
            bank.helperMode = juce::jlimit(1, 9, value.getIntValue());
        else if (key.equalsIgnoreCase("AutoBassScale"))
            bank.autoBassScale = juce::jlimit(1, 31, value.getIntValue());
        else if (key.equalsIgnoreCase("Sf2FileIndex"))
            bank.sf2FileIndex = juce::jlimit(0, safeMaxSf2Index, value.getIntValue());
        else if (key.equalsIgnoreCase("Sf2FileName"))
        {
            if (findSf2IndexByName)
            {
                const int foundIndex = findSf2IndexByName(value);
                if (foundIndex >= 0)
                    bank.sf2FileIndex = juce::jlimit(0, safeMaxSf2Index, foundIndex);
            }
        }
        else if (key.equalsIgnoreCase("PresetIndex"))
            bank.dlsPreset = std::max(0, value.getIntValue());
        else if (key.equalsIgnoreCase("PartMode"))
            bank.songPresetMode = value.equalsIgnoreCase("Song");
        else if (key.equalsIgnoreCase("MmiSongPartWithProgram"))
            bank.mmiSongPartWithProgram = dmmfReadBool(value);
        else if (key.equalsIgnoreCase("PcPresetExcludeSongPartLimit"))
            bank.pcPresetExcludeSongPartLimit = dmmfReadBool(value);
        else if (key.equalsIgnoreCase("MuteMelody"))
            bank.tracks[0].mute = dmmfReadBool(value);
        else if (key.equalsIgnoreCase("SoloMelody"))
            bank.tracks[0].solo = dmmfReadBool(value);
        else if (key.equalsIgnoreCase("MuteChord1"))
            bank.tracks[1].mute = dmmfReadBool(value);
        else if (key.equalsIgnoreCase("SoloChord1"))
            bank.tracks[1].solo = dmmfReadBool(value);
        else if (key.equalsIgnoreCase("MuteChord2"))
            bank.tracks[2].mute = dmmfReadBool(value);
        else if (key.equalsIgnoreCase("SoloChord2"))
            bank.tracks[2].solo = dmmfReadBool(value);
        else if (key.equalsIgnoreCase("MuteSong"))
            bank.tracks[3].mute = dmmfReadBool(value);
        else if (key.equalsIgnoreCase("SoloSong"))
            bank.tracks[3].solo = dmmfReadBool(value);
        else if (key.equalsIgnoreCase("Melody"))
            bank.tracks[0].mml = value;
        else if (key.equalsIgnoreCase("Chord1"))
            bank.tracks[1].mml = value;
        else if (key.equalsIgnoreCase("Chord2"))
            bank.tracks[2].mml = value;
        else if (key.equalsIgnoreCase("Song"))
            bank.tracks[3].mml = value;
    }

    if (!sawDmmfSignature || loadedTrackCount <= 0)
    {
        result.errorMessage = "This does not look like a valid .dmmf project file.";
        return result;
    }

    result.ok = true;
    result.loadedTrackCount = loadedTrackCount;
    result.numActiveTracks = juce::jlimit(1, maxBanks, requestedTrackCount > 0 ? requestedTrackCount : loadedTrackCount);
    result.currentTrackIndex = juce::jlimit(0, result.numActiveTracks - 1, requestedCurrentTrack >= 0 ? requestedCurrentTrack : 0);

    return result;
}


namespace
{
    juce::String cleanMmiMmlPart(juce::String value)
    {
        value = value.trim();
        value = value.replace("\r", "").replace("\n", "").replace("\t", "").replace(" ", "");
        if (value.startsWithIgnoreCase("MML@"))
            value = value.substring(4);

        while (value.endsWithChar(';'))
            value = value.dropLastCharacters(1).trim();

        return value;
    }

    juce::String safeMmiLineValue(juce::String value)
    {
        return value.replace("\r", " ").replace("\n", " ").trim();
    }

    int extractFirstTempoFromBanks(const InstrumentBank* banks, int numActiveTracks, int fallbackTempo)
    {
        if (banks == nullptr)
            return fallbackTempo;

        for (int bankIdx = 0; bankIdx < numActiveTracks; ++bankIdx)
        {
            for (int partIdx = 0; partIdx < 4; ++partIdx)
            {
                const auto src = banks[bankIdx].tracks[partIdx].mml;
                for (int i = 0; i < src.length(); ++i)
                {
                    const auto c = juce::CharacterFunctions::toLowerCase(src[i]);
                    if (c != 't')
                        continue;

                    juce::String num;
                    int j = i + 1;
                    while (j < src.length() && juce::CharacterFunctions::isDigit(src[j]))
                    {
                        num += src[j];
                        ++j;
                    }

                    const int tempo = num.getIntValue();
                    if (tempo > 0 && tempo <= 1000)
                        return tempo;
                }
            }
        }

        return fallbackTempo;
    }

    int presetIndexToMabiiccoProgram(juce::String sf2Stem, int presetIndex)
    {
        sf2Stem = sf2Stem.upToFirstOccurrenceOf(".", false, false).trim().toLowerCase();
        presetIndex = juce::jmax(0, presetIndex);

        if (sf2Stem == "msxspirit01")
        {
            if (presetIndex <= 23) return presetIndex;
            if (presetIndex == 24) return 25;
            if (presetIndex == 25) return 26;
            if (presetIndex >= 26 && presetIndex <= 38) return 65 + (presetIndex - 26);
            return presetIndex;
        }

        if (sf2Stem == "msxspirit02")
        {
            static const int songPrograms[] = { 80, 81, 82, 83, 84, 90, 91, 92, 93, 94, 100, 110, 120, 121 };
            if (presetIndex >= 0 && presetIndex < 14)
                return songPrograms[presetIndex];

            return 80;
        }

        if (sf2Stem == "msxspirit03")
        {
            if (presetIndex >= 0 && presetIndex <= 4)
                return 50 + presetIndex;

            return 50;
        }

        if (sf2Stem == "msxspirit04")
        {
            static const int msx04Programs[] = { 24, 27, 28, 55, 56 };
            if (presetIndex >= 0 && presetIndex < 5)
                return msx04Programs[presetIndex];

            return 24;
        }

        if (sf2Stem == "msxspirit05")
            return 29;

        return presetIndex;
    }

    struct MabiiccoPresetTarget
    {
        bool valid = false;
        juce::String stem;
        int presetIndex = 0;
    };

    MabiiccoPresetTarget knownMabiiccoProgramTarget(int program)
    {
        if (program >= 80 && program <= 84) return { true, "MSXspirit02", program - 80 };
        if (program >= 90 && program <= 94) return { true, "MSXspirit02", 5 + (program - 90) };
        if (program == 100) return { true, "MSXspirit02", 10 };
        if (program == 110) return { true, "MSXspirit02", 11 };
        if (program == 120) return { true, "MSXspirit02", 12 };
        if (program == 121) return { true, "MSXspirit02", 13 };

        if (program >= 50 && program <= 54) return { true, "MSXspirit03", program - 50 };

        if (program == 24) return { true, "MSXspirit04", 0 };
        if (program == 27) return { true, "MSXspirit04", 1 };
        if (program == 28) return { true, "MSXspirit04", 2 };
        if (program == 55) return { true, "MSXspirit04", 3 };
        if (program == 56) return { true, "MSXspirit04", 4 };

        if (program == 29) return { true, "MSXspirit05", 0 };

        if (program >= 65 && program <= 77) return { true, "MSXspirit01", 26 + (program - 65) };
        if (program == 25) return { true, "MSXspirit01", 24 };
        if (program == 26) return { true, "MSXspirit01", 25 };
        if (program >= 0 && program <= 23) return { true, "MSXspirit01", program };

        return {};
    }

    juce::StringArray splitMmiTrackParts(const juce::String& mmlData)
    {
        juce::StringArray parts;
        int lastComma = 0;

        for (int c = 0; c < mmlData.length(); ++c)
        {
            if (mmlData[c] == ',')
            {
                parts.add(mmlData.substring(lastComma, c));
                lastComma = c + 1;
            }
        }

        parts.add(mmlData.substring(lastComma));
        return parts;
    }
}

bool ProjectFileIO::saveMmiFile(const juce::File& file,
                                const InstrumentBank* banks,
                                int numActiveTracks,
                                const MmiSaveOptions& options)
{
    if (file == juce::File() || banks == nullptr || numActiveTracks <= 0)
        return false;

    const int tempo = extractFirstTempoFromBanks(banks, numActiveTracks, options.fallbackTempo > 0 ? options.fallbackTempo : 120);
    const juce::String title = safeMmiLineValue(options.title.trim().isNotEmpty() ? options.title : juce::String("Atelier de Derstin"));

    juce::String text;
    text << "[mml-score]\n"
            "version=1\n"
         << "title=" << title << "\n"
         << "author=\n"
         << "time=" << ((options.timeSignatureId == 2) ? "3/4" : "4/4") << "\n"
         << "tempo=0T" << tempo << "\n";

    int writtenTracks = 0;

    for (int i = 0; i < numActiveTracks; ++i)
    {
        const auto& bank = banks[i];
        const auto melody = cleanMmiMmlPart(bank.tracks[0].mml);
        const auto chord1 = cleanMmiMmlPart(bank.tracks[1].mml);
        const auto chord2 = cleanMmiMmlPart(bank.tracks[2].mml);
        const auto song = cleanMmiMmlPart(bank.tracks[3].mml);

        if (melody.isEmpty() && chord1.isEmpty() && chord2.isEmpty() && song.isEmpty())
            continue;

        juce::String trackName = options.getTrackName ? options.getTrackName(i) : getFallbackTrackName(i);
        if (trackName.trim().isEmpty())
            trackName = getFallbackTrackName(i);

        juce::String sf2Stem;
        if (options.getSf2Name)
            sf2Stem = options.getSf2Name(bank.sf2FileIndex).upToFirstOccurrenceOf(".", false, false).trim();

        const bool vocalBank = sf2Stem.equalsIgnoreCase("MSXspirit02");
        const int mabiiccoProgram = presetIndexToMabiiccoProgram(sf2Stem, bank.dlsPreset);

        text << "mml-track=MML@" << melody << "," << chord1 << "," << chord2 << "," << song << ";\n"
             << "name=" << safeMmiLineValue(trackName) << "\n";

        if (vocalBank)
            text << "program=0\n" << "songProgram=" << mabiiccoProgram << "\n";
        else
            text << "program=" << mabiiccoProgram << "\n" << "songProgram=80\n";

        text << "panpot=64\n"
                "volume=100\n"
                "visible=true\n";

        ++writtenTracks;
    }

    if (writtenTracks == 0)
    {
        juce::String trackName = options.getTrackName ? options.getTrackName(0) : getFallbackTrackName(0);
        if (trackName.trim().isEmpty())
            trackName = getFallbackTrackName(0);

        text << "mml-track=MML@r1,,,;\n"
             << "name=" << safeMmiLineValue(trackName) << "\n"
                "program=0\n"
                "songProgram=0\n"
                "panpot=64\n"
                "volume=100\n"
                "visible=true\n";
    }

    return file.replaceWithText(text);
}

ProjectFileIO::MmiLoadResult ProjectFileIO::importMmiFile(const juce::File& file,
                                                          InstrumentBank* banks,
                                                          const MmiLoadOptions& options)
{
    MmiLoadResult result;

    const int maxBanks = juce::jmax(1, options.maxBanks);
    if (file == juce::File() || banks == nullptr)
    {
        result.errorMessage = "Invalid MMI load target.";
        return result;
    }

    juce::StringArray lines;
    file.readLines(lines);

    if (lines.size() == 0)
    {
        result.errorMessage = "The selected file is empty.";
        return result;
    }

    for (int i = 0; i < maxBanks; ++i)
        resetDmmfBank(banks[i]);

    for (const auto& rawLine : lines)
    {
        const auto line = rawLine.trim();
        if (line.startsWithIgnoreCase("title="))
        {
            result.title = line.substring(6).trim();
            break;
        }
    }

    int loadedTracks = 0;

    auto setPresetByStemAndIndex = [&](int bankIndex, const juce::String& stem, int presetIndex) -> bool
    {
        if (!options.findSf2IndexByStem)
            return false;

        const int fi = options.findSf2IndexByStem(stem);
        if (fi < 0)
            return false;

        int safePreset = juce::jmax(0, presetIndex);
        if (options.getPresetCount)
        {
            const int presetCount = options.getPresetCount(fi);
            if (presetCount > 0)
                safePreset = juce::jlimit(0, presetCount - 1, safePreset);
        }

        auto& bank = banks[bankIndex];
        bank.instrumentWave = 5;
        bank.sf2FileIndex = fi;
        bank.dlsPreset = safePreset;

        if (options.refreshSongPresetModeForBank)
            bank.songPresetMode = options.refreshSongPresetModeForBank(bankIndex);
        else
            bank.songPresetMode = stem.equalsIgnoreCase("MSXspirit02");

        return true;
    };

    auto applyMabiiccoProgramToBank = [&](int bankIndex, int program)
    {
        const auto knownTarget = knownMabiiccoProgramTarget(program);
        if (knownTarget.valid && setPresetByStemAndIndex(bankIndex, knownTarget.stem, knownTarget.presetIndex))
            return;

        int targetFileIdx = 0;
        int targetPresetIdx = juce::jmax(0, program);
        bool found = false;

        if (options.findPresetIndexByBankAndProgram)
        {
            const int maxSf2Files = options.maxSf2Files > 0 ? options.maxSf2Files : maxBanks;
            for (int fi = 0; fi < maxSf2Files; ++fi)
            {
                int idx = options.findPresetIndexByBankAndProgram(fi, 0, program);
                if (idx >= 0)
                {
                    targetFileIdx = fi;
                    targetPresetIdx = idx;
                    found = true;
                    break;
                }

                for (int midiBank = 1; midiBank <= 128; ++midiBank)
                {
                    idx = options.findPresetIndexByBankAndProgram(fi, midiBank, program);
                    if (idx >= 0)
                    {
                        targetFileIdx = fi;
                        targetPresetIdx = idx;
                        found = true;
                        break;
                    }
                }

                if (found)
                    break;
            }
        }

        auto& bank = banks[bankIndex];
        bank.instrumentWave = 5;
        bank.sf2FileIndex = found ? targetFileIdx : 0;
        bank.dlsPreset = found ? targetPresetIdx : juce::jmax(0, program);

        if (options.refreshSongPresetModeForBank)
            bank.songPresetMode = options.refreshSongPresetModeForBank(bankIndex);
        else
            bank.songPresetMode = false;
    };

    for (int i = 0; i < lines.size(); ++i)
    {
        juce::String line = lines[i].trim();
        if (!line.startsWithIgnoreCase("mml-track=MML@"))
            continue;

        if (loadedTracks >= maxBanks)
            break;

        juce::String mmlData = line.substring(14);
        if (mmlData.endsWithChar(';'))
            mmlData = mmlData.dropLastCharacters(1);

        const auto parts = splitMmiTrackParts(mmlData);
        auto& bank = banks[loadedTracks];

        bank.tracks[0].mml = parts.size() > 0 ? parts[0] : "";
        bank.tracks[1].mml = parts.size() > 1 ? parts[1] : "";
        bank.tracks[2].mml = parts.size() > 2 ? parts[2] : "";
        bank.tracks[3].mml = parts.size() > 3 ? parts[3] : "";

        int normalProgram = -1;
        int songProgram = -1;
        int j = i + 1;

        while (j < lines.size()
            && !lines[j].trim().startsWithIgnoreCase("mml-track=")
            && !lines[j].trim().startsWith("[marker]")
            && !lines[j].trim().startsWith("[time-signature]")
            && !lines[j].trim().startsWith("[bar-line-type]"))
        {
            juce::String propLine = lines[j].trim();
            if (propLine.startsWithIgnoreCase("program="))
                normalProgram = propLine.substring(8).getIntValue();
            else if (propLine.startsWithIgnoreCase("songProgram="))
                songProgram = propLine.substring(12).getIntValue();

            ++j;
        }

        const bool hasNormalPart = bank.tracks[0].mml.trim().isNotEmpty()
                            || bank.tracks[1].mml.trim().isNotEmpty()
                            || bank.tracks[2].mml.trim().isNotEmpty();
        const bool hasSongPart = bank.tracks[3].mml.trim().isNotEmpty();

        bank.mmiSongPartWithProgram = hasSongPart && hasNormalPart;

        if (hasSongPart && !hasNormalPart && songProgram >= 0)
            applyMabiiccoProgramToBank(loadedTracks, songProgram);
        else if (normalProgram >= 0)
            applyMabiiccoProgramToBank(loadedTracks, normalProgram);
        else if (songProgram >= 0)
            applyMabiiccoProgramToBank(loadedTracks, songProgram);

        if (bank.mmiSongPartWithProgram)
            bank.songPresetMode = false;

        ++loadedTracks;
    }

    if (loadedTracks <= 0)
    {
        result.errorMessage = "No MMI tracks were found.";
        return result;
    }

    result.ok = true;
    result.loadedTrackCount = loadedTracks;
    result.numActiveTracks = loadedTracks;
    return result;
}


bool ProjectFileIO::saveMidiFile(const juce::File& file,
                                 const InstrumentBank* banks,
                                 int numActiveTracks,
                                 const MidiSaveOptions& options)
{
    if (banks == nullptr || numActiveTracks <= 0)
        return false;

    constexpr int ticksPerQuarter = 480;
    juce::MidiFile midi;
    midi.setTicksPerQuarterNote(ticksPerQuarter);

    juce::MidiMessageSequence meta;
    const double bpm = options.bpm > 0.0 ? options.bpm : 120.0;
    meta.addEvent(juce::MidiMessage::tempoMetaEvent(static_cast<int>(60000000.0 / bpm)));
    if (options.timeSignatureId == 2)
        meta.addEvent(juce::MidiMessage::timeSignatureMetaEvent(3, 4));
    else
        meta.addEvent(juce::MidiMessage::timeSignatureMetaEvent(4, 4));
    midi.addTrack(meta);

    bool hasAnyNotes = false;
    const int limitedTrackCount = juce::jlimit(0, 16, numActiveTracks);

    for (int i = 0; i < limitedTrackCount; ++i)
    {
        juce::MidiMessageSequence seq;
        const int channel = (i % 16) + 1;
        const int program = juce::jlimit(0, 127, banks[i].dlsPreset);

        auto pc = juce::MidiMessage::programChange(channel, program);
        pc.setTimeStamp(0.0);
        seq.addEvent(pc);

        for (int j = 0; j < 4; ++j)
        {
            const bool partActive = options.isPartActiveForBank ? options.isPartActiveForBank(i, j) : true;
            if (!partActive)
                continue;

            const auto& track = banks[i].tracks[j];
            if (track.sequence.empty())
                continue;

            for (const auto& note : track.sequence)
            {
                if (note.frequency <= 0.0)
                    continue;

                const int midiNote = juce::jlimit(0, 127, static_cast<int>(std::round(69.0 + 12.0 * std::log2(note.frequency / 440.0))));
                const int velocity = juce::jlimit(1, 127, static_cast<int>(std::round(note.volume * 127.0f)));
                const double startTick = std::max(0.0, note.startBeat * ticksPerQuarter);
                const double endTick = std::max(startTick + 1.0, note.endBeat * ticksPerQuarter);

                auto on = juce::MidiMessage::noteOn(channel, midiNote, static_cast<juce::uint8>(velocity));
                on.setTimeStamp(startTick);
                seq.addEvent(on);

                auto off = juce::MidiMessage::noteOff(channel, midiNote);
                off.setTimeStamp(endTick);
                seq.addEvent(off);
                hasAnyNotes = true;
            }
        }

        seq.sort();
        midi.addTrack(seq);
    }

    if (!hasAnyNotes)
        return false;

    if (file.existsAsFile())
        file.deleteFile();

    auto out = file.createOutputStream();
    return out != nullptr && midi.writeTo(*out);
}

ProjectFileIO::MidiLoadResult ProjectFileIO::importMidiFile(const juce::File& file,
                                                            InstrumentBank* banks,
                                                            const MidiLoadOptions& options)
{
    MidiLoadResult result;

    if (banks == nullptr)
    {
        result.errorMessage = "Invalid destination bank buffer.";
        return result;
    }

    juce::MemoryBlock midiData;
    if (!file.loadFileAsData(midiData) || midiData.getSize() < 14)
    {
        result.errorMessage = "Could not read the MIDI file.";
        return result;
    }

    const auto* bytes = static_cast<const juce::uint8*>(midiData.getData());
    const size_t dataSize = midiData.getSize();

    auto hasBytes = [dataSize](size_t at, size_t count) -> bool { return at <= dataSize && count <= dataSize - at; };
    auto chunkIs = [&](size_t at, const char* id) -> bool {
        return hasBytes(at, 4)
            && bytes[at] == static_cast<juce::uint8>(id[0])
            && bytes[at + 1] == static_cast<juce::uint8>(id[1])
            && bytes[at + 2] == static_cast<juce::uint8>(id[2])
            && bytes[at + 3] == static_cast<juce::uint8>(id[3]);
    };
    auto readU16 = [&](size_t at) -> int { return (static_cast<int>(bytes[at]) << 8) | static_cast<int>(bytes[at + 1]); };
    auto readU32 = [&](size_t at) -> uint32_t {
        return (static_cast<uint32_t>(bytes[at]) << 24)
             | (static_cast<uint32_t>(bytes[at + 1]) << 16)
             | (static_cast<uint32_t>(bytes[at + 2]) << 8)
             | static_cast<uint32_t>(bytes[at + 3]);
    };
    auto readVarLen = [&](size_t& p, size_t limit, uint32_t& value) -> bool {
        value = 0;
        for (int i = 0; i < 4; ++i)
        {
            if (p >= limit)
                return false;
            const auto b = bytes[p++];
            value = (value << 7) | static_cast<uint32_t>(b & 0x7f);
            if ((b & 0x80) == 0)
                return true;
        }
        return false;
    };

    if (!chunkIs(0, "MThd") || !hasBytes(4, 10))
    {
        result.errorMessage = "This is not a standard MIDI file.";
        return result;
    }

    const uint32_t headerLength = readU32(4);
    if (headerLength < 6 || !hasBytes(8, headerLength))
    {
        result.errorMessage = "The MIDI header is damaged.";
        return result;
    }

    const int declaredTrackCount = readU16(10);
    const int division = readU16(12);
    int ticksPerQuarter = 480;
    if ((division & 0x8000) == 0 && division > 0)
        ticksPerQuarter = division;

    double bpm = 120.0;
    int numerator = 4;
    int denominator = 4;

    struct ImportedMidiNote
    {
        double startTick = 0.0;
        double endTick = 0.0;
        int midiNote = 60;
        int velocity = 96;
        int channel = 0;
        int program = 0;
    };

    struct ImportedMidiTrack
    {
        juce::String name;
        int channel = 0;
        int program = 0;
        std::vector<ImportedMidiNote> notes;
    };

    struct ActiveMidiNote
    {
        double startTick = 0.0;
        int velocity = 96;
        int program = 0;
    };

    std::vector<ImportedMidiTrack> rawTracks;
    size_t pos = 8 + static_cast<size_t>(headerLength);
    int chunksSeen = 0;

    while (hasBytes(pos, 8) && chunksSeen < declaredTrackCount)
    {
        const bool isTrackChunk = chunkIs(pos, "MTrk");
        const uint32_t chunkLength = readU32(pos + 4);
        pos += 8;
        if (!hasBytes(pos, chunkLength))
            break;

        const size_t trackEnd = pos + static_cast<size_t>(chunkLength);
        ++chunksSeen;
        if (!isTrackChunk)
        {
            pos = trackEnd;
            continue;
        }

        ImportedMidiTrack track;
        std::map<int, ActiveMidiNote> activeNotes;
        int channelPrograms[16] = {};
        uint8_t runningStatus = 0;
        double absoluteTick = 0.0;
        size_t tpos = pos;

        while (tpos < trackEnd)
        {
            uint32_t delta = 0;
            if (!readVarLen(tpos, trackEnd, delta))
                break;

            absoluteTick += static_cast<double>(delta);
            if (tpos >= trackEnd)
                break;

            int status = bytes[tpos++];
            int firstDataByte = -1;

            if (status < 0x80)
            {
                if (runningStatus == 0)
                    break;
                firstDataByte = status;
                status = runningStatus;
            }
            else if (status < 0xf0)
            {
                runningStatus = static_cast<uint8_t>(status);
            }
            else
            {
                runningStatus = 0;
            }

            if (status == 0xff)
            {
                if (tpos >= trackEnd)
                    break;

                const int metaType = bytes[tpos++];
                uint32_t metaLength = 0;
                if (!readVarLen(tpos, trackEnd, metaLength) || !hasBytes(tpos, metaLength) || tpos + metaLength > trackEnd)
                    break;

                if (metaType == 0x03 && metaLength > 0)
                    track.name = juce::String::fromUTF8(reinterpret_cast<const char*>(bytes + tpos), static_cast<int>(metaLength));
                else if (metaType == 0x51 && metaLength == 3)
                {
                    const int microsecondsPerQuarter = (static_cast<int>(bytes[tpos]) << 16)
                                                     | (static_cast<int>(bytes[tpos + 1]) << 8)
                                                     | static_cast<int>(bytes[tpos + 2]);
                    if (microsecondsPerQuarter > 0)
                        bpm = 60000000.0 / static_cast<double>(microsecondsPerQuarter);
                }
                else if (metaType == 0x58 && metaLength >= 2)
                {
                    numerator = juce::jlimit(1, 32, static_cast<int>(bytes[tpos]));
                    denominator = 1 << juce::jlimit(0, 7, static_cast<int>(bytes[tpos + 1]));
                }

                tpos += static_cast<size_t>(metaLength);
                continue;
            }

            if (status == 0xf0 || status == 0xf7)
            {
                uint32_t sysexLength = 0;
                if (!readVarLen(tpos, trackEnd, sysexLength) || !hasBytes(tpos, sysexLength) || tpos + sysexLength > trackEnd)
                    break;
                tpos += static_cast<size_t>(sysexLength);
                continue;
            }

            if (status >= 0xf0)
            {
                int systemDataBytes = 0;
                if (status == 0xf1 || status == 0xf3)
                    systemDataBytes = 1;
                else if (status == 0xf2)
                    systemDataBytes = 2;

                if (!hasBytes(tpos, static_cast<size_t>(systemDataBytes)))
                    break;
                tpos += static_cast<size_t>(systemDataBytes);
                continue;
            }

            const int command = status & 0xf0;
            const int channel = status & 0x0f;
            const int dataBytes = (command == 0xc0 || command == 0xd0) ? 1 : 2;

            int data1 = firstDataByte;
            if (data1 < 0)
            {
                if (tpos >= trackEnd)
                    break;
                data1 = bytes[tpos++];
            }

            int data2 = 0;
            if (dataBytes == 2)
            {
                if (tpos >= trackEnd)
                    break;
                data2 = bytes[tpos++];
            }

            if (command == 0xc0)
            {
                channelPrograms[channel] = juce::jlimit(0, 127, data1);
                continue;
            }

            if (command != 0x80 && command != 0x90)
                continue;

            const int noteNumber = juce::jlimit(0, 127, data1);
            const int key = channel * 128 + noteNumber;
            const bool noteOn = (command == 0x90 && data2 > 0);

            if (noteOn)
            {
                auto existing = activeNotes.find(key);
                if (existing != activeNotes.end() && absoluteTick > existing->second.startTick + 0.5)
                    track.notes.push_back({ existing->second.startTick, absoluteTick, noteNumber, existing->second.velocity, channel, existing->second.program });

                activeNotes[key] = { absoluteTick, juce::jlimit(1, 127, data2), channelPrograms[channel] };
            }
            else
            {
                auto existing = activeNotes.find(key);
                if (existing != activeNotes.end())
                {
                    if (absoluteTick > existing->second.startTick + 0.5)
                        track.notes.push_back({ existing->second.startTick, absoluteTick, noteNumber, existing->second.velocity, channel, existing->second.program });
                    activeNotes.erase(existing);
                }
            }
        }

        for (const auto& openNote : activeNotes)
        {
            const double startTick = openNote.second.startTick;
            const double endTick = std::max(startTick + static_cast<double>(ticksPerQuarter), absoluteTick);
            const int noteNumber = openNote.first % 128;
            const int channel = openNote.first / 128;
            track.notes.push_back({ startTick, endTick, noteNumber, openNote.second.velocity, channel, openNote.second.program });
        }

        if (!track.notes.empty())
            rawTracks.push_back(std::move(track));

        pos = trackEnd;
    }

    std::vector<ImportedMidiTrack> channelTracks(16);
    bool channelUsed[16] = {};

    for (const auto& raw : rawTracks)
    {
        for (const auto& note : raw.notes)
        {
            const int ch = juce::jlimit(0, 15, note.channel);
            if (!channelUsed[ch])
            {
                channelUsed[ch] = true;
                channelTracks[static_cast<size_t>(ch)].channel = ch;
                channelTracks[static_cast<size_t>(ch)].name = raw.name.isNotEmpty()
                    ? raw.name + " / Ch " + juce::String(ch + 1)
                    : "MIDI Ch " + juce::String(ch + 1);
            }
            channelTracks[static_cast<size_t>(ch)].notes.push_back(note);
        }
    }

    std::vector<ImportedMidiTrack> parsedTracks;
    for (int ch = 0; ch < 16; ++ch)
    {
        auto& channelTrack = channelTracks[static_cast<size_t>(ch)];
        if (!channelUsed[ch] || channelTrack.notes.empty())
            continue;

        int programCounts[128] = {};
        for (const auto& note : channelTrack.notes)
            ++programCounts[juce::jlimit(0, 127, note.program)];

        int bestProgram = 0;
        int bestCount = -1;
        for (int p = 0; p < 128; ++p)
        {
            if (programCounts[p] > bestCount)
            {
                bestProgram = p;
                bestCount = programCounts[p];
            }
        }

        channelTrack.program = bestProgram;
        std::sort(channelTrack.notes.begin(), channelTrack.notes.end(), [](const auto& a, const auto& b) {
            if (std::abs(a.startTick - b.startTick) > 0.5)
                return a.startTick < b.startTick;
            if (a.channel != b.channel)
                return a.channel < b.channel;
            return a.midiNote > b.midiNote;
        });
        parsedTracks.push_back(std::move(channelTrack));
    }

    if (parsedTracks.empty())
    {
        result.errorMessage = "No note tracks were found in the MIDI file.";
        return result;
    }

    const int roundedBpm = juce::jlimit(20, 300, static_cast<int>(std::round(bpm)));

    auto splitMidiNotesToMmlParts = [roundedBpm, ticksPerQuarter](std::vector<ImportedMidiNote> notes) -> std::vector<juce::String>
    {
        std::vector<juce::String> resultParts(3);
        if (notes.empty())
            return resultParts;

        const double quantizeGrid = std::max(1.0, static_cast<double>(ticksPerQuarter) / 12.0);

        int minMidiNote = 127;
        int maxMidiNote = 0;
        for (const auto& n : notes)
        {
            minMidiNote = std::min(minMidiNote, n.midiNote);
            maxMidiNote = std::max(maxMidiNote, n.midiNote);
        }

        const bool hasSeparatedLowAccompaniment = (maxMidiNote >= 64 && minMidiNote <= 55);
        std::map<int, std::vector<ImportedMidiNote>> groupedByStart;

        for (auto n : notes)
        {
            const int startStep = static_cast<int>(std::round(n.startTick / quantizeGrid));
            const int endStep = std::max(startStep + 1, static_cast<int>(std::round(n.endTick / quantizeGrid)));
            n.startTick = static_cast<double>(startStep) * quantizeGrid;
            n.endTick = static_cast<double>(endStep) * quantizeGrid;
            groupedByStart[startStep].push_back(n);
        }

        std::vector<std::vector<ImportedMidiNote>> voices(3);
        for (auto& item : groupedByStart)
        {
            auto& group = item.second;
            std::sort(group.begin(), group.end(), [](const auto& a, const auto& b) {
                if (a.midiNote != b.midiNote)
                    return a.midiNote > b.midiNote;
                return a.endTick < b.endTick;
            });

            if (group.size() == 1 && hasSeparatedLowAccompaniment)
            {
                const double noteDuration = group[0].endTick - group[0].startTick;
                if ((group[0].midiNote <= 62 && noteDuration >= static_cast<double>(ticksPerQuarter) / 2.0) || group[0].midiNote < 60)
                {
                    voices[1].push_back(group[0]);
                    continue;
                }
            }

            const int maxVoices = std::min(3, static_cast<int>(group.size()));
            for (int v = 0; v < maxVoices; ++v)
                voices[static_cast<size_t>(v)].push_back(group[static_cast<size_t>(v)]);
        }

        auto appendTimedSymbol = [&](juce::String& out, const juce::String& symbol, bool isRest, double durationTicks) -> double
        {
            const double baseUnitTicks = std::max(1.0, static_cast<double>(ticksPerQuarter) / 12.0);
            int remainingUnits = std::max(1, static_cast<int>(std::round(durationTicks / baseUnitTicks)));

            struct DurationCandidate { int units; const char* token; };
            static const DurationCandidate candidates[] = {
                { 72, "1." }, { 48, "1" }, { 36, "2." }, { 24, "2" },
                { 18, "4." }, { 16, "3" }, { 12, "4" }, { 9, "8." },
                { 8, "6" }, { 6, "8" }, { 4, "12" }, { 3, "16" },
                { 2, "24" }, { 1, "48" }
            };

            int writtenUnits = 0;
            bool firstPiece = true;
            while (remainingUnits > 0)
            {
                const DurationCandidate* selected = &candidates[sizeof(candidates) / sizeof(candidates[0]) - 1];
                for (const auto& candidate : candidates)
                {
                    if (candidate.units <= remainingUnits)
                    {
                        selected = &candidate;
                        break;
                    }
                }

                if (!isRest && !firstPiece)
                    out << "&";
                out << symbol << selected->token;
                remainingUnits -= selected->units;
                writtenUnits += selected->units;
                firstPiece = false;
            }

            return static_cast<double>(writtenUnits) * baseUnitTicks;
        };

        auto getNoteName = [](int midiNote) -> juce::String
        {
            static const char* names[] = { "c", "c+", "d", "d+", "e", "f", "f+", "g", "g+", "a", "a+", "b" };
            return juce::String(names[((midiNote % 12) + 12) % 12]);
        };

        auto voiceToMml = [&](std::vector<ImportedMidiNote> voiceNotes) -> juce::String
        {
            if (voiceNotes.empty())
                return {};

            std::sort(voiceNotes.begin(), voiceNotes.end(), [](const auto& a, const auto& b) {
                if (std::abs(a.startTick - b.startTick) > 0.5)
                    return a.startTick < b.startTick;
                return a.midiNote > b.midiNote;
            });

            for (size_t i = 0; i + 1 < voiceNotes.size(); ++i)
            {
                if (voiceNotes[i].endTick > voiceNotes[i + 1].startTick && voiceNotes[i + 1].startTick > voiceNotes[i].startTick + 0.5)
                    voiceNotes[i].endTick = voiceNotes[i + 1].startTick;
            }

            voiceNotes.erase(std::remove_if(voiceNotes.begin(), voiceNotes.end(), [quantizeGrid](const auto& n) {
                return n.endTick <= n.startTick + 0.5;
            }), voiceNotes.end());

            juce::String out;
            out << "MML@t" << roundedBpm << "v12";
            double currentTick = 0.0;
            int currentOctave = 4;
            bool hasWrittenOctave = false;
            const double minGapTicks = static_cast<double>(ticksPerQuarter) / 24.0;

            for (auto n : voiceNotes)
            {
                if (n.endTick <= n.startTick + 0.5)
                    n.endTick = n.startTick + quantizeGrid;

                double startTick = n.startTick;
                double endTick = n.endTick;

                if (startTick < currentTick - minGapTicks)
                {
                    if (endTick <= currentTick + minGapTicks)
                        continue;
                    startTick = currentTick;
                }

                const double restTicks = startTick - currentTick;
                if (restTicks > minGapTicks)
                    currentTick += appendTimedSymbol(out, "r", true, restTicks);
                else if (std::abs(restTicks) <= minGapTicks)
                    currentTick = startTick;

                const int midiNote = juce::jlimit(0, 127, n.midiNote);
                const int noteOctave = juce::jlimit(0, 9, (midiNote / 12) - 1);
                if (!hasWrittenOctave || noteOctave != currentOctave)
                {
                    out << "o" << noteOctave;
                    currentOctave = noteOctave;
                    hasWrittenOctave = true;
                }

                currentTick += appendTimedSymbol(out, getNoteName(midiNote), false, endTick - startTick);
            }

            out << ";";
            return out;
        };

        for (int i = 0; i < 3; ++i)
            resultParts[static_cast<size_t>(i)] = voiceToMml(voices[static_cast<size_t>(i)]);

        return resultParts;
    };

    auto applyMabinogiPreset = [&](InstrumentBank& bank, const juce::String& sf2Stem, int presetIndex, int fallbackPreset)
    {
        int fileIndex = options.findSf2IndexByStem ? options.findSf2IndexByStem(sf2Stem) : -1;
        int targetPreset = presetIndex;

        if (fileIndex < 0)
        {
            fileIndex = 0;
            targetPreset = fallbackPreset;
        }

        const int presetCount = options.getPresetCount ? options.getPresetCount(fileIndex) : 0;
        if (presetCount > 0)
            targetPreset = juce::jlimit(0, presetCount - 1, targetPreset);

        bank.instrumentWave = 5;
        bank.sf2FileIndex = std::max(0, fileIndex);
        bank.dlsPreset = std::max(0, targetPreset);
    };

    auto setBankPresetFromMidi = [&](InstrumentBank& bank, int midiChannel, int gmProgram, const std::vector<ImportedMidiNote>& notes)
    {
        int averageNote = 60;
        if (!notes.empty())
        {
            int64_t sum = 0;
            for (const auto& n : notes)
                sum += n.midiNote;
            averageNote = static_cast<int>(sum / static_cast<int64_t>(notes.size()));
        }

        if (midiChannel == 9) applyMabinogiPreset(bank, "MSXspirit04", 1, 0);
        else if (gmProgram <= 7) applyMabinogiPreset(bank, "MSXspirit01", 21, 0);
        else if (gmProgram <= 15) applyMabinogiPreset(bank, "MSXspirit01", 38, 0);
        else if (gmProgram <= 23) applyMabinogiPreset(bank, "MSXspirit01", 19, 0);
        else if (gmProgram <= 31) { if (gmProgram <= 25) applyMabinogiPreset(bank, "MSXspirit01", 0, 0); else applyMabinogiPreset(bank, "MSXspirit01", 20, 0); }
        else if (gmProgram <= 39) applyMabinogiPreset(bank, "MSXspirit05", 0, 0);
        else if (gmProgram <= 47) applyMabinogiPreset(bank, "MSXspirit01", averageNote < 55 ? 23 : 22, 0);
        else if (gmProgram <= 55) applyMabinogiPreset(bank, "MSXspirit01", averageNote < 55 ? 23 : 22, 0);
        else if (gmProgram <= 63) applyMabinogiPreset(bank, "MSXspirit01", 18, 0);
        else if (gmProgram <= 71) applyMabinogiPreset(bank, "MSXspirit01", 6, 0);
        else if (gmProgram <= 79) applyMabinogiPreset(bank, "MSXspirit01", gmProgram == 78 ? 3 : 5, 0);
        else if (gmProgram <= 87) applyMabinogiPreset(bank, "MSXspirit01", 20, 0);
        else if (gmProgram <= 95) applyMabinogiPreset(bank, "MSXspirit01", 19, 0);
        else if (gmProgram <= 103) applyMabinogiPreset(bank, "MSXspirit01", 38, 0);
        else if (gmProgram <= 111) applyMabinogiPreset(bank, "MSXspirit04", 2, 0);
        else if (gmProgram <= 119) applyMabinogiPreset(bank, "MSXspirit04", 1, 0);
        else applyMabinogiPreset(bank, "MSXspirit01", 38, 0);
    };

    const int maxBanks = juce::jlimit(1, 16, options.maxBanks);

    for (int i = 0; i < maxBanks; ++i)
    {
        banks[i].instrumentWave = 5;
        banks[i].helperMode = 1;
        banks[i].autoBassScale = 1;
        banks[i].sf2FileIndex = 0;
        banks[i].dlsPreset = 0;
        banks[i].songPresetMode = false;
        banks[i].mmiSongPartWithProgram = false;

        for (int j = 0; j < 4; ++j)
        {
            banks[i].tracks[j].mml.clear();
            banks[i].tracks[j].mute = false;
            banks[i].tracks[j].solo = false;
            banks[i].tracks[j].sequence.clear();
            banks[i].tracks[j].noteIndex = 0;
            banks[i].tracks[j].currentAngle = 0.0;
        }
    }

    const int loadedTracks = juce::jlimit(1, maxBanks, static_cast<int>(parsedTracks.size()));
    for (int i = 0; i < loadedTracks; ++i)
    {
        const auto& midiTrack = parsedTracks[static_cast<size_t>(i)];
        auto& bank = banks[i];
        setBankPresetFromMidi(bank, midiTrack.channel, midiTrack.program, midiTrack.notes);

        const auto parts = splitMidiNotesToMmlParts(midiTrack.notes);
        bank.tracks[0].mml = parts.size() > 0 ? parts[0] : juce::String();
        bank.tracks[1].mml = parts.size() > 1 ? parts[1] : juce::String();
        bank.tracks[2].mml = parts.size() > 2 ? parts[2] : juce::String();
        bank.tracks[3].mml.clear();
    }

    result.ok = true;
    result.loadedTrackCount = loadedTracks;
    result.numActiveTracks = loadedTracks;
    result.numerator = numerator;
    result.denominator = denominator;
    result.roundedBpm = roundedBpm;
    return result;
}

ProjectFileIO::WavExportResult ProjectFileIO::exportWavFile(const juce::File& file,
                                                               InstrumentBank* banks,
                                                               int numActiveTracks,
                                                               const WavExportOptions& options)
{
    WavExportResult result;

    if (file == juce::File())
    {
        result.errorMessage = "No output file was selected.";
        return result;
    }

    if (banks == nullptr)
    {
        result.errorMessage = "Invalid project data.";
        return result;
    }

    if (options.sampleRate <= 0.0)
    {
        result.errorMessage = "Invalid audio sample rate.";
        return result;
    }

    if (!options.renderAudioBlock)
    {
        result.errorMessage = "Audio render callback is not configured.";
        return result;
    }

    const int trackCount = juce::jlimit(0, 16, numActiveTracks);
    int64_t maxLengthInSamples = 0;
    bool hasAnyNotes = false;

    auto isPartActive = [&options](int bankIdx, int trackIdx)
    {
        if (options.isPartActiveForBank)
            return options.isPartActiveForBank(bankIdx, trackIdx);

        return trackIdx >= 0 && trackIdx < 3;
    };

    for (int i = 0; i < trackCount; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            banks[i].tracks[j].noteIndex = 0;
            banks[i].tracks[j].currentAngle = 0.0;

            if (isPartActive(i, j) && !banks[i].tracks[j].sequence.empty())
            {
                maxLengthInSamples = std::max(maxLengthInSamples, banks[i].tracks[j].sequence.back().endSample);
                hasAnyNotes = true;
            }
        }
    }

    result.hasAnyNotes = hasAnyNotes;
    result.maxLengthInSamples = maxLengthInSamples;

    if (!hasAnyNotes || maxLengthInSamples <= 0)
    {
        result.errorMessage = "The score is empty.";
        return result;
    }

    if (file.existsAsFile() && !file.deleteFile())
    {
        result.errorMessage = "Could not replace the existing WAV file.";
        return result;
    }

    std::unique_ptr<juce::OutputStream> outStream = file.createOutputStream();
    if (outStream == nullptr)
    {
        result.errorMessage = "Could not create the WAV file.";
        return result;
    }

    juce::WavAudioFormat wavFormat;
    auto writer = wavFormat.createWriterFor(outStream,
                                            juce::AudioFormatWriterOptions()
                                                .withSampleRate(options.sampleRate)
                                                .withNumChannels(juce::jmax(1, options.numChannels))
                                                .withBitsPerSample(juce::jmax(8, options.bitsPerSample)));

    if (writer == nullptr)
    {
        result.errorMessage = "Could not create a WAV writer.";
        return result;
    }

    const int numChannels = juce::jmax(1, options.numChannels);
    const int blockSize = juce::jmax(64, options.blockSize);

    int64_t exportSampleCounter = 0;
    juce::AudioBuffer<float> exportBuffer(numChannels, blockSize);

    if (options.stopAllNotes)
        options.stopAllNotes();

    while (exportSampleCounter < maxLengthInSamples)
    {
        exportBuffer.clear();

        const int samplesToProcess = static_cast<int>(std::min(static_cast<int64_t>(blockSize),
                                                               maxLengthInSamples - exportSampleCounter));

        juce::AudioBuffer<float> tempBuf(exportBuffer.getArrayOfWritePointers(), numChannels, samplesToProcess);
        bool isPlayingFlag = true;

        options.renderAudioBlock(tempBuf, exportSampleCounter, isPlayingFlag);
        writer->writeFromAudioSampleBuffer(exportBuffer, 0, samplesToProcess);

        exportSampleCounter += samplesToProcess;
    }

    if (options.stopAllNotes)
        options.stopAllNotes();

    result.ok = true;
    return result;
}


bool ProjectFileIO::saveDmmfProject(const juce::File& file, const InstrumentBank* banks, int numActiveTracks)
{
    DmmfSaveOptions options;
    return saveDmmfProject(file, banks, numActiveTracks, options);
}

bool ProjectFileIO::loadDmmfProject(const juce::File& file, InstrumentBank* banks, int& outNumActiveTracks)
{
    const int fallbackMaxBanks = 16;
    auto result = loadDmmfProject(file, banks, fallbackMaxBanks, 0, std::function<int(const juce::String&)> {});
    outNumActiveTracks = result.numActiveTracks;
    return result.ok;
}

bool ProjectFileIO::importMmiFile(const juce::File& file, InstrumentBank* banks, int& outNumActiveTracks)
{
    MmiLoadOptions options;
    options.maxBanks = 16;
    const auto result = importMmiFile(file, banks, options);
    outNumActiveTracks = result.numActiveTracks;
    return result.ok;
}
