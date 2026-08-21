#include "MainComponent.h"
#include <cmath> 
#include <algorithm>
#include <map>
#include <limits>
#include <functional>


#include "MmlData/MmlPianoRollMml.h"

namespace
{


    class AtelierCloseablePopupWindow final : public juce::DocumentWindow
    {
    public:
        AtelierCloseablePopupWindow(const juce::String& title,
                                    juce::Colour backgroundColour,
                                    std::function<void()> onCloseCallback)
            : juce::DocumentWindow(title, backgroundColour, juce::DocumentWindow::closeButton),
              onClose(std::move(onCloseCallback))
        {
        }

        void closeButtonPressed() override
        {
            if (onClose != nullptr)
                onClose();
        }

    private:
        std::function<void()> onClose;
    };

    class LicenceCreditsContent final : public juce::Component
    {
    public:
        LicenceCreditsContent(int themeId, juce::String bodyText)
            : text(std::move(bodyText))
        {
            textEditor.setMultiLine(true);
            textEditor.setReadOnly(true);
            textEditor.setCaretVisible(false);
            textEditor.setScrollbarsShown(true);
            textEditor.setText(text, juce::dontSendNotification);
            addAndMakeVisible(textEditor);
            applyTheme(themeId);
        }

        void applyTheme(int themeId)
        {
            panel = CustomUI::getThemeColour(themeId, "panel");
            panel2 = CustomUI::getThemeColour(themeId, "panel2");
            accent = CustomUI::getThemeColour(themeId, "accent");
            textColour = CustomUI::getReadableTextColour(panel2);

            textEditor.setColour(juce::TextEditor::backgroundColourId, panel2);
            textEditor.setColour(juce::TextEditor::textColourId, textColour);
            textEditor.setColour(juce::TextEditor::highlightColourId, accent.withAlpha(0.35f));
            textEditor.setColour(juce::TextEditor::highlightedTextColourId, textColour);
            textEditor.setColour(juce::TextEditor::outlineColourId, accent.withAlpha(0.55f));
            textEditor.setColour(juce::TextEditor::focusedOutlineColourId, accent.withAlpha(0.85f));
            textEditor.applyFontToAllText(juce::FontOptions(14.0f));
            repaint();
        }

        void paint(juce::Graphics& g) override
        {
            g.fillAll(panel);
            auto area = getLocalBounds().toFloat().reduced(6.0f);
            g.setColour(panel2);
            g.fillRoundedRectangle(area, 8.0f);
            g.setColour(accent.withAlpha(0.70f));
            g.drawRoundedRectangle(area, 8.0f, 1.2f);
        }

        void resized() override
        {
            textEditor.setBounds(getLocalBounds().reduced(14));
        }

    private:
        juce::String text;
        juce::TextEditor textEditor;
        juce::Colour panel, panel2, accent, textColour;
    };


    constexpr int atelierButtonBorderColourId = 0x2000401;

    class AtelierButtonBorderLookAndFeel final : public juce::LookAndFeel_V4
    {
    public:
        void drawButtonBackground(juce::Graphics& g,
                                  juce::Button& button,
                                  const juce::Colour& backgroundColour,
                                  bool shouldDrawButtonAsHighlighted,
                                  bool shouldDrawButtonAsDown) override
        {
            auto area = button.getLocalBounds().toFloat().reduced(0.5f);

            auto fill = backgroundColour;
            if (!button.isEnabled())
                fill = fill.withAlpha(0.45f);
            else if (shouldDrawButtonAsDown)
                fill = fill.darker(0.18f);
            else if (shouldDrawButtonAsHighlighted)
                fill = fill.brighter(0.08f);

            g.setColour(fill);
            g.fillRoundedRectangle(area, 4.0f);

            auto border = button.findColour(atelierButtonBorderColourId, false);
            if (border.isTransparent())
                border = backgroundColour.contrasting(0.25f);

            if (!button.isEnabled())
                border = border.withAlpha(0.28f);
            else if (shouldDrawButtonAsHighlighted || button.getToggleState())
                border = border.withAlpha(0.95f);
            else
                border = border.withAlpha(0.65f);

            g.setColour(border);
            g.drawRoundedRectangle(area, 4.0f, (shouldDrawButtonAsHighlighted || button.getToggleState()) ? 1.45f : 1.05f);
        }
    };

    AtelierButtonBorderLookAndFeel& getAtelierButtonBorderLookAndFeel()
    {
        static AtelierButtonBorderLookAndFeel lookAndFeel;
        return lookAndFeel;
    }


    juce::String restoreLanguagePresetTranslation(const juce::String& currentName,
                                                        const juce::String& sf2FileName,
                                                        int presetIdx,
                                                        int languageId)
    {
        // Fury_Sound_Pack_v150 / Mabinogi Mobile preset translation ONLY.
        // Do not touch PC MSXspirit01~05 DLS labels here.
        const bool useEnglish = (languageId == 1);
        const bool useKorean = (languageId == 2);
        const bool useJapanese = (languageId == 3);
        if (!useEnglish && !useKorean && !useJapanese)
            return currentName;

        const juce::String lowerFileName = sf2FileName.toLowerCase();
        const bool isFuryMobilePack =
            lowerFileName.contains("fury_sound_pack_v150")
            || lowerFileName.contains("fury_sound_pack")
            || lowerFileName.contains("fury sound pack")
            || lowerFileName.contains("mabinogi mobile instrument set")
            || lowerFileName.contains("mabinogimobile")
            || lowerFileName.contains("mabinogi_mobile")
            || lowerFileName.contains("mobile instrument");

        if (!isFuryMobilePack)
            return currentName;

        struct MobilePresetName
        {
            int preset;
            const char* en;
            const wchar_t* ko;
            const wchar_t* ja;
        };

        // Fury_Sound_Pack_v150 exact preset table.
        static const MobilePresetName mobileNames[] = {
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

        for (const auto& entry : mobileNames)
        {
            if (entry.preset == presetIdx)
            {
                if (useKorean) return juce::String(entry.ko);
                if (useJapanese) return juce::String(entry.ja);
                return juce::String(entry.en);
            }
        }

        return currentName;
    }

    using MmlPianoRoll::midiToFrequencyForMml;
    using MmlPianoRoll::frequencyToMidiForMml;
    using MmlPianoRoll::quantizeBeat64;
    using MmlPianoRoll::volumeFloatToMml;
    using MmlPianoRoll::volumeMmlToFloat;
    using MmlPianoRoll::getInheritedVolumeAtBeat;
    using MmlPianoRoll::PianoRollTempoEvent;
    using MmlPianoRoll::extractTempoEventsFromMml;
    using MmlPianoRoll::buildMmlFromPianoRollSequenceWithTempoEvents;
    using MmlPianoRoll::buildMmlFromPianoRollSequence;
    using MmlPianoRoll::buildOptimizedMmlFromPianoRollSequence;
    using MmlPianoRoll::buildOptimizedMmlFromPianoRollSequenceWithTempoEventsClampedToSequenceEnd;
    using MmlPianoRoll::sortAndDeduplicateMeterChanges;

    constexpr double defaultTimelinePixelsPerBeatFor3MLE = 50.0;

    int mapCaretAfterAutomaticTextSync(const juce::String& beforeText,
                                       const juce::String& afterText,
                                       int oldCaretPosition)
    {
        const int beforeLength = beforeText.length();
        const int afterLength = afterText.length();

        oldCaretPosition = juce::jlimit(0, beforeLength, oldCaretPosition);

        if (beforeText == afterText)
            return oldCaretPosition;

        int commonPrefix = 0;
        const int prefixLimit = juce::jmin(beforeLength, afterLength);
        while (commonPrefix < prefixLimit && beforeText[commonPrefix] == afterText[commonPrefix])
            ++commonPrefix;

        int commonSuffix = 0;
        while (commonSuffix < beforeLength - commonPrefix
            && commonSuffix < afterLength - commonPrefix
            && beforeText[beforeLength - 1 - commonSuffix] == afterText[afterLength - 1 - commonSuffix])
        {
            ++commonSuffix;
        }

        const int beforeChangedEnd = beforeLength - commonSuffix;
        const int afterChangedEnd = afterLength - commonSuffix;

        // If the user was typing at the point where an automatic tempo command
        // was inserted, keep the caret after the inserted command.
        // Example: before t120cdefga|  -> after t120cdefgat130|
        if (oldCaretPosition >= commonPrefix && oldCaretPosition <= beforeChangedEnd)
            return juce::jlimit(0, afterLength, afterChangedEnd);

        // If the caret was after the changed span, preserve its logical offset.
        if (oldCaretPosition > beforeChangedEnd)
            return juce::jlimit(0, afterLength, oldCaretPosition + (afterLength - beforeLength));

        return juce::jlimit(0, afterLength, oldCaretPosition);
    }
}



class HelperPopupContent : public juce::Component
{
public:
    HelperPopupContent(int languageIdIn,
                       int currentHelperMode,
                       int currentScaleId,
                       std::function<void(int)> helperChanged,
                       std::function<void(int)> scaleChanged,
                       std::function<int()> detectScaleClicked,
                       std::function<juce::String(int)> scaleTextProvider)
        : languageId(languageIdIn),
          onHelperChanged(std::move(helperChanged)),
          onScaleChanged(std::move(scaleChanged)),
          onDetectScaleClicked(std::move(detectScaleClicked)),
          getScaleText(std::move(scaleTextProvider))
    {
        helperLabel.setText(txt("Helper", L"도우미", L"ヘルパー"), juce::dontSendNotification);
        scaleLabel.setText(txt("Scale", L"스케일", L"スケール"), juce::dontSendNotification);
        keyLabel.setText(txt("Key", L"조표", L"調号"), juce::dontSendNotification);
        detectButton.setButtonText(txt("Detect Key", L"조표읽기", L"調号読取"));

        addAndMakeVisible(helperLabel);
        addAndMakeVisible(helperModeCombo);
        addAndMakeVisible(scaleLabel);
        addAndMakeVisible(scaleCombo);
        addAndMakeVisible(signatureLabel);
        addAndMakeVisible(detectButton);

        helperLabel.setJustificationType(juce::Justification::centredRight);
        scaleLabel.setJustificationType(juce::Justification::centredRight);
        keyLabel.setJustificationType(juce::Justification::centredRight);
        signatureLabel.setJustificationType(juce::Justification::centredLeft);

        helperModeCombo.addItem(txt("Off", L"끄기", L"オフ"), 1);
        helperModeCombo.addItem(txt("Auto Chord", L"자동 화음", L"自動和音"), 2);
        helperModeCombo.addItem(txt("Arp (1-3-5)", L"아르페지오 (1 - 3 - 5)", L"アルペジオ (1 - 3 - 5)"), 3);
        helperModeCombo.addItem(txt("Arp (1-5)", L"아르페지오 (1 - 5)", L"アルペジオ (1 - 5)"), 5);
        helperModeCombo.addItem(txt("Arp (1-5-High 1)", L"아르페지오 (1 - 5 - 높은음 1)", L"アルペジオ (1 - 5 - 高音1)"), 6);
        helperModeCombo.addItem(txt("Arp (1-5-High 3)", L"아르페지오 (1 - 5 - 높은음 3)", L"アルペジオ (1 - 5 - 高音3)"), 7);
        helperModeCombo.addItem(txt("Arp (1-5-H1-H3)", L"아르페지오 (1 - 5 - 높은음 1 - 높은음 3)", L"アルペジオ (1 - 5 - 高音1 - 高音3)"), 8);
        helperModeCombo.addItem(txt("Arp (1-5-H2-H3)", L"아르페지오 (1 - 5 - 높은음 2 - 높은음 3)", L"アルペジオ (1 - 5 - 高音2 - 高音3)"), 9);
        helperModeCombo.addItem(txt("Auto Bass", L"베이스 자동 (낮은음)", L"自動ベース (低音)"), 4);
        helperModeCombo.setSelectedId(currentHelperMode > 0 ? currentHelperMode : 1, juce::dontSendNotification);

        populateScaleCombo();
        scaleCombo.setSelectedId(currentScaleId > 0 ? currentScaleId : 1, juce::dontSendNotification);
        updateSignatureLabel();
        updateScaleControlsVisibility();

        helperModeCombo.onChange = [this]
        {
            const int mode = helperModeCombo.getSelectedId() > 0 ? helperModeCombo.getSelectedId() : 1;
            if (onHelperChanged)
                onHelperChanged(mode);
            updateScaleControlsVisibility();
        };

        scaleCombo.onChange = [this]
        {
            const int scaleId = scaleCombo.getSelectedId() > 0 ? scaleCombo.getSelectedId() : 1;
            if (onScaleChanged)
                onScaleChanged(scaleId);
            updateSignatureLabel();
        };

        detectButton.onClick = [this]
        {
            if (onDetectScaleClicked)
            {
                const int detected = onDetectScaleClicked();
                if (detected > 0)
                    scaleCombo.setSelectedId(detected, juce::dontSendNotification);
            }
            updateSignatureLabel();
        };

        setSize(430, 220);
    }

    void applyTheme(int themeId)
    {
        const int safeThemeId = juce::jlimit(1, 12, themeId);
        backgroundTop = CustomUI::getThemeColour(safeThemeId, "backgroundTop");
        backgroundBottom = CustomUI::getThemeColour(safeThemeId, "backgroundBottom");
        panel = CustomUI::getThemeColour(safeThemeId, "panel");
        panel2 = CustomUI::getThemeColour(safeThemeId, "panel2");
        accent = CustomUI::getThemeColour(safeThemeId, "accent");
        accent2 = CustomUI::getThemeColour(safeThemeId, "accent2");
        text = CustomUI::getThemeColour(safeThemeId, "text");
        mutedText = CustomUI::getThemeColour(safeThemeId, "mutedText");

        auto styleLabel = [this](juce::Label& label, bool muted = false)
        {
            label.setColour(juce::Label::textColourId, muted ? mutedText : text);
            label.setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);
        };

        auto styleCombo = [this](juce::ComboBox& combo)
        {
            const auto comboText = CustomUI::getReadableTextColour(panel2);
            combo.setColour(juce::ComboBox::backgroundColourId, panel2);
            combo.setColour(juce::ComboBox::textColourId, comboText);
            combo.setColour(juce::ComboBox::outlineColourId, accent.withAlpha(0.65f));
            combo.setColour(juce::ComboBox::buttonColourId, panel.brighter(0.12f));
            combo.setColour(juce::ComboBox::arrowColourId, accent);
            combo.setColour(juce::PopupMenu::backgroundColourId, panel);
            combo.setColour(juce::PopupMenu::textColourId, CustomUI::getReadableTextColour(panel));
            combo.setColour(juce::PopupMenu::highlightedBackgroundColourId, accent);
            combo.setColour(juce::PopupMenu::highlightedTextColourId, CustomUI::getReadableTextColour(accent));
        };

        const auto buttonBase = CustomUI::getThemeColour(safeThemeId, "button").brighter(0.08f);
        detectButton.setColour(juce::TextButton::buttonColourId, buttonBase);
        detectButton.setColour(juce::TextButton::buttonOnColourId, accent2);
        detectButton.setColour(juce::TextButton::textColourOffId, CustomUI::getReadableTextColour(buttonBase));
        detectButton.setColour(juce::TextButton::textColourOnId, CustomUI::getReadableTextColour(accent2));

        styleLabel(helperLabel);
        styleLabel(scaleLabel);
        styleLabel(keyLabel);
        styleLabel(signatureLabel, true);
        styleCombo(helperModeCombo);
        styleCombo(scaleCombo);
        repaint();
    }

    void paint(juce::Graphics& g) override
    {
        juce::ColourGradient bg(backgroundTop, 0.0f, 0.0f, backgroundBottom, 0.0f, static_cast<float>(getHeight()), false);
        g.setGradientFill(bg);
        g.fillRect(getLocalBounds().toFloat());

        auto r = getLocalBounds().reduced(10).toFloat();
        g.setColour(panel.withAlpha(0.92f));
        g.fillRoundedRectangle(r, 8.0f);
        g.setColour(accent.withAlpha(0.55f));
        g.drawRoundedRectangle(r, 8.0f, 1.2f);

        auto inner = r.reduced(1.5f);
        g.setColour(panel2.withAlpha(0.20f));
        g.drawRoundedRectangle(inner, 7.0f, 1.0f);
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced(18);
        area.removeFromTop(6);

        auto row = area.removeFromTop(30);
        helperLabel.setBounds(row.removeFromLeft(70));
        helperModeCombo.setBounds(row.reduced(4, 0));

        area.removeFromTop(12);
        row = area.removeFromTop(30);
        scaleLabel.setBounds(row.removeFromLeft(70));
        scaleCombo.setBounds(row.reduced(4, 0));

        area.removeFromTop(12);
        row = area.removeFromTop(30);
        keyLabel.setBounds(row.removeFromLeft(70));
        signatureLabel.setBounds(row.removeFromLeft(190).reduced(4, 0));
        detectButton.setBounds(row.removeFromLeft(110).reduced(4, 0));
    }

private:
    juce::Colour backgroundTop{ juce::Colour(0xff20253a) };
    juce::Colour backgroundBottom{ juce::Colour(0xff10131f) };
    juce::Colour panel{ juce::Colour(0xff2a2f45) };
    juce::Colour panel2{ juce::Colour(0xff161a2a) };
    juce::Colour accent{ juce::Colour(0xffffd166) };
    juce::Colour accent2{ juce::Colour(0xff78dcca) };
    juce::Colour text{ juce::Colour(0xfffff5d6) };
    juce::Colour mutedText{ juce::Colour(0xffc8b98f) };

    int languageId = 2;
    juce::Label helperLabel, scaleLabel, keyLabel, signatureLabel;
    juce::ComboBox helperModeCombo, scaleCombo;
    juce::TextButton detectButton;
    std::function<void(int)> onHelperChanged;
    std::function<void(int)> onScaleChanged;
    std::function<int()> onDetectScaleClicked;
    std::function<juce::String(int)> getScaleText;

    juce::String txt(const char* en, const wchar_t* ko, const wchar_t* ja = nullptr) const
    {
        if (languageId == 2) return juce::String(ko);
        if (languageId == 3 && ja != nullptr) return juce::String(ja);
        return juce::String(en);
    }

    bool modeUsesScale(int mode) const
    {
        return mode == 4 || mode == 3 || (mode >= 5 && mode <= 9);
    }

    void updateScaleControlsVisibility()
    {
        const bool show = modeUsesScale(helperModeCombo.getSelectedId());
        scaleLabel.setVisible(show);
        scaleCombo.setVisible(show);
        keyLabel.setVisible(show);
        signatureLabel.setVisible(show);
        detectButton.setVisible(show);
        repaint();
    }

    void updateSignatureLabel()
    {
        const int scaleId = scaleCombo.getSelectedId() > 0 ? scaleCombo.getSelectedId() : 1;
        signatureLabel.setText(getScaleText ? getScaleText(scaleId) : juce::String(), juce::dontSendNotification);
    }

    void populateScaleCombo()
    {
        scaleCombo.clear(juce::dontSendNotification);
        scaleCombo.addItem(txt("None", L"None (기본음)", L"なし (基本音)"), 1);
        scaleCombo.addSeparator();
        scaleCombo.addItem(txt("C Major (0)", L"C Major (0)", L"C長調 (0)"), 2); scaleCombo.addItem(txt("G Major (#1)", L"G Major (#1)", L"G長調 (#1)"), 3); scaleCombo.addItem(txt("D Major (#2)", L"D Major (#2)", L"D長調 (#2)"), 4); scaleCombo.addItem(txt("A Major (#3)", L"A Major (#3)", L"A長調 (#3)"), 5); scaleCombo.addItem(txt("E Major (#4)", L"E Major (#4)", L"E長調 (#4)"), 6); scaleCombo.addItem(txt("B Major (#5)", L"B Major (#5)", L"B長調 (#5)"), 7); scaleCombo.addItem(txt("F# Major (#6)", L"F# Major (#6)", L"F#長調 (#6)"), 8); scaleCombo.addItem(txt("C# Major (#7)", L"C# Major (#7)", L"C#長調 (#7)"), 9);
        scaleCombo.addItem(txt("F Major (b1)", L"F Major (b1)", L"F長調 (b1)"), 10); scaleCombo.addItem(txt("Bb Major (b2)", L"Bb Major (b2)", L"Bb長調 (b2)"), 11); scaleCombo.addItem(txt("Eb Major (b3)", L"Eb Major (b3)", L"Eb長調 (b3)"), 12); scaleCombo.addItem(txt("Ab Major (b4)", L"Ab Major (b4)", L"Ab長調 (b4)"), 13); scaleCombo.addItem(txt("Db Major (b5)", L"Db Major (b5)", L"Db長調 (b5)"), 14); scaleCombo.addItem(txt("Gb Major (b6)", L"Gb Major (b6)", L"Gb長調 (b6)"), 15); scaleCombo.addItem(txt("Cb Major (b7)", L"Cb Major (b7)", L"Cb長調 (b7)"), 16);
        scaleCombo.addSeparator();
        scaleCombo.addItem(txt("A Minor (0)", L"A Minor (0)", L"A短調 (0)"), 17); scaleCombo.addItem(txt("E Minor (#1)", L"E Minor (#1)", L"E短調 (#1)"), 18); scaleCombo.addItem(txt("B Minor (#2)", L"B Minor (#2)", L"B短調 (#2)"), 19); scaleCombo.addItem(txt("F# Minor (#3)", L"F# Minor (#3)", L"F#短調 (#3)"), 20); scaleCombo.addItem(txt("C# Minor (#4)", L"C# Minor (#4)", L"C#短調 (#4)"), 21); scaleCombo.addItem(txt("G# Minor (#5)", L"G# Minor (#5)", L"G#短調 (#5)"), 22); scaleCombo.addItem(txt("D# Minor (#6)", L"D# Minor (#6)", L"D#短調 (#6)"), 23); scaleCombo.addItem(txt("A# Minor (#7)", L"A# Minor (#7)", L"A#短調 (#7)"), 24);
        scaleCombo.addItem(txt("D Minor (b1)", L"D Minor (b1)", L"D短調 (b1)"), 25); scaleCombo.addItem(txt("G Minor (b2)", L"G Minor (b2)", L"G短調 (b2)"), 26); scaleCombo.addItem(txt("C Minor (b3)", L"C Minor (b3)", L"C短調 (b3)"), 27); scaleCombo.addItem(txt("F Minor (b4)", L"F Minor (b4)", L"F短調 (b4)"), 28); scaleCombo.addItem(txt("Bb Minor (b5)", L"Bb Minor (b5)", L"Bb短調 (b5)"), 29); scaleCombo.addItem(txt("Eb Minor (b6)", L"Eb Minor (b6)", L"Eb短調 (b6)"), 30); scaleCombo.addItem(txt("Ab Minor (b7)", L"Ab Minor (b7)", L"Ab短調 (b7)"), 31);
    }
};


class TimeSignaturePopupContent : public juce::Component
{
public:
    TimeSignaturePopupContent(int languageIdIn,
                              int currentTimeSignatureId,
                              std::function<void(int)> timeSignatureChanged)
        : languageId(languageIdIn), onTimeSignatureChanged(std::move(timeSignatureChanged))
    {
        titleLabel.setText(txt("Time Signature", L"박자 선택", L"拍子選択"), juce::dontSendNotification);
        titleLabel.setJustificationType(juce::Justification::centredLeft);
        addAndMakeVisible(titleLabel);

        addAndMakeVisible(timeSignatureCombo);
        populateTimeSignatureCombo(currentTimeSignatureId);
        timeSignatureCombo.onChange = [this]
        {
            const int id = timeSignatureCombo.getSelectedId();
            if (id > 0 && onTimeSignatureChanged)
                onTimeSignatureChanged(id);
        };

        setSize(360, 110);
    }

    void applyTheme(int themeId)
    {
        const int safeThemeId = juce::jlimit(1, 12, themeId);
        backgroundTop = CustomUI::getThemeColour(safeThemeId, "backgroundTop");
        backgroundBottom = CustomUI::getThemeColour(safeThemeId, "backgroundBottom");
        panel = CustomUI::getThemeColour(safeThemeId, "panel");
        panel2 = CustomUI::getThemeColour(safeThemeId, "panel2");
        accent = CustomUI::getThemeColour(safeThemeId, "accent");
        text = CustomUI::getThemeColour(safeThemeId, "text");

        titleLabel.setColour(juce::Label::textColourId, text);
        titleLabel.setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);

        const auto comboText = CustomUI::getReadableTextColour(panel2);
        timeSignatureCombo.setColour(juce::ComboBox::backgroundColourId, panel2);
        timeSignatureCombo.setColour(juce::ComboBox::textColourId, comboText);
        timeSignatureCombo.setColour(juce::ComboBox::outlineColourId, accent.withAlpha(0.65f));
        timeSignatureCombo.setColour(juce::ComboBox::buttonColourId, panel.brighter(0.12f));
        timeSignatureCombo.setColour(juce::ComboBox::arrowColourId, accent);
        timeSignatureCombo.setColour(juce::PopupMenu::backgroundColourId, panel);
        timeSignatureCombo.setColour(juce::PopupMenu::textColourId, CustomUI::getReadableTextColour(panel));
        timeSignatureCombo.setColour(juce::PopupMenu::highlightedBackgroundColourId, accent);
        timeSignatureCombo.setColour(juce::PopupMenu::highlightedTextColourId, CustomUI::getReadableTextColour(accent));
        repaint();
    }

    void paint(juce::Graphics& g) override
    {
        juce::ColourGradient bg(backgroundTop, 0.0f, 0.0f, backgroundBottom, 0.0f, static_cast<float>(getHeight()), false);
        g.setGradientFill(bg);
        g.fillRect(getLocalBounds().toFloat());

        auto r = getLocalBounds().reduced(10).toFloat();
        g.setColour(panel.withAlpha(0.92f));
        g.fillRoundedRectangle(r, 8.0f);
        g.setColour(accent.withAlpha(0.55f));
        g.drawRoundedRectangle(r, 8.0f, 1.2f);
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced(18);
        titleLabel.setBounds(area.removeFromTop(26));
        area.removeFromTop(8);
        timeSignatureCombo.setBounds(area.removeFromTop(30));
    }

private:
    int languageId = 2;
    juce::Label titleLabel;
    juce::ComboBox timeSignatureCombo;
    std::function<void(int)> onTimeSignatureChanged;

    juce::Colour backgroundTop{ juce::Colour(0xff20253a) };
    juce::Colour backgroundBottom{ juce::Colour(0xff10131f) };
    juce::Colour panel{ juce::Colour(0xff2a2f45) };
    juce::Colour panel2{ juce::Colour(0xff161a2a) };
    juce::Colour accent{ juce::Colour(0xffffd166) };
    juce::Colour text{ juce::Colour(0xfffff5d6) };

    juce::String txt(const char* en, const wchar_t* ko, const wchar_t* ja = nullptr) const
    {
        if (languageId == 2) return juce::String(ko);
        if (languageId == 3 && ja != nullptr) return juce::String(ja);
        return juce::String(en);
    }

    void populateTimeSignatureCombo(int currentId)
    {
        timeSignatureCombo.clear(juce::dontSendNotification);
        timeSignatureCombo.addSectionHeading(txt("Simple", L"단순박자", L"単純拍子"));
        timeSignatureCombo.addItem(txt("Simple 4/2", L"단순박자 4/2", L"単純拍子 4/2"), 101);
        timeSignatureCombo.addItem(txt("Simple 4/3", L"단순박자 4/3", L"単純拍子 4/3"), 102);
        timeSignatureCombo.addItem(txt("Simple 4/4", L"단순박자 4/4", L"単純拍子 4/4"), 1);
        timeSignatureCombo.addSeparator();
        timeSignatureCombo.addSectionHeading(txt("Compound", L"복합박자", L"複合拍子"));
        timeSignatureCombo.addItem(txt("Compound 6/8", L"복합박자 6/8", L"複合拍子 6/8"), 103);
        timeSignatureCombo.addItem(txt("Compound 9/8", L"복합박자 9/8", L"複合拍子 9/8"), 104);
        timeSignatureCombo.addItem(txt("Compound 12/8", L"복합박자 12/8", L"複合拍子 12/8"), 105);
        timeSignatureCombo.addSeparator();
        timeSignatureCombo.addSectionHeading(txt("Mixed", L"혼합박자", L"混合拍子"));
        timeSignatureCombo.addItem(txt("Mixed 5/4", L"혼합박자 5/4", L"混合拍子 5/4"), 106);
        timeSignatureCombo.addItem(txt("Mixed 7/4", L"혼합박자 7/4", L"混合拍子 7/4"), 107);
        timeSignatureCombo.addItem(txt("Mixed 8/4", L"혼합박자 8/4", L"混合拍子 8/4"), 108);
        timeSignatureCombo.addItem(txt("Mixed 9/8", L"혼합박자 9/8", L"混合拍子 9/8"), 109);
        timeSignatureCombo.addItem(txt("Mixed 11/4", L"혼합박자 11/4", L"混合拍子 11/4"), 110);
        timeSignatureCombo.setSelectedId(currentId > 0 ? currentId : 1, juce::dontSendNotification);
    }
};

class HelperModelessWindow : public juce::DocumentWindow
{
public:
    HelperModelessWindow(const juce::String& title, juce::Colour backgroundColour, std::function<void()> closeCallback)
        : juce::DocumentWindow(title, backgroundColour, juce::DocumentWindow::closeButton),
          onClose(std::move(closeCallback))
    {
        setUsingNativeTitleBar(true);
        setResizable(false, false);
        setAlwaysOnTop(true);
        setBroughtToFrontOnMouseClick(true);
    }

    void closeButtonPressed() override
    {
        setVisible(false);
        auto closeCopy = onClose;
        juce::MessageManager::callAsync([closeCopy]
        {
            if (closeCopy)
                closeCopy();
        });
    }

private:
    std::function<void()> onClose;
};

MainComponent::MainComponent()
{
    setWantsKeyboardFocus(true);
    formatManager.registerBasicFormats();

    {
        const auto exeFolder = juce::File::getSpecialLocation(juce::File::currentExecutableFile).getParentDirectory();
        juce::Array<juce::File> logoCandidates;
        logoCandidates.add(exeFolder.getChildFile("Assets").getChildFile("Atelier de Derstin.png"));
        logoCandidates.add(exeFolder.getChildFile("Atelier de Derstin.png"));
        logoCandidates.add(juce::File::getCurrentWorkingDirectory().getChildFile("Assets").getChildFile("Atelier de Derstin.png"));
        logoCandidates.add(juce::File::getCurrentWorkingDirectory().getChildFile("Atelier de Derstin.png"));
        logoCandidates.add(juce::File(R"(C:\Users\User\Documents\MabinogiMMLEditor\MmlEditor\Assets\Atelier de Derstin.png)"));

        for (const auto& candidate : logoCandidates) {
            if (candidate.existsAsFile()) {
                startupLoadingLogo = juce::ImageFileFormat::loadFrom(candidate);
                if (startupLoadingLogo.isValid()) break;
            }
        }
    }

    loadWorkLoadingImage();

    auto useThemedIconBorder = [](juce::TextButton& button)
    {
        button.setLookAndFeel(&getAtelierButtonBorderLookAndFeel());
    };

    useThemedIconBorder(playButton);
    useThemedIconBorder(stopButton);
    useThemedIconBorder(trackNameFromPresetButton);
    useThemedIconBorder(rewindButton);
    useThemedIconBorder(importButton);
    useThemedIconBorder(exportButton);
    useThemedIconBorder(openProjectButton);
    useThemedIconBorder(saveProjectButton);
    useThemedIconBorder(themeButton);
    useThemedIconBorder(licenceButton);
    useThemedIconBorder(screenSwitchButton);
    useThemedIconBorder(copyMabi3PartButton);
    useThemedIconBorder(helperButton);
    useThemedIconBorder(meterButton);
    useThemedIconBorder(optimizeButton);
    useThemedIconBorder(loadSampleBtn);
    useThemedIconBorder(detectScaleButton);
    useThemedIconBorder(addTrackButton);
    useThemedIconBorder(removeTrackButton);

    for (int i = 0; i < MAX_BANKS; ++i)
        useThemedIconBorder(tabButtons[i]);

    for (int i = 0; i < 4; ++i)
    {
        useThemedIconBorder(muteBtns[i]);
        useThemedIconBorder(soloBtns[i]);
    }

    addAndMakeVisible(pianoRoll);
    pianoRoll.setModel(banks, &numActiveTracks, &currentBankIndex, &cachedEventList, &tempoMap);
    pianoRoll.setMeterChanges(&meterChanges);
    pianoRoll.isPartActiveForBank = [this](int bankIdx, int partIdx) { return isPartActiveForBank(bankIdx, partIdx); };
    pianoRoll.isPartEditable = [this](int partIdx) {
        if (partIdx < 0 || partIdx >= 4 || !trackEditors[partIdx].isVisible()) return false;

        // 3mle 화면은 MML 텍스트 편집 전용이다.
        // 피아노롤에서 노트 이동/길이 조절은 mabbiico 화면에서만 허용한다.
        if (!isSubScreenVisible) return false;

        // mabbiico 화면에서는 현재 선택된 파트만 편집 대상으로 둔다.
        // 일반 악기: 멜로디/화음1/화음2 중 체크된 파트
        // 노래 프리셋: 노래 파트만
        return canEditMabbiicoPart(partIdx);
    };
    pianoRoll.onSeekBeat = [this](double beat) { seekToBeat(beat); };
    pianoRoll.onTimelineTempoRequested = [this](double beat) { showPianoRollTempoDialog(beat); };
    pianoRoll.onNoteDoubleClicked = [this](int partIdx, int noteIdx, int textStart, int textEnd) { handlePianoRollNoteDoubleClick(partIdx, noteIdx, textStart, textEnd); };
    pianoRoll.onNoteSelected = [this](int partIdx, int noteIdx) { handlePianoRollNoteSelected(partIdx, noteIdx); };
    pianoRoll.onNotesSelected = [this](const std::vector<std::pair<int, int>>& notes) { handlePianoRollNotesSelected(notes); };
    pianoRoll.onNoteEdited = [this](int partIdx, int noteIdx, int midiNote, double lengthBeats) { handlePianoRollNoteEdit(partIdx, noteIdx, midiNote, lengthBeats); };
    pianoRoll.onNoteInserted = [this](int partIdx, int midiNote, double startBeat, double lengthBeats) { handlePianoRollNoteInsert(partIdx, midiNote, startBeat, lengthBeats); };
    pianoRoll.onDeleteSelectedNote = [this] { deleteSelectedPianoRollNote(); };
    pianoRoll.onDeleteSelectedNotes = [this] { deleteSelectedPianoRollNotes(); };
    pianoRoll.onSelectedNotesMove = [this](int semitoneDelta, double beatDelta) { return moveSelectedPianoRollNotes(semitoneDelta, beatDelta); };
    pianoRoll.onMoveSnapshotRequested = [this] { beginMabbiicoMoveSnapshot(); };
    pianoRoll.onCopySelectedNotes = [this] { copySelectedPianoRollNotes(); };
    pianoRoll.onPasteCopiedNotes = [this] { pasteCopiedPianoRollNotes(); };
    pianoRoll.onUndoRequested = [this] { undoMabbiicoEdit(); };
    pianoRoll.onEditGestureStarted = [this] { beginMabbiicoUndoTransaction(); };
    pianoRoll.onEditGestureFinished = [this] { endMabbiicoUndoTransaction(); };
    pianoRoll.onPreviewMidiChanged = [this](int midiNote) { previewPianoRollMidi(midiNote); };
    pianoRoll.onHorizontalScrollRequested = [this](double value) { scrollX = value; horizontalScrollBar.setCurrentRangeStart(value); pianoRoll.setScroll(scrollX, scrollY, fixedRowHeight); repaint(); };
    pianoRoll.onVerticalScrollRequested = [this](double value) { scrollY = value; verticalScrollBar.setCurrentRangeStart(value); pianoRoll.setScroll(scrollX, scrollY, fixedRowHeight); repaint(); };
    pianoRoll.onTimelineZoomRequested = [this](float localMouseX, float wheelDeltaY) { zoomMabbiicoTimeline(localMouseX, wheelDeltaY); };
    pianoRoll.onHorizontalRangeChanged = [this](double totalWidthPixels, double visibleWidth) {
        const double maxScrollX = std::max(0.0, totalWidthPixels - visibleWidth);
        scrollX = juce::jlimit(0.0, maxScrollX, scrollX);
        horizontalScrollBar.setRangeLimits(0.0, totalWidthPixels);
        horizontalScrollBar.setCurrentRange(scrollX, visibleWidth);
    };

    addAndMakeVisible(blankSubScreen);
    addAndMakeVisible(bottomPanelSeparator);
    bottomPanelSeparator.setText({}, juce::dontSendNotification);
    bottomPanelSeparator.setInterceptsMouseClicks(false, false);
    blankSubScreen.setVisible(false);

    addAndMakeVisible(playButton); playButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff2d3e50));
    addAndMakeVisible(stopButton); stopButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff2d3e50));
    addAndMakeVisible(trackNameFromPresetButton); trackNameFromPresetButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff4f5f7f));
    addAndMakeVisible(rewindButton); rewindButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff2d3e50));

    addAndMakeVisible(importButton);
    importButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff8b4513));
    importButton.onClick = [this] { showLoadMenu(); };

    addAndMakeVisible(exportButton);
    exportButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgreen);
    exportButton.onClick = [this] { showSaveMenu(); };

    openProjectButton.onClick = [this] { loadDmmfProject(); };
    saveProjectButton.onClick = [this] { saveDmmfProject(); };

    addAndMakeVisible(themeButton);
    themeButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff4b3f72));
    themeButton.onClick = [this] { showThemeDialog(); };

    addAndMakeVisible(licenceButton);
    licenceButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff4b3f72));
    licenceButton.onClick = [this]
    {
        juce::String message;
        juce::String title;

        if (languageCombo.getSelectedId() == 2)
        {
            title = juce::String(L"라이선스 / 크레딧");
            message =
                juce::String(L"# Atelier de Derstin\n\n")
                + juce::String(L"C++와 JUCE로 제작한 마비노기 MML 편집 도구입니다.\n\n")
                + juce::String(L"## 라이선스\n\n")
                + juce::String(L"이 프로젝트는 AGPL-3.0-or-later 라이선스를 따릅니다.\n\n")
                + juce::String(L"서드파티 고지는 NOTICE.md에 정리되어 있습니다.\n\n")
                + juce::String(L"## 크레딧\n\n")
                + juce::String(L"이 프로젝트는 기존 마비노기 MML 도구와 리소스에서 영감을 받아 제작되었습니다.\n\n")
                + juce::String(L"- FurySound / Fury Sound Pack - Lisedrika, DovidicGJ\n")
                + juce::String(L"- MabiIcco / mmlTools - たんらる (fourthline)\n")
                + juce::String(L"- 3MLE / 3ML Editor - うに (Uni)\n")
                + juce::String(L"- JUCE Framework - Raw Material Software Limited\n\n")
                + juce::String(L"이 프로젝트는 독립 구현이며 NEXON 또는 Mabinogi와 제휴되어 있지 않습니다.");
        }
        else if (languageCombo.getSelectedId() == 3)
        {
            title = juce::String(L"ライセンス / クレジット");
            message =
                juce::String(L"# Atelier de Derstin\n\n")
                + juce::String(L"C++ と JUCE で制作された Mabinogi MML エディターツールです。\n\n")
                + juce::String(L"## ライセンス\n\n")
                + juce::String(L"このプロジェクトは AGPL-3.0-or-later ライセンスの下で公開されています。\n\n")
                + juce::String(L"サードパーティーに関する表記は NOTICE.md に記載されています。\n\n")
                + juce::String(L"## クレジット\n\n")
                + juce::String(L"このプロジェクトは、既存の Mabinogi MML ツールおよびリソースから着想を得て制作されました。\n\n")
                + juce::String(L"- FurySound / Fury Sound Pack - Lisedrika, DovidicGJ\n")
                + juce::String(L"- MabiIcco / mmlTools - たんらる (fourthline)\n")
                + juce::String(L"- 3MLE / 3ML Editor - うに (Uni)\n")
                + juce::String(L"- JUCE Framework - Raw Material Software Limited\n\n")
                + juce::String(L"このプロジェクトは独立した実装であり、NEXON または Mabinogi とは提携していません。");
        }
        else
        {
            title = "Licence / Credits";
            message =
                juce::String(L"# Atelier de Derstin\n\n")
                + juce::String(L"A Mabinogi MML editor tool built with C++ and JUCE.\n\n")
                + juce::String(L"## License\n\n")
                + juce::String(L"This project is licensed under AGPL-3.0-or-later.\n\n")
                + juce::String(L"Third-party notices are listed in NOTICE.md.\n\n")
                + juce::String(L"## Credits\n\n")
                + juce::String(L"This project was inspired by existing Mabinogi MML tools and resources:\n\n")
                + juce::String(L"- FurySound / Fury Sound Pack - Lisedrika, DovidicGJ\n")
                + juce::String(L"- MabiIcco / mmlTools - たんらる (fourthline)\n")
                + juce::String(L"- 3MLE / 3ML Editor - うに (Uni)\n")
                + juce::String(L"- JUCE Framework - Raw Material Software Limited\n\n")
                + juce::String(L"This project is an independent implementation and is not affiliated with NEXON or Mabinogi.");
        }

        const auto panel = CustomUI::getThemeColour(currentThemeId, "panel");
        const auto textColour = CustomUI::getReadableTextColour(panel);

        licencePopupWindow.reset(new AtelierCloseablePopupWindow(
            title,
            panel,
            [this] { licencePopupWindow.reset(); }));

        auto* content = new LicenceCreditsContent(currentThemeId, message);
        licencePopupWindow->setUsingNativeTitleBar(false);
        licencePopupWindow->setColour(juce::DocumentWindow::textColourId, textColour);
        licencePopupWindow->setColour(juce::ResizableWindow::backgroundColourId, panel);
        licencePopupWindow->setContentOwned(content, true);
        licencePopupWindow->setResizable(true, true);
        licencePopupWindow->setResizeLimits(520, 360, 900, 720);
        licencePopupWindow->centreAroundComponent(this, 660, 520);
        licencePopupWindow->setVisible(true);
        licencePopupWindow->toFront(true);
    };

    addAndMakeVisible(screenSwitchButton);
    screenSwitchButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff6a5acd));
    screenSwitchButton.onClick = [this] { setSubScreenMode(!isSubScreenVisible); };

    addAndMakeVisible(copyMabi3PartButton);
    copyMabi3PartButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff3f6b5f));
    copyMabi3PartButton.onClick = [this] { copyMabinogi3PartsToClipboard(); };

    addAndMakeVisible(helperButton);
    helperButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xffb88745));
    helperButton.onClick = [this] { showHelperMenu(); };

    addAndMakeVisible(meterButton);
    meterButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff6f5aa7));
    meterButton.onClick = [this] { showTimeSignatureWindow(); };

    addAndMakeVisible(optimizeButton);
    optimizeButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff7a4f9a));
    optimizeButton.onClick = [this] { optimizeCurrentBankMml(); };

    addChildComponent(timeSignatureCombo);
    populateTimeSignatureComboItems();
    setTimeSignatureId(1, juce::dontSendNotification);
    meterButton.setButtonText(T("Meter", L"박자"));

    // ★ 콤보박스 변경 시 즉각 아르페지오/베이스 재계산
    timeSignatureCombo.onChange = [this] {
        meterButton.setButtonText(T("Meter", L"박자"));
        markProjectDirty();
        int mode = helperCombo.getSelectedId();
        if (mode == 4 || mode == 3 || (mode >= 5 && mode <= 9)) {
            trackEditors[2].setText(MmlLogic::transformMML(trackEditors[0].getText(), mode, banks[currentBankIndex].autoBassScale, getTimeSignatureBeatsPerMeasure()), false);
            updateAllSequences(); refreshEditorTextColours();
        }
        refreshPianoRollModel();
        repaint();
    };

    addChildComponent(helperCombo);

    addAndMakeVisible(noteLengthLabel);
    noteLengthLabel.setColour(juce::Label::textColourId, juce::Colours::black);
    noteLengthLabel.setJustificationType(juce::Justification::centredRight);
    noteLengthLabel.setVisible(false);

    addAndMakeVisible(noteLengthCombo);
    noteLengthCombo.setVisible(false);
    noteLengthCombo.setSelectedId(4, juce::dontSendNotification);
    noteLengthCombo.onChange = [this] {
        if (suppressNoteLengthComboCallback) return;
        pianoRoll.setResizeQuantizeStepBeats(getSelectedNoteLengthBeats());
        if (isSubScreenVisible)
            applySelectedNoteLengthToPianoRollNote();
    };

    addAndMakeVisible(autoBassScaleCombo);
    addAndMakeVisible(autoBassScaleLabel);
    addAndMakeVisible(scaleSignatureLabel);
    addAndMakeVisible(detectScaleButton);
    detectScaleButton.onClick = [this] { detectScaleFromMelodyAndApply(); };
    autoBassScaleLabel.setColour(juce::Label::textColourId, juce::Colours::black);
    scaleSignatureLabel.setColour(juce::Label::textColourId, juce::Colours::black);
    scaleSignatureLabel.setJustificationType(juce::Justification::centredLeft);
    autoBassScaleLabel.setVisible(false); autoBassScaleCombo.setVisible(false); scaleSignatureLabel.setVisible(false); detectScaleButton.setVisible(false);

    addAndMakeVisible(trackInstrumentLabel); trackInstrumentLabel.setColour(juce::Label::textColourId, juce::Colours::black);
    addAndMakeVisible(trackInstrumentCombo);

    addAndMakeVisible(dlsPresetLabel); dlsPresetLabel.setColour(juce::Label::textColourId, juce::Colours::black);
    addAndMakeVisible(dlsPresetCombo);
    dlsPresetLabel.setVisible(false); dlsPresetCombo.setVisible(false);

    addAndMakeVisible(pcExcludeSongPartLimitToggle);
    pcExcludeSongPartLimitToggle.setVisible(false);
    pcExcludeSongPartLimitToggle.setClickingTogglesState(true);
    pcExcludeSongPartLimitToggle.onClick = [this]
    {
        banks[currentBankIndex].pcPresetExcludeSongPartLimit = pcExcludeSongPartLimitToggle.getToggleState();
        markProjectDirty();

        // When song part is excluded, the Song part must not remain selected/active in Mabiicco mode.
        if (pcExcludeSongPartLimitToggle.getToggleState() && activeMabbiicoPartIdx == 3)
            selectMabbiicoPart(0);

        updatePresetCombo();
        banks[currentBankIndex].songPresetMode = computeSongPresetModeForBank(currentBankIndex);
        banks[currentBankIndex].xylophonePresetMode = computeXylophonePresetModeForBank(currentBankIndex);

        updatePartEditorVisibility();
        updateMabbiicoEditorLineMode();
        updateAllSequences();
        refreshEditorTextColours();
        refreshPianoRollModel();
        updateMmlCharCountLabels();
        resized();
        repaint();
    };

    addAndMakeVisible(compositionRankGuideLabel);
    compositionRankGuideLabel.setJustificationType(juce::Justification::centredRight);
    compositionRankGuideLabel.setVisible(false);

    addAndMakeVisible(loadSampleBtn);
    loadSampleBtn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff18188b));
    loadSampleBtn.setVisible(false);

    addAndMakeVisible(languageCombo);
    languageCombo.addItem("English", 1);
    languageCombo.addItem(L"한국어", 2);
    languageCombo.addItem(L"日本語", 3);
    languageCombo.setSelectedId(2, juce::dontSendNotification);

    loadUserSettings();

    languageCombo.onChange = [this] {
        updateUITexts(); applyTheme(currentThemeId); saveUserSettings();
    };
    updateUITexts();

    autoBassScaleCombo.onChange = [this] {
        markProjectDirty();
        banks[currentBankIndex].autoBassScale = autoBassScaleCombo.getSelectedId();
        updateScaleSignatureLabel();
        int mode = helperCombo.getSelectedId();
        if (mode == 4 || mode == 3 || (mode >= 5 && mode <= 9)) {
            trackEditors[2].setText(MmlLogic::transformMML(trackEditors[0].getText(), mode, banks[currentBankIndex].autoBassScale, getTimeSignatureBeatsPerMeasure()), false);
            updateAllSequences(); refreshEditorTextColours();
        }
    };

    helperCombo.onChange = [this] {
        markProjectDirty();
        int mode = helperCombo.getSelectedId(); banks[currentBankIndex].helperMode = mode; applyHelperModeState(mode);
        const bool showScale = (mode == 4 || mode == 3 || (mode >= 5 && mode <= 9));
        autoBassScaleLabel.setVisible(false); autoBassScaleCombo.setVisible(false); scaleSignatureLabel.setVisible(false); detectScaleButton.setVisible(false); updateScaleSignatureLabel(); resized();
        juce::String t1 = trackEditors[0].getText();
        if (mode == 2) { trackEditors[1].setText(MmlLogic::transformMML(t1, 1, 1, 4), false); trackEditors[2].setText(MmlLogic::transformMML(t1, 2, 1, 4), false); }
        else if (mode == 3 || (mode >= 5 && mode <= 9)) { trackEditors[2].setText(MmlLogic::transformMML(t1, mode, banks[currentBankIndex].autoBassScale, getTimeSignatureBeatsPerMeasure()), false); }
        else if (mode == 4) { trackEditors[2].setText(MmlLogic::transformMML(t1, 4, banks[currentBankIndex].autoBassScale, getTimeSignatureBeatsPerMeasure()), false); }
        updateAllSequences(); refreshEditorTextColours();
    };

    trackInstrumentCombo.onChange = [this] {
        if (suppressPresetCallbacks) return;
        markProjectDirty();
        const int selectedInstrument = trackInstrumentCombo.getSelectedId(); banks[currentBankIndex].instrumentWave = selectedInstrument;
        const bool useMabi = isMabinogiPresetInstrument(selectedInstrument);
        const bool usePcMabi = selectedInstrument == 5;
        dlsPresetLabel.setVisible(useMabi); dlsPresetCombo.setVisible(useMabi); loadSampleBtn.setVisible(useMabi);
        pcExcludeSongPartLimitToggle.setVisible(usePcMabi);
        pcExcludeSongPartLimitToggle.setToggleState(usePcMabi && banks[currentBankIndex].pcPresetExcludeSongPartLimit, juce::dontSendNotification);
        if (useMabi)
            updatePresetCombo();
        else
            banks[currentBankIndex].songPresetMode = false;
        banks[currentBankIndex].songPresetMode = computeSongPresetModeForBank(currentBankIndex);
        banks[currentBankIndex].xylophonePresetMode = computeXylophonePresetModeForBank(currentBankIndex);
        if (banks[currentBankIndex].xylophonePresetMode && activeMabbiicoPartIdx != 0) selectMabbiicoPart(0);
        updatePartEditorVisibility(); updateAllSequences(); updateMmlCharCountLabels(); refreshEditorTextColours(); refreshPianoRollModel(); resized(); repaint();
    };

    dlsPresetCombo.onChange = [this] {
        if (suppressPresetCallbacks) return;
        markProjectDirty();
        const int selectedId = dlsPresetCombo.getSelectedId();
        if (selectedId > 0) { const int encoded = selectedId - 1; banks[currentBankIndex].sf2FileIndex = encoded / 10000; banks[currentBankIndex].dlsPreset = encoded % 10000; }
        banks[currentBankIndex].songPresetMode = computeSongPresetModeForBank(currentBankIndex);
        banks[currentBankIndex].xylophonePresetMode = computeXylophonePresetModeForBank(currentBankIndex);
        if (banks[currentBankIndex].xylophonePresetMode && activeMabbiicoPartIdx != 0) selectMabbiicoPart(0);
        updatePartEditorVisibility(); updateAllSequences(); updateMmlCharCountLabels(); refreshEditorTextColours(); refreshPianoRollModel(); resized(); repaint();
    };

    for (int i = 0; i < MAX_BANKS; ++i) {
        tabButtons[i].setTrackIndex(i);
        tabButtons[i].setRadioGroupId(1); tabButtons[i].setClickingTogglesState(true);
        tabButtons[i].setColour(juce::TextButton::buttonColourId, juce::Colour(0xff666666)); tabButtons[i].setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xff888888));
        addChildComponent(tabButtons[i]);
        tabButtons[i].onClick = [this, i] { switchBank(i); };
        tabButtons[i].onTabMouseDown = [this](int tabIndex, const juce::MouseEvent& event) -> bool
        {
            if (event.mods.isRightButtonDown())
            {
                showTrackContextMenu(tabIndex);
                return true;
            }

            return false;
        };
    }

    for (int i = 0; i < numActiveTracks; ++i) tabButtons[i].setVisible(true);
    tabButtons[0].setToggleState(true, juce::dontSendNotification);

    addAndMakeVisible(addTrackButton); addTrackButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff555555));
    addTrackButton.onClick = [this] { addNewTrack(); };

    addAndMakeVisible(removeTrackButton); removeTrackButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff555555));
    removeTrackButton.onClick = [this] { deleteTrackAtIndex(currentBankIndex); };

    const juce::File devMmlEditorFolder =
        juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
            .getChildFile("MabinogiMMLEditor")
            .getChildFile("MmlEditor");
    const juce::File pcDlsPathMemoFile = devMmlEditorFolder.getChildFile("MabinogiPresetDlsPath.txt");
    const juce::File mobileDlsPathMemoFile = devMmlEditorFolder.getChildFile("MabinogiMobilePresetDlsPath.txt");
    const juce::File legacyDlsPathMemoFile = devMmlEditorFolder.getChildFile("MabinogiDlsPath.txt");

    auto isDlsFile = [](const juce::File& file) -> bool {
        return file.existsAsFile() && file.hasFileExtension(".dls");
    };

    auto hasDlsFileInFolder = [](const juce::File& folder) -> bool {
        if (!folder.isDirectory()) return false;
        juce::Array<juce::File> files;
        folder.findChildFiles(files, juce::File::findFiles, false, "*.dls");
        return files.size() > 0;
    };

    auto normaliseDlsFolder = [hasDlsFileInFolder](const juce::File& selectedFolder) -> juce::File {
        if (hasDlsFileInFolder(selectedFolder))
            return selectedFolder;

        const auto childDlsFolder = selectedFolder.getChildFile("MabinogiDls");
        if (hasDlsFileInFolder(childDlsFolder))
            return childDlsFolder;

        const auto childDlsFolder2 = selectedFolder.getChildFile("DLS");
        if (hasDlsFileInFolder(childDlsFolder2))
            return childDlsFolder2;

        return selectedFolder;
    };

    auto addUniqueDlsFile = [](juce::Array<juce::File>& files, const juce::File& candidate) {
        if (!candidate.existsAsFile() || !candidate.hasFileExtension(".dls"))
            return;

        for (int i = 0; i < files.size(); ++i) {
            if (files.getReference(i).getFullPathName().equalsIgnoreCase(candidate.getFullPathName()))
                return;
        }
        files.add(candidate);
    };

    auto collectDlsFilesFromFolder = [addUniqueDlsFile](const juce::File& folder, juce::Array<juce::File>& foundFiles) {
        if (!folder.isDirectory()) return;

        juce::Array<juce::File> directFiles;
        folder.findChildFiles(directFiles, juce::File::findFiles, false, "*.dls");
        for (int i = 0; i < directFiles.size(); ++i)
            addUniqueDlsFile(foundFiles, directFiles.getReference(i));

        // 사용자가 게임 루트 폴더를 선택한 경우도 고려해 1회 선택 시에는 하위 폴더까지 찾는다.
        if (foundFiles.size() == 0) {
            juce::Array<juce::File> recursiveFiles;
            folder.findChildFiles(recursiveFiles, juce::File::findFiles, true, "*.dls");
            for (int i = 0; i < recursiveFiles.size(); ++i)
                addUniqueDlsFile(foundFiles, recursiveFiles.getReference(i));
        }
    };

    auto collectDlsFilesFromPath = [isDlsFile, normaliseDlsFolder, collectDlsFilesFromFolder, addUniqueDlsFile](const juce::File& path, juce::Array<juce::File>& foundFiles) {
        if (isDlsFile(path)) {
            addUniqueDlsFile(foundFiles, path);
            return;
        }

        if (path.isDirectory())
            collectDlsFilesFromFolder(normaliseDlsFolder(path), foundFiles);
    };

    auto loadDlsFilesFromPaths = [this, collectDlsFilesFromPath](const juce::Array<juce::File>& paths) -> int {
        juce::Array<juce::File> uniqueFiles;

        for (int i = 0; i < paths.size(); ++i)
            collectDlsFilesFromPath(paths.getReference(i), uniqueFiles);

        juce::String gainInfo;
        int skippedCount = 0;
        const int loadedCount = audioEngine.loadDlsFiles(uniqueFiles, true, gainInfo, skippedCount);

        banks[currentBankIndex].sf2FileIndex = 0;
        banks[currentBankIndex].dlsPreset = 0;
        refreshSongPresetModeCache();
        updatePresetCombo();
        if (dlsPresetCombo.getNumItems() > 0)
            dlsPresetCombo.setSelectedItemIndex(0, juce::dontSendNotification);

        return loadedCount;
    };

    auto saveDlsPathMemo = [](const juce::File& memoFile, const juce::Array<juce::File>& paths) {
        juce::String text;
        juce::StringArray uniquePathTexts;

        for (int i = 0; i < paths.size(); ++i)
        {
            const auto fullPath = paths.getReference(i).getFullPathName();
            if (fullPath.isEmpty())
                continue;

            if (!uniquePathTexts.contains(fullPath, true))
                uniquePathTexts.add(fullPath);
        }

        for (const auto& pathText : uniquePathTexts)
            text << pathText.quoted() << "\n";

        memoFile.getParentDirectory().createDirectory();
        memoFile.replaceWithText(text);
    };

    auto launchThemedDlsFileChooser =
        [this](const juce::String& title,
               const juce::File& startFolder,
               bool allowFolderSelection,
               std::function<void(juce::Array<juce::File>)> onChosen)
    {
        // Use JUCE's FileChooserDialogBox directly instead of FileChooser.
        // FileChooserDialogBox takes the outer window background colour in its constructor,
        // so the outside frame can be themed reliably.
        // Do not call applyTheme() here. Theme changes are handled live by the Theme dialog.

        const auto outerPanel = CustomUI::getThemeColour(currentThemeId, "panel");
        const auto innerPanel = CustomUI::getThemeColour(currentThemeId, "panel2");
        const auto accent = CustomUI::getThemeColour(currentThemeId, "accent");
        const auto buttonBase = CustomUI::getThemeColour(currentThemeId, "button").brighter(0.08f);
        const auto outerText = CustomUI::getReadableTextColour(outerPanel);
        const auto innerText = CustomUI::getReadableTextColour(innerPanel);

        auto* chooserLf = new juce::LookAndFeel_V4();
        auto& lf = *chooserLf;
        lf.setColour(juce::FileChooserDialogBox::titleTextColourId, outerText);
        lf.setColour(juce::ResizableWindow::backgroundColourId, outerPanel);
        lf.setColour(juce::DocumentWindow::backgroundColourId, outerPanel);
        lf.setColour(juce::DocumentWindow::textColourId, outerText);

        lf.setColour(juce::FileBrowserComponent::currentPathBoxBackgroundColourId, innerPanel);
        lf.setColour(juce::FileBrowserComponent::currentPathBoxTextColourId, innerText);
        lf.setColour(juce::FileBrowserComponent::currentPathBoxArrowColourId, accent);
        lf.setColour(juce::FileBrowserComponent::filenameBoxBackgroundColourId, innerPanel);
        lf.setColour(juce::FileBrowserComponent::filenameBoxTextColourId, innerText);

        lf.setColour(juce::DirectoryContentsDisplayComponent::highlightColourId, accent.withAlpha(0.70f));
        lf.setColour(juce::DirectoryContentsDisplayComponent::textColourId, innerText);
        lf.setColour(juce::DirectoryContentsDisplayComponent::highlightedTextColourId, CustomUI::getReadableTextColour(accent));

        lf.setColour(juce::ListBox::backgroundColourId, innerPanel);
        lf.setColour(juce::ListBox::outlineColourId, accent.withAlpha(0.65f));
        lf.setColour(juce::ListBox::textColourId, innerText);

        lf.setColour(juce::ComboBox::backgroundColourId, innerPanel);
        lf.setColour(juce::ComboBox::textColourId, innerText);
        lf.setColour(juce::ComboBox::outlineColourId, accent.withAlpha(0.65f));
        lf.setColour(juce::ComboBox::buttonColourId, buttonBase);
        lf.setColour(juce::ComboBox::arrowColourId, accent);

        lf.setColour(juce::TextEditor::backgroundColourId, innerPanel);
        lf.setColour(juce::TextEditor::textColourId, innerText);
        lf.setColour(juce::TextEditor::highlightColourId, accent.withAlpha(0.35f));
        lf.setColour(juce::TextEditor::highlightedTextColourId, CustomUI::getReadableTextColour(accent));
        lf.setColour(juce::TextEditor::outlineColourId, accent.withAlpha(0.55f));
        lf.setColour(juce::TextEditor::focusedOutlineColourId, accent);

        lf.setColour(juce::Label::textColourId, outerText);
        lf.setColour(juce::TextButton::buttonColourId, buttonBase);
        lf.setColour(juce::TextButton::buttonOnColourId, accent);
        lf.setColour(juce::TextButton::textColourOffId, CustomUI::getReadableTextColour(buttonBase));
        lf.setColour(juce::TextButton::textColourOnId, CustomUI::getReadableTextColour(accent));

        const int flags = juce::FileBrowserComponent::openMode
            | juce::FileBrowserComponent::canSelectFiles
            | juce::FileBrowserComponent::canSelectMultipleItems
            | (allowFolderSelection ? juce::FileBrowserComponent::canSelectDirectories : 0);

        auto* filter = new juce::WildcardFileFilter("*.dls", "*", T("DLS files", L"DLS 파일"));
        auto* browser = new juce::FileBrowserComponent(flags, startFolder, filter, nullptr);
        browser->setLookAndFeel(&lf);
        browser->setFilenameBoxLabel("file:");
        browser->setColour(juce::FileBrowserComponent::currentPathBoxBackgroundColourId, innerPanel);
        browser->setColour(juce::FileBrowserComponent::currentPathBoxTextColourId, innerText);
        browser->setColour(juce::FileBrowserComponent::currentPathBoxArrowColourId, accent);
        browser->setColour(juce::FileBrowserComponent::filenameBoxBackgroundColourId, innerPanel);
        browser->setColour(juce::FileBrowserComponent::filenameBoxTextColourId, innerText);

        auto* dialog = new juce::FileChooserDialogBox(
            title,
            juce::String(),
            *browser,
            false,
            outerPanel,
            this);

        dialog->setLookAndFeel(&lf);
        dialog->setColour(juce::ResizableWindow::backgroundColourId, outerPanel);
        dialog->setColour(juce::DocumentWindow::backgroundColourId, outerPanel);
        dialog->setColour(juce::DocumentWindow::textColourId, outerText);
        dialog->setColour(juce::FileChooserDialogBox::titleTextColourId, outerText);
        dialog->setAlwaysOnTop(true);
        dialog->centreWithDefaultSize(this);

        dialog->enterModalState(true,
            juce::ModalCallbackFunction::create(
                [browser, filter, dialog, chooserLf, onChosen](int result) mutable
                {
                    if (result != 0)
                    {
                        juce::Array<juce::File> selectedFiles;
                        const int selectedCount = browser->getNumSelectedFiles();

                        for (int i = 0; i < selectedCount; ++i)
                        {
                            const auto file = browser->getSelectedFile(i);
                            if (file != juce::File())
                                selectedFiles.add(file);
                        }

                        if (selectedFiles.size() == 0)
                        {
                            const auto highlighted = browser->getHighlightedFile();
                            if (highlighted != juce::File())
                                selectedFiles.add(highlighted);
                        }

                        if (selectedFiles.size() > 0 && onChosen)
                            onChosen(selectedFiles);
                    }

                    dialog->setLookAndFeel(nullptr);
                    browser->setLookAndFeel(nullptr);

                    delete dialog;
                    delete browser;
                    delete filter;
                    delete chooserLf;
                }),
            false);
    };

    auto autoLoadDlsFiles = [this, devMmlEditorFolder, pcDlsPathMemoFile, mobileDlsPathMemoFile, legacyDlsPathMemoFile, loadDlsFilesFromPaths, saveDlsPathMemo, launchThemedDlsFileChooser](bool showAlert) -> int {
        const auto exeBaseFolder = juce::File::getSpecialLocation(juce::File::currentExecutableFile).getParentDirectory();
        const juce::File exeDlsFolder = exeBaseFolder.getChildFile("MabinogiPresetDls");
        const juce::File exeMobileDlsFolder = exeBaseFolder.getChildFile("MabinogiMobilePresetDls");
        const juce::File devDlsFolder = devMmlEditorFolder.getChildFile("MabinogiPresetDls");
        const juce::File devMobileDlsFolder = devMmlEditorFolder.getChildFile("MabinogiMobilePresetDls");
        const juce::File defaultMabinogiGameDlsFolder(R"(C:\Nexon\Mabinogi\mp3)");
        exeDlsFolder.createDirectory();
        exeMobileDlsFolder.createDirectory();
        devDlsFolder.createDirectory();
        devMobileDlsFolder.createDirectory();

        juce::Array<juce::File> candidatePaths;
        auto addCandidatePath = [&candidatePaths](const juce::File& path) {
            for (int i = 0; i < candidatePaths.size(); ++i) {
                if (candidatePaths.getReference(i).getFullPathName().equalsIgnoreCase(path.getFullPathName()))
                    return;
            }
            candidatePaths.add(path);
        };

        auto addSavedMemoPaths = [&addCandidatePath](const juce::File& memoFile) {
            if (!memoFile.existsAsFile())
                return;

            juce::StringArray savedLines;
            savedLines.addLines(memoFile.loadFileAsString());
            for (int i = 0; i < savedLines.size(); ++i) {
                const auto savedPath = savedLines[i].trim().unquoted();
                if (savedPath.isNotEmpty())
                    addCandidatePath(juce::File(savedPath));
            }
        };

        addSavedMemoPaths(pcDlsPathMemoFile);
        addSavedMemoPaths(mobileDlsPathMemoFile);
        addSavedMemoPaths(legacyDlsPathMemoFile);

        // Default Mabinogi PC installation path.
        // Only paths are added here. DLS files are NOT bundled with this application.
        // The actual files are loaded at runtime from the user's local game folder.
        addCandidatePath(defaultMabinogiGameDlsFolder.getChildFile("MSXspirit01.dls"));
        addCandidatePath(defaultMabinogiGameDlsFolder.getChildFile("MSXspirit02.dls"));
        addCandidatePath(defaultMabinogiGameDlsFolder.getChildFile("MSXspirit03.dls"));
        addCandidatePath(defaultMabinogiGameDlsFolder.getChildFile("MSXspirit04.dls"));
        addCandidatePath(defaultMabinogiGameDlsFolder.getChildFile("MSXspirit05.dls"));
        addCandidatePath(defaultMabinogiGameDlsFolder);

        addCandidatePath(exeDlsFolder);
        addCandidatePath(devDlsFolder);
        addCandidatePath(exeMobileDlsFolder);
        addCandidatePath(devMobileDlsFolder);

        const int loadedCount = loadDlsFilesFromPaths(candidatePaths);

        if (showAlert) {
            if (loadedCount > 0) {
                juce::String msg = juce::String(loadedCount) + T(" DLS files loaded.", L"개의 DLS 파일을 로드했습니다.");
                showThemedMessageBoxAsync(juce::AlertWindow::InfoIcon, T("DLS Auto Load", L"DLS 자동 불러오기"), msg);
            }
            else {
                launchThemedDlsFileChooser(
                    T("DLS files not found. Please choose DLS files again.", L"DLS 파일을 찾지 못했습니다. DLS 파일을 다시 선택해주세요."),
                    devDlsFolder,
                    false,
                    [this, loadDlsFilesFromPaths, saveDlsPathMemo, pcDlsPathMemoFile](juce::Array<juce::File> selectedFiles) mutable
                    {
                        if (selectedFiles.size() == 0)
                            return;

                        saveDlsPathMemo(pcDlsPathMemoFile, selectedFiles);

                        const int reloadedCount = loadDlsFilesFromPaths(selectedFiles);

                        if (reloadedCount > 0) {
                            juce::String msg = juce::String(reloadedCount) + T(" DLS files loaded.", L"개의 DLS 파일을 로드했습니다.");
                            showThemedMessageBoxAsync(juce::AlertWindow::InfoIcon, T("DLS Auto Load", L"DLS 자동 불러오기"), msg);
                        }
                        else {
                            showThemedMessageBoxAsync(juce::AlertWindow::WarningIcon,
                                T("DLS Auto Load Failed", L"DLS 자동 불러오기 실패"),
                                T("No DLS files found.\nPlease choose DLS files again.", L"DLS 파일을 찾지 못했습니다.\nDLS 파일을 다시 선택해주세요."));
                        }
                    });
            }
        }

        return loadedCount;
    };

    loadSampleBtn.onClick = [this, autoLoadDlsFiles, saveDlsPathMemo, pcDlsPathMemoFile, mobileDlsPathMemoFile, devMmlEditorFolder, launchThemedDlsFileChooser] {
        juce::PopupMenu menu;
        menu.addItem(1, T("Mabinogi Preset Path...", L"마비노기 프리셋 경로 설정"));
        menu.addItem(2, T("Mabinogi Mobile Preset Path...", L"마비노기 모바일 프리셋 경로 설정"));
        menu.addSeparator();
        menu.addItem(3, T("Reload saved DLS paths", L"저장된 DLS 경로 다시 불러오기"));

        menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(loadSampleBtn),
            [this, autoLoadDlsFiles, saveDlsPathMemo, pcDlsPathMemoFile, mobileDlsPathMemoFile, devMmlEditorFolder, launchThemedDlsFileChooser](int result) mutable
            {
                if (result == 0)
                    return;

                if (result == 3)
                {
                    showWorkLoadingOverlay(T("Reloading DLS files...", L"DLS 파일을 다시 불러오는 중..."));
                    juce::Timer::callAfterDelay(80, [this, autoLoadDlsFiles]() mutable {
                        autoLoadDlsFiles(true);
                        hideWorkLoadingOverlay();
                    });
                    return;
                }

                const bool mobilePreset = (result == 2);
                const juce::File memoFile = mobilePreset ? mobileDlsPathMemoFile : pcDlsPathMemoFile;
                const juce::File defaultFolder = mobilePreset
                    ? devMmlEditorFolder.getChildFile("MabinogiMobilePresetDls")
                    : devMmlEditorFolder.getChildFile("MabinogiPresetDls");

                defaultFolder.createDirectory();

                launchThemedDlsFileChooser(
                    mobilePreset
                        ? T("Choose Mabinogi Mobile preset DLS file or folder", L"마비노기 모바일 프리셋 DLS 파일 또는 폴더 선택")
                        : T("Choose Mabinogi preset DLS file or folder", L"마비노기 프리셋 DLS 파일 또는 폴더 선택"),
                    defaultFolder,
                    true,
                    [this, autoLoadDlsFiles, saveDlsPathMemo, memoFile, mobilePreset](juce::Array<juce::File> selectedPaths) mutable
                    {
                        if (selectedPaths.size() == 0)
                            return;

                        saveDlsPathMemo(memoFile, selectedPaths);

                        showWorkLoadingOverlay(mobilePreset
                            ? T("Loading Mabinogi Mobile preset DLS...", L"마비노기 모바일 프리셋 DLS를 불러오는 중...")
                            : T("Loading Mabinogi preset DLS...", L"마비노기 프리셋 DLS를 불러오는 중..."));

                        juce::Timer::callAfterDelay(80, [this, autoLoadDlsFiles]() mutable {
                            autoLoadDlsFiles(true);
                            hideWorkLoadingOverlay();
                        });
                    });
            });
    };

    auto setupTrackUI = [this](juce::Label& lbl, juce::Label& countLbl, juce::TextEditor& ed, juce::TextButton& m, juce::TextButton& s) {
        addAndMakeVisible(lbl); lbl.setColour(juce::Label::textColourId, juce::Colours::black); lbl.setJustificationType(juce::Justification::centredLeft);
        addAndMakeVisible(countLbl); countLbl.setColour(juce::Label::textColourId, juce::Colours::darkgrey); countLbl.setJustificationType(juce::Justification::centredRight); countLbl.setFont(juce::Font(juce::FontOptions(11.0f)));
        addAndMakeVisible(ed); ed.setMultiLine(true); ed.setReturnKeyStartsNewLine(false); ed.setColour(juce::TextEditor::backgroundColourId, juce::Colours::white); ed.setColour(juce::TextEditor::textColourId, juce::Colours::black);
        addAndMakeVisible(m); m.setButtonText("M"); m.setColour(juce::TextButton::buttonColourId, juce::Colour(0xffeeeeee)); m.setColour(juce::TextButton::textColourOffId, juce::Colours::black); m.setClickingTogglesState(true);
        addAndMakeVisible(s); s.setButtonText("S"); s.setColour(juce::TextButton::buttonColourId, juce::Colour(0xffeeeeee)); s.setColour(juce::TextButton::textColourOffId, juce::Colours::black); s.setClickingTogglesState(true);
    };

    for (int i = 0; i < 4; ++i) {
        setupTrackUI(trackLabels[i], trackCountLabels[i], trackEditors[i], muteBtns[i], soloBtns[i]);

        if (i < 3) {
            addAndMakeVisible(subPartCheckBoxes[i]);
            subPartCheckBoxes[i].setClickingTogglesState(true);
            subPartCheckBoxes[i].setToggleState(i == 0, juce::dontSendNotification);
            subPartCheckBoxes[i].setVisible(false);
            subPartCheckBoxes[i].onClick = [this, i]
            {
                if (suppressSubPartCheckBoxCallback) return;
                selectMabbiicoPart(i);
            };
        }

        trackEditors[i].onTextChange = [this, i] {
            markProjectDirty();
            if (i == 0) {
                sanitizeNewLines(trackEditors[0]);
                int mode = helperCombo.getSelectedId();
                juce::String t1 = trackEditors[0].getText();
                if (mode == 2) { trackEditors[1].setText(MmlLogic::transformMML(t1, 1, 1, 4), false); trackEditors[2].setText(MmlLogic::transformMML(t1, 2, 1, 4), false); }
                else if (mode == 3 || (mode >= 5 && mode <= 9)) { trackEditors[2].setText(MmlLogic::transformMML(t1, mode, banks[currentBankIndex].autoBassScale, getTimeSignatureBeatsPerMeasure()), false); }
                else if (mode == 4) { trackEditors[2].setText(MmlLogic::transformMML(t1, 4, banks[currentBankIndex].autoBassScale, getTimeSignatureBeatsPerMeasure()), false); }
            }
            updateAllSequences();
            updateMmlCharCountLabels();
            refreshEditorTextColours();
        };

        muteBtns[i].onClick = [this, i] {
            markProjectDirty();
            bool m = muteBtns[i].getToggleState();
            for (int j = 0; j < 4; ++j) {
                banks[currentBankIndex].tracks[j].mute = m;
                muteBtns[j].setToggleState(m, juce::dontSendNotification);
                if (m) { banks[currentBankIndex].tracks[j].solo = false; soloBtns[j].setToggleState(false, juce::dontSendNotification); }
            }
        };
        soloBtns[i].onClick = [this, i] {
            markProjectDirty();
            bool s = soloBtns[i].getToggleState();
            for (int j = 0; j < 4; ++j) {
                banks[currentBankIndex].tracks[j].solo = s;
                soloBtns[j].setToggleState(s, juce::dontSendNotification);
                if (s) { banks[currentBankIndex].tracks[j].mute = false; muteBtns[j].setToggleState(false, juce::dontSendNotification); }
            }
        };
    }

    for (int i = 0; i < MAX_BANKS; ++i) {
        banks[i].instrumentWave = 5; banks[i].helperMode = 1; banks[i].autoBassScale = 1;
        banks[i].sf2FileIndex = 0; banks[i].dlsPreset = 0; banks[i].songPresetMode = false; banks[i].xylophonePresetMode = false; banks[i].mmiSongPartWithProgram = false;
        for (int j = 0; j < 4; ++j) { banks[i].tracks[j].mute = false; banks[i].tracks[j].solo = false; }
    }

    loadBank(0);
    updateMmlCharCountLabels();
    refreshPianoRollModel();
    addAndMakeVisible(verticalScrollBar); verticalScrollBar.addListener(this); addAndMakeVisible(horizontalScrollBar); horizontalScrollBar.addListener(this);

    playButton.onClick = [this] {
        // 피아노롤 미리듣기 음이 남아 있으면 재생 시작 시 퍽/버벅 소리가 날 수 있으므로 먼저 정리한다.
        stopPianoRollPreviewNote();
        clearPlaybackEditorHighlights(); isPlaying = false; updateAllSequences(); bool hasAnyNotes = false;
        {
            const juce::ScopedLock sl(audioEngine.getLock()); audioEngine.stopAllNotes();
            for (int i = 0; i < numActiveTracks; ++i) {
                for (int j = 0; j < 4; ++j) {
                    banks[i].tracks[j].noteIndex = 0;
                    while (banks[i].tracks[j].noteIndex < banks[i].tracks[j].sequence.size() && globalSampleCount >= banks[i].tracks[j].sequence[banks[i].tracks[j].noteIndex].endSample) { banks[i].tracks[j].noteIndex++; }
                    banks[i].tracks[j].currentAngle = 0.0;
                    if (isPartActiveForBank(i, j) && !banks[i].tracks[j].sequence.empty()) hasAnyNotes = true;
                }
            }
            if (hasAnyNotes) isPlaying = true;
        }
        updatePlaybackEditorHighlights(); grabKeyboardFocus();
    };

    stopButton.onClick = [this] { stopPianoRollPreviewNote(); { const juce::ScopedLock sl(audioEngine.getLock()); isPlaying = false; audioEngine.stopAllNotes(); } clearPlaybackEditorHighlights(); };
    trackNameFromPresetButton.onClick = [this] { toggleTrackNamesFromPresets(); };
    rewindButton.onClick = [this] {
        {
            const juce::ScopedLock sl(audioEngine.getLock()); globalSampleCount = 0; scrollX = 0; horizontalScrollBar.setCurrentRangeStart(0); audioEngine.stopAllNotes();
            for (int i = 0; i < MAX_BANKS; ++i) { for (int j = 0; j < 4; ++j) banks[i].tracks[j].noteIndex = 0; }
        }
        clearPlaybackEditorHighlights(); repaint();
    };

    exportButton.onClick = [this] { showSaveMenu(); };
    setSize(1280, 800); setAudioChannels(0, 2); startTimerHz(30); applyTheme(currentThemeId);
    juce::MessageManager::callAsync([this] { updateWindowTitle(); }); setMainUiVisible(false);

    isStartupLoading = true; startupLoadingText = T("Loading Mabinogi presets...", L"마비노기 프리셋을 불러오는 중..."); repaint();
    juce::Timer::callAfterDelay(250, [this, autoLoadDlsFiles]() mutable {
        if (this == nullptr) return;
        isStartupLoading = true; setMainUiVisible(false); startupLoadingText = T("Loading DLS files...", L"DLS 파일을 불러오는 중..."); repaint();
        autoLoadDlsFiles(false);
        startupLoadingText = T("Ready", L"준비 완료"); isStartupLoading = false; setMainUiVisible(true); markProjectClean(); resized(); repaint();
    });
    setName(getWindowTitleText());
    juce::Component::SafePointer<MainComponent> safeThis(this);
    juce::Timer::callAfterDelay(120, [safeThis]() { if (safeThis != nullptr) safeThis->updateWindowTitle(); });
}

MainComponent::~MainComponent()
{
    auto clearThemedIconBorder = [](juce::TextButton& button)
    {
        button.setLookAndFeel(nullptr);
    };

    clearThemedIconBorder(playButton);
    clearThemedIconBorder(stopButton);
    clearThemedIconBorder(trackNameFromPresetButton);
    clearThemedIconBorder(rewindButton);
    clearThemedIconBorder(importButton);
    clearThemedIconBorder(exportButton);
    clearThemedIconBorder(openProjectButton);
    clearThemedIconBorder(saveProjectButton);
    clearThemedIconBorder(themeButton);
    clearThemedIconBorder(licenceButton);
    clearThemedIconBorder(screenSwitchButton);
    clearThemedIconBorder(copyMabi3PartButton);
    clearThemedIconBorder(helperButton);
    clearThemedIconBorder(meterButton);
    clearThemedIconBorder(optimizeButton);
    clearThemedIconBorder(loadSampleBtn);
    clearThemedIconBorder(detectScaleButton);
    clearThemedIconBorder(addTrackButton);
    clearThemedIconBorder(removeTrackButton);

    for (int i = 0; i < MAX_BANKS; ++i)
        clearThemedIconBorder(tabButtons[i]);

    for (int i = 0; i < 4; ++i)
    {
        clearThemedIconBorder(muteBtns[i]);
        clearThemedIconBorder(soloBtns[i]);
    }

    helperPopupWindow.reset();
    meterPopupWindow.reset();
    licencePopupWindow.reset();
    stopTimer();
    shutdownAudio();
}

juce::String MainComponent::getWindowTitleText() const {
    const auto title = currentProjectTitle.trim(); const juce::String dirtyMark = hasUnsavedChanges ? "* " : "";
    if (title.isEmpty() || title.equalsIgnoreCase("Atelier de Derstin")) return dirtyMark + "Atelier de Derstin";
    return dirtyMark + title + " - Atelier de Derstin";
}

juce::String MainComponent::getSafeProjectTitleForFile() const {
    auto title = currentProjectTitle.trim(); if (title.isEmpty()) title = "Atelier de Derstin";
    title = title.removeCharacters(R"(\/:*?"<>|)").trim(); if (title.isEmpty()) title = "Atelier de Derstin"; return title;
}

void MainComponent::updateWindowTitle() {
    const auto title = getWindowTitleText();
    auto applyNativeTitle = [title](juce::Component* target) -> bool {
        if (target == nullptr) return false; target->setName(title);
        if (auto* peer = target->getPeer()) { peer->setTitle(title); target->repaint(); return true; }
        target->repaint(); return false;
    };
    if (auto* window = findParentComponentOfClass<juce::DocumentWindow>()) { if (applyNativeTitle(window)) return; }
    if (auto* top = getTopLevelComponent()) { if (applyNativeTitle(top)) return; }
    juce::Component::SafePointer<MainComponent> safeThis(this);
    juce::Timer::callAfterDelay(80, [safeThis]() {
        if (safeThis != nullptr) {
            const auto delayedTitle = safeThis->getWindowTitleText();
            if (auto* window = safeThis->findParentComponentOfClass<juce::DocumentWindow>()) { window->setName(delayedTitle); if (auto* peer = window->getPeer()) peer->setTitle(delayedTitle); window->repaint(); return; }
            if (auto* top = safeThis->getTopLevelComponent()) { top->setName(delayedTitle); if (auto* peer = top->getPeer()) peer->setTitle(delayedTitle); top->repaint(); }
        }
    });
}

void MainComponent::setCurrentProjectTitle(const juce::String& title) { auto cleaned = title.trim(); if (cleaned.isEmpty()) cleaned = "Atelier de Derstin"; currentProjectTitle = cleaned; updateWindowTitle(); }
void MainComponent::setCurrentProjectTitleFromFile(const juce::File& file) { if (file != juce::File()) setCurrentProjectTitle(file.getFileNameWithoutExtension()); }
bool MainComponent::isProjectDirty() const { return hasUnsavedChanges; }
void MainComponent::markProjectDirty() { if (suppressDirtyTracking) return; if (!hasUnsavedChanges) { hasUnsavedChanges = true; updateWindowTitle(); } }
void MainComponent::markProjectClean() { hasUnsavedChanges = false; updateWindowTitle(); }

void MainComponent::quitApplicationNow() {
    closePromptVisible = false; pendingCloseAfterSave = false;
    { const juce::ScopedLock sl(audioEngine.getLock()); isPlaying = false; audioEngine.stopAllNotes(); }
    clearPlaybackEditorHighlights(); juce::JUCEApplicationBase::quit();
}

void MainComponent::requestCloseWithSavePrompt() {
    saveCurrentBank(); if (!hasUnsavedChanges) { quitApplicationNow(); return; } if (closePromptVisible) return; closePromptVisible = true;
    auto* alert = new juce::AlertWindow(T("Save changes?", L"저장하시겠습니까?"), T("This project has unsaved changes. Save before closing?", L"저장하지 않은 변경사항이 있습니다. 종료하기 전에 저장할까요?"), juce::AlertWindow::QuestionIcon);
    const auto panel = CustomUI::getThemeColour(currentThemeId, "panel"); const auto panel2 = CustomUI::getThemeColour(currentThemeId, "panel2"); const auto text = CustomUI::getReadableTextColour(panel); const auto accent = CustomUI::getThemeColour(currentThemeId, "accent"); const auto button = CustomUI::getThemeColour(currentThemeId, "button");
    alert->setColour(juce::AlertWindow::backgroundColourId, panel); alert->setColour(juce::AlertWindow::textColourId, text); alert->setColour(juce::AlertWindow::outlineColourId, accent); alert->setColour(juce::TextButton::buttonColourId, button); alert->setColour(juce::TextButton::textColourOffId, CustomUI::getReadableTextColour(button)); alert->setColour(juce::ComboBox::backgroundColourId, panel2);
    alert->addButton(T("Yes", L"예"), 1); alert->addButton(T("No", L"아니오"), 2); alert->addButton(T("Cancel", L"취소"), 0, juce::KeyPress(juce::KeyPress::escapeKey));
    alert->enterModalState(true, juce::ModalCallbackFunction::create([this](int result) { closePromptVisible = false; if (result == 1) { pendingCloseAfterSave = true; saveDmmfProject(); } else if (result == 2) { markProjectClean(); quitApplicationNow(); } }), true);
}

juce::String MainComponent::T(const juce::String& en, const juce::String& ko) const
{
    const int lang = languageCombo.getSelectedId();
    if (lang == 2)
        return ko;

    if (lang == 3)
    {
        struct JaEntry { const char* en; const wchar_t* ja; };
        static const JaEntry jaMap[] = {
            { "Meter", L"拍子" },
            { " DLS files loaded.", L" 個のDLSファイルを読み込みました。" },
            { "DLS Auto Load", L"DLS自動読み込み" },
            { "DLS files not found. Please choose DLS files again.", L"DLSファイルが見つかりません。DLSファイルを再選択してください。" },
            { "DLS Auto Load Failed", L"DLS自動読み込み失敗" },
            { "No DLS files found.\nPlease choose DLS files again.", L"DLSファイルが見つかりません。\nDLSファイルを再選択してください。" },
            { "Reloading DLS files...", L"DLSファイルを再読み込み中..." },
            { "DLS Settings", L"DLS設定" },
            { "Mabinogi Preset Path...", L"マビノギプリセットのパス設定" },
            { "Mabinogi Mobile Preset Path...", L"マビノギモバイルプリセットのパス設定" },
            { "Reload saved DLS paths", L"保存済みDLSパスを再読み込み" },
            { "Choose Mabinogi preset DLS file or folder", L"マビノギプリセットDLSファイルまたはフォルダーを選択" },
            { "Choose Mabinogi Mobile preset DLS file or folder", L"マビノギモバイルプリセットDLSファイルまたはフォルダーを選択" },
            { "Loading Mabinogi preset DLS...", L"マビノギプリセットDLSを読み込み中..." },
            { "Loading Mabinogi Mobile preset DLS...", L"マビノギモバイルプリセットDLSを読み込み中..." },
            { "Loading Mabinogi presets...", L"マビノギプリセットを読み込み中..." },
            { "Loading DLS files...", L"DLSファイルを読み込み中..." },
            { "Ready", L"準備完了" },
            { "Save changes?", L"変更を保存しますか？" },
            { "This project has unsaved changes. Save before closing?", L"未保存の変更があります。終了前に保存しますか？" },
            { "Yes", L"はい" },
            { "No", L"いいえ" },
            { "Cancel", L"キャンセル" },
            { "Play", L"再生" },
            { "Stop", L"停止" },
            { "Rewind (|<)", L"先頭へ (|<)" },
            { "Name Tracks by Instrument", L"楽器名で表示" },
            { "Show Track Numbers", L"トラック番号で表示" },
            { "Load", L"読み込み" },
            { "Save", L"保存" },
            { "Open DMMF", L"DMMFを開く" },
            { "Save DMMF", L"DMMF保存" },
            { "Theme", L"テーマ" },
            { "Copy Tracks", L"トラックコピー" },
            { "Helper", L"ヘルパー" },
            { "Reload", L"DLS再読み込み" },
            { "Melody", L"メロディ" },
            { "Chord 1", L"和音 1" },
            { "Chord 2", L"和音 2" },
            { "Song", L"歌" },
            { "Instrument: ", L"楽器: " },
            { "Scale: ", L"スケール: " },
            { "Preset: ", L"プリセット: " },
            { "Note:", L"音符:" },
            { "Detect Key", L"調号読取" },
            { "Helper: Off", L"ヘルパー: オフ" },
            { "Helper: Auto Chord", L"ヘルパー: 自動和音" },
            { "Helper: Arp (1-3-5)", L"ヘルパー: アルペジオ (1 - 3 - 5)" },
            { "Helper: Arp (1-5)", L"ヘルパー: アルペジオ (1 - 5)" },
            { "Helper: Arp (1-5-High 1)", L"ヘルパー: アルペジオ (1 - 5 - 高音1)" },
            { "Helper: Arp (1-5-High 3)", L"ヘルパー: アルペジオ (1 - 5 - 高音3)" },
            { "Helper: Arp (1-5-H1-H3)", L"ヘルパー: アルペジオ (1 - 5 - 高音1 - 高音3)" },
            { "Helper: Arp (1-5-H2-H3)", L"ヘルパー: アルペジオ (1 - 5 - 高音2 - 高音3)" },
            { "Helper: Auto Bass", L"ヘルパー: 自動ベース (低音)" },
            { "Whole note", L"全音符" },
            { "Half note", L"2分音符" },
            { "Quarter note", L"4分音符" },
            { "8th note", L"8分音符" },
            { "16th note", L"16分音符" },
            { "32nd note", L"32分音符" },
            { "64th note", L"64分音符" },
            { "Acoustic Piano", L"アコースティックピアノ" },
            { "Square (8-bit)", L"矩形波 (8-bit)" },
            { "Sawtooth (String)", L"ノコギリ波 (String)" },
            { "Sampler (Loaded WAV)", L"サンプラー (WAV読み込み)" },
            { "Mabinogi Preset", L"マビノギプリセット" },
            { "Mabinogi Mobile Preset", L"マビノギモバイルプリセット" },
            { "None", L"なし (基本音)" },
            { "Track ", L"トラック " },
            { "Open load menu (Ctrl+O) / DMMF open (Ctrl+Shift+O)", L"読み込みメニューを開く (Ctrl+O) / DMMFを開く (Ctrl+Shift+O)" },
            { "Open save menu (Ctrl+Shift+S) / Save DMMF (Ctrl+S)", L"保存メニューを開く (Ctrl+Shift+S) / DMMF保存 (Ctrl+S)" },
            { "Play / Stop (Space)", L"再生 / 停止 (Space)" },
            { "Toggle all track names between Track numbers and Mabinogi preset instrument names", L"全トラック名をトラック番号とマビノギプリセット楽器名で切り替えます" },
            { "Switch between the main editor and a blank sub screen", L"メイン編集画面とサブ画面を切り替えます" },
            { "Copy active tracks as MML@...; for Mabinogi", L"有効なトラックをマビノギ用 MML@...; 形式でコピー" },
            { "Open helper window", L"ヘルパーウィンドウを開きます" },
            { "Open time signature window", L"拍子選択ウィンドウを開きます" },
            { "mabiicco note length / resize grid", L"mabiicco ノート長 / 伸縮基準" },
            { "Key: none", L"調号: なし" },
            { "Warning", L"通知" },
            { "Info", L"情報" },
            { "OK", L"確認" },
            { "Classic Light", L"クラシックライト" },
            { "Dark Studio", L"ダークスタジオ" },
            { "Mabinogi Fantasy", L"マビノギファンタジー" },
            { "Neon Night", L"ネオンナイト" },
            { "Orchestra Brown", L"オーケストラブラウン" },
            { "Saint White", L"セイントホワイト" },
            { "Spring Mint", L"スプリングミント" },
            { "Sky Blue", L"スカイブルー" },
            { "Peach Cream", L"ピーチクリーム" },
            { "Morrighan", L"モリアン" },
            { "Cichol", L"キホール" },
            { "Milletian", L"ミレシアン" },
            { "Theme Settings", L"テーマ設定" },
            { "Choose the editor theme. Audio, MML, DLS and MMI logic are not changed.", L"エディターのテーマを選択してください。オーディオ、MML、DLS、MMIロジックは変更されません。" },
            { "Apply", L"適用" },
            { "No Mabinogi Mobile DLS loaded", L"読み込まれたマビノギモバイルDLSはありません" },
            { "No Mabinogi DLS loaded", L"読み込まれたマビノギDLSはありません" },
            { "Mabinogi character limit guide", L"マビノギ文字数制限の目安" },
            { "Copied", L"コピー完了" },
            { "Copied Melody/Chord1/Chord2 as Mabinogi MML@...; text.", L"メロディ/和音1/和音2をマビノギ MML@...; 形式でコピーしました。" },
            { " characters", L" 文字" },
            { "Note Volume", L"ノート音量" },
            { "Select volume for this note. It will be kept until the next volume change.", L"このノートから次の音量変更まで維持する音量を選択してください。" },
            { "Volume", L"音量" },
            { "Working...", L"作業中..." },
            { "Key Detection", L"調号読取" },
            { "Melody is empty.", L"メロディが空です。" },
            { "Could not find enough notes in the melody.", L"メロディ内に十分な音符がありません。" },
            { "Detected key: ", L"推定スケール: " },
            { "Import MMI  Ctrl+Shift+M", L"MMI読み込み  Ctrl+Shift+M" },
            { "Import DMMF  Ctrl+Shift+O", L"DMMF読み込み  Ctrl+Shift+O" },
            { "Import MIDI  Ctrl+Shift+I", L"MIDI読み込み  Ctrl+Shift+I" },
            { "Save DMMF  Ctrl+S", L"DMMF保存  Ctrl+S" },
            { "Save DMMF As...  Ctrl+Alt+S", L"DMMF名前を付けて保存  Ctrl+Alt+S" },
            { "Save as MMI  Ctrl+M", L"MMI形式で保存  Ctrl+M" },
            { "Save as MIDI  Ctrl+I", L"MIDI形式で保存  Ctrl+I" },
            { "Export WAV  Ctrl+W", L"WAV書き出し  Ctrl+W" },
            { "Time Signature", L"拍子" },
            { "Tempo Setting", L"テンポ設定" },
            { "Set tempo at the red line.", L"赤い線の位置にテンポを設定します。" },
            { "Tempo must be between 32 and 255.", L"テンポは32から255の間で入力してください。" },
            { "Delete Tempo", L"テンポ削除" },
            { "No tempo marker exists at the red line.", L"赤い線の位置に削除できるテンポがありません。" },
            { "BPM", L"BPM" },
            { "Save MabiIcco MMI file...", L"MabiIcco MMIファイルを保存..." },
            { "Saving MabiIcco MMI file...", L"MabiIcco MMIファイルを保存中..." },
            { "MMI Saved", L"MMI保存完了" },
            { " saved in MabiIcco-compatible format.", L" をMabiIcco互換形式で保存しました。" },
            { "Save Failed", L"保存失敗" },
            { "Could not save the MMI file.", L"MMIファイルを保存できませんでした。" },
            { "Save to WAV...", L"WAVファイルとして保存..." },
            { "Exporting WAV file...", L"WAVファイルを書き出し中..." },
            { "Could not export the WAV file.", L"WAVファイルを書き出せませんでした。" },
            { "The score is empty.", L"楽譜が空です。" },
            { "Export Failed", L"書き出し失敗" },
            { "Export Error", L"書き出しエラー" },
            { "Save MIDI file...", L"MIDIファイルとして保存..." },
            { "Saving MIDI file...", L"MIDIファイルを保存中..." },
            { "MIDI Saved", L"MIDI保存完了" },
            { " saved successfully.", L" を保存しました。" },
            { "Could not save the MIDI file.", L"MIDIファイルを保存できませんでした。" },
            { "Saving DMMF project...", L"DMMFプロジェクトを保存中..." },
            { "Save DMMF project as...", L"DMMFに名前を付けて保存..." },
            { "DMMF Saved", L"DMMF保存完了" },
            { "Could not save the DMMF project file.", L"DMMFプロジェクトファイルを保存できませんでした。" },
            { "Open DMMF project...", L"DMMFプロジェクトを開く..." },
            { "Opening DMMF project...", L"DMMFプロジェクトを開いています..." },
            { "Open Failed", L"読み込み失敗" },
            { "The selected file is empty.", L"選択したファイルは空です。" },
            { "This does not look like a valid .dmmf project file.", L"有効な .dmmf プロジェクトファイルではないようです。" },
            { "DMMF Opened", L"DMMF読み込み完了" },
            { " opened successfully.", L" を読み込みました。" },
            { "Open MMI file...", L"MMIファイルを開く..." },
            { "Importing MMI file...", L"MMIファイルを読み込み中..." },
            { "Import Complete", L"読み込み完了" },
            { " tracks imported.", L" 個のトラックを読み込みました。" },
            { "Could not import the MMI file.", L"MMIファイルを読み込めませんでした。" },
            { "Open MIDI file...", L"MIDIファイルを開く..." },
            { "Importing MIDI file...", L"MIDIファイルを読み込み中..." },
            { "Could not import the MIDI file.", L"MIDIファイルを読み込めませんでした。" },
            { " MIDI channels imported as tool tracks.", L" 個のMIDIチャンネルをツールトラックとして読み込みました。" },
            { "MIDI Imported", L"MIDI読み込み完了" },
            { "Add Track", L"トラック追加" },
            { "Delete Track", L"トラック削除" },
            { "Delete Current Track", L"現在のトラックを削除" },
            { "Track Delete", L"トラック削除" },
            { "At least one track must remain.", L"トラックは最低1つ必要です。" },
            { "Right-click a track tab or press Shift+D to delete the selected track.", L"トラックタブを右クリック、または Shift+D で選択中のトラックを削除します。" },
            { "Optimize", L"MML圧縮" },
            { "Optimize current track MML using l/o/./rest/volume/tempo byte golfing", L"l/o/./休符/音量/テンポを使って現在のトラックMMLを圧縮します" },
            { "MML Optimizer", L"MML圧縮" },
            { "MML optimization complete.", L"MML圧縮が完了しました。" },
            { "Before", L"圧縮前" },
            { "After", L"圧縮後" },
            { "Saved", L"削減" }
        };

        if (en.startsWith("Key: "))
            return juce::String(L"調号: ") + en.substring(5);

        for (const auto& entry : jaMap)
        {
            if (en == entry.en)
                return juce::String(entry.ja);
        }
    }

    return en;
}
juce::File MainComponent::getUserSettingsFile() const { const auto exeFolder = juce::File::getSpecialLocation(juce::File::currentExecutableFile).getParentDirectory(); return exeFolder.getChildFile("Atelier de Derstin.ini"); }

void MainComponent::loadUserSettings() {
    auto settingsFile = getUserSettingsFile();
    if (!settingsFile.existsAsFile()) { const auto exeFolder = juce::File::getSpecialLocation(juce::File::currentExecutableFile).getParentDirectory(); const auto oldSettingsFile = exeFolder.getChildFile("MmlEditorSettings.ini"); if (oldSettingsFile.existsAsFile()) settingsFile = oldSettingsFile; else return; }
    juce::StringArray lines; lines.addLines(settingsFile.loadFileAsString());
    for (auto line : lines) {
        line = line.trim(); if (line.isEmpty() || line.startsWithChar('#') || line.startsWithChar(';')) continue;
        const int eq = line.indexOfChar('='); if (eq <= 0) continue;
        const auto key = line.substring(0, eq).trim(); const auto value = line.substring(eq + 1).trim();
        if (key.equalsIgnoreCase("themeId")) { const int parsed = value.getIntValue(); if (parsed > 0) currentThemeId = juce::jlimit(1, 12, parsed); }
        else if (key.equalsIgnoreCase("languageId")) { const int parsed = value.getIntValue(); if (parsed == 1 || parsed == 2 || parsed == 3) languageCombo.setSelectedId(parsed, juce::dontSendNotification); }
    }
}

void MainComponent::saveUserSettings() const {
    const auto settingsFile = getUserSettingsFile(); juce::String text;
    text << "# Atelier de Derstin user settings\n# themeId: 1=Classic Light, 2=Dark Studio, 3=Mabinogi Fantasy, 4=Neon Night, 5=Orchestra Brown, 6=Saint White, 7=Spring Mint, 8=Sky Blue, 9=Peach Cream, 10=Morrighan, 11=Cichol, 12=Milletian\n# languageId: 1=English, 2=Korean, 3=Japanese\n";
    text << "themeId=" << juce::String(juce::jlimit(1, 12, currentThemeId)) << "\nlanguageId=" << juce::String(juce::jlimit(1, 3, languageCombo.getSelectedId() > 0 ? languageCombo.getSelectedId() : 2)) << "\n";
    settingsFile.replaceWithText(text);
}

void MainComponent::updateUITexts() {
    playButton.setButtonText(T("Play", L"재생")); stopButton.setButtonText(T("Stop", L"정지")); refreshTrackNameFromPresetButtonText(); rewindButton.setButtonText(T("Rewind (|<)", L"처음으로 (|<)"));
    importButton.setButtonText(T("Load", L"불러오기")); exportButton.setButtonText(T("Save", L"저장하기")); openProjectButton.setButtonText(T("Open DMMF", L"DMMF 열기")); saveProjectButton.setButtonText(T("Save DMMF", L"DMMF 저장"));
    themeButton.setButtonText(T("Theme", L"테마")); licenceButton.setButtonText("Licence"); refreshScreenSwitchButtonText(); copyMabi3PartButton.setButtonText(T("Copy Tracks", L"트랙 복사")); helperButton.setButtonText(T("Helper", L"도우미")); meterButton.setButtonText(T("Meter", L"박자")); optimizeButton.setButtonText(T("Optimize", L"MML 압축")); loadSampleBtn.setButtonText(T("DLS Settings", L"DLS 설정"));
    pcExcludeSongPartLimitToggle.setButtonText(
        languageCombo.getSelectedId() == 3
            ? juce::String(L"歌パート除外")
            : T("Exclude Song Part", L"노래 파트 제외"));

    trackLabels[0].setText(T("Melody", L"멜로디"), juce::dontSendNotification);
    trackLabels[1].setText(T("Chord 1", L"화음 1"), juce::dontSendNotification);
    trackLabels[2].setText(T("Chord 2", L"화음 2"), juce::dontSendNotification);
    trackLabels[3].setText(T("Song", L"노래"), juce::dontSendNotification);

    trackInstrumentLabel.setText(T("Instrument: ", L"악기: "), juce::dontSendNotification); autoBassScaleLabel.setText(T("Scale: ", L"스케일: "), juce::dontSendNotification); dlsPresetLabel.setText(T("Preset: ", L"프리셋: "), juce::dontSendNotification); noteLengthLabel.setText(T("Note:", L"음표:"), juce::dontSendNotification); detectScaleButton.setButtonText(T("Detect Key", L"조표 읽기"));
    updateScaleSignatureLabel();

    // ★ 패턴별로 아르페지오 메뉴가 훨씬 직관적으로 바뀌었습니다!
    int savedHelperId = helperCombo.getSelectedId() > 0 ? helperCombo.getSelectedId() : 1; helperCombo.clear(juce::dontSendNotification);
    helperCombo.addItem(T("Helper: Off", L"도우미: 끄기"), 1);
    helperCombo.addItem(T("Helper: Auto Chord", L"도우미: 자동 화음"), 2);
    helperCombo.addItem(T("Helper: Arp (1-3-5)", L"도우미: 아르페지오 (1 - 3 - 5)"), 3);
    helperCombo.addItem(T("Helper: Arp (1-5)", L"도우미: 아르페지오 (1 - 5)"), 5);
    helperCombo.addItem(T("Helper: Arp (1-5-High 1)", L"도우미: 아르페지오 (1 - 5 - 높은음 1)"), 6);
    helperCombo.addItem(T("Helper: Arp (1-5-High 3)", L"도우미: 아르페지오 (1 - 5 - 높은음 3)"), 7);
    helperCombo.addItem(T("Helper: Arp (1-5-H1-H3)", L"도우미: 아르페지오 (1 - 5 - 높은음 1 - 높은음 3)"), 8);
    helperCombo.addItem(T("Helper: Arp (1-5-H2-H3)", L"도우미: 아르페지오 (1 - 5 - 높은음 2 - 높은음 3)"), 9);
    helperCombo.addItem(T("Helper: Auto Bass", L"도우미: 베이스 자동 (낮은음)"), 4);
    helperCombo.setSelectedId(savedHelperId, juce::dontSendNotification);

    int savedNoteLengthId = noteLengthCombo.getSelectedId() > 0 ? noteLengthCombo.getSelectedId() : 4;
    suppressNoteLengthComboCallback = true;
    noteLengthCombo.clear(juce::dontSendNotification);
    noteLengthCombo.addItem(T("Whole note", L"온음표"), 1);
    noteLengthCombo.addItem(T("Half note", L"2분음표"), 2);
    noteLengthCombo.addItem(T("Quarter note", L"4분음표"), 4);
    noteLengthCombo.addItem(T("8th note", L"8분음표"), 8);
    noteLengthCombo.addItem(T("16th note", L"16분음표"), 16);
    noteLengthCombo.addItem(T("32nd note", L"32분음표"), 32);
    noteLengthCombo.addItem(T("64th note", L"64분음표"), 64);
    noteLengthCombo.setSelectedId(savedNoteLengthId, juce::dontSendNotification);
    suppressNoteLengthComboCallback = false;
    pianoRoll.setResizeQuantizeStepBeats(getSelectedNoteLengthBeats());

    int savedInstId = trackInstrumentCombo.getSelectedId() > 0 ? trackInstrumentCombo.getSelectedId() : 1; trackInstrumentCombo.clear(juce::dontSendNotification);
    trackInstrumentCombo.addItem(T("Acoustic Piano", L"어쿠스틱 피아노"), 1); trackInstrumentCombo.addItem(T("Square (8-bit)", L"사각파 (8-bit)"), 2); trackInstrumentCombo.addItem(T("Sawtooth (String)", L"톱니파 (String)"), 3); trackInstrumentCombo.addItem(T("Sampler (Loaded WAV)", L"샘플러 (WAV 로드)"), 4); trackInstrumentCombo.addItem(T("Mabinogi Preset", L"마비노기 프리셋"), 5); trackInstrumentCombo.addItem(T("Mabinogi Mobile Preset", L"마비노기 모바일 프리셋"), 6); trackInstrumentCombo.setSelectedId(savedInstId, juce::dontSendNotification);

    int savedScaleId = autoBassScaleCombo.getSelectedId() > 0 ? autoBassScaleCombo.getSelectedId() : 1; autoBassScaleCombo.clear(juce::dontSendNotification);
    auto autoScaleText = [this](const char* en, const wchar_t* ja) { return languageCombo.getSelectedId() == 3 ? juce::String(ja) : juce::String(en); };
    autoBassScaleCombo.addItem(T("None", L"None (기본음)"), 1); autoBassScaleCombo.addSeparator();
    autoBassScaleCombo.addItem(autoScaleText("C Major (0)", L"C長調 (0)"), 2); autoBassScaleCombo.addItem(autoScaleText("G Major (#1)", L"G長調 (#1)"), 3); autoBassScaleCombo.addItem(autoScaleText("D Major (#2)", L"D長調 (#2)"), 4); autoBassScaleCombo.addItem(autoScaleText("A Major (#3)", L"A長調 (#3)"), 5); autoBassScaleCombo.addItem(autoScaleText("E Major (#4)", L"E長調 (#4)"), 6); autoBassScaleCombo.addItem(autoScaleText("B Major (#5)", L"B長調 (#5)"), 7); autoBassScaleCombo.addItem(autoScaleText("F# Major (#6)", L"F#長調 (#6)"), 8); autoBassScaleCombo.addItem(autoScaleText("C# Major (#7)", L"C#長調 (#7)"), 9);
    autoBassScaleCombo.addItem(autoScaleText("F Major (b1)", L"F長調 (b1)"), 10); autoBassScaleCombo.addItem(autoScaleText("Bb Major (b2)", L"Bb長調 (b2)"), 11); autoBassScaleCombo.addItem(autoScaleText("Eb Major (b3)", L"Eb長調 (b3)"), 12); autoBassScaleCombo.addItem(autoScaleText("Ab Major (b4)", L"Ab長調 (b4)"), 13); autoBassScaleCombo.addItem(autoScaleText("Db Major (b5)", L"Db長調 (b5)"), 14); autoBassScaleCombo.addItem(autoScaleText("Gb Major (b6)", L"Gb長調 (b6)"), 15); autoBassScaleCombo.addItem(autoScaleText("Cb Major (b7)", L"Cb長調 (b7)"), 16); autoBassScaleCombo.addSeparator();
    autoBassScaleCombo.addItem(autoScaleText("A Minor (0)", L"A短調 (0)"), 17); autoBassScaleCombo.addItem(autoScaleText("E Minor (#1)", L"E短調 (#1)"), 18); autoBassScaleCombo.addItem(autoScaleText("B Minor (#2)", L"B短調 (#2)"), 19); autoBassScaleCombo.addItem(autoScaleText("F# Minor (#3)", L"F#短調 (#3)"), 20); autoBassScaleCombo.addItem(autoScaleText("C# Minor (#4)", L"C#短調 (#4)"), 21); autoBassScaleCombo.addItem(autoScaleText("G# Minor (#5)", L"G#短調 (#5)"), 22); autoBassScaleCombo.addItem(autoScaleText("D# Minor (#6)", L"D#短調 (#6)"), 23); autoBassScaleCombo.addItem(autoScaleText("A# Minor (#7)", L"A#短調 (#7)"), 24);
    autoBassScaleCombo.addItem(autoScaleText("D Minor (b1)", L"D短調 (b1)"), 25); autoBassScaleCombo.addItem(autoScaleText("G Minor (b2)", L"G短調 (b2)"), 26); autoBassScaleCombo.addItem(autoScaleText("C Minor (b3)", L"C短調 (b3)"), 27); autoBassScaleCombo.addItem(autoScaleText("F Minor (b4)", L"F短調 (b4)"), 28); autoBassScaleCombo.addItem(autoScaleText("Bb Minor (b5)", L"Bb短調 (b5)"), 29); autoBassScaleCombo.addItem(autoScaleText("Eb Minor (b6)", L"Eb短調 (b6)"), 30); autoBassScaleCombo.addItem(autoScaleText("Ab Minor (b7)", L"Ab短調 (b7)"), 31);
    autoBassScaleCombo.setSelectedId(savedScaleId, juce::dontSendNotification); updateScaleSignatureLabel();

    if (trackNamesUsePresetInstruments)
    {
        for (int i = 0; i < numActiveTracks; ++i)
        {
            const auto presetName = getPresetInstrumentDisplayNameForBank(i).trim();
            customTrackNames[i] = presetName.isNotEmpty()
                ? presetName
                : (T("Track ", L"트랙 ") + juce::String(i + 1));
        }
    }
    refreshTrackTabTexts();
    addTrackButton.setTooltip(T("Add Track", L"트랙 추가"));
    removeTrackButton.setTooltip(T("Delete Current Track", L"현재 트랙 삭제"));
    importButton.setTooltip(T("Open load menu (Ctrl+O) / DMMF open (Ctrl+Shift+O)", L"불러오기 메뉴 열기 (Ctrl+O) / DMMF 불러오기 (Ctrl+Shift+O)"));
    exportButton.setTooltip(T("Open save menu (Ctrl+Shift+S) / Save DMMF (Ctrl+S)", L"저장하기 메뉴 열기 (Ctrl+Shift+S) / DMMF 저장 (Ctrl+S)"));
    playButton.setTooltip(T("Play / Stop (Space)", L"재생 / 정지 (Space)"));
    trackNameFromPresetButton.setTooltip(T("Toggle all track names between Track numbers and Mabinogi preset instrument names", L"전체 트랙 이름을 Track 번호와 마비노기 프리셋 악기명으로 전환합니다"));
    screenSwitchButton.setTooltip(T("Switch between the main editor and a blank sub screen", L"메인 편집 화면과 빈 서브 화면을 전환합니다"));
    copyMabi3PartButton.setTooltip(T("Copy active tracks as MML@...; for Mabinogi", L"활성화된 트랙을 마비노기용 MML@...; 형식으로 복사"));
    helperButton.setTooltip(T("Open helper window", L"도우미 창을 엽니다"));
    meterButton.setTooltip(T("Open time signature window", L"박자 선택 창을 엽니다"));
    optimizeButton.setTooltip(T("Optimize current track MML using l/o/./rest/volume/tempo byte golfing", L"l/o/./쉼표/볼륨/템포를 계산해서 현재 트랙 MML을 압축합니다"));
    noteLengthCombo.setTooltip(T("mabiicco note length / resize grid", L"mabiicco 노트 길이 / 늘리기 기준"));
    updatePresetCombo(); updatePartEditorVisibility(); updateMmlCharCountLabels(); refreshPianoRollModel(); repaint();
}

void MainComponent::updateScaleSignatureLabel() {
    const int scaleId = autoBassScaleCombo.getSelectedId() > 0 ? autoBassScaleCombo.getSelectedId() : 1;
    juce::String sigStr = T("Key: none", L"조표: 없음");
    struct SigInfo { int acc = 0; bool sharp = true; }; SigInfo info;
    switch (scaleId) {
    case 2: case 17: info = { 0, true }; break; case 3: case 18: info = { 1, true }; break; case 4: case 19: info = { 2, true }; break; case 5: case 20: info = { 3, true }; break; case 6: case 21: info = { 4, true }; break; case 7: case 22: info = { 5, true }; break; case 8: case 23: info = { 6, true }; break; case 9: case 24: info = { 7, true }; break;
    case 10: case 25: info = { 1, false }; break; case 11: case 26: info = { 2, false }; break; case 12: case 27: info = { 3, false }; break; case 13: case 28: info = { 4, false }; break; case 14: case 29: info = { 5, false }; break; case 15: case 30: info = { 6, false }; break; case 16: case 31: info = { 7, false }; break;
    }
    if (info.acc > 0) { juce::String accStr = info.sharp ? "#" : "b"; sigStr = T(juce::String("Key: ") + accStr + " " + juce::String(info.acc), juce::String(L"조표: ") + accStr + " " + juce::String(info.acc) + juce::String(L"개")); }
    scaleSignatureLabel.setText(sigStr, juce::dontSendNotification); scaleSignatureLabel.repaint();
}

void MainComponent::showThemedMessageBoxAsync(juce::AlertWindow::AlertIconType iconType, const juce::String& title, const juce::String& message) {
    juce::String decTitle = title; juce::String iconGlyph = "i";
    if (iconType == juce::AlertWindow::WarningIcon) { decTitle = T("Warning", L"알림") + " - " + title; iconGlyph = "!"; }
    else if (iconType == juce::AlertWindow::InfoIcon) { decTitle = T("Info", L"정보") + " - " + title; iconGlyph = "i"; }
    const auto panel = CustomUI::getThemeColour(currentThemeId, "panel"); const auto panel2 = CustomUI::getThemeColour(currentThemeId, "panel2"); const auto accent = CustomUI::getThemeColour(currentThemeId, "accent"); const auto accent2 = CustomUI::getThemeColour(currentThemeId, "accent2");
    auto* content = new CustomUI::ThemedMessageContent(decTitle, message, T("OK", L"확인"), iconGlyph, panel, panel2, accent, accent2, CustomUI::getReadableTextColour(panel), CustomUI::getThemeColour(currentThemeId, "mutedText"), CustomUI::getReadableTextColour(accent));
    juce::DialogWindow::LaunchOptions options; options.content.setOwned(content); options.dialogTitle = juce::String(); options.dialogBackgroundColour = panel; options.escapeKeyTriggersCloseButton = false; options.useNativeTitleBar = false; options.resizable = false;
    if (auto* dialog = options.launchAsync()) { dialog->setAlwaysOnTop(true); dialog->centreAroundComponent(this, content->getWidth(), content->getHeight()); dialog->setColour(juce::ResizableWindow::backgroundColourId, panel); }
}

void MainComponent::showThemeDialog() {
    const int prevThemeId = currentThemeId;
    juce::StringArray themes; themes.add(T("Classic Light", L"클래식 라이트")); themes.add(T("Dark Studio", L"다크 스튜디오")); themes.add(T("Mabinogi Fantasy", L"마비노기 판타지")); themes.add(T("Neon Night", L"네온 나이트")); themes.add(T("Orchestra Brown", L"오케스트라 브라운")); themes.add(T("Saint White", L"세인트 화이트")); themes.add(T("Spring Mint", L"스프링 민트")); themes.add(T("Sky Blue", L"스카이 블루")); themes.add(T("Peach Cream", L"피치 크림")); themes.add(T("Morrighan", L"모리안")); themes.add(T("Cichol", L"키홀")); themes.add(T("Milletian", L"밀레시안"));
    auto* content = new CustomUI::ThemeDialogContent(T("Theme Settings", L"테마 설정"), T("Choose the editor theme. Audio, MML, DLS and MMI logic are not changed.", L"에디터 테마를 선택하세요. 오디오, MML, DLS, MMI 로직은 변경하지 않습니다."), T("Theme", L"테마"), T("Apply", L"적용"), T("Cancel", L"취소"), themes, currentThemeId);
    auto restyleContent = [this, content]() { content->applyDialogTheme(CustomUI::getThemeColour(currentThemeId, "panel"), CustomUI::getThemeColour(currentThemeId, "panel2"), CustomUI::getThemeColour(currentThemeId, "accent"), CustomUI::getThemeColour(currentThemeId, "accent2"), CustomUI::getThemeColour(currentThemeId, "button")); };
    content->themeCombo.onChange = [this, content, restyleContent] { applyTheme(content->getSelectedThemeId()); restyleContent(); };
    content->applyButton.onClick = [content] { if (auto* dialog = content->findParentComponentOfClass<juce::DialogWindow>()) dialog->exitModalState(1); };
    content->cancelButton.onClick = [this, content, prevThemeId] { applyTheme(prevThemeId); if (auto* dialog = content->findParentComponentOfClass<juce::DialogWindow>()) dialog->exitModalState(0); };
    restyleContent();
    juce::DialogWindow::LaunchOptions options; options.content.setOwned(content); options.dialogTitle = juce::String(); options.dialogBackgroundColour = CustomUI::getThemeColour(currentThemeId, "panel"); options.escapeKeyTriggersCloseButton = false; options.useNativeTitleBar = false; options.resizable = false;
    if (auto* dialog = options.launchAsync()) { dialog->setAlwaysOnTop(true); dialog->centreAroundComponent(this, content->getWidth(), content->getHeight()); dialog->setColour(juce::ResizableWindow::backgroundColourId, CustomUI::getThemeColour(currentThemeId, "panel")); }
}

void MainComponent::applyTheme(int themeId) {
    currentThemeId = juce::jlimit(1, 12, themeId);
    const auto text = CustomUI::getThemeColour(currentThemeId, "text");
    const auto mutedText = CustomUI::getThemeColour(currentThemeId, "mutedText");
    const auto buttonBase = CustomUI::getThemeColour(currentThemeId, "button");
    const auto accent = CustomUI::getThemeColour(currentThemeId, "accent");
    const auto accent2 = CustomUI::getThemeColour(currentThemeId, "accent2");

    const auto panel = CustomUI::getThemeColour(currentThemeId, "panel");
    const auto panel2 = CustomUI::getThemeColour(currentThemeId, "panel2");

    auto& lf = juce::LookAndFeel::getDefaultLookAndFeel();
    lf.setColour(juce::PopupMenu::backgroundColourId, panel);
    lf.setColour(juce::PopupMenu::textColourId, CustomUI::getReadableTextColour(panel));
    lf.setColour(juce::PopupMenu::highlightedBackgroundColourId, accent);
    lf.setColour(juce::PopupMenu::highlightedTextColourId, CustomUI::getReadableTextColour(accent));
    lf.setColour(juce::PopupMenu::headerTextColourId, accent2);

    auto styleLabel = [&](juce::Label& label, bool muted = false) {
        label.setColour(juce::Label::textColourId, muted ? mutedText : text);
        label.setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    };

    auto styleCombo = [&](juce::ComboBox& combo) {
        const auto comboBg = panel2;
        const auto comboText = CustomUI::getReadableTextColour(comboBg);
        combo.setColour(juce::ComboBox::backgroundColourId, comboBg);
        combo.setColour(juce::ComboBox::textColourId, comboText);
        combo.setColour(juce::ComboBox::outlineColourId, accent.withAlpha(0.65f));
        combo.setColour(juce::ComboBox::buttonColourId, buttonBase.brighter(0.25f));
        combo.setColour(juce::ComboBox::arrowColourId, accent);

        combo.setColour(juce::PopupMenu::backgroundColourId, panel);
        combo.setColour(juce::PopupMenu::textColourId, CustomUI::getReadableTextColour(panel));
        combo.setColour(juce::PopupMenu::highlightedBackgroundColourId, accent);
        combo.setColour(juce::PopupMenu::highlightedTextColourId, CustomUI::getReadableTextColour(accent));
    };

    auto styleButton = [&](juce::TextButton& button, juce::Colour base, juce::Colour on = juce::Colours::transparentBlack) {
        const auto onColour = on == juce::Colours::transparentBlack ? accent : on;
        button.setColour(juce::TextButton::buttonColourId, base);
        button.setColour(juce::TextButton::buttonOnColourId, onColour);
        button.setColour(juce::TextButton::textColourOffId, CustomUI::getReadableTextColour(base));
        button.setColour(juce::TextButton::textColourOnId, CustomUI::getReadableTextColour(onColour));
    };

    styleButton(playButton, juce::Colour(0xff217a44), juce::Colour(0xff2fbf71));
    styleButton(stopButton, juce::Colour(0xff82383f), juce::Colour(0xffc94c55));
    styleButton(trackNameFromPresetButton, CustomUI::getThemeColour(currentThemeId, "button").brighter(0.10f), accent.withAlpha(0.85f));
    styleButton(rewindButton, buttonBase.brighter(0.05f));
    styleButton(exportButton, juce::Colour(0xff2f6b46), juce::Colour(0xff3fa56a));
    styleButton(importButton, juce::Colour(0xff8b5a2b), juce::Colour(0xffb87836));
    styleButton(openProjectButton, CustomUI::getThemeColour(currentThemeId, "button").brighter(0.10f), accent.withAlpha(0.75f));
    styleButton(saveProjectButton, CustomUI::getThemeColour(currentThemeId, "button").brighter(0.10f), accent.withAlpha(0.75f));
    styleButton(themeButton, CustomUI::getThemeColour(currentThemeId, "button").brighter(0.15f), accent);
    styleButton(licenceButton, CustomUI::getThemeColour(currentThemeId, "button").brighter(0.15f), accent);
    styleButton(screenSwitchButton, CustomUI::getThemeColour(currentThemeId, "button").brighter(0.12f), accent2.withAlpha(0.90f));
    styleButton(copyMabi3PartButton, CustomUI::getThemeColour(currentThemeId, "button").brighter(0.10f), accent2.withAlpha(0.85f));
    styleButton(helperButton, CustomUI::getThemeColour(currentThemeId, "button").brighter(0.12f), accent.withAlpha(0.85f));
    styleButton(meterButton, CustomUI::getThemeColour(currentThemeId, "button").brighter(0.12f), accent2.withAlpha(0.85f));
    styleButton(optimizeButton, CustomUI::getThemeColour(currentThemeId, "button").brighter(0.18f), accent.withAlpha(0.90f));
    styleButton(loadSampleBtn, juce::Colour(0xff263f8b), juce::Colour(0xff3d63d9));
    styleButton(detectScaleButton, CustomUI::getThemeColour(currentThemeId, "button").brighter(0.08f), accent.withAlpha(0.85f));
    styleButton(addTrackButton, buttonBase.brighter(0.1f));
    styleButton(removeTrackButton, buttonBase.darker(0.05f));

    for (int i = 0; i < MAX_BANKS; ++i) {
        const auto tabOff = buttonBase.withAlpha(0.85f);
        const auto tabOn = CustomUI::getBankColor(i).interpolatedWith(accent, 0.35f);
        tabButtons[i].setColour(juce::TextButton::buttonColourId, tabOff);
        tabButtons[i].setColour(juce::TextButton::buttonOnColourId, tabOn);
        tabButtons[i].setColour(juce::TextButton::textColourOffId, CustomUI::getReadableTextColour(tabOff));
        tabButtons[i].setColour(juce::TextButton::textColourOnId, CustomUI::getReadableTextColour(tabOn));
    }

    for (int i = 0; i < 4; ++i) {
        styleButton(muteBtns[i], panel2, juce::Colours::red.withAlpha(0.8f));
        styleButton(soloBtns[i], panel2, juce::Colours::orange.withAlpha(0.85f));
        styleLabel(trackLabels[i]);
        styleLabel(trackCountLabels[i], true);
    }

    for (int i = 0; i < 3; ++i) {
        subPartCheckBoxes[i].setColour(juce::ToggleButton::textColourId, text);
        subPartCheckBoxes[i].setColour(juce::ToggleButton::tickColourId, accent);
        subPartCheckBoxes[i].setColour(juce::ToggleButton::tickDisabledColourId, mutedText);
    }

    pcExcludeSongPartLimitToggle.setColour(juce::ToggleButton::textColourId, text);
    pcExcludeSongPartLimitToggle.setColour(juce::ToggleButton::tickColourId, accent);
    pcExcludeSongPartLimitToggle.setColour(juce::ToggleButton::tickDisabledColourId, mutedText);
    compositionRankGuideLabel.setColour(juce::Label::textColourId, text);
    compositionRankGuideLabel.setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);

    styleCombo(timeSignatureCombo); styleCombo(helperCombo); styleCombo(noteLengthCombo); styleCombo(autoBassScaleCombo); styleCombo(languageCombo); styleCombo(trackInstrumentCombo); styleCombo(dlsPresetCombo);

    // Live UI sync: same real-time theme path as the preset ComboBox border.
    // Do not move this into DLS Settings button handlers.
    bottomPanelSeparator.setColour(juce::Label::backgroundColourId,
        dlsPresetCombo.findColour(juce::ComboBox::outlineColourId));
    bottomPanelSeparator.repaint();

    // Icon/button borders follow the same live theme border as the Mabinogi preset ComboBox.
    const auto themedIconBorder = dlsPresetCombo.findColour(juce::ComboBox::outlineColourId);

    auto syncIconBorder = [&](juce::TextButton& button)
    {
        button.setColour(atelierButtonBorderColourId, themedIconBorder);
        button.repaint();
    };

    syncIconBorder(playButton);
    syncIconBorder(stopButton);
    syncIconBorder(trackNameFromPresetButton);
    syncIconBorder(rewindButton);
    syncIconBorder(importButton);
    syncIconBorder(exportButton);
    syncIconBorder(openProjectButton);
    syncIconBorder(saveProjectButton);
    syncIconBorder(themeButton);
    syncIconBorder(licenceButton);
    syncIconBorder(screenSwitchButton);
    syncIconBorder(copyMabi3PartButton);
    syncIconBorder(helperButton);
    syncIconBorder(meterButton);
    syncIconBorder(optimizeButton);
    syncIconBorder(loadSampleBtn);
    syncIconBorder(detectScaleButton);
    syncIconBorder(addTrackButton);
    syncIconBorder(removeTrackButton);

    for (int i = 0; i < MAX_BANKS; ++i)
        syncIconBorder(tabButtons[i]);

    for (int i = 0; i < 4; ++i)
    {
        syncIconBorder(muteBtns[i]);
        syncIconBorder(soloBtns[i]);
    }


    // Keep the bottom gap line synced with the live-selected UI theme.
    // This must happen here, immediately after styleCombo(dlsPresetCombo), because this is
    // where the preset ComboBox outlineColourId is actually updated for the selected theme.
    bottomPanelSeparator.setColour(juce::Label::backgroundColourId,
        dlsPresetCombo.findColour(juce::ComboBox::outlineColourId));
    bottomPanelSeparator.repaint();

    styleLabel(autoBassScaleLabel, true); styleLabel(scaleSignatureLabel, true); styleLabel(trackInstrumentLabel, true); styleLabel(dlsPresetLabel, true); styleLabel(noteLengthLabel, true);

    applyHelperModeState(helperCombo.getSelectedId());
    refreshEditorTextColours();

    verticalScrollBar.setColour(juce::ScrollBar::backgroundColourId, panel2);
    verticalScrollBar.setColour(juce::ScrollBar::thumbColourId, accent.withAlpha(0.75f));
    horizontalScrollBar.setColour(juce::ScrollBar::backgroundColourId, panel2);
    horizontalScrollBar.setColour(juce::ScrollBar::thumbColourId, accent.withAlpha(0.75f));

    updateMmlCharCountLabels();
    refreshPianoRollModel();
    refreshWorkLoadingOverlayTheme();

    if (helperPopupWindow != nullptr)
    {
        helperPopupWindow->setColour(juce::ResizableWindow::backgroundColourId, panel);
        if (auto* helperContent = dynamic_cast<HelperPopupContent*>(helperPopupWindow->getContentComponent()))
            helperContent->applyTheme(currentThemeId);
        helperPopupWindow->repaint();
    }

    if (meterPopupWindow != nullptr)
    {
        meterPopupWindow->setColour(juce::ResizableWindow::backgroundColourId, panel);
        if (auto* meterContent = dynamic_cast<TimeSignaturePopupContent*>(meterPopupWindow->getContentComponent()))
            meterContent->applyTheme(currentThemeId);
        meterPopupWindow->repaint();
    }

    if (licencePopupWindow != nullptr)
    {
        licencePopupWindow->setColour(juce::ResizableWindow::backgroundColourId, panel);
        licencePopupWindow->setColour(juce::DocumentWindow::textColourId, CustomUI::getReadableTextColour(panel));
        if (auto* licenceContent = dynamic_cast<LicenceCreditsContent*>(licencePopupWindow->getContentComponent()))
            licenceContent->applyTheme(currentThemeId);
        licencePopupWindow->repaint();
    }

    saveUserSettings();

    for (auto* child : getChildren()) { if (child != nullptr) child->repaint(); }
    repaint();
}

void MainComponent::updatePresetCombo() {
    const juce::ScopedValueSetter<bool> presetGuard(suppressPresetCallbacks, true);
    dlsPresetCombo.clear(juce::dontSendNotification);

    const int instrumentId = banks[currentBankIndex].instrumentWave;
    const bool wantsMobilePreset = isMobilePresetInstrument(instrumentId);
    const bool hideSongPresets = (instrumentId == 5 && banks[currentBankIndex].pcPresetExcludeSongPartLimit);

    for (int fileIdx = 0; fileIdx < audioEngine.getNumEngines(); ++fileIdx) {
        const bool mobileFile = isMobileSf2Engine(fileIdx);
        if (isMabinogiPresetInstrument(instrumentId) && mobileFile != wantsMobilePreset)
            continue;

        const int presetCount = audioEngine.getPresetCount(fileIdx);
        for (int presetIdx = 0; presetIdx < presetCount; ++presetIdx) {
            const char* rawName = audioEngine.getPresetName(fileIdx, presetIdx);
            const int itemId = fileIdx * 10000 + presetIdx + 1;
            const juce::String fileName = audioEngine.getSf2Name(fileIdx);
            juce::String displayFileName = fileName;
            if (mobileFile && displayFileName.equalsIgnoreCase("Fury Sound Pack - Mabinogi Mobile Instrument Set"))
                displayFileName = "Fury Sound Pack";

            juce::String presetName = rawName != nullptr ? juce::String::fromUTF8(rawName) : juce::String("Preset ") + juce::String(presetIdx);
            const juce::String rawPresetNameForFilter = presetName;
            presetName = ProjectFileIO::getLocalizedSf2PresetName(presetName, fileName, presetIdx, languageCombo.getSelectedId());
            presetName = restoreLanguagePresetTranslation(presetName, fileName, presetIdx, languageCombo.getSelectedId());

            const auto rawLower = rawPresetNameForFilter.toLowerCase();
            const auto presetLower = presetName.toLowerCase();
            const auto fileLower = fileName.toLowerCase();

            const bool isNotUsedPreset = rawLower.contains("not used")
                                      || presetLower.contains("not used");
            if (isNotUsedPreset)
                continue;

            const juce::String englishPresetName =
                ProjectFileIO::getLocalizedSf2PresetName(rawPresetNameForFilter, fileName, presetIdx, 1).toLowerCase();
            const juce::String koreanPresetName =
                ProjectFileIO::getLocalizedSf2PresetName(rawPresetNameForFilter, fileName, presetIdx, 2).toLowerCase();
            const juce::String japanesePresetName =
                ProjectFileIO::getLocalizedSf2PresetName(rawPresetNameForFilter, fileName, presetIdx, 3).toLowerCase();

            const bool isSongVoicePreset =
                fileLower.startsWith("msxspirit02")
                || rawLower.contains("song")
                || rawLower.contains("voice")
                || presetLower.contains("song")
                || presetLower.contains("voice")
                || presetLower.contains(juce::String(L"노래"))
                || presetLower.contains(juce::String(L"歌"))
                || presetLower.contains(juce::String(L"ボーカル"))
                || englishPresetName.contains("song")
                || englishPresetName.contains("voice")
                || koreanPresetName.contains(juce::String(L"노래"))
                || japanesePresetName.contains(juce::String(L"歌"))
                || japanesePresetName.contains(juce::String(L"ボーカル"));

            if (hideSongPresets && isSongVoicePreset)
                continue;

            dlsPresetCombo.addItem(displayFileName + " / " + juce::String(presetIdx) + ": " + presetName, itemId);
        }
    }

    if (dlsPresetCombo.getNumItems() == 0) {
        dlsPresetCombo.addItem(hideSongPresets ? T("No non-song Mabinogi preset available", L"노래 제외 후 표시할 마비노기 프리셋 없음") : (wantsMobilePreset ? T("No Mabinogi Mobile DLS loaded", L"불러온 마비노기 모바일 DLS 없음") : T("No Mabinogi DLS loaded", L"불러온 마비노기 DLS 없음")), 1);
        dlsPresetCombo.setSelectedId(1, juce::dontSendNotification);
        banks[currentBankIndex].songPresetMode = false;
        return;
    }

    int selectedId = banks[currentBankIndex].sf2FileIndex * 10000 + banks[currentBankIndex].dlsPreset + 1;
    dlsPresetCombo.setSelectedId(selectedId, juce::dontSendNotification);

    if (dlsPresetCombo.getSelectedId() == 0) {
        dlsPresetCombo.setSelectedItemIndex(0, juce::dontSendNotification);
        selectedId = dlsPresetCombo.getSelectedId();
        if (selectedId > 0) {
            const int encoded = selectedId - 1;
            banks[currentBankIndex].sf2FileIndex = encoded / 10000;
            banks[currentBankIndex].dlsPreset = encoded % 10000;
        }
    }

    banks[currentBankIndex].songPresetMode = hideSongPresets ? false : computeSongPresetModeForBank(currentBankIndex);
    banks[currentBankIndex].xylophonePresetMode = computeXylophonePresetModeForBank(currentBankIndex);
}

void MainComponent::refreshEditorTextColours() {
    const auto normalBg = CustomUI::getThemeColour(currentThemeId, "editorBg"); const auto readOnlyBg = CustomUI::getThemeColour(currentThemeId, "editorReadOnly"); const auto accent = CustomUI::getThemeColour(currentThemeId, "accent"); const auto accent2 = CustomUI::getThemeColour(currentThemeId, "accent2"); const auto activeText = CustomUI::getThemeColour(currentThemeId, "text"); const auto inactiveText = CustomUI::getThemeColour(currentThemeId, "mutedText");
    const int mode = helperCombo.getSelectedId(); const bool track2ReadOnly = (mode == 2);
    const bool track3ReadOnly = (mode == 2 || mode == 3 || mode == 4 || (mode >= 5 && mode <= 9));
    const bool xylophoneMode = isXylophonePresetSelected();

    auto styleEditorLive = [&](juce::TextEditor& editor, bool readOnly, int trackIdx) {
        const auto bg = readOnly ? readOnlyBg : normalBg; const auto fg = readOnly ? inactiveText : activeText;
        editor.setReadOnly(readOnly); editor.setColour(juce::TextEditor::backgroundColourId, bg); editor.setColour(juce::TextEditor::textColourId, fg); editor.setColour(juce::TextEditor::outlineColourId, accent.withAlpha(0.55f)); editor.setColour(juce::TextEditor::focusedOutlineColourId, accent2); editor.setColour(juce::TextEditor::highlightColourId, getEditorPlaybackHighlightColour(trackIdx)); editor.setColour(juce::TextEditor::highlightedTextColourId, CustomUI::getReadableTextColour(getEditorPlaybackHighlightColour(trackIdx))); editor.setColour(juce::CaretComponent::caretColourId, accent2); editor.setTextToShowWhenEmpty(juce::String(), inactiveText.withAlpha(0.7f));
        editor.applyColourToAllText(fg); editor.applyFontToAllText(editor.getFont()); editor.repaint();
    };

    const bool mabbiicoReadOnlyRows = isSubScreenVisible;
    styleEditorLive(trackEditors[0], mabbiicoReadOnlyRows, 0);
    styleEditorLive(trackEditors[1], xylophoneMode || mabbiicoReadOnlyRows || track2ReadOnly, 1);
    styleEditorLive(trackEditors[2], xylophoneMode || mabbiicoReadOnlyRows || track3ReadOnly, 2);
    trackEditors[1].setEnabled(!xylophoneMode);
    trackEditors[2].setEnabled(!xylophoneMode);
    styleEditorLive(trackEditors[3], isSubScreenVisible && isSongPresetSelected(), 3);
    lastEditorTextThemeId = currentThemeId;
}

juce::Colour MainComponent::getEditorPlaybackHighlightColour(int trackIdx) const {
    const auto bg = CustomUI::getThemeColour(currentThemeId, "editorBg"); auto c = CustomUI::getTrackThemeColour(currentThemeId, trackIdx);
    if (bg.getPerceivedBrightness() > 0.55f) c = c.darker(0.55f).withSaturation(0.95f); else c = c.brighter(0.45f).withSaturation(0.95f); return c.withAlpha(0.88f);
}


void MainComponent::refreshPianoRollModel() {
    pianoRoll.setModel(banks, &numActiveTracks, &currentBankIndex, &cachedEventList, &tempoMap);
    pianoRoll.setMeterChanges(&meterChanges);
    pianoRoll.setEventListVisible(!isSubScreenVisible);
    pianoRoll.setSeekOnlyFromTimeline(isSubScreenVisible);
    pianoRoll.setTheme(currentThemeId, languageCombo.getSelectedId(), getTimeSignatureBeatsPerMeasure());

    // Mabiicco mode keeps its Ctrl+wheel zoom state, but 3MLE must always use
    // the original timeline scale so Mabiicco zoom never leaks into 3MLE.
    const double activeTimelinePixelsPerBeat = isSubScreenVisible
        ? timelinePixelsPerBeat
        : defaultTimelinePixelsPerBeatFor3MLE;
    pianoRoll.setTimelineZoom(activeTimelinePixelsPerBeat);

    const auto rollArea = pianoRoll.getRollAreaBounds();
    if (rollArea.getWidth() > 0)
    {
        const double maxScrollX = std::max(0.0, pianoRoll.getTotalWidthPixels() - static_cast<double>(rollArea.getWidth()));
        scrollX = juce::jlimit(0.0, maxScrollX, scrollX);
    }

    pianoRoll.setScroll(scrollX, scrollY, fixedRowHeight);
    pianoRoll.setTransport(audioEngine.getSampleRate(), &globalSampleCount, &isPlaying);
    pianoRoll.setResizeQuantizeStepBeats(getSelectedNoteLengthBeats());
    pianoRoll.repaint();
}


void MainComponent::zoomMabbiicoTimeline(float localMouseX, float wheelDeltaY)
{
    // Ctrl + wheel horizontal zoom is a mabbiico-only editing gesture.
    // Wheel up expands the timeline horizontally, wheel down contracts it.
    if (!isSubScreenVisible || wheelDeltaY == 0.0f)
        return;

    const double oldPixelsPerBeat = timelinePixelsPerBeat;
    const double zoomStep = 1.18;
    const double zoomFactor = wheelDeltaY > 0.0f ? zoomStep : (1.0 / zoomStep);
    const double newPixelsPerBeat = juce::jlimit(18.0, 260.0, oldPixelsPerBeat * zoomFactor);

    if (std::abs(newPixelsPerBeat - oldPixelsPerBeat) < 0.001)
        return;

    const auto rollArea = pianoRoll.getRollAreaBounds();
    const double mouseOffsetX = juce::jlimit(0.0,
                                            static_cast<double>(rollArea.getWidth()),
                                            static_cast<double>(localMouseX - static_cast<float>(rollArea.getX())));

    // Keep the beat under the mouse as stable as possible while zooming.
    const double beatUnderMouse = oldPixelsPerBeat > 0.0 ? ((scrollX + mouseOffsetX) / oldPixelsPerBeat) : 0.0;

    timelinePixelsPerBeat = newPixelsPerBeat;
    pianoRoll.setTimelineZoom(timelinePixelsPerBeat);

    const double totalWidthPixels = pianoRoll.getTotalWidthPixels();
    const double visibleWidth = static_cast<double>(rollArea.getWidth());
    const double maxScroll = std::max(0.0, totalWidthPixels - visibleWidth);
    scrollX = juce::jlimit(0.0,
                           maxScroll,
                           beatUnderMouse * timelinePixelsPerBeat - mouseOffsetX);

    horizontalScrollBar.setRangeLimits(0.0, totalWidthPixels);
    horizontalScrollBar.setCurrentRange(scrollX, visibleWidth);
    pianoRoll.setScroll(scrollX, scrollY, fixedRowHeight);
    pianoRoll.repaint();
    repaint();
}

void MainComponent::updateMmlCharCountLabels() {
    // Character limits are different between PC Mabinogi preset, PC "Exclude Song Part" mode,
    // and Mabinogi Mobile preset.
    // PC Mabinogi:                  Melody 1200 / Chord1 800  / Chord2 500
    // PC Mabinogi + Exclude Song:   Melody 1600 / Chord1 1200 / Chord2 900
    // Mabinogi Mobile:              Melody 2400 / Chord1 2400 / Chord2 2400
    static constexpr int pcLimits[4] = { 1200, 800, 500, 1200 };
    static constexpr int pcExcludeSongLimits[4] = { 1600, 1200, 900, 1200 };
    static constexpr int mobileLimits[4] = { 2400, 2400, 2400, 2400 };

    struct ComposeRankLimit
    {
        const char* rank;
        int normalMelody;
        int normalChord1;
        int normalChord2;
        int excludeMelody;
        int excludeChord1;
        int excludeChord2;
    };

    static constexpr ComposeRankLimit rankLimits[] = {
        { "F", 400, 200, 100,  533,  333, 233 },
        { "E", 500, 200, 100,  666,  366, 266 },
        { "D", 600, 250, 150,  800,  450, 350 },
        { "C", 650, 250, 200,  866,  466, 416 },
        { "B", 700, 300, 200,  933,  533, 433 },
        { "A", 750, 300, 200, 1000,  550, 450 },
        { "9", 800, 350, 200, 1066,  616, 466 },
        { "8", 850, 400, 200, 1133,  683, 483 },
        { "7", 900, 400, 200, 1200,  700, 500 },
        { "6", 950, 450, 200, 1266,  766, 516 },
        { "5",1000, 500, 250, 1333,  833, 583 },
        { "4",1050, 550, 300, 1400,  900, 650 },
        { "3",1100, 600, 350, 1466,  966, 716 },
        { "2",1150, 700, 400, 1533, 1083, 783 },
        { "1",1200, 800, 500, 1600, 1200, 900 }
    };

    const int instrumentId = banks[currentBankIndex].instrumentWave;
    const bool pcMabinogiLimitMode = (instrumentId == 5);
    const bool mobileLimitMode = isMobilePresetInstrument(instrumentId);
    const bool pcExcludeSongLimitMode = (pcMabinogiLimitMode && pcExcludeSongPartLimitToggle.getToggleState());

    const int* limits = mobileLimitMode ? mobileLimits : (pcExcludeSongLimitMode ? pcExcludeSongLimits : pcLimits);

    const auto muted = CustomUI::getThemeColour(currentThemeId, "mutedText");
    const auto warn = juce::Colours::orange;
    const auto danger = juce::Colours::red;

    auto requiredComposeRank = [&]() -> juce::String
    {
        const int melodyCount = trackEditors[0].getText().length();
        const int chord1Count = trackEditors[1].getText().length();
        const int chord2Count = trackEditors[2].getText().length();

        for (const auto& entry : rankLimits)
        {
            const int melodyLimit = pcExcludeSongLimitMode ? entry.excludeMelody : entry.normalMelody;
            const int chord1Limit = pcExcludeSongLimitMode ? entry.excludeChord1 : entry.normalChord1;
            const int chord2Limit = pcExcludeSongLimitMode ? entry.excludeChord2 : entry.normalChord2;

            // One rank is valid only when all three parts fit inside that rank.
            // If even one part exceeds F, the required rank becomes E; if it exceeds E, D, and so on.
            if (melodyCount <= melodyLimit && chord1Count <= chord1Limit && chord2Count <= chord2Limit)
                return juce::String(entry.rank);
        }

        return T("Over", L"초과");
    };

    for (int i = 0; i < 4; ++i) {
        const int count = trackEditors[i].getText().length();
        const int limit = limits[i];
        trackCountLabels[i].setText(juce::String(count) + "/" + juce::String(limit), juce::dontSendNotification);
        trackCountLabels[i].setColour(juce::Label::textColourId, count > limit ? danger : (count > static_cast<int>(limit * 0.9) ? warn : muted));

        const juce::String limitTooltip =
            mobileLimitMode
                ? (languageCombo.getSelectedId() == 3
                    ? juce::String(L"マビノギモバイル文字数制限の目安")
                    : T("Mabinogi Mobile character limit guide", L"마비노기 모바일 글자수 제한 참고"))
                : (pcExcludeSongLimitMode
                    ? (languageCombo.getSelectedId() == 3
                        ? juce::String(L"マビノギ歌パート除外文字数制限の目安")
                        : T("Mabinogi character limit guide - excluding song part", L"마비노기 노래 파트 제외 글자수 제한 참고"))
                    : (languageCombo.getSelectedId() == 3
                        ? juce::String(L"マビノギ文字数制限の目安")
                        : T("Mabinogi character limit guide", L"마비노기 글자수 제한 참고")));

        trackCountLabels[i].setTooltip(limitTooltip + ": " + juce::String(limit));
    }

    if (pcMabinogiLimitMode)
    {
        const juce::String requiredRank = requiredComposeRank();

        const bool mabiiccoCountMode = isSubScreenVisible;
        const bool songPartExcluded = pcMabinogiLimitMode && pcExcludeSongPartLimitToggle.getToggleState();
        const bool songPartCountMode = mabiiccoCountMode && isSongPresetSelected() && !songPartExcluded;

        juce::String countSummary;
        if (mabiiccoCountMode)
        {
            if (songPartCountMode)
            {
                countSummary = juce::String(trackEditors[3].getText().length());
            }
            else
            {
                countSummary =
                    juce::String(trackEditors[0].getText().length()) + "/" +
                    juce::String(trackEditors[1].getText().length()) + "/" +
                    juce::String(trackEditors[2].getText().length());
            }
        }

        juce::String labelText;
        if (languageCombo.getSelectedId() == 3)
        {
            labelText = juce::String(L"ランク: ") + requiredRank;
            if (mabiiccoCountMode)
                labelText << "  " << juce::String(L"文字数: ") << countSummary;
        }
        else
        {
            labelText = T("Rank: ", L"랭크: ") + requiredRank;
            if (mabiiccoCountMode)
                labelText << "  " << T("Chars: ", L"글자수: ") << countSummary;
        }

        compositionRankGuideLabel.setText(labelText, juce::dontSendNotification);

        const bool overLimit =
            trackEditors[0].getText().length() > limits[0]
            || trackEditors[1].getText().length() > limits[1]
            || trackEditors[2].getText().length() > limits[2];

        compositionRankGuideLabel.setColour(juce::Label::textColourId, overLimit ? danger : muted);
        compositionRankGuideLabel.setVisible(true);
    }
    else
    {
        compositionRankGuideLabel.setText({}, juce::dontSendNotification);
        compositionRankGuideLabel.setVisible(false);
    }
}

juce::String MainComponent::getTrackDisplayName(int trackIndex) const
{
    if (trackIndex >= 0 && trackIndex < MAX_BANKS)
    {
        const auto customName = customTrackNames[trackIndex].trim();
        if (customName.isNotEmpty())
            return customName;
    }

    return T("Track ", L"트랙 ") + juce::String(trackIndex + 1);
}

void MainComponent::refreshTrackTabTexts()
{
    for (int i = 0; i < MAX_BANKS; ++i)
        tabButtons[i].setButtonText(getTrackDisplayName(i));
}

void MainComponent::refreshTrackNameFromPresetButtonText()
{
    trackNameFromPresetButton.setButtonText(
        trackNamesUsePresetInstruments
            ? T("Show Track Numbers", L"트랙으로 표기")
            : T("Name Tracks by Instrument", L"트랙 악기로 표기"));
}

juce::String MainComponent::getPresetInstrumentDisplayNameForBank(int bankIndex) const
{
    if (bankIndex < 0 || bankIndex >= MAX_BANKS)
        return {};

    const auto& bank = banks[bankIndex];
    if (!isMabinogiPresetInstrument(bank.instrumentWave))
        return {};

    const int fileIdx = bank.sf2FileIndex;
    const int presetIdx = bank.dlsPreset;

    if (fileIdx < 0 || fileIdx >= audioEngine.getNumEngines())
        return {};

    const juce::String fileName = audioEngine.getSf2Name(fileIdx);

    if (const char* rawName = audioEngine.getPresetName(fileIdx, presetIdx))
    {
        juce::String presetName = juce::String::fromUTF8(rawName);
        presetName = ProjectFileIO::getLocalizedSf2PresetName(presetName, fileName, presetIdx, languageCombo.getSelectedId());
        presetName = restoreLanguagePresetTranslation(presetName, fileName, presetIdx, languageCombo.getSelectedId());
        return presetName.trim();
    }

    return {};
}

void MainComponent::toggleTrackNamesFromPresets()
{
    saveCurrentBank();

    trackNamesUsePresetInstruments = !trackNamesUsePresetInstruments;

    if (trackNamesUsePresetInstruments)
    {
        for (int i = 0; i < numActiveTracks; ++i)
        {
            const auto presetName = getPresetInstrumentDisplayNameForBank(i).trim();
            customTrackNames[i] = presetName.isNotEmpty()
                ? presetName
                : (T("Track ", L"트랙 ") + juce::String(i + 1));
        }
    }
    else
    {
        for (int i = 0; i < MAX_BANKS; ++i)
            customTrackNames[i].clear();
    }

    refreshTrackTabTexts();
    refreshTrackNameFromPresetButtonText();
    markProjectDirty();

    showThemedMessageBoxAsync(juce::AlertWindow::InfoIcon,
        T("Info", L"정보"),
        trackNamesUsePresetInstruments
            ? T("All tracks are now displayed by instrument preset names.", L"전체 트랙을 악기 프리셋 이름으로 표기했습니다.")
            : T("All tracks are now displayed as Track1, Track2, ...", L"전체 트랙을 Track1, Track2 형식으로 표기했습니다."));
}

void MainComponent::copyMabinogi3PartsToClipboard() {
    saveCurrentBank();

    juce::StringArray mmlParts;

    // Mabinogi MML normally uses Melody/Chord1/Chord2.
    // If the current preset activates Song, keep the 4th field so the active Song part is preserved.
    const bool songActive = isPartActiveForBank(currentBankIndex, 3);
    const int partCount = songActive ? 4 : 3;

    for (int partIdx = 0; partIdx < partCount; ++partIdx)
    {
        const bool active = isPartActiveForBank(currentBankIndex, partIdx);
        const juce::String partMml = active ? banks[currentBankIndex].tracks[partIdx].mml.removeCharacters("\r\n") : juce::String();
        mmlParts.add(partMml);
    }

    const juce::String mabiText = juce::String("MML@") + mmlParts.joinIntoString(",") + ";";
    juce::SystemClipboard::copyTextToClipboard(mabiText);

    const juce::String selectedTrackName = "Track" + juce::String(currentBankIndex + 1);

    juce::String message;
    if (languageCombo.getSelectedId() == 3)
    {
        message = selectedTrackName + juce::String(L"を Mabinogi MML@...; 形式でコピーしました。");
    }
    else
    {
        message = selectedTrackName + T(" was copied as Mabinogi MML@...; text.", L"을 마비노기 MML@...; 형식으로 복사했습니다.");
    }

    showThemedMessageBoxAsync(juce::AlertWindow::InfoIcon,
        T("Copy Complete", L"복사 완료"),
        message);
}
void MainComponent::optimizeCurrentBankMml()
{
    if (currentBankIndex < 0 || currentBankIndex >= numActiveTracks)
        return;

    saveCurrentBank();
    updateAllSequences();

    int beforeTotal = 0;
    int afterTotal = 0;
    int optimizedParts = 0;
    juce::String report;

    for (int partIdx = 0; partIdx < 4; ++partIdx)
    {
        if (!isPartActiveForBank(currentBankIndex, partIdx))
            continue;

        auto& track = banks[currentBankIndex].tracks[partIdx];
        const juce::String before = track.mml.removeCharacters("\r\n");
        if (before.trim().isEmpty())
            continue;

        const auto optimized = buildOptimizedMmlFromPianoRollSequence(track.sequence, before);
        if (optimized.trim().isEmpty())
            continue;

        beforeTotal += before.length();

        // Safe optimizer rule:
        // The byte-golf builder regenerates MML from piano-roll events.  For some hand-written
        // scores, the original text is already shorter than the regenerated form.  In that case
        // pressing "MML 압축" must not make the score longer.
        if (optimized.length() >= before.length())
        {
            afterTotal += before.length();
            report << trackLabels[partIdx].getText() << ": "
                   << before.length() << " -> " << before.length()
                   << " " << T("(kept original; optimized candidate was longer by ", L"(원본 유지; 후보가 ")
                   << (optimized.length() - before.length())
                   << T(" chars)", L"글자 더 김)")
                   << "\n";
            continue;
        }

        afterTotal += optimized.length();
        ++optimizedParts;

        trackEditors[partIdx].setText(optimized, false);
        report << trackLabels[partIdx].getText() << ": "
               << before.length() << " -> " << optimized.length()
               << " (-" << (before.length() - optimized.length()) << ")\n";
    }

    if (beforeTotal <= 0)
    {
        showThemedMessageBoxAsync(juce::AlertWindow::InfoIcon,
            T("MML Optimizer", L"MML 압축"),
            T("The score is empty.", L"악보가 비어 있습니다."));
        return;
    }

    if (optimizedParts > 0)
    {
        markProjectDirty();
        saveCurrentBank();
        updateAllSequences();
        updateMmlCharCountLabels();
        refreshEditorTextColours();
        refreshPianoRollModel();
        repaint();
    }

    juce::String summary;
    summary << T("MML optimization complete.", L"MML 압축이 완료되었습니다.") << "\n\n"
            << report << "\n"
            << T("Before", L"압축 전") << ": " << beforeTotal << T(" characters", L" 글자") << "\n"
            << T("After", L"압축 후") << ": " << afterTotal << T(" characters", L" 글자");

    if (beforeTotal > afterTotal)
        summary << "\n" << T("Saved", L"절약") << ": " << (beforeTotal - afterTotal) << T(" characters", L" 글자");
    else
        summary << "\n" << T("No shorter rewrite found; original MML kept.", L"더 짧은 변환을 찾지 못해서 원본 MML을 유지했습니다.");

    showThemedMessageBoxAsync(juce::AlertWindow::InfoIcon,
        T("MML Optimizer", L"MML 압축"),
        summary);
}

juce::String MainComponent::makeMmlTokenForNote(int midiNote, double lengthBeats) const {
    midiNote = juce::jlimit(12, 119, midiNote);
    const int octave = juce::jlimit(0, 9, (midiNote / 12) - 1);
    const int noteIndex = ((midiNote % 12) + 12) % 12;
    static const char* names[12] = { "c", "c#", "d", "d#", "e", "f", "f#", "g", "g#", "a", "a#", "b" };

    struct Candidate { int length; bool dotted; double beats; };
    const Candidate candidates[] = {
        { 1, false, 4.0 }, { 1, true, 6.0 },
        { 2, false, 2.0 }, { 2, true, 3.0 },
        { 4, false, 1.0 }, { 4, true, 1.5 },
        { 8, false, 0.5 }, { 8, true, 0.75 },
        { 16, false, 0.25 }, { 16, true, 0.375 },
        { 32, false, 0.125 }, { 32, true, 0.1875 },
        { 64, false, 0.0625 }, { 64, true, 0.09375 }
    };

    const Candidate* best = &candidates[0];
    double bestDistance = std::abs(lengthBeats - best->beats);
    for (const auto& c : candidates) {
        const double d = std::abs(lengthBeats - c.beats);
        if (d < bestDistance) { bestDistance = d; best = &c; }
    }

    return juce::String("o") + juce::String(octave) + juce::String(names[noteIndex]) + juce::String(best->length) + (best->dotted ? "." : "");
}


void MainComponent::previewPianoRollMidi(int midiNote)
{
    if (midiNote < 0)
    {
        stopPianoRollPreviewNote();
        return;
    }

    if (!isSubScreenVisible || isPlaying || currentBankIndex < 0 || currentBankIndex >= MAX_BANKS)
    {
        stopPianoRollPreviewNote();
        return;
    }

    const auto& bank = banks[currentBankIndex];
    const int safeMidi = juce::jlimit(0, 127, midiNote);

    // 같은 음높이도 다시 들리도록 기존 미리듣기 음을 끊고 새로 건다.
    stopPianoRollPreviewNote();

    // Acoustic Piano / Square / Sawtooth / Sampler도 Mabinogi DLS 프리셋처럼
    // 피아노롤에서 노트를 찍거나 드래그할 때 바로 확인음이 나도록 처리한다.
    if (bank.instrumentWave >= 1 && bank.instrumentWave <= 4)
    {
        pianoRollPreviewSf2Index = -1;
        pianoRollPreviewPresetIndex = -1;
        pianoRollPreviewInstrumentWave = bank.instrumentWave;
        pianoRollPreviewMidi = safeMidi;
        audioEngine.previewSynthNoteOn(bank.instrumentWave, safeMidi, 0.7f);
        return;
    }

    // Mabinogi / Mabinogi Mobile 프리셋은 현재 선택된 DLS 프리셋으로 낸다.
    if (!isMabinogiPresetInstrument(bank.instrumentWave))
        return;

    const int sf2Index = bank.sf2FileIndex;
    if (sf2Index < 0 || sf2Index >= audioEngine.getNumEngines())
        return;

    const int presetCount = audioEngine.getPresetCount(sf2Index);
    if (presetCount <= 0)
        return;

    const int presetIndex = juce::jlimit(0, presetCount - 1, bank.dlsPreset);

    pianoRollPreviewSf2Index = sf2Index;
    pianoRollPreviewPresetIndex = presetIndex;
    pianoRollPreviewInstrumentWave = 0;
    pianoRollPreviewMidi = safeMidi;
    audioEngine.previewNoteOn(sf2Index, presetIndex, safeMidi, 0.7f);
}

void MainComponent::stopPianoRollPreviewNote()
{
    if (pianoRollPreviewMidi < 0)
        return;

    if (pianoRollPreviewSf2Index >= 0)
        audioEngine.previewNoteOff(pianoRollPreviewSf2Index, pianoRollPreviewPresetIndex, pianoRollPreviewMidi);
    else if (pianoRollPreviewInstrumentWave >= 1 && pianoRollPreviewInstrumentWave <= 4)
        audioEngine.previewSynthNoteOff();

    pianoRollPreviewSf2Index = -1;
    pianoRollPreviewPresetIndex = -1;
    pianoRollPreviewInstrumentWave = 0;
    pianoRollPreviewMidi = -1;
}

void MainComponent::pushMabbiicoUndoState() {
    if (!isSubScreenVisible)
        return;

    saveCurrentBank();

    MabbiicoUndoSnapshot snapshot;
    snapshot.bankIndex = currentBankIndex;
    snapshot.activePartIdx = activeMabbiicoPartIdx;
    for (int i = 0; i < 4; ++i)
        snapshot.partMml[i] = trackEditors[i].getText();

    if (!mabbiicoUndoStack.empty())
    {
        const auto& last = mabbiicoUndoStack.back();
        bool same = last.bankIndex == snapshot.bankIndex && last.activePartIdx == snapshot.activePartIdx;
        for (int i = 0; same && i < 4; ++i)
            same = (last.partMml[i] == snapshot.partMml[i]);
        if (same)
            return;
    }

    mabbiicoUndoStack.push_back(snapshot);
    if (mabbiicoUndoStack.size() > static_cast<size_t>(MAX_MABBIICO_UNDO_HISTORY))
        mabbiicoUndoStack.erase(mabbiicoUndoStack.begin());
}

void MainComponent::beginMabbiicoUndoTransaction() {
    if (!isSubScreenVisible || mabbiicoUndoTransactionActive)
        return;

    pushMabbiicoUndoState();
    mabbiicoUndoTransactionActive = true;
    beginMabbiicoMoveSnapshot();
}

void MainComponent::endMabbiicoUndoTransaction() {
    mabbiicoUndoTransactionActive = false;
    clearMabbiicoMoveSnapshot();
}

void MainComponent::beginMabbiicoMoveSnapshot() {
    if (!isSubScreenVisible || currentBankIndex < 0)
        return;

    const int partIdx = getActiveMabbiicoPartIndex();
    if (!canEditMabbiicoPart(partIdx))
        return;

    mabbiicoMoveSnapshotActive = true;
    mabbiicoMoveSnapshotBankIndex = currentBankIndex;
    mabbiicoMoveSnapshotPartIdx = partIdx;
    mabbiicoMoveSnapshotSequence = banks[currentBankIndex].tracks[partIdx].sequence;
    mabbiicoMoveSnapshotSelection = selectedPianoRollNotes;
}

void MainComponent::clearMabbiicoMoveSnapshot() {
    mabbiicoMoveSnapshotActive = false;
    mabbiicoMoveSnapshotBankIndex = -1;
    mabbiicoMoveSnapshotPartIdx = -1;
    mabbiicoMoveSnapshotSequence.clear();
    mabbiicoMoveSnapshotSelection.clear();
}

void MainComponent::ensureMabbiicoUndoState() {
    if (!isSubScreenVisible)
        return;

    if (!mabbiicoUndoTransactionActive)
        pushMabbiicoUndoState();
}

void MainComponent::undoMabbiicoEdit() {
    if (!isSubScreenVisible || mabbiicoUndoStack.empty())
        return;

    endMabbiicoUndoTransaction();

    const auto snapshot = mabbiicoUndoStack.back();
    mabbiicoUndoStack.pop_back();

    if (snapshot.bankIndex >= 0 && snapshot.bankIndex < numActiveTracks && snapshot.bankIndex != currentBankIndex)
        switchBank(snapshot.bankIndex);

    {
        juce::ScopedValueSetter<bool> dirtyGuard(suppressDirtyTracking, true);
        for (int i = 0; i < 4; ++i)
            trackEditors[i].setText(snapshot.partMml[i], false);
    }

    activeMabbiicoPartIdx = snapshot.activePartIdx;
    saveCurrentBank();
    updateAllSequences();
    updateMmlCharCountLabels();
    refreshEditorTextColours();
    refreshPianoRollModel();
    updateMabbiicoEditorLineMode();

    selectedPianoRollPartIdx = -1;
    selectedPianoRollNoteIdx = -1;
    selectedPianoRollNotes.clear();
    pianoRoll.setSelectedNotes(selectedPianoRollNotes);

    selectMabbiicoPart(isSongPresetSelected() ? 3 : activeMabbiicoPartIdx);

    markProjectDirty();
    resized();
    repaint();
}

void MainComponent::clearMabbiicoUndoHistory() {
    mabbiicoUndoStack.clear();
    mabbiicoUndoTransactionActive = false;
    clearMabbiicoMoveSnapshot();
}

void MainComponent::handlePianoRollNoteDoubleClick(int partIdx, int noteIdx, int textStart, int textEnd) {
    if (partIdx < 0 || partIdx >= 4)
        return;

    if (isSubScreenVisible)
    {
        showPianoRollNoteVolumeDialog(partIdx, noteIdx);
        return;
    }

    if (textEnd <= textStart)
        return;

    focusThreeMleEditorAtTextRange(partIdx, textStart, textEnd);
}



int MainComponent::getTempoAtBeat(double beatPosition) const
{
    int bpm = 120;
    const double targetBeat = juce::jmax(0.0, beatPosition);

    for (const auto& tc : tempoMap)
    {
        if (tc.beatPosition <= targetBeat + 0.0001)
            bpm = static_cast<int>(std::round(tc.bpm));
        else
            break;
    }

    return juce::jlimit(32, 255, bpm);
}

void MainComponent::showPianoRollTempoDialog(double beatPosition)
{
    if (!isSubScreenVisible)
        return;

    const int partIdx = shouldUseMabbiicoTrack1TempoMap() ? 0 : getActiveMabbiicoPartIndex();
    if (shouldUseMabbiicoTrack1TempoMap())
    {
        if (getMabbiicoTrack1TempoMasterBankIndex() < 0)
            return;
    }
    else if (!canEditMabbiicoPart(partIdx))
        return;

    beatPosition = juce::jmax(0.0, quantizeBeat64(beatPosition));
    seekToBeat(beatPosition);

    auto* alert = new juce::AlertWindow(T("Tempo Setting", L"템포 설정"),
                                        T("Set tempo at the red line.", L"붉은 줄 위치에 템포를 설정합니다."),
                                        juce::AlertWindow::QuestionIcon);

    const auto panel = CustomUI::getThemeColour(currentThemeId, "panel");
    const auto panel2 = CustomUI::getThemeColour(currentThemeId, "panel2");
    const auto accent = CustomUI::getThemeColour(currentThemeId, "accent");
    const auto text = CustomUI::getReadableTextColour(panel);
    alert->setColour(juce::AlertWindow::backgroundColourId, panel);
    alert->setColour(juce::AlertWindow::textColourId, text);
    alert->setColour(juce::AlertWindow::outlineColourId, accent);
    alert->setColour(juce::TextButton::buttonColourId, accent);
    alert->setColour(juce::TextButton::textColourOffId, CustomUI::getReadableTextColour(accent));
    alert->setColour(juce::TextEditor::backgroundColourId, panel2);
    alert->setColour(juce::TextEditor::textColourId, CustomUI::getReadableTextColour(panel2));
    alert->setColour(juce::TextEditor::outlineColourId, accent.withAlpha(0.6f));
    alert->setColour(juce::CaretComponent::caretColourId, accent);

    const int currentTempo = getTempoAtBeat(beatPosition);
    alert->addTextEditor("tempoBpm", juce::String(currentTempo), T("BPM", L"BPM"));
    alert->addButton(T("Apply", L"적용"), 1, juce::KeyPress(juce::KeyPress::returnKey));
    alert->addButton(T("Delete Tempo", L"템포 삭제"), 2);
    alert->addButton(T("Cancel", L"취소"), 0, juce::KeyPress(juce::KeyPress::escapeKey));

    juce::Component::SafePointer<juce::AlertWindow> safeAlert(alert);
    alert->enterModalState(true, juce::ModalCallbackFunction::create([this, safeAlert, beatPosition](int result) mutable
    {
        if (safeAlert == nullptr)
            return;

        if (result == 2)
        {
            removePianoRollTempoAtBeat(beatPosition);
            return;
        }

        if (result != 1)
            return;

        const int bpm = safeAlert->getTextEditorContents("tempoBpm").retainCharacters("0123456789").getIntValue();
        if (bpm < 32 || bpm > 255)
        {
            showThemedMessageBoxAsync(juce::AlertWindow::WarningIcon,
                                      T("Tempo Setting", L"템포 설정"),
                                      T("Tempo must be between 32 and 255.", L"템포는 32에서 255 사이로 입력해주세요."));
            return;
        }

        applyPianoRollTempoAtBeat(beatPosition, bpm);
    }), true);
}

void MainComponent::applyPianoRollTempoAtBeat(double beatPosition, int bpm)
{
    if (!isSubScreenVisible)
        return;

    const int partIdx = shouldUseMabbiicoTrack1TempoMap() ? 0 : getActiveMabbiicoPartIndex();
    if (shouldUseMabbiicoTrack1TempoMap())
    {
        if (getMabbiicoTrack1TempoMasterBankIndex() < 0)
            return;
    }
    else if (!canEditMabbiicoPart(partIdx))
        return;

    beatPosition = juce::jmax(0.0, quantizeBeat64(beatPosition));
    bpm = juce::jlimit(32, 255, bpm);

    const int targetBankIdx = shouldUseMabbiicoTrack1TempoMap() ? getMabbiicoTrack1TempoMasterBankIndex() : currentBankIndex;
    if (targetBankIdx < 0 || targetBankIdx >= numActiveTracks)
        return;

    auto& track = banks[targetBankIdx].tracks[partIdx];

    auto tempoEvents = extractTempoEventsFromMml(track.mml);
    tempoEvents.erase(std::remove_if(tempoEvents.begin(), tempoEvents.end(), [beatPosition](const PianoRollTempoEvent& ev)
    {
        return std::abs(ev.beat - beatPosition) <= 0.0001;
    }), tempoEvents.end());

    int leftTempo = 120;
    for (const auto& ev : tempoEvents)
    {
        if (ev.beat < beatPosition - 0.0001)
            leftTempo = ev.bpm;
        else
            break;
    }

    // If the selected tempo is already active immediately to the left, remove
    // the marker at this point instead of making a redundant tempo event.
    if (beatPosition <= 0.0001 || bpm != leftTempo)
        tempoEvents.push_back({ beatPosition, bpm });

    ensureMabbiicoUndoState();

    const auto regeneratedMml = buildMmlFromPianoRollSequenceWithTempoEvents(track.sequence, track.mml, tempoEvents);
    track.mml = regeneratedMml;
    if (targetBankIdx == currentBankIndex)
        trackEditors[partIdx].setText(regeneratedMml, false);

    if (shouldUseMabbiicoTrack1TempoMap())
        syncMabbiicoTempoEventsFromTrack1();

    markProjectDirty();
    saveCurrentBank();
    updateAllSequences();
    updateMmlCharCountLabels();
    refreshEditorTextColours();
    refreshPianoRollModel();
    seekToBeat(beatPosition);
    pianoRoll.repaint();
}

void MainComponent::removePianoRollTempoAtBeat(double beatPosition)
{
    if (!isSubScreenVisible)
        return;

    const int partIdx = shouldUseMabbiicoTrack1TempoMap() ? 0 : getActiveMabbiicoPartIndex();
    if (shouldUseMabbiicoTrack1TempoMap())
    {
        if (getMabbiicoTrack1TempoMasterBankIndex() < 0)
            return;
    }
    else if (!canEditMabbiicoPart(partIdx))
        return;

    beatPosition = juce::jmax(0.0, quantizeBeat64(beatPosition));

    const int targetBankIdx = shouldUseMabbiicoTrack1TempoMap() ? getMabbiicoTrack1TempoMasterBankIndex() : currentBankIndex;
    if (targetBankIdx < 0 || targetBankIdx >= numActiveTracks)
        return;

    auto& track = banks[targetBankIdx].tracks[partIdx];
    auto tempoEvents = extractTempoEventsFromMml(track.mml);

    const auto oldSize = tempoEvents.size();
    tempoEvents.erase(std::remove_if(tempoEvents.begin(), tempoEvents.end(), [beatPosition](const PianoRollTempoEvent& ev)
    {
        return std::abs(ev.beat - beatPosition) <= 0.0001;
    }), tempoEvents.end());

    if (tempoEvents.size() == oldSize)
    {
        showThemedMessageBoxAsync(juce::AlertWindow::InfoIcon,
                                  T("Tempo Setting", L"템포 설정"),
                                  T("No tempo marker exists at the red line.", L"붉은 줄 위치에 삭제할 템포가 없습니다."));
        return;
    }

    ensureMabbiicoUndoState();

    const auto regeneratedMml = buildMmlFromPianoRollSequenceWithTempoEvents(track.sequence, track.mml, tempoEvents);
    track.mml = regeneratedMml;
    if (targetBankIdx == currentBankIndex)
        trackEditors[partIdx].setText(regeneratedMml, false);

    if (shouldUseMabbiicoTrack1TempoMap())
        syncMabbiicoTempoEventsFromTrack1();

    markProjectDirty();
    saveCurrentBank();
    updateAllSequences();
    updateMmlCharCountLabels();
    refreshEditorTextColours();
    refreshPianoRollModel();
    seekToBeat(beatPosition);
    pianoRoll.repaint();
}

void MainComponent::showPianoRollNoteVolumeDialog(int partIdx, int noteIdx) {
    const bool canEditVolume = canEditMabbiicoPart(partIdx)
        && noteIdx >= 0;
    if (!canEditVolume)
        return;

    const auto& seq = banks[currentBankIndex].tracks[partIdx].sequence;
    if (noteIdx >= static_cast<int>(seq.size()) || seq[static_cast<size_t>(noteIdx)].frequency <= 0.0)
        return;

    const int currentVolume = volumeFloatToMml(seq[static_cast<size_t>(noteIdx)].volume);
    juce::StringArray volumeItems;
    for (int i = 0; i <= 15; ++i)
        volumeItems.add(juce::String("v") + juce::String(i));

    auto* alert = new juce::AlertWindow(T("Note Volume", L"노트 볼륨"),
                                        T("Select volume for this note. It will be kept until the next volume change.",
                                          L"이 노트부터 다음 볼륨 변경 전까지 유지할 볼륨을 선택하세요."),
                                        juce::AlertWindow::QuestionIcon);

    const auto panel = CustomUI::getThemeColour(currentThemeId, "panel");
    const auto panel2 = CustomUI::getThemeColour(currentThemeId, "panel2");
    const auto accent = CustomUI::getThemeColour(currentThemeId, "accent");
    const auto text = CustomUI::getReadableTextColour(panel);
    alert->setColour(juce::AlertWindow::backgroundColourId, panel);
    alert->setColour(juce::AlertWindow::textColourId, text);
    alert->setColour(juce::AlertWindow::outlineColourId, accent);
    alert->setColour(juce::TextButton::buttonColourId, accent);
    alert->setColour(juce::TextButton::textColourOffId, CustomUI::getReadableTextColour(accent));
    alert->setColour(juce::ComboBox::backgroundColourId, panel2);
    alert->setColour(juce::ComboBox::textColourId, CustomUI::getReadableTextColour(panel2));
    alert->setColour(juce::ComboBox::outlineColourId, accent.withAlpha(0.6f));

    alert->addComboBox("noteVolume", volumeItems, T("Volume", L"볼륨"));
    if (auto* combo = alert->getComboBoxComponent("noteVolume"))
        combo->setSelectedId(currentVolume + 1, juce::dontSendNotification);

    alert->addButton(T("Apply", L"적용"), 1, juce::KeyPress(juce::KeyPress::returnKey));
    alert->addButton(T("Cancel", L"취소"), 0, juce::KeyPress(juce::KeyPress::escapeKey));

    juce::Component::SafePointer<juce::AlertWindow> safeAlert(alert);
    alert->enterModalState(true, juce::ModalCallbackFunction::create([this, safeAlert, partIdx, noteIdx](int result) mutable
    {
        if (result != 1 || safeAlert == nullptr)
            return;

        if (auto* combo = safeAlert->getComboBoxComponent("noteVolume"))
            applyPianoRollNoteVolume(partIdx, noteIdx, combo->getSelectedId() - 1);
    }), true);
}

void MainComponent::applyPianoRollNoteVolume(int partIdx, int noteIdx, int volumeValue) {
    const bool canEditVolume = canEditMabbiicoPart(partIdx)
        && noteIdx >= 0;
    if (!canEditVolume)
        return;

    auto& track = banks[currentBankIndex].tracks[partIdx];
    if (noteIdx >= static_cast<int>(track.sequence.size()))
        return;

    const auto originalNote = track.sequence[static_cast<size_t>(noteIdx)];
    if (originalNote.frequency <= 0.0)
        return;

    ensureMabbiicoUndoState();

    const float oldVolume = juce::jlimit(0.0f, 1.0f, originalNote.volume);
    const float newVolume = volumeMmlToFloat(volumeValue);

    auto editedSequence = track.sequence;
    for (int i = noteIdx; i < static_cast<int>(editedSequence.size()); ++i)
    {
        auto& ev = editedSequence[static_cast<size_t>(i)];
        if (std::abs(ev.volume - oldVolume) > 0.0001f)
            break;
        ev.volume = newVolume;
    }

    const auto regeneratedMml = buildMmlFromPianoRollSequence(editedSequence, track.mml);
    trackEditors[partIdx].setText(regeneratedMml, false);

    markProjectDirty();
    saveCurrentBank();
    updateAllSequences();
    updateMmlCharCountLabels();
    refreshEditorTextColours();
    refreshPianoRollModel();

    selectedPianoRollPartIdx = partIdx;
    selectedPianoRollNoteIdx = -1;
    selectedPianoRollNotes.clear();

    const auto& refreshedSeq = banks[currentBankIndex].tracks[partIdx].sequence;
    for (int i = 0; i < static_cast<int>(refreshedSeq.size()); ++i)
    {
        const auto& n = refreshedSeq[static_cast<size_t>(i)];
        if (std::abs(n.startBeat - originalNote.startBeat) < 0.0001
            && std::abs(n.endBeat - originalNote.endBeat) < 0.0001
            && frequencyToMidiForMml(n.frequency) == frequencyToMidiForMml(originalNote.frequency))
        {
            selectedPianoRollNoteIdx = i;
            selectedPianoRollNotes.push_back({ partIdx, i });
            break;
        }
    }

    pianoRoll.setSelectedNotes(selectedPianoRollNotes);
    pianoRoll.repaint();
}


void MainComponent::focusThreeMleEditorAtTextRange(int partIdx, int textStart, int textEnd)
{
    if (isSubScreenVisible)
        return;

    if (partIdx < 0 || partIdx >= 4)
        return;

    auto* editor = &trackEditors[partIdx];
    if (!editor->isVisible())
        return;

    const int textLength = editor->getText().length();
    const int start = juce::jlimit(0, textLength, textStart);
    const int end = juce::jlimit(start, textLength, textEnd);

    if (end <= start)
        return;

    // Clear only the visual selection from the other editors while preserving their caret positions.
    for (int i = 0; i < 4; ++i)
    {
        if (i == partIdx || !trackEditors[i].isVisible())
            continue;

        const int caret = juce::jlimit(0, trackEditors[i].getText().length(), trackEditors[i].getCaretPosition());
        trackEditors[i].setHighlightedRegion(juce::Range<int>(caret, caret));
    }

    const auto editorHighlight = getEditorPlaybackHighlightColour(partIdx);
    editor->setColour(juce::TextEditor::highlightColourId, editorHighlight);
    editor->setColour(juce::TextEditor::highlightedTextColourId, CustomUI::getReadableTextColour(editorHighlight));

    editor->grabKeyboardFocus();
    editor->setCaretPosition(start);
    editor->setHighlightedRegion(juce::Range<int>(start, end));
    editor->repaint();

    // Manual 3MLE note selection is not playback highlight.
    // Keep this false so timer/updatePlaybackEditorHighlights does not clear it while stopped.
    playbackEditorHighlightStart[partIdx] = start;
    playbackEditorHighlightEnd[partIdx] = end;
    playbackEditorHighlightActive = false;
}

void MainComponent::focusThreeMleEditorAtPianoRollNote(int partIdx, int noteIdx)
{
    if (isSubScreenVisible)
        return;

    if (partIdx < 0 || partIdx >= 4 || noteIdx < 0)
        return;

    const auto& sequence = banks[currentBankIndex].tracks[partIdx].sequence;
    if (noteIdx >= static_cast<int>(sequence.size()))
        return;

    const auto& note = sequence[static_cast<size_t>(noteIdx)];
    if (note.textStart < 0 || note.textEnd <= note.textStart)
        return;

    focusThreeMleEditorAtTextRange(partIdx, note.textStart, note.textEnd);
}

void MainComponent::handlePianoRollNoteSelected(int partIdx, int noteIdx) {
    selectedPianoRollPartIdx = partIdx;
    selectedPianoRollNoteIdx = noteIdx;
    selectedPianoRollNotes.clear();
    if (partIdx >= 0 && noteIdx >= 0)
    {
        selectedPianoRollNotes.push_back({ partIdx, noteIdx });
        focusThreeMleEditorAtPianoRollNote(partIdx, noteIdx);
    }
}

void MainComponent::handlePianoRollNotesSelected(const std::vector<std::pair<int, int>>& notes) {
    selectedPianoRollNotes = notes;
    if (selectedPianoRollNotes.size() == 1)
    {
        selectedPianoRollPartIdx = selectedPianoRollNotes.front().first;
        selectedPianoRollNoteIdx = selectedPianoRollNotes.front().second;
        focusThreeMleEditorAtPianoRollNote(selectedPianoRollPartIdx, selectedPianoRollNoteIdx);
    }
    else
    {
        selectedPianoRollPartIdx = selectedPianoRollNotes.empty() ? -1 : selectedPianoRollNotes.front().first;
        selectedPianoRollNoteIdx = -1;
    }
}

void MainComponent::copySelectedPianoRollNotes() {
    copiedPianoRollNotes.clear();
    copiedPianoRollBaseBeat = 0.0;

    if (!isSubScreenVisible || selectedPianoRollNotes.empty())
        return;

    const int partIdx = getActiveMabbiicoPartIndex();
    if (!canEditMabbiicoPart(partIdx))
        return;

    const auto& seq = banks[currentBankIndex].tracks[partIdx].sequence;
    double minBeat = std::numeric_limits<double>::max();

    std::vector<int> indices;
    for (const auto& selected : selectedPianoRollNotes)
    {
        if (selected.first != partIdx)
            continue;
        if (selected.second < 0 || selected.second >= static_cast<int>(seq.size()))
            continue;
        indices.push_back(selected.second);
    }

    std::sort(indices.begin(), indices.end());
    indices.erase(std::unique(indices.begin(), indices.end()), indices.end());

    for (int idx : indices)
    {
        const auto& note = seq[static_cast<size_t>(idx)];
        if (note.frequency <= 0.0)
            continue;
        copiedPianoRollNotes.push_back(note);
        minBeat = std::min(minBeat, note.startBeat);
    }

    if (copiedPianoRollNotes.empty())
        return;

    std::stable_sort(copiedPianoRollNotes.begin(), copiedPianoRollNotes.end(), [](const MmlNote& a, const MmlNote& b)
    {
        if (std::abs(a.startBeat - b.startBeat) > 0.0001)
            return a.startBeat < b.startBeat;
        return a.endBeat < b.endBeat;
    });

    copiedPianoRollBaseBeat = minBeat == std::numeric_limits<double>::max() ? 0.0 : minBeat;
}

double MainComponent::getCurrentPlayheadBeatForPaste() const {
    if (audioEngine.getSampleRate() <= 0.0 || tempoMap.empty())
        return 0.0;
    return juce::jmax(0.0, quantizeBeat64(MmlLogic::getBeatFromSample(globalSampleCount, tempoMap, audioEngine.getSampleRate())));
}

void MainComponent::pasteCopiedPianoRollNotes() {
    if (!isSubScreenVisible || copiedPianoRollNotes.empty())
        return;

    const int partIdx = getActiveMabbiicoPartIndex();
    if (!canEditMabbiicoPart(partIdx))
        return;

    ensureMabbiicoUndoState();

    auto& track = banks[currentBankIndex].tracks[partIdx];
    const double pasteBeat = getCurrentPlayheadBeatForPaste();

    std::vector<MmlNote> pastedNotes;
    pastedNotes.reserve(copiedPianoRollNotes.size());
    for (auto note : copiedPianoRollNotes)
    {
        const double offset = note.startBeat - copiedPianoRollBaseBeat;
        const double length = juce::jmax(0.0625, note.endBeat - note.startBeat);
        note.startBeat = juce::jmax(0.0, quantizeBeat64(pasteBeat + offset));
        note.endBeat = juce::jmax(note.startBeat + 0.0625, quantizeBeat64(note.startBeat + length));
        note.textStart = -1;
        note.textEnd = -1;
        pastedNotes.push_back(note);
    }

    std::vector<MmlNote> editedSequence;
    editedSequence.reserve(track.sequence.size() + pastedNotes.size());

    for (const auto& oldNote : track.sequence)
    {
        if (oldNote.frequency <= 0.0)
            continue;

        std::vector<std::pair<double, double>> segments { { oldNote.startBeat, oldNote.endBeat } };
        for (const auto& pasted : pastedNotes)
        {
            std::vector<std::pair<double, double>> nextSegments;
            for (const auto& segment : segments)
            {
                const double s = segment.first;
                const double e = segment.second;
                const bool overlaps = s < pasted.endBeat - 0.0001 && e > pasted.startBeat + 0.0001;
                if (!overlaps)
                {
                    nextSegments.push_back(segment);
                    continue;
                }

                if (s < pasted.startBeat - 0.0001)
                    nextSegments.push_back({ s, pasted.startBeat });
                if (e > pasted.endBeat + 0.0001)
                    nextSegments.push_back({ pasted.endBeat, e });
            }
            segments = std::move(nextSegments);
            if (segments.empty())
                break;
        }

        for (const auto& segment : segments)
        {
            if (segment.second - segment.first < 0.0625 - 0.0001)
                continue;
            auto kept = oldNote;
            kept.startBeat = segment.first;
            kept.endBeat = segment.second;
            editedSequence.push_back(kept);
        }
    }

    for (const auto& pasted : pastedNotes)
        editedSequence.push_back(pasted);

    std::stable_sort(editedSequence.begin(), editedSequence.end(), [](const MmlNote& a, const MmlNote& b)
    {
        if (std::abs(a.startBeat - b.startBeat) > 0.0001)
            return a.startBeat < b.startBeat;
        return a.endBeat < b.endBeat;
    });

    const auto regeneratedMml = buildMmlFromPianoRollSequence(editedSequence, track.mml);
    trackEditors[partIdx].setText(regeneratedMml, false);

    markProjectDirty();
    saveCurrentBank();
    updateAllSequences();
    updateMmlCharCountLabels();
    refreshEditorTextColours();
    refreshPianoRollModel();

    selectedPianoRollNotes.clear();
    const auto& refreshedSeq = banks[currentBankIndex].tracks[partIdx].sequence;
    std::vector<bool> used(refreshedSeq.size(), false);
    for (const auto& pasted : pastedNotes)
    {
        for (int i = 0; i < static_cast<int>(refreshedSeq.size()); ++i)
        {
            if (used[static_cast<size_t>(i)])
                continue;
            const auto& n = refreshedSeq[static_cast<size_t>(i)];
            if (std::abs(n.startBeat - pasted.startBeat) < 0.0001
                && std::abs(n.endBeat - pasted.endBeat) < 0.0001
                && frequencyToMidiForMml(n.frequency) == frequencyToMidiForMml(pasted.frequency))
            {
                selectedPianoRollNotes.push_back({ partIdx, i });
                used[static_cast<size_t>(i)] = true;
                break;
            }
        }
    }

    if (selectedPianoRollNotes.size() == 1)
    {
        selectedPianoRollPartIdx = selectedPianoRollNotes.front().first;
        selectedPianoRollNoteIdx = selectedPianoRollNotes.front().second;
    }
    else
    {
        selectedPianoRollPartIdx = selectedPianoRollNotes.empty() ? -1 : selectedPianoRollNotes.front().first;
        selectedPianoRollNoteIdx = -1;
    }
    pianoRoll.setSelectedNotes(selectedPianoRollNotes);
    pianoRoll.repaint();
}

void MainComponent::deleteSelectedPianoRollNotes() {
    if (!isSubScreenVisible || selectedPianoRollNotes.empty())
        return;

    const int partIdx = getActiveMabbiicoPartIndex();
    if (!canEditMabbiicoPart(partIdx))
        return;

    auto& track = banks[currentBankIndex].tracks[partIdx];
    std::vector<int> indices;
    for (const auto& selected : selectedPianoRollNotes)
    {
        if (selected.first != partIdx)
            continue;
        if (selected.second >= 0 && selected.second < static_cast<int>(track.sequence.size()))
            indices.push_back(selected.second);
    }

    if (indices.empty())
        return;

    ensureMabbiicoUndoState();

    std::sort(indices.begin(), indices.end());
    indices.erase(std::unique(indices.begin(), indices.end()), indices.end());

    auto editedSequence = track.sequence;
    for (auto it = indices.rbegin(); it != indices.rend(); ++it)
        editedSequence.erase(editedSequence.begin() + *it);

    const auto regeneratedMml = buildMmlFromPianoRollSequence(editedSequence, track.mml);
    trackEditors[partIdx].setText(regeneratedMml, false);
    trackEditors[partIdx].setCaretPosition(0);

    selectedPianoRollNotes.clear();
    selectedPianoRollPartIdx = -1;
    selectedPianoRollNoteIdx = -1;
    pianoRoll.setSelectedNotes(selectedPianoRollNotes);

    markProjectDirty();
    saveCurrentBank();
    updateAllSequences();
    updateMmlCharCountLabels();
    refreshEditorTextColours();
    refreshPianoRollModel();
    pianoRoll.repaint();
}

void MainComponent::moveSelectedPianoRollNotesPitch(int semitoneDelta) {
    (void) moveSelectedPianoRollNotes(semitoneDelta, 0.0);
}

bool MainComponent::moveSelectedPianoRollNotes(int semitoneDelta, double beatDelta) {
    if (!isSubScreenVisible)
        return false;

    if (semitoneDelta == 0 && std::abs(beatDelta) <= 0.0001)
        return false;

    const int partIdx = getActiveMabbiicoPartIndex();
    if (!canEditMabbiicoPart(partIdx))
        return false;

    auto& track = banks[currentBankIndex].tracks[partIdx];

    // "계산해서 누적"하지 않고, 마우스를 누른 순간의 악보를 기준으로
    // 선택된 노트 묶음 전체를 하나의 물체처럼 이동시킨다.
    const bool useSnapshot = mabbiicoMoveSnapshotActive
        && mabbiicoMoveSnapshotBankIndex == currentBankIndex
        && mabbiicoMoveSnapshotPartIdx == partIdx
        && !mabbiicoMoveSnapshotSequence.empty()
        && !mabbiicoMoveSnapshotSelection.empty();

    const auto& baseSequence = useSnapshot ? mabbiicoMoveSnapshotSequence : track.sequence;
    const auto& baseSelection = useSnapshot ? mabbiicoMoveSnapshotSelection : selectedPianoRollNotes;

    std::vector<int> indices;
    for (const auto& selected : baseSelection)
    {
        if (selected.first != partIdx)
            continue;
        if (selected.second >= 0 && selected.second < static_cast<int>(baseSequence.size()))
            indices.push_back(selected.second);
    }

    if (indices.empty())
        return false;

    std::sort(indices.begin(), indices.end());
    indices.erase(std::unique(indices.begin(), indices.end()), indices.end());

    std::vector<bool> selectedMask(baseSequence.size(), false);
    for (int idx : indices)
        selectedMask[static_cast<size_t>(idx)] = true;

    beatDelta = quantizeBeat64(beatDelta);

    // 왼쪽 끝 제한: 선택 묶음의 가장 왼쪽 노트를 기준으로
    // 이동량 자체를 제한한다. 개별 노트를 잘라내거나 재계산하지 않는다.
    double minOriginalStart = std::numeric_limits<double>::max();
    for (int idx : indices)
    {
        const auto& note = baseSequence[static_cast<size_t>(idx)];
        if (note.frequency <= 0.0)
            continue;
        minOriginalStart = std::min(minOriginalStart, quantizeBeat64(note.startBeat));
    }

    if (minOriginalStart == std::numeric_limits<double>::max())
        return false;

    if (beatDelta < -minOriginalStart)
        beatDelta = -minOriginalStart;
    beatDelta = quantizeBeat64(beatDelta);

    auto makeMovedNotes = [&]()
    {
        std::vector<MmlNote> moved;
        moved.reserve(indices.size());

        for (int idx : indices)
        {
            auto note = baseSequence[static_cast<size_t>(idx)];
            if (note.frequency <= 0.0)
                continue;

            const double oldLength = juce::jmax(0.0625, quantizeBeat64(note.endBeat - note.startBeat));
            const double newStartBeat = juce::jmax(0.0, quantizeBeat64(note.startBeat + beatDelta));

            if (semitoneDelta != 0)
            {
                const int oldMidi = frequencyToMidiForMml(note.frequency);
                const int newMidi = juce::jlimit(12, 108, oldMidi + semitoneDelta);
                note.frequency = midiToFrequencyForMml(newMidi);
            }

            note.startBeat = newStartBeat;
            note.endBeat = quantizeBeat64(note.startBeat + oldLength);
            if (note.endBeat <= note.startBeat + 0.0001)
                note.endBeat = note.startBeat + oldLength;

            moved.push_back(note);
        }

        return moved;
    };

    std::vector<MmlNote> movedSelectedNotes = makeMovedNotes();
    if (movedSelectedNotes.empty())
        return false;

    auto overlapsMovedSelection = [&](const MmlNote& other)
    {
        if (other.frequency <= 0.0)
            return false;

        for (const auto& moved : movedSelectedNotes)
        {
            if (moved.frequency <= 0.0)
                continue;

            const bool overlaps = moved.startBeat < other.endBeat - 0.0001
                                && moved.endBeat > other.startBeat + 0.0001;
            if (overlaps)
                return true;
        }

        return false;
    };

    // 왼쪽 마우스 이동은 마우스를 뗀 순간 한 번만 확정한다.
    // 확정 지점에 기존 노트가 있으면, 현재 잡고 이동한 노트가 우선이다.
    // 그래서 기존 비선택 노트와 겹치더라도 이동을 막지 않고,
    // 겹친 기존 노트만 제거한 뒤 이동 노트를 넣는다.

    bool changed = false;
    for (size_t n = 0; n < indices.size() && n < movedSelectedNotes.size(); ++n)
    {
        const auto& oldNote = baseSequence[static_cast<size_t>(indices[n])];
        const auto& newNote = movedSelectedNotes[n];
        if (std::abs(oldNote.startBeat - newNote.startBeat) > 0.0001
            || std::abs(oldNote.endBeat - newNote.endBeat) > 0.0001
            || std::abs(oldNote.frequency - newNote.frequency) > 0.0001)
        {
            changed = true;
            break;
        }
    }

    if (!changed)
        return false;

    std::vector<MmlNote> editedSequence;
    editedSequence.reserve(baseSequence.size());

    size_t movedIndex = 0;
    for (int i = 0; i < static_cast<int>(baseSequence.size()); ++i)
    {
        if (selectedMask[static_cast<size_t>(i)])
        {
            if (movedIndex < movedSelectedNotes.size())
                editedSequence.push_back(movedSelectedNotes[movedIndex++]);
        }
        else
        {
            const auto& other = baseSequence[static_cast<size_t>(i)];
            if (!overlapsMovedSelection(other))
                editedSequence.push_back(other);
        }
    }

    std::stable_sort(editedSequence.begin(), editedSequence.end(), [](const MmlNote& a, const MmlNote& b)
    {
        if (std::abs(a.startBeat - b.startBeat) > 0.0001)
            return a.startBeat < b.startBeat;
        return a.endBeat < b.endBeat;
    });

    ensureMabbiicoUndoState();

    const auto regeneratedMml = buildMmlFromPianoRollSequence(editedSequence, track.mml);
    trackEditors[partIdx].setText(regeneratedMml, false);

    markProjectDirty();
    saveCurrentBank();
    updateAllSequences();
    updateMmlCharCountLabels();
    refreshEditorTextColours();
    refreshPianoRollModel();

    std::vector<std::pair<int, int>> stillSelected;
    const auto& refreshedSeq = banks[currentBankIndex].tracks[partIdx].sequence;
    std::vector<bool> used(refreshedSeq.size(), false);

    for (const auto& moved : movedSelectedNotes)
    {
        int bestIdx = -1;
        double bestScore = 999999.0;
        for (int i = 0; i < static_cast<int>(refreshedSeq.size()); ++i)
        {
            if (used[static_cast<size_t>(i)])
                continue;

            const auto& candidate = refreshedSeq[static_cast<size_t>(i)];
            const double score = std::abs(candidate.startBeat - moved.startBeat)
                               + std::abs(candidate.endBeat - moved.endBeat)
                               + std::abs(candidate.frequency - moved.frequency) * 0.001;
            if (score < bestScore)
            {
                bestScore = score;
                bestIdx = i;
            }
        }

        if (bestIdx >= 0 && bestScore < 0.05)
        {
            used[static_cast<size_t>(bestIdx)] = true;
            stillSelected.push_back({ partIdx, bestIdx });
        }
    }

    selectedPianoRollNotes = stillSelected;
    if (selectedPianoRollNotes.size() == 1)
    {
        selectedPianoRollPartIdx = selectedPianoRollNotes.front().first;
        selectedPianoRollNoteIdx = selectedPianoRollNotes.front().second;
    }
    else
    {
        selectedPianoRollPartIdx = selectedPianoRollNotes.empty() ? -1 : selectedPianoRollNotes.front().first;
        selectedPianoRollNoteIdx = -1;
    }

    pianoRoll.setSelectedNotes(selectedPianoRollNotes);
    pianoRoll.repaint();
    return true;
}

void MainComponent::selectMabbiicoPart(int partIdx) {
    // 일반 mabbiico 모드에서는 멜로디/화음1/화음2 중 하나만 선택한다.
    // 노래 프리셋일 때는 UI 체크박스를 숨기고 노래 파트(3)만 활성화한다.
    // 마비노기 실로폰은 화음1/화음2를 사용하지 않으므로 멜로디만 선택한다.
    if (isXylophonePresetSelected())
        partIdx = 0;

    if (partIdx == 3)
        activeMabbiicoPartIdx = 3;
    else
        activeMabbiicoPartIdx = juce::jlimit(0, 2, partIdx);

    juce::ScopedValueSetter<bool> guard(suppressSubPartCheckBoxCallback, true);
    for (int i = 0; i < 3; ++i)
        subPartCheckBoxes[i].setToggleState(activeMabbiicoPartIdx < 3 && i == activeMabbiicoPartIdx, juce::dontSendNotification);

    selectedPianoRollPartIdx = -1;
    selectedPianoRollNoteIdx = -1;
    selectedPianoRollNotes.clear();
    pianoRoll.setSelectedNotes(selectedPianoRollNotes);
    pianoRoll.repaint();
}

int MainComponent::getActiveMabbiicoPartIndex() const {
    if (!isSubScreenVisible)
        return -1;

    if (isSongPresetSelected())
        return 3;

    if (isXylophonePresetSelected())
        return 0;

    return juce::jlimit(0, 2, activeMabbiicoPartIdx);
}

bool MainComponent::canEditMabbiicoPart(int partIdx) const {
    if (!isSubScreenVisible || partIdx < 0 || partIdx >= 4)
        return false;

    if (isXylophonePresetSelected() && partIdx != 0)
        return false;

    return partIdx == getActiveMabbiicoPartIndex() && trackEditors[partIdx].isVisible();
}

double MainComponent::getSelectedNoteLengthBeats() const {
    const int denom = noteLengthCombo.getSelectedId() > 0 ? noteLengthCombo.getSelectedId() : 4;
    return juce::jlimit(0.0625, 4.0, 4.0 / static_cast<double>(denom));
}

void MainComponent::applySelectedNoteLengthToPianoRollNote() {
    if (!isSubScreenVisible) return;
    const int partIdx = selectedPianoRollPartIdx;
    const int noteIdx = selectedPianoRollNoteIdx;
    if (!canEditMabbiicoPart(partIdx) || noteIdx < 0) return;

    auto& seq = banks[currentBankIndex].tracks[partIdx].sequence;
    if (noteIdx >= static_cast<int>(seq.size())) return;
    const auto& note = seq[static_cast<size_t>(noteIdx)];
    if (note.frequency <= 0.0) return;

    handlePianoRollNoteEdit(partIdx, noteIdx, frequencyToMidiForMml(note.frequency), getSelectedNoteLengthBeats());
}

void MainComponent::handlePianoRollNoteInsert(int partIdx, int midiNote, double startBeat, double lengthBeats) {
    const bool canInsertFromPianoRoll = canEditMabbiicoPart(partIdx);
    if (!canInsertFromPianoRoll) return;

    ensureMabbiicoUndoState();

    startBeat = juce::jmax(0.0, quantizeBeat64(startBeat));
    lengthBeats = juce::jmax(0.0625, quantizeBeat64(lengthBeats));
    const double endBeat = startBeat + lengthBeats;

    auto& track = banks[currentBankIndex].tracks[partIdx];
    std::vector<MmlNote> editedSequence;
    editedSequence.reserve(track.sequence.size() + 1);

    // 한 파트는 MML 특성상 동시에 두 음을 낼 수 없으므로, 새 노트와 겹치는
    // 기존 노트는 잘라내거나 제거해서 mabbiico처럼 깔끔하게 찍히도록 한다.
    for (const auto& oldNote : track.sequence)
    {
        if (oldNote.frequency <= 0.0)
            continue;

        const bool overlaps = oldNote.startBeat < endBeat - 0.0001 && oldNote.endBeat > startBeat + 0.0001;
        if (!overlaps)
        {
            editedSequence.push_back(oldNote);
            continue;
        }

        if (oldNote.startBeat < startBeat - 0.0001)
        {
            auto left = oldNote;
            left.endBeat = startBeat;
            if (left.endBeat - left.startBeat >= 0.0625 - 0.0001)
                editedSequence.push_back(left);
        }

        if (oldNote.endBeat > endBeat + 0.0001)
        {
            auto right = oldNote;
            right.startBeat = endBeat;
            if (right.endBeat - right.startBeat >= 0.0625 - 0.0001)
                editedSequence.push_back(right);
        }
    }

    MmlNote newNote;
    newNote.frequency = midiToFrequencyForMml(midiNote);
    newNote.startBeat = startBeat;
    newNote.endBeat = endBeat;
    newNote.volume = getInheritedVolumeAtBeat(track.sequence, startBeat);
    newNote.isTie = false;
    newNote.startSample = 0;
    newNote.endSample = 0;
    newNote.textStart = -1;
    newNote.textEnd = -1;
    editedSequence.push_back(newNote);

    std::stable_sort(editedSequence.begin(), editedSequence.end(), [](const MmlNote& a, const MmlNote& b)
    {
        if (std::abs(a.startBeat - b.startBeat) > 0.0001)
            return a.startBeat < b.startBeat;
        return a.endBeat < b.endBeat;
    });

    const auto regeneratedMml = buildMmlFromPianoRollSequence(editedSequence, track.mml);
    trackEditors[partIdx].setText(regeneratedMml, false);

    markProjectDirty();
    saveCurrentBank();
    updateAllSequences();
    updateMmlCharCountLabels();
    refreshEditorTextColours();
    refreshPianoRollModel();

    selectedPianoRollPartIdx = partIdx;
    selectedPianoRollNoteIdx = -1;
    auto& refreshedSeq = banks[currentBankIndex].tracks[partIdx].sequence;
    for (int i = 0; i < static_cast<int>(refreshedSeq.size()); ++i)
    {
        const auto& n = refreshedSeq[static_cast<size_t>(i)];
        if (std::abs(n.startBeat - startBeat) < 0.0001
            && std::abs(n.endBeat - endBeat) < 0.0001
            && frequencyToMidiForMml(n.frequency) == frequencyToMidiForMml(newNote.frequency))
        {
            selectedPianoRollNoteIdx = i;
            break;
        }
    }

    selectedPianoRollNotes.clear();
    if (selectedPianoRollNoteIdx >= 0)
    {
        selectedPianoRollNotes.push_back({ partIdx, selectedPianoRollNoteIdx });
        const auto& selected = refreshedSeq[static_cast<size_t>(selectedPianoRollNoteIdx)];
        trackEditors[partIdx].setCaretPosition(juce::jmin(trackEditors[partIdx].getText().length(), selected.textStart >= 0 ? selected.textStart : 0));
    }

    pianoRoll.setSelectedNotes(selectedPianoRollNotes);
    pianoRoll.repaint();
}

void MainComponent::deleteSelectedPianoRollNote() {
    if (selectedPianoRollNotes.size() > 1)
    {
        deleteSelectedPianoRollNotes();
        return;
    }

    // Del/Backspace 삭제는 mabbiico 화면의 선택 파트에서만 허용한다.
    const bool canDeleteFromPianoRoll = canEditMabbiicoPart(selectedPianoRollPartIdx)
        && selectedPianoRollNoteIdx >= 0;
    if (!canDeleteFromPianoRoll)
        return;

    auto& track = banks[currentBankIndex].tracks[selectedPianoRollPartIdx];
    if (selectedPianoRollNoteIdx >= static_cast<int>(track.sequence.size()))
        return;

    ensureMabbiicoUndoState();

    auto editedSequence = track.sequence;
    editedSequence.erase(editedSequence.begin() + selectedPianoRollNoteIdx);

    const auto regeneratedMml = buildMmlFromPianoRollSequence(editedSequence, track.mml);
    trackEditors[selectedPianoRollPartIdx].setText(regeneratedMml, false);
    trackEditors[selectedPianoRollPartIdx].setCaretPosition(juce::jmin(trackEditors[selectedPianoRollPartIdx].getText().length(), 0));

    selectedPianoRollNoteIdx = -1;
    selectedPianoRollNotes.clear();
    pianoRoll.setSelectedNotes(selectedPianoRollNotes);

    markProjectDirty();
    saveCurrentBank();
    updateAllSequences();
    updateMmlCharCountLabels();
    refreshEditorTextColours();
    refreshPianoRollModel();
    pianoRoll.repaint();
}

void MainComponent::handlePianoRollNoteEdit(int partIdx, int noteIdx, int midiNote, double lengthBeats) {
    if (partIdx < 0 || partIdx >= 4 || noteIdx < 0) return;

    // 3mle 화면에서는 피아노롤 편집으로 MML을 바꾸지 않는다.
    // 노트 이동/길이 조절은 mabbiico 화면의 선택 파트에서만 허용한다.
    const bool canEditFromPianoRoll = canEditMabbiicoPart(partIdx);
    if (!canEditFromPianoRoll) return;

    auto& track = banks[currentBankIndex].tracks[partIdx];
    if (noteIdx >= static_cast<int>(track.sequence.size())) return;
    const auto note = track.sequence[static_cast<size_t>(noteIdx)];
    if (note.frequency <= 0.0) return;

    ensureMabbiicoUndoState();

    if (isSubScreenVisible && canEditMabbiicoPart(partIdx))
    {
        auto editedSequence = track.sequence;
        auto& editedNote = editedSequence[static_cast<size_t>(noteIdx)];
        editedNote.frequency = midiToFrequencyForMml(midiNote);
        editedNote.endBeat = editedNote.startBeat + juce::jmax(0.0625, lengthBeats);

        const auto regeneratedMml = buildMmlFromPianoRollSequence(editedSequence, track.mml);
        trackEditors[partIdx].setText(regeneratedMml, false);
        trackEditors[partIdx].setCaretPosition(juce::jmin(regeneratedMml.length(), note.textStart >= 0 ? note.textStart : 0));
    }
    else
    {
        if (note.textStart < 0 || note.textEnd <= note.textStart) return;

        auto text = trackEditors[partIdx].getText();
        if (note.textStart >= text.length()) return;
        const int safeEnd = juce::jmin(note.textEnd, text.length());
        text = text.substring(0, note.textStart) + makeMmlTokenForNote(midiNote, lengthBeats) + text.substring(safeEnd);

        trackEditors[partIdx].setText(text, false);
        trackEditors[partIdx].setCaretPosition(note.textStart);
    }

    markProjectDirty();
    saveCurrentBank();
    updateAllSequences();
    updateMmlCharCountLabels();
    refreshEditorTextColours();
    refreshPianoRollModel();
}

void MainComponent::clearPlaybackEditorHighlights() {
    if (!playbackEditorHighlightActive) return;
    for (int i = 0; i < 4; ++i) {
        playbackEditorHighlightStart[i] = -1; playbackEditorHighlightEnd[i] = -1;

        // Do not force the caret to the beginning when clearing playback highlight.
        // This was causing 3MLE note selection to jump back to position 0.
        const int caret = juce::jlimit(0, trackEditors[i].getText().length(), trackEditors[i].getCaretPosition());
        trackEditors[i].setHighlightedRegion(juce::Range<int>(caret, caret));
        trackEditors[i].repaint();
    }
    playbackEditorHighlightActive = false;
}

void MainComponent::updatePlaybackEditorHighlights() {
    if (!isPlaying || audioEngine.getSampleRate() <= 0.0) { clearPlaybackEditorHighlights(); return; }
    for (int i = 0; i < 4; ++i) {
        int hStart = -1, hEnd = -1;
        if (isPartActiveForCurrentPreset(i) && trackEditors[i].isVisible()) {
            for (const auto& note : banks[currentBankIndex].tracks[i].sequence) {
                if (note.frequency > 0.0 && globalSampleCount >= note.startSample && globalSampleCount < note.endSample && note.textStart >= 0 && note.textEnd > note.textStart) { hStart = note.textStart; hEnd = note.textEnd; break; }
            }
        }
        if (hStart != playbackEditorHighlightStart[i] || hEnd != playbackEditorHighlightEnd[i]) {
            playbackEditorHighlightStart[i] = hStart; playbackEditorHighlightEnd[i] = hEnd;
            if (hStart >= 0 && hEnd > hStart) {
                const auto hc = getEditorPlaybackHighlightColour(i);
                trackEditors[i].setColour(juce::TextEditor::highlightColourId, hc);
                trackEditors[i].setColour(juce::TextEditor::highlightedTextColourId, CustomUI::getReadableTextColour(hc));
                trackEditors[i].setHighlightedRegion(juce::Range<int>(hStart, hEnd));
                trackEditors[i].repaint();
                playbackEditorHighlightActive = true;
            }
            else { trackEditors[i].setHighlightedRegion(juce::Range<int>(0, 0)); }
        }
    }
}

bool MainComponent::isMabinogiPresetInstrument(int instrumentId) const { return instrumentId == 5 || instrumentId == 6; }
bool MainComponent::isMobilePresetInstrument(int instrumentId) const { return instrumentId == 6; }
bool MainComponent::isMobileSf2Engine(int sf2Index) const {
    if (sf2Index < 0 || sf2Index >= audioEngine.getNumEngines()) return false;
    const auto file = audioEngine.getSf2File(sf2Index);
    const juce::String fullPath = file.getFullPathName().toLowerCase();
    const juce::String name = audioEngine.getSf2Name(sf2Index).toLowerCase();

    return fullPath.contains("mabinogimobilepresetdls")
        || fullPath.contains("mabinogimobiledls")
        || fullPath.contains("mobile")
        || fullPath.contains("fury_sound_pack")
        || fullPath.contains("fury sound pack")
        || name.contains("fury_sound_pack")
        || name.contains("fury sound pack")
        || name.contains("mabinogi mobile");
}

bool MainComponent::isSongPresetSelected() const { return isSongPresetForBank(currentBankIndex); }
bool MainComponent::isSongPresetForBank(int bankIdx) const { if (bankIdx < 0 || bankIdx >= MAX_BANKS) return false; return banks[bankIdx].songPresetMode; }
bool MainComponent::computeSongPresetModeForBank(int bankIdx) const {
    if (bankIdx < 0 || bankIdx >= MAX_BANKS || banks[bankIdx].instrumentWave != 5) return false;
    juce::String fileName = audioEngine.getSf2Name(banks[bankIdx].sf2FileIndex);
    if (fileName.toLowerCase().startsWith("msxspirit02")) return true;
    if (const char* raw = audioEngine.getPresetName(banks[bankIdx].sf2FileIndex, banks[bankIdx].dlsPreset)) { juce::String r = juce::String(raw).toLowerCase(); if (r.contains("song") || r.contains("voice") || r.contains(juce::String(L"노래"))) return true; const juce::String enName = ProjectFileIO::getLocalizedSf2PresetName(r, fileName, banks[bankIdx].dlsPreset, 1).toLowerCase(); const juce::String koName = ProjectFileIO::getLocalizedSf2PresetName(r, fileName, banks[bankIdx].dlsPreset, 2).toLowerCase(); if (enName.contains("song") || enName.contains("voice") || koName.contains(juce::String(L"노래"))) return true; }
    if (bankIdx == currentBankIndex) { juce::String v = dlsPresetCombo.getText().toLowerCase(); if (v.contains("song") || v.contains("voice") || v.contains(juce::String(L"노래"))) return true; }
    return false;
}
bool MainComponent::isXylophonePresetSelected() const { return isXylophonePresetForBank(currentBankIndex); }
bool MainComponent::isXylophonePresetForBank(int bankIdx) const { if (bankIdx < 0 || bankIdx >= MAX_BANKS) return false; return banks[bankIdx].xylophonePresetMode; }
bool MainComponent::computeXylophonePresetModeForBank(int bankIdx) const {
    if (bankIdx < 0 || bankIdx >= MAX_BANKS || banks[bankIdx].instrumentWave != 5) return false;

    const juce::String fileName = audioEngine.getSf2Name(banks[bankIdx].sf2FileIndex);
    const juce::String fileLower = fileName.toLowerCase();
    const int presetIdx = banks[bankIdx].dlsPreset;

    // MSXspirit01 preset 38 is the Mabinogi xylophone preset in our table.
    if (fileLower.startsWith("msxspirit01") && presetIdx == 38)
        return true;

    auto containsXylophoneName = [](const juce::String& text) {
        const juce::String lower = text.toLowerCase();
        return lower.contains("xylophone") || lower.contains(juce::String(L"실로폰"));
    };

    if (const char* raw = audioEngine.getPresetName(banks[bankIdx].sf2FileIndex, presetIdx))
    {
        const juce::String rawName = juce::String::fromUTF8(raw);
        if (containsXylophoneName(rawName))
            return true;

        const juce::String enName = ProjectFileIO::getLocalizedSf2PresetName(rawName, fileName, presetIdx, 1);
        const juce::String koName = ProjectFileIO::getLocalizedSf2PresetName(rawName, fileName, presetIdx, 2);
        if (containsXylophoneName(enName) || containsXylophoneName(koName))
            return true;
    }

    if (bankIdx == currentBankIndex && containsXylophoneName(dlsPresetCombo.getText()))
        return true;

    return false;
}
void MainComponent::refreshSongPresetModeCache() { for (int i = 0; i < MAX_BANKS; ++i) { banks[i].songPresetMode = computeSongPresetModeForBank(i); banks[i].xylophonePresetMode = computeXylophonePresetModeForBank(i); } }
bool MainComponent::isPartActiveForCurrentPreset(int trackIdx) const { return isPartActiveForBank(currentBankIndex, trackIdx); }

bool MainComponent::isPartActiveForBank(int bankIdx, int trackIdx) const
{
    if (bankIdx < 0 || bankIdx >= MAX_BANKS || trackIdx < 0 || trackIdx >= 4)
        return false;

    const bool pcSongPartExcluded = banks[bankIdx].instrumentWave == 5 && banks[bankIdx].pcPresetExcludeSongPartLimit;
    if (trackIdx == 3 && pcSongPartExcluded)
        return false;

    if (isSongPresetForBank(bankIdx))
        return trackIdx == 3;

    if (isXylophonePresetForBank(bankIdx))
        return trackIdx == 0;

    if (trackIdx == 3)
        return banks[bankIdx].mmiSongPartWithProgram && banks[bankIdx].tracks[3].mml.trim().isNotEmpty();

    return trackIdx < 3;
}

void MainComponent::updatePartEditorVisibility() {
    if (isStartupLoading || isSubScreenVisible) return;

    const bool showPcExcludeSongLimitToggle = isMabinogiPresetInstrument(trackInstrumentCombo.getSelectedId()) && !isMobilePresetInstrument(trackInstrumentCombo.getSelectedId());
    const bool songPartExcluded = showPcExcludeSongLimitToggle && pcExcludeSongPartLimitToggle.getToggleState();
    const bool songMode = isSongPresetSelected() && !songPartExcluded;

    pcExcludeSongPartLimitToggle.setVisible(!isStartupLoading && showPcExcludeSongLimitToggle);
    compositionRankGuideLabel.setVisible(!isStartupLoading && showPcExcludeSongLimitToggle);

    const bool mmiMixedSong = !songMode && !songPartExcluded && banks[currentBankIndex].mmiSongPartWithProgram && banks[currentBankIndex].tracks[3].mml.trim().isNotEmpty();

    auto setRowVis = [](juce::Label& l, juce::Label& c, juce::TextEditor& e, juce::TextButton& m, juce::TextButton& s, bool v, bool showButtons) {
        l.setVisible(v); c.setVisible(v); e.setVisible(v);
        m.setVisible(v && showButtons);
        s.setVisible(v && showButtons);
    };

    for (int i = 0; i < 3; ++i)
        setRowVis(trackLabels[i], trackCountLabels[i], trackEditors[i], muteBtns[i], soloBtns[i], !songMode, i == 0);

    setRowVis(trackLabels[3], trackCountLabels[3], trackEditors[3], muteBtns[3], soloBtns[3], !songPartExcluded && (songMode || mmiMixedSong), songMode);

    trackLabels[3].setEnabled(!songPartExcluded);
    trackCountLabels[3].setEnabled(!songPartExcluded);
    trackEditors[3].setEnabled(!songPartExcluded);
    muteBtns[3].setEnabled(!songPartExcluded);
    soloBtns[3].setEnabled(!songPartExcluded);

    const bool xylophoneMode = isXylophonePresetSelected();
    for (int i = 1; i <= 2; ++i) {
        trackLabels[i].setEnabled(!xylophoneMode);
        trackEditors[i].setEnabled(!xylophoneMode);
        muteBtns[i].setEnabled(!xylophoneMode);
        soloBtns[i].setEnabled(!xylophoneMode);
    }
}

void MainComponent::refreshScreenSwitchButtonText() {
    screenSwitchButton.setButtonText(isSubScreenVisible ? "mabiicco" : "3mle");
}

void MainComponent::setSubScreenMode(bool shouldShowSubScreen) {
    if (isSubScreenVisible == shouldShowSubScreen) return;
    stopPianoRollPreviewNote();
    isSubScreenVisible = shouldShowSubScreen;
    if (isSubScreenVisible)
    {
        const bool songPartExcluded = trackInstrumentCombo.getSelectedId() == 5 && pcExcludeSongPartLimitToggle.getToggleState();
        selectMabbiicoPart((isSongPresetSelected() && !songPartExcluded) ? 3 : (isXylophonePresetSelected() ? 0 : activeMabbiicoPartIdx));
    }
    refreshScreenSwitchButtonText();
    updateMabbiicoEditorLineMode();
    setMainUiVisible(!isStartupLoading);
    refreshEditorTextColours();
    updateAllSequences();
    refreshPianoRollModel();
    resized();
    repaint();
}


void MainComponent::updateMabbiicoEditorLineMode() {
    const bool songPartExcluded = trackInstrumentCombo.getSelectedId() == 5 && pcExcludeSongPartLimitToggle.getToggleState();
    const bool songMode = isSubScreenVisible && isSongPresetSelected() && !songPartExcluded;
    for (int i = 0; i < 4; ++i) {
        const bool singleLineMabbiicoRow = isSubScreenVisible && ((i < 3 && !songMode) || (i == 3 && songMode));
        trackEditors[i].setMultiLine(!singleLineMabbiicoRow, !singleLineMabbiicoRow);
        trackEditors[i].setReturnKeyStartsNewLine(false);
        trackEditors[i].setScrollbarsShown(!singleLineMabbiicoRow);
    }
}

void MainComponent::setMainUiVisible(bool shouldBeVisible) {
    updateMabbiicoEditorLineMode();

    const bool showTopControls = shouldBeVisible;
    const bool showMainEditor = shouldBeVisible && !isSubScreenVisible;
    const bool showMabbiicoEditor = shouldBeVisible && isSubScreenVisible;
    const bool showAnyEditor = shouldBeVisible;

    blankSubScreen.setVisible(false);
    screenSwitchButton.setVisible(showTopControls);
    themeButton.setVisible(showTopControls);
    languageCombo.setVisible(showTopControls);

    pianoRoll.setVisible(showAnyEditor);
    playButton.setVisible(showAnyEditor);
    stopButton.setVisible(showAnyEditor);
    rewindButton.setVisible(showAnyEditor);
    exportButton.setVisible(showAnyEditor);
    importButton.setVisible(showAnyEditor);
    copyMabi3PartButton.setVisible(showAnyEditor);
    helperButton.setVisible(showAnyEditor);
    meterButton.setVisible(showAnyEditor);
    optimizeButton.setVisible(showAnyEditor);
    timeSignatureCombo.setVisible(false);
    helperCombo.setVisible(false);
    trackInstrumentLabel.setVisible(showAnyEditor);
    trackInstrumentCombo.setVisible(showAnyEditor);
    noteLengthLabel.setVisible(showMabbiicoEditor);
    noteLengthCombo.setVisible(showMabbiicoEditor);
    verticalScrollBar.setVisible(showAnyEditor);
    horizontalScrollBar.setVisible(showAnyEditor);

    for (int i = 0; i < MAX_BANKS; ++i)
        tabButtons[i].setVisible(showAnyEditor && i < numActiveTracks);
    addTrackButton.setVisible(showAnyEditor && numActiveTracks < MAX_BANKS);
    removeTrackButton.setVisible(showAnyEditor && numActiveTracks > 1);

    const bool songPartExcludedForMabbiico = trackInstrumentCombo.getSelectedId() == 5 && pcExcludeSongPartLimitToggle.getToggleState();
    const bool mabbiicoSongMode = showMabbiicoEditor && isSongPresetSelected() && !songPartExcludedForMabbiico;
    for (int i = 0; i < 4; ++i) {
        const bool showMabbiicoPartRow = showMabbiicoEditor && ((mabbiicoSongMode && i == 3) || (!mabbiicoSongMode && i < 3));
        trackLabels[i].setVisible(showMainEditor || showMabbiicoPartRow);
        trackEditors[i].setVisible(showMainEditor || showMabbiicoPartRow);
        trackCountLabels[i].setVisible(showMainEditor);
        muteBtns[i].setVisible(false);
        soloBtns[i].setVisible(false);
    }

    for (int i = 0; i < 3; ++i)
        subPartCheckBoxes[i].setVisible(showMabbiicoEditor && !mabbiicoSongMode);

    if (!showAnyEditor) {
        autoBassScaleLabel.setVisible(false);
        autoBassScaleCombo.setVisible(false);
        scaleSignatureLabel.setVisible(false);
        detectScaleButton.setVisible(false);
        dlsPresetLabel.setVisible(false);
        dlsPresetCombo.setVisible(false);
        loadSampleBtn.setVisible(false);
        noteLengthLabel.setVisible(false);
        noteLengthCombo.setVisible(false);
        for (int i = 0; i < 3; ++i) subPartCheckBoxes[i].setVisible(false);
        return;
    }

    // Scale/key controls live in the modeless Helper window now.
    autoBassScaleLabel.setVisible(false);
    autoBassScaleCombo.setVisible(false);
    scaleSignatureLabel.setVisible(false);
    detectScaleButton.setVisible(false);

    const bool showPresetUi = isMabinogiPresetInstrument(trackInstrumentCombo.getSelectedId());
    dlsPresetLabel.setVisible(showPresetUi);
    dlsPresetCombo.setVisible(showPresetUi);
    loadSampleBtn.setVisible(showPresetUi);
    noteLengthLabel.setVisible(showMabbiicoEditor);
    noteLengthCombo.setVisible(showMabbiicoEditor);

    if (showMainEditor)
        updatePartEditorVisibility();
    else if (showMabbiicoEditor) {
        const bool songPartExcluded = trackInstrumentCombo.getSelectedId() == 5 && pcExcludeSongPartLimitToggle.getToggleState();
        const bool songMode = isSongPresetSelected() && !songPartExcluded;
        for (int i = 0; i < 4; ++i) {
            const bool visible = songMode ? (i == 3) : (i < 3);
            trackLabels[i].setVisible(visible);
            trackEditors[i].setVisible(visible);
            trackCountLabels[i].setVisible(false);
            muteBtns[i].setVisible(false);
            soloBtns[i].setVisible(false);
        }
        for (int i = 0; i < 3; ++i)
            subPartCheckBoxes[i].setVisible(!songMode);
    }
}

void MainComponent::drawStartupLoadingScreen(juce::Graphics& g) {
    const auto whole = getLocalBounds(); const auto wholeF = whole.toFloat();
    const auto accent = CustomUI::getThemeColour(currentThemeId, "accent"); const auto accent2 = CustomUI::getThemeColour(currentThemeId, "accent2"); const auto panel = CustomUI::getThemeColour(currentThemeId, "panel"); const auto panel2 = CustomUI::getThemeColour(currentThemeId, "panel2"); const auto text = CustomUI::getThemeColour(currentThemeId, "text"); const auto mutedText = CustomUI::getThemeColour(currentThemeId, "mutedText");
    juce::ColourGradient bgGrad(juce::Colours::black.withAlpha(1.0f), wholeF.getCentreX(), wholeF.getY(), panel2.darker(0.72f).withAlpha(1.0f), wholeF.getCentreX(), wholeF.getBottom(), false); g.setGradientFill(bgGrad); g.fillRect(whole);
    const float cardW = juce::jlimit(360.0f, 560.0f, static_cast<float>(whole.getWidth()) * 0.48f); const float cardH = juce::jlimit(300.0f, 430.0f, static_cast<float>(whole.getHeight()) * 0.54f); auto card = juce::Rectangle<float>(0.0f, 0.0f, cardW, cardH).withCentre({ static_cast<float>(whole.getCentreX()), static_cast<float>(whole.getCentreY()) });
    g.setColour(juce::Colours::black.withAlpha(0.40f)); g.fillRoundedRectangle(card.translated(0.0f, 10.0f), 22.0f); g.setColour(panel.darker(0.42f).withAlpha(0.96f)); g.fillRoundedRectangle(card, 22.0f); g.setColour(accent.withAlpha(0.55f)); g.drawRoundedRectangle(card, 22.0f, 1.4f);
    auto inner = card.reduced(34.0f, 26.0f); auto logoArea = inner.removeFromTop(cardH * 0.58f).reduced(8.0f, 4.0f);
    if (startupLoadingLogo.isValid()) { g.setOpacity(1.0f); g.drawImageWithin(startupLoadingLogo, static_cast<int>(logoArea.getX()), static_cast<int>(logoArea.getY()), static_cast<int>(logoArea.getWidth()), static_cast<int>(logoArea.getHeight()), juce::RectanglePlacement(juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize), false); }
    else { g.setColour(accent.withAlpha(0.22f)); g.fillRoundedRectangle(logoArea, 16.0f); g.setColour(text.withAlpha(0.92f)); g.setFont(juce::Font(juce::FontOptions(30.0f, juce::Font::bold))); g.drawText("Atelier de Derstin", logoArea.toNearestInt(), juce::Justification::centred, false); }
    inner.removeFromTop(10.0f); g.setColour(accent2.withAlpha(0.98f)); g.setFont(juce::Font(juce::FontOptions(24.0f, juce::Font::bold))); g.drawText("Atelier de Derstin", inner.removeFromTop(34.0f).toNearestInt(), juce::Justification::centred, false); g.setColour(mutedText.withAlpha(0.95f)); g.setFont(16.0f); g.drawText(startupLoadingText, inner.removeFromTop(28.0f).toNearestInt(), juce::Justification::centred, false);
    inner.removeFromTop(14.0f); auto bar = inner.removeFromTop(6.0f).withSizeKeepingCentre(280.0f, 6.0f); g.setColour(juce::Colours::white.withAlpha(0.15f)); g.fillRoundedRectangle(bar, 3.0f); g.setColour(accent.withAlpha(0.88f)); g.fillRoundedRectangle(bar.withWidth(bar.getWidth() * 0.72f), 3.0f);
}

void MainComponent::loadWorkLoadingImage() {
    const auto exeFolder = juce::File::getSpecialLocation(juce::File::currentExecutableFile).getParentDirectory(); juce::Array<juce::File> candidates; candidates.add(exeFolder.getChildFile("Assets").getChildFile("Loading.png")); candidates.add(exeFolder.getChildFile("Loading.png")); candidates.add(juce::File::getCurrentWorkingDirectory().getChildFile("Assets").getChildFile("Loading.png")); candidates.add(juce::File::getCurrentWorkingDirectory().getChildFile("Loading.png")); candidates.add(juce::File(R"(C:\Users\User\Documents\MabinogiMMLEditor\MmlEditor\Assets\Loading.png)"));
    workLoadingImage = juce::Image(); for (const auto& candidate : candidates) { if (candidate.existsAsFile()) { auto img = juce::ImageFileFormat::loadFrom(candidate); if (img.isValid()) { workLoadingImage = img; break; } } }
}

void MainComponent::showWorkLoadingOverlay(const juce::String& message) {
    if (!workLoadingImage.isValid()) loadWorkLoadingImage(); isWorkLoading = true; workLoadingText = message.isNotEmpty() ? message : T("Working...", L"작업 중...");
    if (workLoadingOverlay == nullptr) { workLoadingOverlay = std::make_unique<CustomUI::WorkLoadingOverlayComponent>(); addAndMakeVisible(workLoadingOverlay.get()); }
    refreshWorkLoadingOverlayTheme(); workLoadingOverlay->setBounds(getLocalBounds()); workLoadingOverlay->setVisible(true); workLoadingOverlay->toFront(false); workLoadingOverlay->grabKeyboardFocus(); repaint();
}

void MainComponent::hideWorkLoadingOverlay() { isWorkLoading = false; if (workLoadingOverlay != nullptr) workLoadingOverlay->setVisible(false); grabKeyboardFocus(); repaint(); }

void MainComponent::refreshWorkLoadingOverlayTheme() {
    if (workLoadingOverlay != nullptr) {
        const auto panel = CustomUI::getThemeColour(currentThemeId, "panel");
        workLoadingOverlay->setVisuals(workLoadingImage, workLoadingText, CustomUI::getThemeColour(currentThemeId, "backgroundTop"), CustomUI::getThemeColour(currentThemeId, "backgroundBottom"), panel, CustomUI::getThemeColour(currentThemeId, "panel2"), CustomUI::getThemeColour(currentThemeId, "accent"), CustomUI::getThemeColour(currentThemeId, "accent2"), CustomUI::getReadableTextColour(panel), CustomUI::getThemeColour(currentThemeId, "mutedText"));
    }
}

juce::String MainComponent::getScaleSignatureText(int scaleId) const {
    struct SigInfo { int acc = 0; bool sharp = true; }; SigInfo info;
    switch (scaleId) { case 2: case 17: info = { 0, true }; break; case 3: case 18: info = { 1, true }; break; case 4: case 19: info = { 2, true }; break; case 5: case 20: info = { 3, true }; break; case 6: case 21: info = { 4, true }; break; case 7: case 22: info = { 5, true }; break; case 8: case 23: info = { 6, true }; break; case 9: case 24: info = { 7, true }; break; case 10: case 25: info = { 1, false }; break; case 11: case 26: info = { 2, false }; break; case 12: case 27: info = { 3, false }; break; case 13: case 28: info = { 4, false }; break; case 14: case 29: info = { 5, false }; break; case 15: case 30: info = { 6, false }; break; case 16: case 31: info = { 7, false }; break; }
                             if (info.acc == 0) return T("Key: none", L"조표: 없음"); const juce::String accStr = info.sharp ? "#" : "b"; return T(juce::String("Key: ") + accStr + " " + juce::String(info.acc), juce::String(L"조표: ") + accStr + " " + juce::String(info.acc) + juce::String(L"개"));
}

void MainComponent::detectScaleFromMelodyAndApply() {
    const auto melody = trackEditors[0].getText().trim(); if (melody.isEmpty()) { showThemedMessageBoxAsync(juce::AlertWindow::WarningIcon, T("Key Detection", L"조표 읽기"), T("Melody is empty.", L"멜로디가 비어 있어요.")); return; }
    const int detectedScaleId = MmlLogic::detectScaleIdFromMelody(melody); if (detectedScaleId <= 1) { showThemedMessageBoxAsync(juce::AlertWindow::WarningIcon, T("Key Detection", L"조표 읽기"), T("Could not find enough notes in the melody.", L"멜로디에서 충분한 음표를 찾지 못했어요.")); return; }
    autoBassScaleCombo.setSelectedId(detectedScaleId, juce::dontSendNotification); banks[currentBankIndex].autoBassScale = detectedScaleId; updateScaleSignatureLabel();
    int mode = helperCombo.getSelectedId();
    if (mode == 4 || mode == 3 || (mode >= 5 && mode <= 9)) { trackEditors[2].setText(MmlLogic::transformMML(trackEditors[0].getText(), mode, banks[currentBankIndex].autoBassScale, getTimeSignatureBeatsPerMeasure()), false); updateAllSequences(); refreshEditorTextColours(); }
    showThemedMessageBoxAsync(juce::AlertWindow::InfoIcon, T("Key Detection", L"조표 읽기"), T("Detected key: ", L"추정 스케일: ") + autoBassScaleCombo.getText() + "\n" + scaleSignatureLabel.getText() + "\n" + "어색한 부분은 직접 수정이 필요합니다!"); resized(); repaint();
}

void MainComponent::switchBank(int index)
{
    if (index < 0 || index >= numActiveTracks)
        return;

    if (currentBankIndex == index)
        return;

    stopPianoRollPreviewNote();
    clearPlaybackEditorHighlights();
    clearMabbiicoUndoHistory();
    saveCurrentBank();
    loadBank(index);
}

void MainComponent::addNewTrack()
{
    if (numActiveTracks >= MAX_BANKS)
        return;

    stopPianoRollPreviewNote();
    clearPlaybackEditorHighlights();
    clearMabbiicoUndoHistory();
    saveCurrentBank();

    const int newIndex = numActiveTracks;
    banks[newIndex] = InstrumentBank();
    // 새 트랙은 기본 악기를 마비노기 프리셋으로 시작한다.
    // InstrumentBank 기본값은 기존 프로젝트/첫 트랙 호환을 위해 유지하고,
    // + 버튼으로 추가되는 트랙에만 적용한다.
    banks[newIndex].instrumentWave = 5; // Mabinogi Preset
    banks[newIndex].sf2FileIndex = 0;
    banks[newIndex].dlsPreset = 0;
    banks[newIndex].songPresetMode = false;
    banks[newIndex].xylophonePresetMode = false;
    ++numActiveTracks;

    markProjectDirty();

    if (trackNamesUsePresetInstruments)
    {
        const auto presetName = getPresetInstrumentDisplayNameForBank(newIndex).trim();
        customTrackNames[newIndex] = presetName.isNotEmpty()
            ? presetName
            : (T("Track ", L"트랙 ") + juce::String(newIndex + 1));
    }
    else
    {
        customTrackNames[newIndex].clear();
    }

    for (int i = 0; i < MAX_BANKS; ++i)
    {
        tabButtons[i].setVisible(i < numActiveTracks);
        tabButtons[i].setToggleState(i == newIndex, juce::dontSendNotification);
    }
    refreshTrackTabTexts();

    loadBank(newIndex);
    resized();
}

void MainComponent::showTrackContextMenu(int trackIndex)
{
    if (trackIndex < 0 || trackIndex >= numActiveTracks)
        return;

    juce::PopupMenu menu;
    menu.addItem(1, T("Delete Track", L"트랙 삭제") + "  Shift+D", numActiveTracks > 1);

    menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&tabButtons[trackIndex]),
        [this, trackIndex](int result)
        {
            if (result == 1)
                deleteTrackAtIndex(trackIndex);
        });
}

void MainComponent::deleteTrackAtIndex(int trackIndex)
{
    if (numActiveTracks <= 1)
    {
        showThemedMessageBoxAsync(juce::AlertWindow::WarningIcon,
            T("Track Delete", L"트랙 삭제"),
            T("At least one track must remain.", L"트랙은 최소 1개가 필요합니다."));
        return;
    }

    if (trackIndex < 0 || trackIndex >= numActiveTracks)
        return;

    stopPianoRollPreviewNote();
    clearPlaybackEditorHighlights();
    clearMabbiicoUndoHistory();
    saveCurrentBank();

    int nextIndex = currentBankIndex;
    if (currentBankIndex > trackIndex)
        nextIndex = currentBankIndex - 1;
    else if (currentBankIndex == trackIndex)
        nextIndex = juce::jmin(trackIndex, numActiveTracks - 2);

    {
        const juce::ScopedLock sl(audioEngine.getLock());
        for (int i = trackIndex; i < numActiveTracks - 1; ++i)
        {
            banks[i] = banks[i + 1];
            customTrackNames[i] = customTrackNames[i + 1];
        }

        banks[numActiveTracks - 1] = InstrumentBank();
        customTrackNames[numActiveTracks - 1].clear();
    }

    --numActiveTracks;
    nextIndex = juce::jlimit(0, numActiveTracks - 1, nextIndex);

    markProjectDirty();

    for (int i = 0; i < MAX_BANKS; ++i)
    {
        tabButtons[i].setVisible(i < numActiveTracks);
        tabButtons[i].setToggleState(i == nextIndex, juce::dontSendNotification);
    }
    refreshTrackTabTexts();

    loadBank(nextIndex);
    resized();
}

void MainComponent::saveCurrentBank() {
    for (int i = 0; i < 4; ++i) {
        banks[currentBankIndex].tracks[i].mml = trackEditors[i].getText();
        banks[currentBankIndex].tracks[i].mute = muteBtns[i].getToggleState();
        banks[currentBankIndex].tracks[i].solo = soloBtns[i].getToggleState();
    }
    banks[currentBankIndex].instrumentWave = trackInstrumentCombo.getSelectedId(); const int selectedPresetId = dlsPresetCombo.getSelectedId(); if (selectedPresetId > 0) { const int encoded = selectedPresetId - 1; banks[currentBankIndex].sf2FileIndex = encoded / 10000; banks[currentBankIndex].dlsPreset = encoded % 10000; }
    banks[currentBankIndex].pcPresetExcludeSongPartLimit = pcExcludeSongPartLimitToggle.getToggleState();
    banks[currentBankIndex].helperMode = helperCombo.getSelectedId(); banks[currentBankIndex].autoBassScale = autoBassScaleCombo.getSelectedId();
}

void MainComponent::loadBank(int index) {
    const juce::ScopedValueSetter<bool> presetGuard(suppressPresetCallbacks, true);
    currentBankIndex = index;
    for (int i = 0; i < 4; ++i) {
        trackEditors[i].setText(banks[index].tracks[i].mml, false);
        muteBtns[i].setToggleState(banks[index].tracks[i].mute, juce::dontSendNotification);
        soloBtns[i].setToggleState(banks[index].tracks[i].solo, juce::dontSendNotification);
    }
    trackInstrumentCombo.setSelectedId(banks[index].instrumentWave, juce::dontSendNotification); const bool useMabi = isMabinogiPresetInstrument(banks[index].instrumentWave); const bool usePcMabi = banks[index].instrumentWave == 5; dlsPresetLabel.setVisible(useMabi); dlsPresetCombo.setVisible(useMabi); loadSampleBtn.setVisible(useMabi); pcExcludeSongPartLimitToggle.setVisible(usePcMabi); pcExcludeSongPartLimitToggle.setToggleState(usePcMabi && banks[index].pcPresetExcludeSongPartLimit, juce::dontSendNotification);
    banks[index].songPresetMode = computeSongPresetModeForBank(index); banks[index].xylophonePresetMode = computeXylophonePresetModeForBank(index); updatePresetCombo(); resized(); helperCombo.setSelectedId(banks[index].helperMode, juce::dontSendNotification); applyHelperModeState(banks[index].helperMode);
    if (banks[index].autoBassScale == 0) banks[index].autoBassScale = 1; autoBassScaleCombo.setSelectedId(banks[index].autoBassScale, juce::dontSendNotification); updateScaleSignatureLabel();
    const bool showScale = (banks[index].helperMode == 4 || banks[index].helperMode == 3 || (banks[index].helperMode >= 5 && banks[index].helperMode <= 9));
    autoBassScaleLabel.setVisible(false); autoBassScaleCombo.setVisible(false); scaleSignatureLabel.setVisible(false); detectScaleButton.setVisible(false);
    updatePartEditorVisibility(); resized(); updateAllSequences(); updateMmlCharCountLabels(); refreshPianoRollModel(); refreshEditorTextColours();
}

bool MainComponent::shouldUseProjectTrack1TempoMap() const {
    // Track 1 / Melody is the master tempo lane for both views.
    // Mabiicco pads/scaffolds the lower editor so tempo markers are visible even
    // before notes are placed.  3MLE keeps each lane length intact and inserts
    // only the master tempo markers that the target lane has actually reached.
    // Song preset mode is a single-part mode, so it keeps the existing behaviour.
    return !isSongPresetSelected();
}

bool MainComponent::shouldUseMabbiicoTrack1TempoMap() const {
    return isSubScreenVisible && shouldUseProjectTrack1TempoMap();
}

bool MainComponent::shouldUseThreeMleTrack1TempoMap() const {
    return !isSubScreenVisible && shouldUseProjectTrack1TempoMap();
}

int MainComponent::getMabbiicoTrack1TempoMasterBankIndex() const {
    return numActiveTracks > 0 ? 0 : currentBankIndex;
}

juce::String MainComponent::getMabbiicoTrack1TempoSourceMml() const {
    const int masterBankIdx = getMabbiicoTrack1TempoMasterBankIndex();
    if (masterBankIdx >= 0 && masterBankIdx < numActiveTracks)
        return banks[masterBankIdx].tracks[0].mml;
    if (currentBankIndex >= 0 && currentBankIndex < numActiveTracks)
        return banks[currentBankIndex].tracks[0].mml;
    return {};
}

void MainComponent::syncMabbiicoTempoEventsFromTrack1() {
    if (!shouldUseMabbiicoTrack1TempoMap())
        return;

    const int masterBankIdx = getMabbiicoTrack1TempoMasterBankIndex();
    if (masterBankIdx < 0 || masterBankIdx >= numActiveTracks)
        return;

    const juce::String masterMml = banks[masterBankIdx].tracks[0].mml.removeCharacters("\r\n");
    auto masterTempoEvents = extractTempoEventsFromMml(masterMml);

    if (masterTempoEvents.empty())
        masterTempoEvents.push_back({ 0.0, 120 });

    std::vector<juce::String> tempoSource;
    tempoSource.push_back(masterMml);
    const auto masterTempoMap = MmlLogic::buildTempoMap(tempoSource, audioEngine.getSampleRate());

    const auto masterSequence = MmlLogic::parseMMLWithTempoMap(masterMml, masterTempoMap, audioEngine.getSampleRate());

    auto getEndBeat = [](const std::vector<MmlNote>& sequence)
    {
        double endBeat = 0.0;
        for (const auto& ev : sequence)
            endBeat = juce::jmax(endBeat, ev.endBeat);
        return quantizeBeat64(endBeat);
    };

    auto hasAudibleNotes = [](const std::vector<MmlNote>& sequence)
    {
        for (const auto& ev : sequence)
            if (ev.frequency > 0.0)
                return true;
        return false;
    };

    double masterEndBeat = getEndBeat(masterSequence);
    for (const auto& ev : masterTempoEvents)
        masterEndBeat = juce::jmax(masterEndBeat, ev.beat);

    bool changed = false;

    // Project Track 1 / Melody is the master lane.  Every other project track's
    // Melody text gets the same tempo commands, padded with rests to Track 1's
    // current length, so the lower editor visibly shows the tempo map even before
    // notes are placed.
    for (int bankIdx = 0; bankIdx < numActiveTracks; ++bankIdx)
    {
        if (bankIdx == masterBankIdx)
            continue;

        auto& target = banks[bankIdx].tracks[0];
        const juce::String before = target.mml.removeCharacters("\r\n");

        auto targetSequence = MmlLogic::parseMMLWithTempoMap(before, masterTempoMap, audioEngine.getSampleRate());

        // A rest-only lane is usually an auto-generated tempo scaffold.  Rebuild
        // it from an empty sequence so old trailing rests disappear when Track 1
        // gets shorter, while real notes on the target lane are preserved.
        if (!hasAudibleNotes(targetSequence))
            targetSequence.clear();

        const auto rebuilt = buildMmlFromPianoRollSequenceWithTempoEvents(targetSequence, before, masterTempoEvents, masterEndBeat)
                               .removeCharacters("\r\n");

        if (rebuilt != before)
        {
            target.mml = rebuilt;
            changed = true;

            if (bankIdx == currentBankIndex)
            {
                const int oldCaret = trackEditors[0].getCaretPosition();
                trackEditors[0].setText(rebuilt, false);
                trackEditors[0].setCaretPosition(mapCaretAfterAutomaticTextSync(before, rebuilt, oldCaret));
            }
        }
    }

    if (changed)
        markProjectDirty();
}

void MainComponent::syncThreeMleTempoEventsFromTrack1() {
    if (!shouldUseThreeMleTrack1TempoMap())
        return;

    const int masterBankIdx = getMabbiicoTrack1TempoMasterBankIndex();
    if (masterBankIdx < 0 || masterBankIdx >= numActiveTracks)
        return;

    const juce::String masterMml = banks[masterBankIdx].tracks[0].mml.removeCharacters("\r\n");
    auto masterTempoEvents = extractTempoEventsFromMml(masterMml);

    const bool hasInitialTempo = std::any_of(masterTempoEvents.begin(), masterTempoEvents.end(), [](const PianoRollTempoEvent& ev)
    {
        return std::abs(ev.beat) <= 0.0001;
    });

    if (!hasInitialTempo)
        masterTempoEvents.push_back({ 0.0, 120 });

    std::vector<juce::String> tempoSource;
    tempoSource.push_back(masterMml);
    const auto masterTempoMap = MmlLogic::buildTempoMap(tempoSource, audioEngine.getSampleRate());

    bool changed = false;

    // 3MLE editor mode: copy Track-1 tempo markers into the other Melody text
    // lanes only when each lane's own length reaches the marker position.
    // Example:
    //   Track1 : t120cdefgabt140
    //   Target : cdefg       -> t120cdefg
    //   Target : cdefgab     -> t120cdefgabt140
    // No rests are inserted just to reach t140.
    for (int bankIdx = 0; bankIdx < numActiveTracks; ++bankIdx)
    {
        if (bankIdx == masterBankIdx)
            continue;

        auto& target = banks[bankIdx].tracks[0];
        const juce::String before = target.mml.removeCharacters("\r\n");
        const auto targetSequence = MmlLogic::parseMMLWithTempoMap(before, masterTempoMap, audioEngine.getSampleRate());

        const auto rebuilt = buildOptimizedMmlFromPianoRollSequenceWithTempoEventsClampedToSequenceEnd(targetSequence,
                                                                                                      before,
                                                                                                      masterTempoEvents,
                                                                                                      true)
                               .removeCharacters("\r\n");

        if (rebuilt != before)
        {
            target.mml = rebuilt;
            changed = true;

            if (bankIdx == currentBankIndex)
            {
                const int oldCaret = trackEditors[0].getCaretPosition();
                trackEditors[0].setText(rebuilt, false);
                trackEditors[0].setCaretPosition(mapCaretAfterAutomaticTextSync(before, rebuilt, oldCaret));
            }
        }
    }

    if (changed)
        markProjectDirty();
}

void MainComponent::buildTempoMap() {
    std::vector<juce::String> tracks;

    if (shouldUseMabbiicoTrack1TempoMap() || shouldUseThreeMleTrack1TempoMap())
    {
        // Project Track 1 / Melody owns the global tempo map in both Mabiicco
        // and 3MLE modes.  Mabiicco additionally writes rest scaffolds to empty
        // target lanes; 3MLE only inserts reached tempo markers in the editor.
        tracks.push_back(getMabbiicoTrack1TempoSourceMml());
    }
    else
    {
        for (int i = 0; i < 4; ++i)
            tracks.push_back(banks[currentBankIndex].tracks[i].mml);
    }

    tempoMap = MmlLogic::buildTempoMap(tracks, audioEngine.getSampleRate());
}

void MainComponent::updateAllSequences() {
    saveCurrentBank();

    for (int bankIdx = 0; bankIdx < numActiveTracks; ++bankIdx)
    {
        banks[bankIdx].songPresetMode = computeSongPresetModeForBank(bankIdx);
        banks[bankIdx].xylophonePresetMode = computeXylophonePresetModeForBank(bankIdx);
    }

    if (shouldUseMabbiicoTrack1TempoMap())
        syncMabbiicoTempoEventsFromTrack1();
    else if (shouldUseThreeMleTrack1TempoMap())
        syncThreeMleTempoEventsFromTrack1();

    const juce::ScopedLock sl(audioEngine.getLock()); buildTempoMap();

    // Reparse every active track tab with the same Track-1 master tempo map.
    // Mabiicco needs this for piano-roll drawing before notes are inserted;
    // 3MLE uses it so reached Track-1 tempo changes affect all target lanes.
    for (int bankIdx = 0; bankIdx < numActiveTracks; ++bankIdx) {
        for (int partIdx = 0; partIdx < 4; ++partIdx) {
            banks[bankIdx].tracks[partIdx].sequence = MmlLogic::parseMMLWithTempoMap(banks[bankIdx].tracks[partIdx].mml, tempoMap, audioEngine.getSampleRate());
            banks[bankIdx].tracks[partIdx].noteIndex = 0;
            while (banks[bankIdx].tracks[partIdx].noteIndex < banks[bankIdx].tracks[partIdx].sequence.size()
                && globalSampleCount >= banks[bankIdx].tracks[partIdx].sequence[banks[bankIdx].tracks[partIdx].noteIndex].endSample)
            {
                banks[bankIdx].tracks[partIdx].noteIndex++;
            }
        }
    }

    cachedEventList.clear(); int beatsPerMeasure = getTimeSignatureBeatsPerMeasure(); juce::String noteNames[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" }; std::vector<EventItem> tempEvents;
    for (int t = 0; t < 4; ++t) { if (!isPartActiveForCurrentPreset(t)) continue; for (const auto& note : banks[currentBankIndex].tracks[t].sequence) { tempEvents.push_back({ note.startBeat, note.endBeat, note.frequency, "", "", "", t }); } }
    for (const auto& tc : tempoMap) { tempEvents.push_back({ tc.beatPosition, tc.beatPosition + 0.001, -2.0, "Tempo", juce::String(static_cast<int>(std::round(tc.bpm))), "", -1 }); }
    std::sort(tempEvents.begin(), tempEvents.end(), [](const EventItem& a, const EventItem& b) { if (std::abs(a.startBeat - b.startBeat) > 0.001) return a.startBeat < b.startBeat; return a.trackIdx < b.trackIdx; });
    for (auto& ev : tempEvents) {
        if (ev.trackIdx < 0 && ev.frequency < -1.0) { cachedEventList.push_back(ev); continue; }
        int meas = static_cast<int>(ev.startBeat / beatsPerMeasure) + 1; int beatInMeas = static_cast<int>(std::fmod(ev.startBeat, beatsPerMeasure)) + 1; int tick = static_cast<int>(std::fmod(ev.startBeat, 1.0) * 480.0 + 0.5); int step = static_cast<int>((ev.endBeat - ev.startBeat) * 480.0 + 0.5);
        ev.timeStr = juce::String::formatted("%03d:%d:%03d", meas, beatInMeas, tick); ev.stepStr = juce::String::formatted("%4d", step);
        if (ev.frequency > 0.0) { int midiNoteInt = static_cast<int>(std::round(69.0 + 12.0 * std::log2(ev.frequency / 440.0))); int octave = (midiNoteInt / 12) - 1; ev.eventType = noteNames[((midiNoteInt % 12) + 12) % 12] + juce::String(octave); }
        else { ev.eventType = "Rest"; }
        cachedEventList.push_back(ev);
    }
    updateMmlCharCountLabels();
    refreshPianoRollModel();
    repaint();
}

void MainComponent::applyHelperModeState(int mode) {
    const auto normalBg = CustomUI::getThemeColour(currentThemeId, "editorBg"); const auto readOnlyBg = CustomUI::getThemeColour(currentThemeId, "editorReadOnly"); const auto outline = CustomUI::getThemeColour(currentThemeId, "accent").withAlpha(0.55f);
    auto styleEditorState = [&](juce::TextEditor& editor, bool readOnly, int trackIdx) {
        const auto bg = readOnly ? readOnlyBg : normalBg; const auto themeText = readOnly ? CustomUI::getThemeColour(currentThemeId, "mutedText") : CustomUI::getThemeColour(currentThemeId, "text");
        editor.setReadOnly(readOnly); editor.setColour(juce::TextEditor::backgroundColourId, bg); editor.setColour(juce::TextEditor::textColourId, themeText); editor.setColour(juce::TextEditor::outlineColourId, outline); editor.setColour(juce::TextEditor::focusedOutlineColourId, CustomUI::getThemeColour(currentThemeId, "accent2")); editor.setColour(juce::TextEditor::highlightColourId, getEditorPlaybackHighlightColour(trackIdx)); editor.setColour(juce::TextEditor::highlightedTextColourId, CustomUI::getReadableTextColour(getEditorPlaybackHighlightColour(trackIdx))); editor.setColour(juce::CaretComponent::caretColourId, CustomUI::getThemeColour(currentThemeId, "accent2")); editor.applyColourToAllText(themeText); editor.applyFontToAllText(editor.getFont()); editor.repaint();
    };
    bool isArpOrBass = (mode == 4 || mode == 3 || (mode >= 5 && mode <= 9));
    const bool xylophoneMode = isXylophonePresetSelected();
    if (xylophoneMode) { styleEditorState(trackEditors[1], true, 1); styleEditorState(trackEditors[2], true, 2); trackEditors[1].setEnabled(false); trackEditors[2].setEnabled(false); return; }
    trackEditors[1].setEnabled(true); trackEditors[2].setEnabled(true);
    if (mode == 1) { styleEditorState(trackEditors[1], false, 1); styleEditorState(trackEditors[2], false, 2); }
    else if (mode == 2) { styleEditorState(trackEditors[1], true, 1); styleEditorState(trackEditors[2], true, 2); }
    else if (isArpOrBass) { styleEditorState(trackEditors[1], false, 1); styleEditorState(trackEditors[2], true, 2); }
}


void MainComponent::seekToBeat(double beat) {
    if (audioEngine.getSampleRate() <= 0) return;
    const int64_t clickedSample = MmlLogic::getSampleFromBeat(juce::jmax(0.0, beat), tempoMap, audioEngine.getSampleRate());
    const juce::ScopedLock sl(audioEngine.getLock());
    globalSampleCount = clickedSample;
    audioEngine.stopAllNotes();
    for (int i = 0; i < numActiveTracks; ++i) {
        for (int j = 0; j < 4; ++j) {
            banks[i].tracks[j].noteIndex = 0;
            while (banks[i].tracks[j].noteIndex < banks[i].tracks[j].sequence.size() && globalSampleCount >= banks[i].tracks[j].sequence[banks[i].tracks[j].noteIndex].endSample) {
                banks[i].tracks[j].noteIndex++;
            }
            banks[i].tracks[j].currentAngle = 0.0;
        }
    }
    refreshPianoRollModel();
    repaint();
}

bool MainComponent::keyPressed(const juce::KeyPress& key) {
    if (isStartupLoading || isWorkLoading) return true;
    if (key == juce::KeyPress::spaceKey) { if (isPlaying) stopButton.triggerClick(); else playButton.triggerClick(); return true; }
    const auto mods = key.getModifiers(); const bool commandDown = mods.isCommandDown() || mods.isCtrlDown(); const bool shiftDown = mods.isShiftDown(); const bool altDown = mods.isAltDown(); const auto keyCode = key.getKeyCode();
    if (!commandDown && shiftDown && !altDown && (keyCode == 'd' || keyCode == 'D')) { deleteTrackAtIndex(currentBankIndex); return true; }
    if (!commandDown && !shiftDown && !altDown && (key == juce::KeyPress::deleteKey || key == juce::KeyPress::backspaceKey)) { deleteSelectedPianoRollNote(); return true; }
    if (commandDown) {
        if (!shiftDown && !altDown && (keyCode == 'z' || keyCode == 'Z')) { undoMabbiicoEdit(); return true; }
        if (!shiftDown && !altDown && (keyCode == 's' || keyCode == 'S')) { saveDmmfProject(); return true; }
        if (shiftDown && !altDown && (keyCode == 's' || keyCode == 'S')) { showSaveMenu(); return true; }
        if (altDown && !shiftDown && (keyCode == 's' || keyCode == 'S')) { saveDmmfProjectAs(); return true; }
        if (!shiftDown && (keyCode == 'o' || keyCode == 'O')) { showLoadMenu(); return true; }
        if (shiftDown && (keyCode == 'o' || keyCode == 'O')) { loadDmmfProject(); return true; }
        if (keyCode == 'm' || keyCode == 'M') { if (shiftDown) importMmiFile(); else saveMmiFile(); return true; }
        if (keyCode == 'i' || keyCode == 'I') { if (shiftDown) importMidiFile(); else exportToMidiFile(); return true; }
        if (keyCode == 'w' || keyCode == 'W') { exportToWavFile(); return true; }
        if (keyCode == '3') { copyMabinogi3PartsToClipboard(); return true; }
        if (keyCode == 'q' || keyCode == 'Q') { requestCloseWithSavePrompt(); return true; }
    }
    return false;
}

void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate) { audioEngine.prepareToPlay(samplesPerBlockExpected, sampleRate); refreshPianoRollModel(); }
void MainComponent::releaseResources() { audioEngine.releaseResources(); }
void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) {
    // 메인 재생이 멈춰 있어도 피아노롤 미리듣기 음은 오디오 콜백에서 렌더링해야 들린다.
    // 미리듣기 음이 없을 때는 기존처럼 완전 무음 처리한다.
    if (!isPlaying) {
        if (pianoRollPreviewMidi >= 0)
            audioEngine.renderAudioBlock(*bufferToFill.buffer, banks, numActiveTracks, globalSampleCount, isPlaying);
        else
            bufferToFill.clearActiveBufferRegion();
        return;
    }

    audioEngine.renderAudioBlock(*bufferToFill.buffer, banks, numActiveTracks, globalSampleCount, isPlaying);
}

void MainComponent::sanitizeNewLines(juce::TextEditor& ed) {
    juce::String txt = ed.getText(); if (txt.containsAnyOf("\r\n")) { int pos = ed.getCaretPosition(); ed.setText(txt.removeCharacters("\r\n"), false); ed.setCaretPosition(pos); refreshEditorTextColours(); }
}

void MainComponent::showLoadMenu() { juce::PopupMenu menu; menu.addItem(1, T("Import MMI  Ctrl+Shift+M", L"MMI 불러오기  Ctrl+Shift+M")); menu.addItem(2, T("Import DMMF  Ctrl+Shift+O", L"DMMF 불러오기  Ctrl+Shift+O")); menu.addItem(3, T("Import MIDI  Ctrl+Shift+I", L"MIDI 불러오기  Ctrl+Shift+I")); menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&importButton), [this](int result) { if (result == 1) importMmiFile(); else if (result == 2) loadDmmfProject(); else if (result == 3) importMidiFile(); }); }
void MainComponent::showSaveMenu() { juce::PopupMenu menu; menu.addItem(1, T("Save DMMF  Ctrl+S", L"DMMF 저장하기  Ctrl+S")); menu.addItem(2, T("Save DMMF As...  Ctrl+Alt+S", L"DMMF 다른 이름으로 저장  Ctrl+Alt+S")); menu.addSeparator(); menu.addItem(3, T("Save as MMI  Ctrl+M", L"MMI 형식으로 저장하기  Ctrl+M")); menu.addItem(4, T("Save as MIDI  Ctrl+I", L"MIDI 형식으로 저장하기  Ctrl+I")); menu.addSeparator(); menu.addItem(5, T("Export WAV  Ctrl+W", L"WAV 내보내기  Ctrl+W")); menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&exportButton), [this](int result) { if (result == 1) saveDmmfProject(); else if (result == 2) saveDmmfProjectAs(); else if (result == 3) saveMmiFile(); else if (result == 4) exportToMidiFile(); else if (result == 5) exportToWavFile(); }); }


void MainComponent::populateTimeSignatureComboItems()
{
    const int currentId = timeSignatureCombo.getSelectedId() > 0 ? timeSignatureCombo.getSelectedId() : 1;
    timeSignatureCombo.clear(juce::dontSendNotification);
    // Legacy ids are kept so older DMMF files do not break: 1 = 4/4, 2 = 3/4.
    timeSignatureCombo.addItem("4/4", 1);
    timeSignatureCombo.addItem("3/4", 2);
    timeSignatureCombo.addItem("4/2", 101);
    timeSignatureCombo.addItem("4/3", 102);
    timeSignatureCombo.addItem("6/8", 103);
    timeSignatureCombo.addItem("9/8", 104);
    timeSignatureCombo.addItem("12/8", 105);
    timeSignatureCombo.addItem("5/4", 106);
    timeSignatureCombo.addItem("7/4", 107);
    timeSignatureCombo.addItem("8/4", 108);
    timeSignatureCombo.addItem("9/8", 109);
    timeSignatureCombo.addItem("11/4", 110);
    timeSignatureCombo.setSelectedId(currentId, juce::dontSendNotification);
    if (timeSignatureCombo.getSelectedId() <= 0)
        timeSignatureCombo.setSelectedId(1, juce::dontSendNotification);
}

void MainComponent::setTimeSignatureId(int id, juce::NotificationType notificationType)
{
    if (id <= 0)
        id = 1;
    timeSignatureCombo.setSelectedId(id, notificationType);
    if (timeSignatureCombo.getSelectedId() <= 0)
        timeSignatureCombo.setSelectedId(1, notificationType);

    MeterChange base;
    base.beatPosition = 0.0;
    base.timeSignatureId = timeSignatureCombo.getSelectedId();
    base.beatsPerMeasure = getTimeSignatureBeatsPerMeasureForId(base.timeSignatureId);
    base.displayText = getTimeSignatureDisplayText(base.timeSignatureId);

    // Beat 0 is the global/base meter. Remove every old beat-0 marker first,
    // otherwise previous labels can remain stacked on the timeline.
    meterChanges.erase(std::remove_if(meterChanges.begin(), meterChanges.end(), [](const MeterChange& m)
    {
        return std::abs(m.beatPosition) <= 0.0001;
    }), meterChanges.end());
    meterChanges.insert(meterChanges.begin(), base);

    sortAndDeduplicateMeterChanges(meterChanges);

    meterButton.setButtonText(T("Meter", L"박자"));
    rebuildMeterChangesForPianoRoll();
}

int MainComponent::getTimeSignatureBeatsPerMeasureForId(int id) const
{
    switch (id)
    {
        case 101: return 2;   // 4/2
        case 102: return 3;   // 4/3
        case 2:   return 3;   // legacy 3/4
        case 103: return 6;   // 6/8
        case 104: return 9;   // 9/8
        case 105: return 12;  // 12/8
        case 106: return 5;   // 5/4
        case 107: return 7;   // 7/4
        case 108: return 8;   // 8/4
        case 109: return 9;   // mixed 9/8
        case 110: return 11;  // 11/4
        case 1:
        default:  return 4;
    }
}

int MainComponent::getTimeSignatureBeatsPerMeasure() const
{
    return getTimeSignatureBeatsPerMeasureForId(timeSignatureCombo.getSelectedId());
}

double MainComponent::getCurrentPlayheadBeat() const
{
    if (audioEngine.getSampleRate() <= 0.0)
        return 0.0;
    return juce::jmax(0.0, MmlLogic::getBeatFromSample(globalSampleCount, tempoMap, audioEngine.getSampleRate()));
}

double MainComponent::getMeasureStartForBeat(double targetBeat) const
{
    targetBeat = juce::jmax(0.0, targetBeat);

    std::vector<MeterChange> changes = meterChanges;
    if (changes.empty())
    {
        MeterChange base;
        base.beatPosition = 0.0;
        base.timeSignatureId = timeSignatureCombo.getSelectedId() > 0 ? timeSignatureCombo.getSelectedId() : 1;
        base.beatsPerMeasure = getTimeSignatureBeatsPerMeasureForId(base.timeSignatureId);
        base.displayText = getTimeSignatureDisplayText(base.timeSignatureId);
        changes.push_back(base);
    }

    std::sort(changes.begin(), changes.end(), [](const MeterChange& a, const MeterChange& b)
    {
        return a.beatPosition < b.beatPosition;
    });

    double measureStart = 0.0;
    int meterBeats = getTimeSignatureBeatsPerMeasureForId(changes.front().timeSignatureId);
    int changeIndex = 0;

    while (changeIndex + 1 < static_cast<int>(changes.size()) && changes[changeIndex + 1].beatPosition <= 0.0001)
    {
        ++changeIndex;
        meterBeats = juce::jmax(1, changes[changeIndex].beatsPerMeasure);
    }

    const int guardMax = 20000;
    for (int guard = 0; guard < guardMax; ++guard)
    {
        double nextStart = measureStart + static_cast<double>(juce::jmax(1, meterBeats));

        if (changeIndex + 1 < static_cast<int>(changes.size()))
        {
            const double nextChangeBeat = changes[changeIndex + 1].beatPosition;
            if (nextChangeBeat > measureStart + 0.0001 && nextChangeBeat < nextStart - 0.0001)
                nextStart = nextChangeBeat;
        }

        if (nextStart > targetBeat + 0.0001)
            break;

        measureStart = nextStart;

        while (changeIndex + 1 < static_cast<int>(changes.size())
               && changes[changeIndex + 1].beatPosition <= measureStart + 0.0001)
        {
            ++changeIndex;
            meterBeats = juce::jmax(1, changes[changeIndex].beatsPerMeasure);
        }
    }

    return juce::jmax(0.0, measureStart);
}

void MainComponent::applyTimeSignatureAtPlayheadMeasure(int id)
{
    if (id <= 0)
        id = 1;

    const double playheadBeat = getCurrentPlayheadBeat();
    const double measureStartBeat = getMeasureStartForBeat(playheadBeat);

    if (measureStartBeat <= 0.0001)
    {
        setTimeSignatureId(id, juce::sendNotification);
        markProjectDirty();
        return;
    }

    sortAndDeduplicateMeterChanges(meterChanges);

    int leftMeterId = timeSignatureCombo.getSelectedId() > 0 ? timeSignatureCombo.getSelectedId() : 1;
    int activeMeasureBeats = getTimeSignatureBeatsPerMeasureForId(leftMeterId);

    for (const auto& existing : meterChanges)
    {
        if (existing.beatPosition < measureStartBeat - 0.0001)
        {
            leftMeterId = existing.timeSignatureId;
            activeMeasureBeats = juce::jmax(1, existing.beatsPerMeasure);
        }
        else if (std::abs(existing.beatPosition - measureStartBeat) <= 0.0001)
        {
            // Use the currently displayed meter at this measure to decide the old
            // measure range to clean, but do not treat it as the "left" meter.
            activeMeasureBeats = juce::jmax(1, existing.beatsPerMeasure);
        }
        else if (existing.beatPosition > measureStartBeat + 0.0001)
        {
            break;
        }
    }

    const double oldMeasureEndBeat = measureStartBeat + static_cast<double>(juce::jmax(1, activeMeasureBeats));

    auto removeMarkersAtCurrentMeasure = [this, measureStartBeat, oldMeasureEndBeat]()
    {
        meterChanges.erase(std::remove_if(meterChanges.begin(), meterChanges.end(), [measureStartBeat, oldMeasureEndBeat](const MeterChange& m)
        {
            if (std::abs(m.beatPosition) <= 0.0001)
                return false;
            if (std::abs(m.beatPosition - measureStartBeat) <= 0.0001)
                return true;
            return m.beatPosition > measureStartBeat + 0.0001
                && m.beatPosition < oldMeasureEndBeat - 0.0001;
        }), meterChanges.end());
    };

    // If the requested meter is the same as the meter immediately to the left,
    // there is no real variable meter here. Remove the redundant marker instead
    // of drawing another label.
    if (id == leftMeterId)
    {
        removeMarkersAtCurrentMeasure();
        sortAndDeduplicateMeterChanges(meterChanges);

        timeSignatureCombo.setSelectedId(id, juce::dontSendNotification);
        meterButton.setButtonText(T("Meter", L"박자"));
        rebuildMeterChangesForPianoRoll();
        markProjectDirty();
        return;
    }

    MeterChange change;
    change.beatPosition = measureStartBeat;
    change.timeSignatureId = id;
    change.beatsPerMeasure = getTimeSignatureBeatsPerMeasureForId(id);
    change.displayText = getTimeSignatureDisplayText(id);

    // Replace the meter at the current left measure start instead of stacking labels.
    // Also remove any accidental meter markers that ended up inside the same measure.
    removeMarkersAtCurrentMeasure();

    meterChanges.push_back(change);
    sortAndDeduplicateMeterChanges(meterChanges);

    timeSignatureCombo.setSelectedId(id, juce::dontSendNotification);
    meterButton.setButtonText(T("Meter", L"박자"));
    rebuildMeterChangesForPianoRoll();
    markProjectDirty();
}

void MainComponent::rebuildMeterChangesForPianoRoll()
{
    if (meterChanges.empty())
    {
        MeterChange base;
        base.beatPosition = 0.0;
        base.timeSignatureId = timeSignatureCombo.getSelectedId() > 0 ? timeSignatureCombo.getSelectedId() : 1;
        base.beatsPerMeasure = getTimeSignatureBeatsPerMeasureForId(base.timeSignatureId);
        base.displayText = getTimeSignatureDisplayText(base.timeSignatureId);
        meterChanges.push_back(base);
    }

    pianoRoll.setMeterChanges(&meterChanges);
    pianoRoll.setTheme(currentThemeId, languageCombo.getSelectedId(), getTimeSignatureBeatsPerMeasure());
    pianoRoll.repaint();
    repaint();
}

juce::String MainComponent::getTimeSignatureDisplayText(int id) const
{
    if (id < 0)
        id = timeSignatureCombo.getSelectedId();
    switch (id)
    {
        case 101: return "4/2";
        case 102: return "4/3";
        case 2:   return "3/4";
        case 103: return "6/8";
        case 104: return "9/8";
        case 105: return "12/8";
        case 106: return "5/4";
        case 107: return "7/4";
        case 108: return "8/4";
        case 109: return "9/8";
        case 110: return "11/4";
        case 1:
        default:  return "4/4";
    }
}

void MainComponent::showTimeSignatureWindow()
{
    if (meterPopupWindow != nullptr)
    {
        meterPopupWindow->setAlwaysOnTop(true);
        meterPopupWindow->toFront(false);
        return;
    }

    auto* content = new TimeSignaturePopupContent(
        languageCombo.getSelectedId(),
        timeSignatureCombo.getSelectedId() > 0 ? timeSignatureCombo.getSelectedId() : 1,
        [this](int meterId)
        {
            applyTimeSignatureAtPlayheadMeasure(meterId);
        });
    content->applyTheme(currentThemeId);

    auto* window = new HelperModelessWindow(T("Time Signature", L"박자"), CustomUI::getThemeColour(currentThemeId, "panel"), [this]
    {
        meterPopupWindow.reset();
    });

    meterPopupWindow.reset(window);
    window->setContentOwned(content, true);
    window->setColour(juce::ResizableWindow::backgroundColourId, CustomUI::getThemeColour(currentThemeId, "panel"));
    window->centreAroundComponent(this, 360, 110);
    window->setAlwaysOnTop(true);
    window->addToDesktop();
    window->setVisible(true);
    window->toFront(false);
}

void MainComponent::showHelperMenu() {
    if (helperPopupWindow != nullptr)
    {
        helperPopupWindow->setAlwaysOnTop(true);
        helperPopupWindow->toFront(false);
        return;
    }

    auto* content = new HelperPopupContent(
        languageCombo.getSelectedId(),
        helperCombo.getSelectedId() > 0 ? helperCombo.getSelectedId() : 1,
        autoBassScaleCombo.getSelectedId() > 0 ? autoBassScaleCombo.getSelectedId() : 1,
        [this](int mode)
        {
            helperCombo.setSelectedId(mode, juce::sendNotification);
        },
        [this](int scaleId)
        {
            autoBassScaleCombo.setSelectedId(scaleId, juce::sendNotification);
        },
        [this]() -> int
        {
            detectScaleFromMelodyAndApply();
            return autoBassScaleCombo.getSelectedId() > 0 ? autoBassScaleCombo.getSelectedId() : 1;
        },
        [this](int scaleId) -> juce::String
        {
            return getScaleSignatureText(scaleId);
        });
    content->applyTheme(currentThemeId);

    auto* window = new HelperModelessWindow(T("Helper", L"도우미"), CustomUI::getThemeColour(currentThemeId, "panel"), [this]
    {
        helperPopupWindow.reset();
    });

    helperPopupWindow.reset(window);
    window->setContentOwned(content, true);
    window->setColour(juce::ResizableWindow::backgroundColourId, CustomUI::getThemeColour(currentThemeId, "panel"));
    window->centreAroundComponent(this, 430, 220);
    window->setAlwaysOnTop(true);
    window->addToDesktop();
    window->setVisible(true);
    window->toFront(false);
}

void MainComponent::paint(juce::Graphics& g) {
    if (isStartupLoading) { drawStartupLoadingScreen(g); return; }
    juce::ColourGradient bgGradient(CustomUI::getThemeColour(currentThemeId, "backgroundTop"), 0.0f, 0.0f, CustomUI::getThemeColour(currentThemeId, "backgroundBottom"), 0.0f, static_cast<float>(getHeight()), false);
    g.setGradientFill(bgGradient);
    g.fillRect(getLocalBounds().toFloat());
}

void MainComponent::layoutSubScreen(juce::Rectangle<int> area, juce::Rectangle<int> tabArea) {
    const int addButtonWidth = numActiveTracks < MAX_BANKS ? 26 : 0;
    const int removeButtonWidth = numActiveTracks > 1 ? 26 : 0;
    int availableWidth = tabArea.getWidth() - addButtonWidth - removeButtonWidth;
    int tabWidth = std::min(80, availableWidth / std::max(1, numActiveTracks));
    for (int i = 0; i < MAX_BANKS; ++i) {
        if (i < numActiveTracks)
            tabButtons[i].setBounds(tabArea.removeFromLeft(tabWidth).withTrimmedRight(2));
        else
            tabButtons[i].setBounds(0, 0, 0, 0);
    }

    if (numActiveTracks < MAX_BANKS) {
        addTrackButton.setBounds(tabArea.removeFromLeft(26).withTrimmedRight(2));
        addTrackButton.setVisible(true);
    }
    else {
        addTrackButton.setBounds(0, 0, 0, 0);
        addTrackButton.setVisible(false);
    }

    if (numActiveTracks > 1) {
        removeTrackButton.setBounds(tabArea.removeFromLeft(26).withTrimmedRight(2));
        removeTrackButton.setVisible(true);
    }
    else {
        removeTrackButton.setBounds(0, 0, 0, 0);
        removeTrackButton.setVisible(false);
    }

    const bool songPartExcluded = trackInstrumentCombo.getSelectedId() == 5 && pcExcludeSongPartLimitToggle.getToggleState();
    const bool songMode = isSongPresetSelected() && !songPartExcluded;
    const bool xylophoneMode = isXylophonePresetSelected();
    if (songMode && activeMabbiicoPartIdx != 3)
        selectMabbiicoPart(3);
    else if (!songMode && xylophoneMode && activeMabbiicoPartIdx != 0)
        selectMabbiicoPart(0);
    else if (!songMode && (activeMabbiicoPartIdx < 0 || activeMabbiicoPartIdx >= 3))
        selectMabbiicoPart(0);

    const int subInstrumentRowHeight = 30;
    const int subInputLineHeight = 24;
    const int subInputRowGap = 5;
    const int visibleMmlRows = songMode ? 1 : 3;
    const int bottomHeight = subInstrumentRowHeight + (subInputLineHeight + subInputRowGap) * visibleMmlRows + 12;
    auto bottomArea = area.removeFromBottom(bottomHeight);
    auto bottomGap = bottomArea.removeFromTop(8);
    bottomPanelSeparator.setBounds(bottomGap.withHeight(1));
    bottomPanelSeparator.setVisible(true);
    bottomPanelSeparator.setColour(juce::Label::backgroundColourId,
        dlsPresetCombo.findColour(juce::ComboBox::outlineColourId));

    auto bottomPanel = bottomArea.withTrimmedLeft(4).withTrimmedRight(4).withTrimmedBottom(4);

    auto instrumentRow = bottomPanel.removeFromTop(subInstrumentRowHeight).withTrimmedBottom(4);
    trackInstrumentLabel.setBounds(instrumentRow.removeFromLeft(70));
    trackInstrumentCombo.setBounds(instrumentRow.removeFromLeft(180).withTrimmedRight(10));

    const bool showPresetUi = isMabinogiPresetInstrument(trackInstrumentCombo.getSelectedId());
    dlsPresetLabel.setVisible(showPresetUi);
    dlsPresetCombo.setVisible(showPresetUi);
    const bool showPcExcludeSongLimitToggle = showPresetUi && !isMobilePresetInstrument(trackInstrumentCombo.getSelectedId());
    pcExcludeSongPartLimitToggle.setVisible(showPcExcludeSongLimitToggle);
    // DLS Settings stays on the top toolbar next to "Copy Mabi 3" in both 3MLE and Mabiicco modes.
    // Do not place it in the Mabiicco bottom instrument row.
    loadSampleBtn.setVisible(showPresetUi);
    if (showPresetUi) {
        instrumentRow.removeFromLeft(20); // wider visual gap between instrument and preset controls
        dlsPresetLabel.setBounds(instrumentRow.removeFromLeft(58).withTrimmedRight(4));
        dlsPresetCombo.setBounds(instrumentRow.removeFromLeft(showPcExcludeSongLimitToggle ? 250 : 350).withTrimmedLeft(2).withTrimmedRight(10));
        if (showPcExcludeSongLimitToggle)
            pcExcludeSongPartLimitToggle.setBounds(instrumentRow.removeFromLeft(132).withTrimmedRight(8));
        else
            pcExcludeSongPartLimitToggle.setBounds(0, 0, 0, 0);
    }
    else {
        dlsPresetLabel.setBounds(0, 0, 0, 0);
        dlsPresetCombo.setBounds(0, 0, 0, 0);
        pcExcludeSongPartLimitToggle.setBounds(0, 0, 0, 0);
    }

    instrumentRow.removeFromLeft(10);
    noteLengthLabel.setBounds(instrumentRow.removeFromLeft(54));
    noteLengthCombo.setBounds(instrumentRow.removeFromLeft(120));
    compositionRankGuideLabel.setVisible(showPresetUi && !isMobilePresetInstrument(trackInstrumentCombo.getSelectedId()));
    compositionRankGuideLabel.setBounds(instrumentRow.withTrimmedLeft(6).withTrimmedRight(4));

    auto rowArea = bottomPanel;
    if (songMode) {
        for (int i = 0; i < 3; ++i) {
            subPartCheckBoxes[i].setBounds(0, 0, 0, 0);
            trackLabels[i].setBounds(0, 0, 0, 0);
            trackCountLabels[i].setBounds(0, 0, 0, 0);
            trackEditors[i].setBounds(0, 0, 0, 0);
            muteBtns[i].setBounds(0, 0, 0, 0);
            soloBtns[i].setBounds(0, 0, 0, 0);
        }

        auto row = rowArea.removeFromTop(subInputLineHeight + subInputRowGap).withTrimmedBottom(subInputRowGap);
        trackLabels[3].setBounds(row.removeFromLeft(54).withTrimmedTop(1));
        trackCountLabels[3].setBounds(0, 0, 0, 0);
        muteBtns[3].setBounds(0, 0, 0, 0);
        soloBtns[3].setBounds(0, 0, 0, 0);
        trackEditors[3].setBounds(row.withTrimmedTop(1).withTrimmedBottom(1));
    }
    else {
        for (int i = 0; i < 3; ++i) {
            auto row = rowArea.removeFromTop(subInputLineHeight + subInputRowGap).withTrimmedBottom(subInputRowGap);

            subPartCheckBoxes[i].setBounds(row.removeFromLeft(28).withTrimmedTop(4).withTrimmedBottom(4));
            const bool xylophoneDisabledPart = xylophoneMode && i > 0;
            subPartCheckBoxes[i].setEnabled(!xylophoneDisabledPart);
            trackLabels[i].setEnabled(!xylophoneDisabledPart);
            trackEditors[i].setEnabled(!xylophoneDisabledPart);
            trackLabels[i].setBounds(row.removeFromLeft(48).withTrimmedTop(1));
            trackCountLabels[i].setBounds(0, 0, 0, 0);
            muteBtns[i].setBounds(0, 0, 0, 0);
            soloBtns[i].setBounds(0, 0, 0, 0);
            trackEditors[i].setBounds(row.withTrimmedTop(1).withTrimmedBottom(1));
        }

        trackLabels[3].setBounds(0, 0, 0, 0);
        trackCountLabels[3].setBounds(0, 0, 0, 0);
        trackEditors[3].setBounds(0, 0, 0, 0);
        muteBtns[3].setBounds(0, 0, 0, 0);
        soloBtns[3].setBounds(0, 0, 0, 0);
    }

    trackLabels[3].setEnabled(!songPartExcluded);
    trackCountLabels[3].setEnabled(!songPartExcluded);
    trackEditors[3].setEnabled(!songPartExcluded);
    muteBtns[3].setEnabled(!songPartExcluded);
    soloBtns[3].setEnabled(!songPartExcluded);

    pianoRoll.setBounds(area);
    refreshPianoRollModel();

    auto rollViewArea = area;
    auto vertScrollArea = rollViewArea.removeFromRight(15);
    auto horizScrollArea = rollViewArea.removeFromBottom(15);
    horizScrollArea.removeFromLeft(30);
    verticalScrollBar.setBounds(vertScrollArea);
    horizontalScrollBar.setBounds(horizScrollArea);

    int minMidi = 12;
    int maxMidi = 108;
    int numNotes = maxMidi - minMidi + 1;
    double totalVirtualHeight = numNotes * fixedRowHeight;
    verticalScrollBar.setRangeLimits(0.0, totalVirtualHeight);
    verticalScrollBar.setCurrentRange(scrollY, rollViewArea.getHeight());
}

void MainComponent::resized() {
    if (isStartupLoading) { setMainUiVisible(false); return; }

    auto area = getLocalBounds();
    const bool showPresetUi = isMabinogiPresetInstrument(trackInstrumentCombo.getSelectedId());

    // Top toolbar: match the user-approved layout screenshot.
    // Keep the language combo and the 3MLE/Mabiicco switch button exactly aligned.
    constexpr int rightToolbarWidth = 250;
    constexpr int rightThemeButtonWidth = 68;
    constexpr int rightLicenceButtonWidth = 76;
    constexpr int rightLanguageWidth = 98;

    auto topArea = area.removeFromTop(32).reduced(4, 4);
    auto rightToolbar = topArea.removeFromRight(rightToolbarWidth);
    licenceButton.setBounds(rightToolbar.removeFromLeft(rightLicenceButtonWidth).reduced(2, 0));
    themeButton.setBounds(rightToolbar.removeFromLeft(rightThemeButtonWidth).reduced(2, 0));
    languageCombo.setBounds(rightToolbar.removeFromLeft(rightLanguageWidth).reduced(2, 0));

    auto placeToolbarButton = [](juce::Rectangle<int>& row, juce::Component& c, int w)
    {
        c.setBounds(row.removeFromLeft(w).reduced(2, 0));
    };

    placeToolbarButton(topArea, playButton, 50);
    placeToolbarButton(topArea, stopButton, 50);

    trackNameFromPresetButton.setVisible(showPresetUi);
    if (showPresetUi)
        placeToolbarButton(topArea, trackNameFromPresetButton, 112);
    else
        trackNameFromPresetButton.setBounds(0, 0, 0, 0);

    placeToolbarButton(topArea, rewindButton, 72);
    placeToolbarButton(topArea, importButton, 88);
    placeToolbarButton(topArea, exportButton, 88);
    placeToolbarButton(topArea, helperButton, 70);
    placeToolbarButton(topArea, meterButton, 60);
    placeToolbarButton(topArea, optimizeButton, 82);
    placeToolbarButton(topArea, copyMabi3PartButton, 88);

    loadSampleBtn.setVisible(showPresetUi);
    if (showPresetUi)
        placeToolbarButton(topArea, loadSampleBtn, 92);
    else
        loadSampleBtn.setBounds(0, 0, 0, 0);

    timeSignatureCombo.setBounds(0, 0, 0, 0);
    helperCombo.setBounds(0, 0, 0, 0);
    if (autoBassScaleCombo.isVisible())
    {
        autoBassScaleLabel.setBounds(topArea.removeFromLeft(50));
        autoBassScaleCombo.setBounds(topArea.removeFromLeft(150).reduced(2, 0));
        scaleSignatureLabel.setBounds(topArea.removeFromLeft(100).reduced(2, 0));
        detectScaleButton.setBounds(topArea.removeFromLeft(88).reduced(2, 0));
    }

    auto tabArea = area.removeFromTop(24).withTrimmedLeft(4).withTrimmedRight(4);

    // Align this button directly under the language combo above.
    auto screenSwitchArea = tabArea.removeFromRight(rightToolbarWidth);
    screenSwitchArea.removeFromLeft(rightLicenceButtonWidth + rightThemeButtonWidth);
    screenSwitchButton.setBounds(screenSwitchArea.removeFromLeft(rightLanguageWidth).reduced(2, 0));
    if (isSubScreenVisible) {
        blankSubScreen.setBounds(0, 0, 0, 0);
        setMainUiVisible(true);
        layoutSubScreen(area, tabArea);
        if (workLoadingOverlay != nullptr) { workLoadingOverlay->setBounds(getLocalBounds()); if (workLoadingOverlay->isVisible()) workLoadingOverlay->toFront(false); }
        return;
    }
    blankSubScreen.setBounds(0, 0, 0, 0);

    const int addButtonWidth = numActiveTracks < MAX_BANKS ? 30 : 0;
    const int removeButtonWidth = numActiveTracks > 1 ? 30 : 0;
    int availableWidth = tabArea.getWidth() - addButtonWidth - removeButtonWidth;
    int tabWidth = std::min(80, availableWidth / std::max(1, numActiveTracks));
    for (int i = 0; i < MAX_BANKS; ++i) { if (i < numActiveTracks) { tabButtons[i].setBounds(tabArea.removeFromLeft(tabWidth).withTrimmedRight(2)); } else { tabButtons[i].setBounds(0, 0, 0, 0); } }
    if (numActiveTracks < MAX_BANKS) { addTrackButton.setBounds(tabArea.removeFromLeft(26).withTrimmedRight(2)); addTrackButton.setVisible(true); }
    else { addTrackButton.setBounds(0, 0, 0, 0); addTrackButton.setVisible(false); }
    if (numActiveTracks > 1) { removeTrackButton.setBounds(tabArea.removeFromLeft(26).withTrimmedRight(2)); removeTrackButton.setVisible(true); }
    else { removeTrackButton.setBounds(0, 0, 0, 0); removeTrackButton.setVisible(false); }

    int bottomHeight = static_cast<int>(getLocalBounds().getHeight() * 0.35f);
    auto bottomArea = area.removeFromBottom(bottomHeight);
    auto bottomGap = bottomArea.removeFromTop(8);
    bottomPanelSeparator.setBounds(bottomGap.withHeight(1));
    bottomPanelSeparator.setVisible(true);
    bottomPanelSeparator.setColour(juce::Label::backgroundColourId,
        dlsPresetCombo.findColour(juce::ComboBox::outlineColourId));

    auto bottomPanel = bottomArea.withTrimmedLeft(4).withTrimmedRight(4).withTrimmedBottom(4);
    auto instrumentRow = bottomPanel.removeFromTop(30).withTrimmedBottom(4);
    trackInstrumentLabel.setBounds(instrumentRow.removeFromLeft(60));
    trackInstrumentCombo.setBounds(instrumentRow.removeFromLeft(190).withTrimmedRight(10));
    dlsPresetLabel.setVisible(showPresetUi);
    dlsPresetCombo.setVisible(showPresetUi);
    const bool showPcExcludeSongLimitToggle = showPresetUi && !isMobilePresetInstrument(trackInstrumentCombo.getSelectedId());
    pcExcludeSongPartLimitToggle.setVisible(showPcExcludeSongLimitToggle);
    if (showPresetUi)
    {
        instrumentRow.removeFromLeft(20); // wider visual gap between instrument and preset controls
        dlsPresetLabel.setBounds(instrumentRow.removeFromLeft(54).withTrimmedRight(4));
        dlsPresetCombo.setBounds(instrumentRow.removeFromLeft(showPcExcludeSongLimitToggle ? 260 : 360).withTrimmedLeft(2).withTrimmedRight(10));
        if (showPcExcludeSongLimitToggle)
            pcExcludeSongPartLimitToggle.setBounds(instrumentRow.removeFromLeft(132).withTrimmedRight(8));
        else
            pcExcludeSongPartLimitToggle.setBounds(0, 0, 0, 0);
    }
    else
    {
        dlsPresetLabel.setBounds(0, 0, 0, 0);
        dlsPresetCombo.setBounds(0, 0, 0, 0);
        pcExcludeSongPartLimitToggle.setBounds(0, 0, 0, 0);
    }
    noteLengthLabel.setBounds(0, 0, 0, 0);
    noteLengthCombo.setBounds(0, 0, 0, 0);
    compositionRankGuideLabel.setVisible(showPresetUi && !isMobilePresetInstrument(trackInstrumentCombo.getSelectedId()));
    compositionRankGuideLabel.setBounds(instrumentRow.withTrimmedLeft(8).withTrimmedRight(6));
    const bool songPartExcluded = showPcExcludeSongLimitToggle && pcExcludeSongPartLimitToggle.getToggleState();
    updatePartEditorVisibility(); const bool songMode = isSongPresetSelected() && !songPartExcluded; const bool mmiMixedSong = !songMode && !songPartExcluded && banks[currentBankIndex].mmiSongPartWithProgram && banks[currentBankIndex].tracks[3].mml.trim().isNotEmpty();
    int visibleEditorCount = songMode ? 1 : (mmiMixedSong ? 4 : 3); int editorHeight = bottomPanel.getHeight() / std::max(1, visibleEditorCount);

    auto layoutEditor = [](juce::Rectangle<int>& box, juce::Label& lbl, juce::Label& countLbl, juce::TextEditor& editor, juce::TextButton& m, juce::TextButton& s, bool showButtons) {
        auto header = box.removeFromTop(22);
        lbl.setBounds(header.removeFromLeft(70));
        if (showButtons) {
            m.setBounds(header.removeFromLeft(24).withTrimmedRight(2).withTrimmedTop(2).withTrimmedBottom(2));
            s.setBounds(header.removeFromLeft(24).withTrimmedRight(2).withTrimmedTop(2).withTrimmedBottom(2));
        }
        else {
            m.setBounds(0, 0, 0, 0);
            s.setBounds(0, 0, 0, 0);
        }
        countLbl.setBounds(header);
        editor.setBounds(box.withTrimmedBottom(4));
    };

    if (songMode) {
        for (int i = 0; i < 3; ++i) { trackLabels[i].setBounds(0, 0, 0, 0); trackCountLabels[i].setBounds(0, 0, 0, 0); trackEditors[i].setBounds(0, 0, 0, 0); muteBtns[i].setBounds(0, 0, 0, 0); soloBtns[i].setBounds(0, 0, 0, 0); }
        auto r4 = bottomPanel; layoutEditor(r4, trackLabels[3], trackCountLabels[3], trackEditors[3], muteBtns[3], soloBtns[3], true);
    }
    else {
        auto r1 = bottomPanel.removeFromTop(editorHeight); layoutEditor(r1, trackLabels[0], trackCountLabels[0], trackEditors[0], muteBtns[0], soloBtns[0], true);
        auto r2 = bottomPanel.removeFromTop(editorHeight); layoutEditor(r2, trackLabels[1], trackCountLabels[1], trackEditors[1], muteBtns[1], soloBtns[1], false);
        auto r3 = mmiMixedSong ? bottomPanel.removeFromTop(editorHeight) : bottomPanel; layoutEditor(r3, trackLabels[2], trackCountLabels[2], trackEditors[2], muteBtns[2], soloBtns[2], false);
        if (mmiMixedSong) { auto r4 = bottomPanel; layoutEditor(r4, trackLabels[3], trackCountLabels[3], trackEditors[3], muteBtns[3], soloBtns[3], false); }
        else { trackLabels[3].setBounds(0, 0, 0, 0); trackCountLabels[3].setBounds(0, 0, 0, 0); trackEditors[3].setBounds(0, 0, 0, 0); muteBtns[3].setBounds(0, 0, 0, 0); soloBtns[3].setBounds(0, 0, 0, 0); }
    }

    trackLabels[3].setEnabled(!songPartExcluded);
    trackCountLabels[3].setEnabled(!songPartExcluded);
    trackEditors[3].setEnabled(!songPartExcluded);
    muteBtns[3].setEnabled(!songPartExcluded);
    soloBtns[3].setEnabled(!songPartExcluded);

    pianoRoll.setBounds(area);
    refreshPianoRollModel();
    auto rollViewArea = area; auto vertScrollArea = rollViewArea.removeFromRight(15); auto horizScrollArea = rollViewArea.removeFromBottom(15); horizScrollArea.removeFromLeft(180); verticalScrollBar.setBounds(vertScrollArea); horizontalScrollBar.setBounds(horizScrollArea);
    int minMidi = 12; int maxMidi = 108; int numNotes = maxMidi - minMidi + 1; double totalVirtualHeight = numNotes * fixedRowHeight; verticalScrollBar.setRangeLimits(0.0, totalVirtualHeight); verticalScrollBar.setCurrentRange(scrollY, rollViewArea.getHeight());
    if (workLoadingOverlay != nullptr) { workLoadingOverlay->setBounds(getLocalBounds()); if (workLoadingOverlay->isVisible()) workLoadingOverlay->toFront(false); }
}

void MainComponent::scrollBarMoved(juce::ScrollBar* scrollBar, double newRangeStart) { if (scrollBar == &verticalScrollBar) scrollY = newRangeStart; else if (scrollBar == &horizontalScrollBar) scrollX = newRangeStart; pianoRoll.setScroll(scrollX, scrollY, fixedRowHeight); pianoRoll.repaint(); repaint(); }
void MainComponent::mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel) {
    if (isStartupLoading || isWorkLoading) return;
    if (isSubScreenVisible && (event.mods.isCtrlDown() || event.mods.isCommandDown())) {
        const auto pianoLocalPoint = pianoRoll.getLocalPoint(this, event.getPosition());
        zoomMabbiicoTimeline(static_cast<float>(pianoLocalPoint.x), wheel.deltaY);
        return;
    }
    if (wheel.deltaX != 0.0 || event.mods.isShiftDown()) { if (horizontalScrollBar.isVisible()) { double current = horizontalScrollBar.getCurrentRangeStart(); double delta = (wheel.deltaX != 0.0) ? wheel.deltaX : wheel.deltaY; double newScroll = current - delta * 300.0; double maxScroll = std::max(0.0, horizontalScrollBar.getRangeLimit().getEnd() - horizontalScrollBar.getCurrentRangeSize()); horizontalScrollBar.setCurrentRangeStart(juce::jlimit(0.0, maxScroll, newScroll)); } }
    else { if (verticalScrollBar.isVisible()) { double current = verticalScrollBar.getCurrentRangeStart(); double newScroll = current - wheel.deltaY * 300.0; double maxScroll = std::max(0.0, verticalScrollBar.getRangeLimit().getEnd() - verticalScrollBar.getCurrentRangeSize()); verticalScrollBar.setCurrentRangeStart(juce::jlimit(0.0, maxScroll, newScroll)); } }
}

void MainComponent::timerCallback() {
    if (isWorkLoading && workLoadingOverlay != nullptr)
    {
        workLoadingOverlay->repaint();
        return;
    }

    if (isPlaying && audioEngine.getSampleRate() > 0) {
        auto layoutArea = pianoRoll.getBounds();
        auto rollViewArea = layoutArea.withTrimmedLeft(isSubScreenVisible ? 30 : 210).withTrimmedRight(15).withTrimmedBottom(15);
        rollViewArea.removeFromTop(20);
        int rollWidth = rollViewArea.getWidth();
        const double activeTimelinePixelsPerBeat = isSubScreenVisible ? timelinePixelsPerBeat : defaultTimelinePixelsPerBeatFor3MLE;
        double playheadBeat = MmlLogic::getBeatFromSample(globalSampleCount, tempoMap, audioEngine.getSampleRate());
        double playheadPixel = playheadBeat * activeTimelinePixelsPerBeat;
        if (playheadPixel > scrollX + rollWidth * 0.95 || playheadPixel < scrollX) { scrollX = std::max(0.0, playheadPixel - rollWidth * 0.1); horizontalScrollBar.setCurrentRangeStart(scrollX); }
        refreshPianoRollModel();
        updatePlaybackEditorHighlights();
        repaint();
        return;
    }

    // Idle optimization:
    // The old timer rebuilt the piano-roll model and repainted the whole UI 60 times/sec even while stopped.
    // Edits already call refreshPianoRollModel() directly, so while idle we only need to clear playback highlight once.
    if (playbackEditorHighlightActive)
    {
        clearPlaybackEditorHighlights();
        pianoRoll.repaint();
        repaint();
    }
}

void MainComponent::saveMmiFile() {
    saveCurrentBank();
    fileChooser = std::make_unique<juce::FileChooser>(T("Save MabiIcco MMI file...", L"마비꼬 MMI 파일로 저장..."), juce::File::getSpecialLocation(juce::File::userDocumentsDirectory).getChildFile(getSafeProjectTitleForFile() + ".mmi"), "*.mmi");
    fileChooser->launchAsync(juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles, [this](const juce::FileChooser& fc) {
        auto file = fc.getResult();
        if (file == juce::File()) return;
        if (!file.hasFileExtension(".mmi")) file = file.withFileExtension(".mmi");

        showWorkLoadingOverlay(T("Saving MabiIcco MMI file...", L"마비꼬 MMI 파일을 저장하는 중..."));

        ProjectFileIO::MmiSaveOptions options;
        options.title = currentProjectTitle;
        options.timeSignatureId = timeSignatureCombo.getSelectedId();
        options.languageId = languageCombo.getSelectedId();
        options.getSf2Name = [this](int sf2FileIndex) -> juce::String {
            if (sf2FileIndex >= 0 && sf2FileIndex < audioEngine.getNumEngines())
                return audioEngine.getSf2Name(sf2FileIndex).trim();
            return {};
        };
        options.getTrackName = [this](int trackIndex) -> juce::String {
            return getTrackDisplayName(trackIndex);
        };

        const bool ok = ProjectFileIO::saveMmiFile(file, banks, numActiveTracks, options);
        hideWorkLoadingOverlay();

        if (ok) {
            setCurrentProjectTitleFromFile(file);
            markProjectClean();
            showThemedMessageBoxAsync(juce::AlertWindow::InfoIcon, T("MMI Saved", L"MMI 저장 완료"), file.getFileName() + T(" saved in MabiIcco-compatible format.", L" 파일을 마비꼬 호환 형식으로 저장했습니다."));
        }
        else {
            showThemedMessageBoxAsync(juce::AlertWindow::WarningIcon, T("Save Failed", L"저장 실패"), T("Could not save the MMI file.", L"MMI 파일을 저장하지 못했습니다."));
        }
    });
}


void MainComponent::exportToWavFile() {
    isPlaying = false; updateAllSequences();

    ProjectFileIO::WavExportOptions options;
    options.sampleRate = audioEngine.getSampleRate(); options.numChannels = 2; options.bitsPerSample = 16; options.blockSize = 512;
    options.isPartActiveForBank = [this](int bankIdx, int trackIdx) { return isPartActiveForBank(bankIdx, trackIdx); };
    options.stopAllNotes = [this]() { audioEngine.stopAllNotes(); };
    options.renderAudioBlock = [this](juce::AudioBuffer<float>& buffer, int64_t sampleCounter, bool& isPlayingFlag) { audioEngine.renderAudioBlock(buffer, banks, numActiveTracks, sampleCounter, isPlayingFlag); };

    fileChooser = std::make_unique<juce::FileChooser>(T("Save to WAV...", L"WAV 파일로 저장..."), juce::File::getSpecialLocation(juce::File::userDocumentsDirectory).getChildFile(getSafeProjectTitleForFile() + ".wav"), "*.wav");
    fileChooser->launchAsync(juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles, [this, options](const juce::FileChooser& fc) {
        auto outputFile = fc.getResult(); if (outputFile == juce::File()) return;
        if (!outputFile.hasFileExtension(".wav")) outputFile = outputFile.withFileExtension(".wav");

        showWorkLoadingOverlay(T("Exporting WAV file...", L"WAV 파일을 내보내는 중..."));
        auto result = ProjectFileIO::exportWavFile(outputFile, banks, numActiveTracks, options);
        hideWorkLoadingOverlay();

        if (result.ok) { setCurrentProjectTitleFromFile(outputFile); }
        else {
            const juce::String fallbackMessage = result.hasAnyNotes ? T("Could not export the WAV file.", L"WAV 파일을 내보내지 못했습니다.") : T("The score is empty.", L"악보가 비어 있습니다.");
            const juce::String message = result.hasAnyNotes && result.errorMessage.isNotEmpty() ? result.errorMessage : fallbackMessage;
            showThemedMessageBoxAsync(juce::AlertWindow::WarningIcon, result.hasAnyNotes ? T("Export Failed", L"내보내기 실패") : T("Export Error", L"내보내기 오류"), message);
        }
    });
}

void MainComponent::exportToMidiFile() {
    saveCurrentBank();
    updateAllSequences();

    fileChooser = std::make_unique<juce::FileChooser>(T("Save MIDI file...", L"MIDI 파일로 저장..."), juce::File::getSpecialLocation(juce::File::userDocumentsDirectory).getChildFile(getSafeProjectTitleForFile() + ".mid"), "*.mid");
    fileChooser->launchAsync(juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles, [this](const juce::FileChooser& fc) {
        auto file = fc.getResult();
        if (file == juce::File())
            return;

        if (!file.hasFileExtension(".mid") && !file.hasFileExtension(".midi"))
            file = file.withFileExtension(".mid");

        showWorkLoadingOverlay(T("Saving MIDI file...", L"MIDI 파일을 저장하는 중..."));

        bool hasAnyNotes = false;
        for (int i = 0; i < numActiveTracks && !hasAnyNotes; ++i) {
            for (int j = 0; j < 4 && !hasAnyNotes; ++j) {
                if (!isPartActiveForBank(i, j))
                    continue;
                for (const auto& note : banks[i].tracks[j].sequence) {
                    if (note.frequency > 0.0) { hasAnyNotes = true; break; }
                }
            }
        }

        if (!hasAnyNotes) {
            hideWorkLoadingOverlay();
            showThemedMessageBoxAsync(juce::AlertWindow::WarningIcon, T("Export Error", L"내보내기 오류"), T("The score is empty.", L"악보가 비어 있습니다."));
            return;
        }

        ProjectFileIO::MidiSaveOptions options;
        options.timeSignatureId = timeSignatureCombo.getSelectedId();
        options.bpm = (!tempoMap.empty() && tempoMap.front().bpm > 0.0) ? tempoMap.front().bpm : 120.0;
        options.isPartActiveForBank = [this](int bankIdx, int trackIdx) { return isPartActiveForBank(bankIdx, trackIdx); };

        const bool ok = ProjectFileIO::saveMidiFile(file, banks, numActiveTracks, options);
        hideWorkLoadingOverlay();

        if (ok) {
            setCurrentProjectTitleFromFile(file);
            markProjectClean();
            showThemedMessageBoxAsync(juce::AlertWindow::InfoIcon, T("MIDI Saved", L"MIDI 저장 완료"), file.getFileName() + T(" saved successfully.", L" 파일을 저장했습니다."));
        }
        else {
            showThemedMessageBoxAsync(juce::AlertWindow::WarningIcon, T("Save Failed", L"저장 실패"), T("Could not save the MIDI file.", L"MIDI 파일을 저장하지 못했습니다."));
        }
    });
}

void MainComponent::saveDmmfProject() {
    saveCurrentBank();
    if (currentDmmfProjectFile != juce::File()) {
        showWorkLoadingOverlay(T("Saving DMMF project...", L"DMMF 프로젝트를 저장하는 중..."));
        const auto targetFile = currentDmmfProjectFile;
        juce::Timer::callAfterDelay(80, [this, targetFile]() { writeDmmfProjectToFile(targetFile, false); }); return;
    }
    saveDmmfProjectAs();
}

void MainComponent::saveDmmfProjectAs() {
    saveCurrentBank();
    fileChooser = std::make_unique<juce::FileChooser>(T("Save DMMF project as...", L"DMMF 다른 이름으로 저장..."), juce::File::getSpecialLocation(juce::File::userDocumentsDirectory).getChildFile(getSafeProjectTitleForFile() + ".dmmf"), "*.dmmf");
    fileChooser->launchAsync(juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles, [this](const juce::FileChooser& fc) {
        auto file = fc.getResult(); if (file == juce::File()) { pendingCloseAfterSave = false; return; }
        if (!file.hasFileExtension(".dmmf")) file = file.withFileExtension(".dmmf");
        showWorkLoadingOverlay(T("Saving DMMF project...", L"DMMF 프로젝트를 저장하는 중..."));
        const auto targetFile = file; juce::Timer::callAfterDelay(80, [this, targetFile]() { writeDmmfProjectToFile(targetFile, true); });
    });
}

void MainComponent::writeDmmfProjectToFile(const juce::File& file, bool showSuccessMessage) {
    if (file == juce::File()) { pendingCloseAfterSave = false; hideWorkLoadingOverlay(); return; }

    saveCurrentBank();

    ProjectFileIO::DmmfSaveOptions options;
    options.title = getSafeProjectTitleForFile();
    options.currentTrackIndex = currentBankIndex;
    options.timeSignatureId = timeSignatureCombo.getSelectedId();
    options.themeId = currentThemeId;
    options.languageId = languageCombo.getSelectedId();
    options.getSf2Name = [this](int sf2Index) -> juce::String { return audioEngine.getSf2Name(sf2Index); };
    options.getPresetName = [this](int sf2Index, int presetIndex) -> juce::String {
        if (const char* rawName = audioEngine.getPresetName(sf2Index, presetIndex)) return juce::String(rawName);
        return juce::String();
    };
    options.getTrackName = [this](int trackIndex) -> juce::String { return getTrackDisplayName(trackIndex); };

    const bool ok = ProjectFileIO::saveDmmfProject(file, banks, numActiveTracks, options);
    hideWorkLoadingOverlay();

    if (ok) {
        currentDmmfProjectFile = file; setCurrentProjectTitleFromFile(file); markProjectClean();
        if (pendingCloseAfterSave) { pendingCloseAfterSave = false; quitApplicationNow(); return; }
        if (showSuccessMessage) showThemedMessageBoxAsync(juce::AlertWindow::InfoIcon, T("DMMF Saved", L"DMMF 저장 완료"), file.getFileName() + T(" saved successfully.", L" 파일을 저장했습니다."));
    }
    else { pendingCloseAfterSave = false; showThemedMessageBoxAsync(juce::AlertWindow::WarningIcon, T("Save Failed", L"저장 실패"), T("Could not save the DMMF project file.", L"DMMF 프로젝트 파일을 저장하지 못했습니다.")); }
}

void MainComponent::loadDmmfProject() {
    fileChooser = std::make_unique<juce::FileChooser>(T("Open DMMF project...", L"DMMF 프로젝트 열기..."), juce::File::getSpecialLocation(juce::File::userDocumentsDirectory), "*.dmmf");
    fileChooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles, [this](const juce::FileChooser& fc) {
        const auto file = fc.getResult(); if (file == juce::File()) return;
        showWorkLoadingOverlay(T("Opening DMMF project...", L"DMMF 프로젝트를 여는 중..."));

        { const juce::ScopedLock sl(audioEngine.getLock()); isPlaying = false; globalSampleCount = 0; scrollX = 0.0; audioEngine.stopAllNotes(); }

        auto findSf2IndexByName = [this](const juce::String& name) -> int {
            const auto trimmed = name.trim(); if (trimmed.isEmpty()) return -1;
            for (int i = 0; i < audioEngine.getNumEngines(); ++i) { juce::String sf2 = audioEngine.getSf2Name(i); if (sf2.equalsIgnoreCase(trimmed) || sf2.equalsIgnoreCase(trimmed + ".sf2") || sf2.equalsIgnoreCase(trimmed + ".dls")) return i; }
            return -1;
        };

        const auto loadResult = ProjectFileIO::loadDmmfProject(file, banks, MAX_BANKS, audioEngine.getNumEngines() - 1, findSf2IndexByName);
        if (!loadResult.ok) {
            hideWorkLoadingOverlay();
            const bool isEmptyFile = loadResult.errorMessage.containsIgnoreCase("empty");
            showThemedMessageBoxAsync(juce::AlertWindow::WarningIcon,
                                      T("Open Failed", L"열기 실패"),
                                      isEmptyFile ? T("The selected file is empty.", L"선택한 파일이 비어 있습니다.")
                                                  : T("This does not look like a valid .dmmf project file.", L"올바른 .dmmf 프로젝트 파일로 보이지 않습니다."));
            return;
        }

        numActiveTracks = loadResult.numActiveTracks;
        for (int i = 0; i < MAX_BANKS; ++i)
            customTrackNames[i].clear();
        trackNamesUsePresetInstruments = false;
        refreshTrackNameFromPresetButtonText();
        meterChanges.clear();
        setTimeSignatureId(loadResult.timeSignatureId, juce::dontSendNotification);
        if (loadResult.themeId >= 1 && loadResult.themeId <= 12) applyTheme(loadResult.themeId);
        if (loadResult.languageId == 1 || loadResult.languageId == 2 || loadResult.languageId == 3) languageCombo.setSelectedId(loadResult.languageId, juce::dontSendNotification);

        for (int i = 0; i < MAX_BANKS; ++i) tabButtons[i].setVisible(i < numActiveTracks);
        refreshSongPresetModeCache(); updateUITexts();
        const int targetTrack = loadResult.currentTrackIndex;
        tabButtons[targetTrack].setToggleState(true, juce::dontSendNotification); loadBank(targetTrack);
        int previousBank = currentBankIndex; currentBankIndex = 0; buildTempoMap(); currentBankIndex = previousBank;
        { const juce::ScopedLock sl(audioEngine.getLock()); for (int i = 0; i < numActiveTracks; ++i) { for (int j = 0; j < 4; ++j) { banks[i].tracks[j].sequence = MmlLogic::parseMMLWithTempoMap(banks[i].tracks[j].mml, tempoMap, audioEngine.getSampleRate()); banks[i].tracks[j].noteIndex = 0; banks[i].tracks[j].currentAngle = 0.0; } } }
        horizontalScrollBar.setCurrentRangeStart(0.0); scrollX = 0.0; updateMmlCharCountLabels(); refreshPianoRollModel(); resized(); repaint();
        currentDmmfProjectFile = file; setCurrentProjectTitleFromFile(file); markProjectClean(); hideWorkLoadingOverlay();
        showThemedMessageBoxAsync(juce::AlertWindow::InfoIcon, T("DMMF Opened", L"DMMF 열기 완료"), file.getFileName() + T(" opened successfully.", L" 파일을 불러왔습니다."));
    });
}

void MainComponent::importMmiFile() {
    fileChooser = std::make_unique<juce::FileChooser>(T("Open MMI file...", L"MMI 파일 열기..."), juce::File::getSpecialLocation(juce::File::userDocumentsDirectory), "*.mmi");
    fileChooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles, [this](const juce::FileChooser& fc) {
        auto file = fc.getResult();
        if (file == juce::File()) return;

        showWorkLoadingOverlay(T("Importing MMI file...", L"MMI 파일을 불러오는 중..."));

        // Crash-safe MMI import:
        // 14-person MMI files can take long enough to rebuild banks/sequences that the audio
        // callback may still touch the old data if playback/preview voices are alive.
        // Stop every transport/preview/DLS voice before ProjectFileIO resets banks.
        stopPianoRollPreviewNote();
        {
            const juce::ScopedLock sl(audioEngine.getLock());
            isPlaying = false;
            globalSampleCount = 0;
            scrollX = 0.0;
            audioEngine.stopAllNotes();

            for (int bi = 0; bi < MAX_BANKS; ++bi)
            {
                for (int pj = 0; pj < 4; ++pj)
                {
                    banks[bi].tracks[pj].noteIndex = 0;
                    banks[bi].tracks[pj].currentAngle = 0.0;
                    banks[bi].tracks[pj].sequence.clear();
                }
            }
        }
        clearPlaybackEditorHighlights();

        ProjectFileIO::MmiLoadOptions options;
        options.maxBanks = MAX_BANKS;
        options.maxSf2Files = audioEngine.getNumEngines();
        options.findSf2IndexByStem = [this](const juce::String& stem) -> int {
            const auto wanted = stem.upToFirstOccurrenceOf(".", false, false).trim();
            for (int fi = 0; fi < audioEngine.getNumEngines(); ++fi) {
                const auto name = audioEngine.getSf2Name(fi).upToFirstOccurrenceOf(".", false, false).trim();
                if (name.equalsIgnoreCase(wanted)) return fi;
            }
            return -1;
        };
        options.getPresetCount = [this](int sf2FileIndex) -> int {
            if (sf2FileIndex < 0 || sf2FileIndex >= audioEngine.getNumEngines()) return 0;
            return audioEngine.getPresetCount(sf2FileIndex);
        };
        options.findPresetIndexByBankAndProgram = [this](int sf2FileIndex, int midiBank, int program) -> int {
            if (sf2FileIndex < 0 || sf2FileIndex >= audioEngine.getNumEngines()) return -1;
            const juce::ScopedLock sl(audioEngine.getLock());
            return audioEngine.getPresetIndex(sf2FileIndex, midiBank, program);
        };
        options.refreshSongPresetModeForBank = [this](int bankIndex) -> bool {
            return computeSongPresetModeForBank(bankIndex);
        };

        const auto loadResult = ProjectFileIO::importMmiFile(file, banks, options);

        if (loadResult.ok) {
            numActiveTracks = juce::jlimit(1, MAX_BANKS, loadResult.numActiveTracks);
            for (int i = 0; i < MAX_BANKS; ++i) tabButtons[i].setVisible(i < numActiveTracks);

            int prevBank = juce::jlimit(0, numActiveTracks - 1, currentBankIndex);
            currentBankIndex = 0;
            buildTempoMap();
            currentBankIndex = prevBank;

            {
                const juce::ScopedLock sl(audioEngine.getLock());
                isPlaying = false;
                audioEngine.stopAllNotes();

                const double safeSampleRate = audioEngine.getSampleRate() > 0.0 ? audioEngine.getSampleRate() : 44100.0;
                for (int i = 0; i < numActiveTracks; ++i) {
                    for (int j = 0; j < 4; ++j) {
                        banks[i].tracks[j].noteIndex = 0;
                        banks[i].tracks[j].currentAngle = 0.0;
                        banks[i].tracks[j].sequence = MmlLogic::parseMMLWithTempoMap(banks[i].tracks[j].mml, tempoMap, safeSampleRate);
                    }
                }
            }

            currentBankIndex = 0;
            tabButtons[0].setToggleState(true, juce::dontSendNotification);
            loadBank(0);
            updateMmlCharCountLabels();
            refreshPianoRollModel();
            resized();
            repaint();
            currentDmmfProjectFile = juce::File();

            if (loadResult.title.isNotEmpty()) setCurrentProjectTitle(loadResult.title);
            else setCurrentProjectTitleFromFile(file);

            markProjectClean();
            hideWorkLoadingOverlay();
            showThemedMessageBoxAsync(juce::AlertWindow::InfoIcon, T("Import Complete", L"불러오기 완료"), juce::String(loadResult.loadedTrackCount) + T(" tracks imported.", L"개의 트랙을 성공적으로 불러왔습니다."));
        }
        else {
            hideWorkLoadingOverlay();
            const juce::String message = loadResult.errorMessage.isNotEmpty() ? loadResult.errorMessage : T("Could not import the MMI file.", L"MMI 파일을 불러오지 못했습니다.");
            showThemedMessageBoxAsync(juce::AlertWindow::WarningIcon, T("Open Failed", L"열기 실패"), message);
        }
    });
}


void MainComponent::importMidiFile() {
    fileChooser = std::make_unique<juce::FileChooser>(T("Open MIDI file...", L"MIDI 파일 열기..."), juce::File::getSpecialLocation(juce::File::userDocumentsDirectory), "*.mid;*.midi");
    fileChooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles, [this](const juce::FileChooser& fc) {
        const auto file = fc.getResult();
        if (file == juce::File())
            return;

        showWorkLoadingOverlay(T("Importing MIDI file...", L"MIDI 파일을 불러오는 중..."));

        ProjectFileIO::MidiLoadOptions options;
        options.maxBanks = MAX_BANKS;
        options.findSf2IndexByStem = [this](const juce::String& stem) -> int {
            for (int i = 0; i < audioEngine.getNumEngines(); ++i) {
                if (audioEngine.getSf2Name(i).equalsIgnoreCase(stem))
                    return i;
            }
            return -1;
        };
        options.getPresetCount = [this](int fileIndex) -> int {
            return audioEngine.getPresetCount(fileIndex);
        };

        const auto result = ProjectFileIO::importMidiFile(file, banks, options);
        if (!result.ok) {
            hideWorkLoadingOverlay();
            const auto message = result.errorMessage.isNotEmpty() ? result.errorMessage : T("Could not import the MIDI file.", L"MIDI 파일을 불러오지 못했습니다.");
            showThemedMessageBoxAsync(juce::AlertWindow::WarningIcon, T("Open Failed", L"열기 실패"), message);
            return;
        }

        numActiveTracks = juce::jlimit(1, MAX_BANKS, result.numActiveTracks);
        for (int i = 0; i < MAX_BANKS; ++i) {
            tabButtons[i].setVisible(i < numActiveTracks);
            tabButtons[i].setToggleState(false, juce::dontSendNotification);
        }

        if (result.numerator == 3 && result.denominator == 4)
            setTimeSignatureId(2, juce::dontSendNotification);
        else
            setTimeSignatureId(1, juce::dontSendNotification);

        currentBankIndex = 0;
        buildTempoMap();

        {
            const juce::ScopedLock sl(audioEngine.getLock());
            for (int i = 0; i < numActiveTracks; ++i) {
                for (int j = 0; j < 4; ++j) {
                    banks[i].tracks[j].sequence = MmlLogic::parseMMLWithTempoMap(banks[i].tracks[j].mml, tempoMap, audioEngine.getSampleRate());
                    banks[i].tracks[j].noteIndex = 0;
                    banks[i].tracks[j].currentAngle = 0.0;
                }
            }
        }

        refreshSongPresetModeCache();
        tabButtons[0].setToggleState(true, juce::dontSendNotification);
        loadBank(0);
        updateUITexts();
        updateMmlCharCountLabels();
        refreshPianoRollModel();
        resized();
        repaint();

        currentDmmfProjectFile = juce::File();
        setCurrentProjectTitleFromFile(file);
        markProjectClean();

        hideWorkLoadingOverlay();
        juce::String detail;
        detail << file.getFileName() << "\n" << juce::String(numActiveTracks) << T(" MIDI channels imported as tool tracks.", L"개의 MIDI 채널을 툴 트랙으로 불러왔습니다.");
        showThemedMessageBoxAsync(juce::AlertWindow::InfoIcon, T("MIDI Imported", L"MIDI 불러오기 완료"), detail);
    });
}
