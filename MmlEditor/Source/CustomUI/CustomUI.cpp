#include "CustomUI.h"

namespace CustomUI
{
    juce::Colour getReadableTextColour(juce::Colour background) {
        const float r = background.getRed() / 255.0f;
        const float g = background.getGreen() / 255.0f;
        const float b = background.getBlue() / 255.0f;
        const float luminance = 0.2126f * r + 0.7152f * g + 0.0722f * b;
        return luminance > 0.55f ? juce::Colour(0xff111827) : juce::Colour(0xfff8fafc);
    }

    juce::Colour getThemeColour(int themeId, const juce::String& role) {
        const int theme = juce::jlimit(1, 12, themeId);
        if (theme == 1) { // Classic Light
            if (role == "backgroundTop") return juce::Colour(0xfff5f5f5); if (role == "backgroundBottom") return juce::Colour(0xffd8d8d8);
            if (role == "panel") return juce::Colour(0xfff0f0f0); if (role == "panel2") return juce::Colour(0xffdedede);
            if (role == "timeline") return juce::Colour(0xff333333); if (role == "timeline2") return juce::Colour(0xff444444);
            if (role == "gridWhite") return juce::Colour(0xff585858); if (role == "gridBlack") return juce::Colour(0xff424242); if (role == "gridLine") return juce::Colour(0xff303030);
            if (role == "pianoWhite") return juce::Colour(0xffeeeeee); if (role == "pianoBlack") return juce::Colour(0xff222222);
            if (role == "text") return juce::Colour(0xff101010); if (role == "textOnDark") return juce::Colours::white; if (role == "mutedText") return juce::Colour(0xff666666);
            if (role == "editorBg") return juce::Colours::white; if (role == "editorReadOnly") return juce::Colour(0xffeeeeee);
            if (role == "button") return juce::Colour(0xff555555); if (role == "accent") return juce::Colour(0xff2b6cb0); if (role == "accent2") return juce::Colour(0xffd69e2e); if (role == "playhead") return juce::Colours::red;
        }
        else if (theme == 2) { // Dark Studio
            if (role == "backgroundTop") return juce::Colour(0xff171b22); if (role == "backgroundBottom") return juce::Colour(0xff0c0f14);
            if (role == "panel") return juce::Colour(0xff202632); if (role == "panel2") return juce::Colour(0xff121720);
            if (role == "timeline") return juce::Colour(0xff0d1118); if (role == "timeline2") return juce::Colour(0xff171d29);
            if (role == "gridWhite") return juce::Colour(0xff2b3240); if (role == "gridBlack") return juce::Colour(0xff222836); if (role == "gridLine") return juce::Colour(0xff3c4658);
            if (role == "pianoWhite") return juce::Colour(0xffd8dde8); if (role == "pianoBlack") return juce::Colour(0xff151922);
            if (role == "text") return juce::Colour(0xffe6edf7); if (role == "textOnDark") return juce::Colour(0xffe6edf7); if (role == "mutedText") return juce::Colour(0xff9aa5b8);
            if (role == "editorBg") return juce::Colour(0xff10141c); if (role == "editorReadOnly") return juce::Colour(0xff1b202b);
            if (role == "button") return juce::Colour(0xff2d3648); if (role == "accent") return juce::Colour(0xff5eead4); if (role == "accent2") return juce::Colour(0xff60a5fa); if (role == "playhead") return juce::Colour(0xffff6b6b);
        }
        else if (theme == 3) { // Mabinogi Fantasy
            if (role == "backgroundTop") return juce::Colour(0xff20253a); if (role == "backgroundBottom") return juce::Colour(0xff10131f);
            if (role == "panel") return juce::Colour(0xff2a2f45); if (role == "panel2") return juce::Colour(0xff161a2a);
            if (role == "timeline") return juce::Colour(0xff151827); if (role == "timeline2") return juce::Colour(0xff202842);
            if (role == "gridWhite") return juce::Colour(0xff2c344e); if (role == "gridBlack") return juce::Colour(0xff22283e); if (role == "gridLine") return juce::Colour(0xff47506f);
            if (role == "pianoWhite") return juce::Colour(0xfff1ead9); if (role == "pianoBlack") return juce::Colour(0xff1c2030);
            if (role == "text") return juce::Colour(0xfffff5d6); if (role == "textOnDark") return juce::Colour(0xfffff5d6); if (role == "mutedText") return juce::Colour(0xffc8b98f);
            if (role == "editorBg") return juce::Colour(0xff111522); if (role == "editorReadOnly") return juce::Colour(0xff252b3c);
            if (role == "button") return juce::Colour(0xff4b3f72); if (role == "accent") return juce::Colour(0xffffd166); if (role == "accent2") return juce::Colour(0xff78dcca); if (role == "playhead") return juce::Colour(0xffff6b6b);
        }
        else if (theme == 4) { // Neon Night
            if (role == "backgroundTop") return juce::Colour(0xff0b1020); if (role == "backgroundBottom") return juce::Colour(0xff02040a);
            if (role == "panel") return juce::Colour(0xff12182d); if (role == "panel2") return juce::Colour(0xff070b16);
            if (role == "timeline") return juce::Colour(0xff050812); if (role == "timeline2") return juce::Colour(0xff10172b);
            if (role == "gridWhite") return juce::Colour(0xff18213a); if (role == "gridBlack") return juce::Colour(0xff11182b); if (role == "gridLine") return juce::Colour(0xff243154);
            if (role == "pianoWhite") return juce::Colour(0xffe8f7ff); if (role == "pianoBlack") return juce::Colour(0xff080b14);
            if (role == "text") return juce::Colour(0xffdff7ff); if (role == "textOnDark") return juce::Colour(0xffdff7ff); if (role == "mutedText") return juce::Colour(0xff88a7ff);
            if (role == "editorBg") return juce::Colour(0xff050914); if (role == "editorReadOnly") return juce::Colour(0xff151b2d);
            if (role == "button") return juce::Colour(0xff25164a); if (role == "accent") return juce::Colour(0xffff4fd8); if (role == "accent2") return juce::Colour(0xff37f7ff); if (role == "playhead") return juce::Colour(0xffff2f6d);
        }
        else if (theme == 5) { // Orchestra Brown
            if (role == "backgroundTop") return juce::Colour(0xff2a2119); if (role == "backgroundBottom") return juce::Colour(0xff120d09);
            if (role == "panel") return juce::Colour(0xff3a2b1f); if (role == "panel2") return juce::Colour(0xff21170f);
            if (role == "timeline") return juce::Colour(0xff1a120b); if (role == "timeline2") return juce::Colour(0xff2f2116);
            if (role == "gridWhite") return juce::Colour(0xff3b2b1d); if (role == "gridBlack") return juce::Colour(0xff2b1f15); if (role == "gridLine") return juce::Colour(0xff6b4b2d);
            if (role == "pianoWhite") return juce::Colour(0xffead8bd); if (role == "pianoBlack") return juce::Colour(0xff1a120b);
            if (role == "text") return juce::Colour(0xffffeed2); if (role == "textOnDark") return juce::Colour(0xffffeed2); if (role == "mutedText") return juce::Colour(0xffc7a77d);
            if (role == "editorBg") return juce::Colour(0xff1b120b); if (role == "editorReadOnly") return juce::Colour(0xff332418);
            if (role == "button") return juce::Colour(0xff5a3a22); if (role == "accent") return juce::Colour(0xffd6a257); if (role == "accent2") return juce::Colour(0xfff2c879); if (role == "playhead") return juce::Colour(0xffffd166);
        }
        else if (theme == 6) { // Saint White
            if (role == "backgroundTop") return juce::Colour(0xffffffff); if (role == "backgroundBottom") return juce::Colour(0xffeef2f8);
            if (role == "panel") return juce::Colour(0xfff9fbff); if (role == "panel2") return juce::Colour(0xffe7edf7);
            if (role == "timeline") return juce::Colour(0xffd6dfed); if (role == "timeline2") return juce::Colour(0xffeef4ff);
            if (role == "gridWhite") return juce::Colour(0xfffbfdff); if (role == "gridBlack") return juce::Colour(0xffedf2fa); if (role == "gridLine") return juce::Colour(0xffc7d3e5);
            if (role == "pianoWhite") return juce::Colour(0xffffffff); if (role == "pianoBlack") return juce::Colour(0xff2f3644);
            if (role == "text") return juce::Colour(0xff1f2937); if (role == "textOnDark") return juce::Colour(0xff1f2937); if (role == "mutedText") return juce::Colour(0xff6b7280);
            if (role == "editorBg") return juce::Colour(0xffffffff); if (role == "editorReadOnly") return juce::Colour(0xffedf2f9);
            if (role == "button") return juce::Colour(0xffd8e1ef); if (role == "accent") return juce::Colour(0xffb98f3f); if (role == "accent2") return juce::Colour(0xff5b8def); if (role == "playhead") return juce::Colour(0xffd94854);
        }
        else if (theme == 7) { // Spring Mint
            if (role == "backgroundTop") return juce::Colour(0xfff2fff9); if (role == "backgroundBottom") return juce::Colour(0xffdff7ec);
            if (role == "panel") return juce::Colour(0xffecfff5); if (role == "panel2") return juce::Colour(0xffd3f2e2);
            if (role == "timeline") return juce::Colour(0xffbde8d2); if (role == "timeline2") return juce::Colour(0xffe1faee);
            if (role == "gridWhite") return juce::Colour(0xfff7fffb); if (role == "gridBlack") return juce::Colour(0xffe5f7ee); if (role == "gridLine") return juce::Colour(0xffabcdbb);
            if (role == "pianoWhite") return juce::Colour(0xffffffff); if (role == "pianoBlack") return juce::Colour(0xff243d36);
            if (role == "text") return juce::Colour(0xff173b32); if (role == "textOnDark") return juce::Colour(0xff173b32); if (role == "mutedText") return juce::Colour(0xff5f7f72);
            if (role == "editorBg") return juce::Colour(0xffffffff); if (role == "editorReadOnly") return juce::Colour(0xffe2f4ea);
            if (role == "button") return juce::Colour(0xffb7e3cd); if (role == "accent") return juce::Colour(0xff2fbf8f); if (role == "accent2") return juce::Colour(0xff5aa9e6); if (role == "playhead") return juce::Colour(0xffff6b6b);
        }
        else if (theme == 8) { // Sky Blue
            if (role == "backgroundTop") return juce::Colour(0xfff0f8ff); if (role == "backgroundBottom") return juce::Colour(0xffd7ecff);
            if (role == "panel") return juce::Colour(0xffeaf6ff); if (role == "panel2") return juce::Colour(0xffcfe7fb);
            if (role == "timeline") return juce::Colour(0xffb9d9f4); if (role == "timeline2") return juce::Colour(0xffe0f1ff);
            if (role == "gridWhite") return juce::Colour(0xfff8fcff); if (role == "gridBlack") return juce::Colour(0xffe6f2fb); if (role == "gridLine") return juce::Colour(0xffa8c7df);
            if (role == "pianoWhite") return juce::Colour(0xffffffff); if (role == "pianoBlack") return juce::Colour(0xff24364a);
            if (role == "text") return juce::Colour(0xff14304a); if (role == "textOnDark") return juce::Colour(0xff14304a); if (role == "mutedText") return juce::Colour(0xff5d7893);
            if (role == "editorBg") return juce::Colour(0xffffffff); if (role == "editorReadOnly") return juce::Colour(0xffe2edf8);
            if (role == "button") return juce::Colour(0xffb5d7f2); if (role == "accent") return juce::Colour(0xff2b8fd8); if (role == "accent2") return juce::Colour(0xffffb74d); if (role == "playhead") return juce::Colour(0xffff4f6f);
        }
        else if (theme == 9) { // Peach Cream
            if (role == "backgroundTop") return juce::Colour(0xfffffbf4); if (role == "backgroundBottom") return juce::Colour(0xffffe4d6);
            if (role == "panel") return juce::Colour(0xfffff3e8); if (role == "panel2") return juce::Colour(0xffffdccb);
            if (role == "timeline") return juce::Colour(0xffffcbb6); if (role == "timeline2") return juce::Colour(0xffffeadf);
            if (role == "gridWhite") return juce::Colour(0xfffffbf8); if (role == "gridBlack") return juce::Colour(0xffffeee5); if (role == "gridLine") return juce::Colour(0xffe1aa93);
            if (role == "pianoWhite") return juce::Colour(0xfffffff9); if (role == "pianoBlack") return juce::Colour(0xff4b2e2a);
            if (role == "text") return juce::Colour(0xff4a2a22); if (role == "textOnDark") return juce::Colour(0xff4a2a22); if (role == "mutedText") return juce::Colour(0xff9a6b5d);
            if (role == "editorBg") return juce::Colour(0xffffffff); if (role == "editorReadOnly") return juce::Colour(0xffffeadf);
            if (role == "button") return juce::Colour(0xffffc9a8); if (role == "accent") return juce::Colour(0xffff8a65); if (role == "accent2") return juce::Colour(0xffd6a257); if (role == "playhead") return juce::Colour(0xffe84a5f);
        }
        else if (theme == 10) { // Morrighan
            if (role == "backgroundTop") return juce::Colour(0xff191713); if (role == "backgroundBottom") return juce::Colour(0xff020201);
            if (role == "panel") return juce::Colour(0xff221910); if (role == "panel2") return juce::Colour(0xff0c0705);
            if (role == "timeline") return juce::Colour(0xff050504); if (role == "timeline2") return juce::Colour(0xff16110a);
            if (role == "gridWhite") return juce::Colour(0xff3c3930); if (role == "gridBlack") return juce::Colour(0xff191713); if (role == "gridLine") return juce::Colour(0xff4c4435);
            if (role == "pianoWhite") return juce::Colour(0xfffffefc); if (role == "pianoBlack") return juce::Colour(0xff070706);
            if (role == "text") return juce::Colour(0xfffdedce); if (role == "textOnDark") return juce::Colour(0xfffdedce); if (role == "mutedText") return juce::Colour(0xff887763);
            if (role == "editorBg") return juce::Colour(0xff0c0705); if (role == "editorReadOnly") return juce::Colour(0xff221910);
            if (role == "button") return juce::Colour(0xff3c3930); if (role == "accent") return juce::Colour(0xffcfb8a4); if (role == "accent2") return juce::Colour(0xff887763); if (role == "playhead") return juce::Colour(0xfffdedce);
        }
        else if (theme == 11) { // Cichol
            if (role == "backgroundTop") return juce::Colour(0xff1c1c13); if (role == "backgroundBottom") return juce::Colour(0xff000000);
            if (role == "panel") return juce::Colour(0xff2d2516); if (role == "panel2") return juce::Colour(0xff100f0a);
            if (role == "timeline") return juce::Colour(0xff050101); if (role == "timeline2") return juce::Colour(0xff1a1c14);
            if (role == "gridWhite") return juce::Colour(0xff2d2516); if (role == "gridBlack") return juce::Colour(0xff1c1c13); if (role == "gridLine") return juce::Colour(0xff59492e);
            if (role == "pianoWhite") return juce::Colour(0xfffffefb); if (role == "pianoBlack") return juce::Colour(0xff000000);
            if (role == "text") return juce::Colour(0xffece2cc); if (role == "textOnDark") return juce::Colour(0xffece2cc); if (role == "mutedText") return juce::Colour(0xffa7916c);
            if (role == "editorBg") return juce::Colour(0xff050101); if (role == "editorReadOnly") return juce::Colour(0xff1c1c13);
            if (role == "button") return juce::Colour(0xff2d2516); if (role == "accent") return juce::Colour(0xffc8b38e); if (role == "accent2") return juce::Colour(0xffffffff); if (role == "playhead") return juce::Colour(0xffffffff);
        }
        else { // Milletian
            if (role == "backgroundTop") return juce::Colour(0xfffffdef); if (role == "backgroundBottom") return juce::Colour(0xfffde9cd);
            if (role == "panel") return juce::Colour(0xfffff0de); if (role == "panel2") return juce::Colour(0xffe9c998);
            if (role == "timeline") return juce::Colour(0xffe9c998); if (role == "timeline2") return juce::Colour(0xfffffdef);
            if (role == "gridWhite") return juce::Colour(0xfffffdef); if (role == "gridBlack") return juce::Colour(0xfffef0de); if (role == "gridLine") return juce::Colour(0xff8e9879);
            if (role == "pianoWhite") return juce::Colour(0xfffffdef); if (role == "pianoBlack") return juce::Colour(0xff0b1e14);
            if (role == "text") return juce::Colour(0xff0b1e14); if (role == "textOnDark") return juce::Colour(0xff0b1e14); if (role == "mutedText") return juce::Colour(0xff4c574b);
            if (role == "editorBg") return juce::Colour(0xfffffdef); if (role == "editorReadOnly") return juce::Colour(0xfffde9cd);
            if (role == "button") return juce::Colour(0xffe9c998); if (role == "accent") return juce::Colour(0xff345449); if (role == "accent2") return juce::Colour(0xffd9a259); if (role == "playhead") return juce::Colour(0xff345449);
        }
        return juce::Colours::white;
    }

    juce::Colour getTrackThemeColour(int themeId, int trackIdx) {
        const int theme = juce::jlimit(1, 12, themeId);
        const int t = ((trackIdx % 4) + 4) % 4;
        if (theme == 1) { const juce::Colour colors[] = { juce::Colour(0xffd84c72), juce::Colour(0xff2f9e44), juce::Colour(0xff228be6), juce::Colour(0xff9c36b5) }; return colors[t]; }
        if (theme == 2) { const juce::Colour colors[] = { juce::Colour(0xff60a5fa), juce::Colour(0xff34d399), juce::Colour(0xffc084fc), juce::Colour(0xfffbbf24) }; return colors[t]; }
        if (theme == 3) { const juce::Colour colors[] = { juce::Colour(0xffffd166), juce::Colour(0xff78dcca), juce::Colour(0xff9db4ff), juce::Colour(0xffff9ccf) }; return colors[t]; }
        if (theme == 4) { const juce::Colour colors[] = { juce::Colour(0xffff4fd8), juce::Colour(0xff37f7ff), juce::Colour(0xffb6ff3b), juce::Colour(0xffffb84d) }; return colors[t]; }
        if (theme == 5) { const juce::Colour colors[] = { juce::Colour(0xffd6a257), juce::Colour(0xffb87a3d), juce::Colour(0xffe7c078), juce::Colour(0xffa66a3f) }; return colors[t]; }
        if (theme == 6) { const juce::Colour colors[] = { juce::Colour(0xffc9a64f), juce::Colour(0xff6aa6f8), juce::Colour(0xff8b7cf6), juce::Colour(0xffe07a9a) }; return colors[t]; }
        if (theme == 7) { const juce::Colour colors[] = { juce::Colour(0xff2fbf8f), juce::Colour(0xff4aa3df), juce::Colour(0xff79c46b), juce::Colour(0xffffb86b) }; return colors[t]; }
        if (theme == 8) { const juce::Colour colors[] = { juce::Colour(0xff2b8fd8), juce::Colour(0xff35b7d5), juce::Colour(0xff7e9cf5), juce::Colour(0xffffb74d) }; return colors[t]; }
        if (theme == 9) { const juce::Colour colors[] = { juce::Colour(0xffff8a65), juce::Colour(0xfff4b266), juce::Colour(0xffe2778f), juce::Colour(0xffc58bd6) }; return colors[t]; }
        if (theme == 10) { const juce::Colour colors[] = { juce::Colour(0xfffdedce), juce::Colour(0xff887763), juce::Colour(0xff3c3930), juce::Colour(0xffcfb8a4) }; return colors[t]; }
        if (theme == 11) { const juce::Colour colors[] = { juce::Colour(0xffffffff), juce::Colour(0xffc8b38e), juce::Colour(0xff59492e), juce::Colour(0xffcebea6) }; return colors[t]; }
        const juce::Colour colors[] = { juce::Colour(0xff345449), juce::Colour(0xffd9a259), juce::Colour(0xff89613c), juce::Colour(0xffe9c998) }; return colors[t];
    }

    juce::Colour getBankColor(int index) {
        switch (index) {
        case 0: return juce::Colours::red; case 1: return juce::Colours::lightgreen; case 2: return juce::Colours::blue; case 3: return juce::Colours::orange;
        case 4: return juce::Colours::yellow; case 5: return juce::Colours::green; case 6: return juce::Colour(0xff000080); case 7: return juce::Colours::purple;
        case 8: return juce::Colour(0xff8b008b); case 9: return juce::Colours::teal; case 10: return juce::Colours::greenyellow; case 11: return juce::Colours::grey;
        case 12: return juce::Colours::skyblue; case 13: return juce::Colours::coral; case 14: return juce::Colours::white; case 15: return juce::Colours::black;
        default: return juce::Colours::white;
        }
    }

    // --------------------------------------------------------
    // WorkLoadingOverlayComponent 구현
    // --------------------------------------------------------
    WorkLoadingOverlayComponent::WorkLoadingOverlayComponent() {
        setInterceptsMouseClicks(true, true);
        setWantsKeyboardFocus(true);
        setAlwaysOnTop(true);
    }

    void WorkLoadingOverlayComponent::setVisuals(juce::Image newImage, juce::String newText,
                                                 juce::Colour newBackgroundTop, juce::Colour newBackgroundBottom,
                                                 juce::Colour newPanel, juce::Colour newPanel2,
                                                 juce::Colour newAccent, juce::Colour newAccent2,
                                                 juce::Colour newTextColour, juce::Colour newMutedTextColour) {
        image = newImage; message = newText;
        backgroundTop = newBackgroundTop; backgroundBottom = newBackgroundBottom;
        panel = newPanel; panel2 = newPanel2;
        accent = newAccent; accent2 = newAccent2;
        textColour = newTextColour; mutedTextColour = newMutedTextColour;
        repaint();
    }

    void WorkLoadingOverlayComponent::paint(juce::Graphics& g) {
        const auto area = getLocalBounds();
        const auto areaF = area.toFloat();

        juce::ColourGradient bg(backgroundTop.withAlpha(0.88f), areaF.getCentreX(), areaF.getY(),
            backgroundBottom.withAlpha(0.94f), areaF.getCentreX(), areaF.getBottom(), false);
        g.setGradientFill(bg);
        g.fillRect(area);

        const float cardW = juce::jlimit(420.0f, 680.0f, static_cast<float>(area.getWidth()) * 0.50f);
        const float cardH = juce::jlimit(290.0f, 420.0f, static_cast<float>(area.getHeight()) * 0.48f);
        auto card = juce::Rectangle<float>(0.0f, 0.0f, cardW, cardH)
            .withCentre({ static_cast<float>(area.getCentreX()), static_cast<float>(area.getCentreY()) });

        g.setColour(juce::Colours::black.withAlpha(0.34f));
        g.fillRoundedRectangle(card.translated(0.0f, 10.0f), 22.0f);
        g.setColour(panel.withAlpha(0.96f));
        g.fillRoundedRectangle(card, 22.0f);
        g.setColour(accent.withAlpha(0.72f));
        g.drawRoundedRectangle(card, 22.0f, 1.6f);

        auto inner = card.reduced(30.0f, 22.0f);
        auto imageArea = inner.removeFromTop(cardH * 0.62f).reduced(8.0f, 0.0f);

        if (image.isValid()) {
            g.drawImageWithin(image, static_cast<int>(imageArea.getX()), static_cast<int>(imageArea.getY()),
                              static_cast<int>(imageArea.getWidth()), static_cast<int>(imageArea.getHeight()),
                              juce::RectanglePlacement(juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize), false);
        } else {
            g.setColour(panel2.withAlpha(0.70f));
            g.fillRoundedRectangle(imageArea, 16.0f);
            g.setColour(textColour.withAlpha(0.92f));
            g.setFont(juce::Font(juce::FontOptions(24.0f, juce::Font::bold)));
            g.drawText("Loading...", imageArea.toNearestInt(), juce::Justification::centred, false);
        }

        inner.removeFromTop(8.0f);
        g.setColour(textColour.withAlpha(0.96f));
        g.setFont(juce::Font(juce::FontOptions(18.0f, juce::Font::bold)));
        g.drawText(message, inner.removeFromTop(30.0f).toNearestInt(), juce::Justification::centred, false);

        g.setColour(mutedTextColour.withAlpha(0.88f));
        g.setFont(13.0f);
        g.drawText(juce::String::fromUTF8("작업이 끝날 때까지 잠시만 기다려 주세요."), inner.removeFromTop(22.0f).toNearestInt(), juce::Justification::centred, false);

        inner.removeFromTop(10.0f);
        auto bar = inner.removeFromTop(7.0f).withSizeKeepingCentre(300.0f, 7.0f);
        g.setColour(panel2.withAlpha(0.82f));
        g.fillRoundedRectangle(bar, 3.5f);

        const double now = juce::Time::getMillisecondCounterHiRes() * 0.001;
        const float phase = static_cast<float>(std::fmod(now * 0.55, 1.0));
        const float fillW = bar.getWidth() * (0.35f + 0.45f * phase);
        g.setColour(accent2.withAlpha(0.88f));
        g.fillRoundedRectangle(bar.withWidth(fillW), 3.5f);
    }

    // --------------------------------------------------------
    // ThemedMessageContent 구현
    // --------------------------------------------------------
    ThemedMessageContent::ThemedMessageContent(const juce::String& titleText, const juce::String& bodyText, const juce::String& okText, const juce::String& iconText,
                                               juce::Colour panelColour, juce::Colour panel2Colour, juce::Colour accentColour, juce::Colour accent2Colour,
                                               juce::Colour textColour, juce::Colour mutedTextColour, juce::Colour buttonTextColour)
        : title(titleText), body(bodyText), icon(iconText), panel(panelColour), panel2(panel2Colour),
          accent(accentColour), accent2(accent2Colour), text(textColour), mutedText(mutedTextColour), buttonText(buttonTextColour)
    {
        addAndMakeVisible(okButton);
        okButton.setButtonText(okText);
        okButton.setColour(juce::TextButton::buttonColourId, accent);
        okButton.setColour(juce::TextButton::buttonOnColourId, accent2);
        okButton.setColour(juce::TextButton::textColourOffId, buttonText);
        okButton.setColour(juce::TextButton::textColourOnId, getReadableTextColour(accent2));
        okButton.onClick = [this] { if (auto* dialog = findParentComponentOfClass<juce::DialogWindow>()) dialog->exitModalState(1); };

        juce::StringArray bodyLines; bodyLines.addLines(body);
        const int wrappedExtraLines = body.length() / 42;
        const int estimatedLines = juce::jlimit(2, 8, bodyLines.size() + wrappedExtraLines);
        setSize(560, juce::jlimit(245, 390, 170 + estimatedLines * 28));
    }

    void ThemedMessageContent::paint(juce::Graphics& g) {
        auto area = getLocalBounds().toFloat();
        g.setColour(panel); g.fillRoundedRectangle(area.reduced(1.0f), 8.0f);
        g.setColour(accent.withAlpha(0.95f)); g.drawRoundedRectangle(area.reduced(1.0f), 8.0f, 1.4f);

        auto content = getLocalBounds().reduced(30, 24);
        auto titleArea = content.removeFromTop(62);
        auto iconArea = titleArea.removeFromLeft(58).reduced(4);
        
        g.setColour(accent.withAlpha(0.18f)); g.fillEllipse(iconArea.toFloat());
        g.setColour(accent); g.setFont(juce::Font(juce::FontOptions(32.0f, juce::Font::bold)));
        g.drawText(icon, iconArea, juce::Justification::centred, false);

        titleArea.removeFromLeft(14);
        g.setColour(text); g.setFont(juce::Font(juce::FontOptions(18.0f, juce::Font::bold)));
        g.drawFittedText(title, titleArea, juce::Justification::centredLeft, 2, 0.86f);

        content.removeFromTop(6);
        auto bodyArea = content; bodyArea.removeFromBottom(62);
        g.setColour(panel2.withAlpha(0.35f)); g.fillRoundedRectangle(bodyArea.toFloat(), 6.0f);
        g.setColour(text.withAlpha(0.95f)); g.setFont(15.5f);
        g.drawFittedText(body, bodyArea.reduced(18, 14), juce::Justification::centred, 8, 0.82f);
    }

    void ThemedMessageContent::resized() {
        okButton.setBounds(getWidth() / 2 - 48, getHeight() - 58, 96, 38);
    }

    // --------------------------------------------------------
    // ThemeDialogContent 구현
    // --------------------------------------------------------
    ThemeDialogContent::ThemeDialogContent(const juce::String& titleText, const juce::String& descriptionText,
                                           const juce::String& themeLabelText, const juce::String& applyText, const juce::String& cancelText,
                                           const juce::StringArray& themeItems, int selectedThemeId)
        : title(titleText), description(descriptionText), themeLabel(themeLabelText)
    {
        addAndMakeVisible(themeCombo);
        themeCombo.addItemList(themeItems, 1);
        themeCombo.setSelectedId(juce::jlimit(1, themeItems.size(), selectedThemeId), juce::dontSendNotification);

        addAndMakeVisible(applyButton); applyButton.setButtonText(applyText);
        addAndMakeVisible(cancelButton); cancelButton.setButtonText(cancelText);

        setSize(460, 270);
    }

    void ThemeDialogContent::applyDialogTheme(juce::Colour panelColour, juce::Colour panel2Colour, juce::Colour accentColour, juce::Colour accent2Colour, juce::Colour buttonColour) {
        panel = panelColour; panel2 = panel2Colour; accent = accentColour; accent2 = accent2Colour; button = buttonColour;
        text = getReadableTextColour(panel); subText = text.withAlpha(0.78f); comboText = getReadableTextColour(panel2);

        themeCombo.setColour(juce::ComboBox::backgroundColourId, panel2);
        themeCombo.setColour(juce::ComboBox::textColourId, comboText);
        themeCombo.setColour(juce::ComboBox::outlineColourId, accent.withAlpha(0.85f));
        themeCombo.setColour(juce::ComboBox::buttonColourId, button.brighter(0.12f));
        themeCombo.setColour(juce::ComboBox::arrowColourId, accent);
        themeCombo.setColour(juce::PopupMenu::backgroundColourId, panel2);
        themeCombo.setColour(juce::PopupMenu::textColourId, comboText);
        themeCombo.setColour(juce::PopupMenu::highlightedBackgroundColourId, accent);
        themeCombo.setColour(juce::PopupMenu::highlightedTextColourId, getReadableTextColour(accent));

        styleButton(applyButton, accent, accent2);
        styleButton(cancelButton, button, accent2);

        repaint(); themeCombo.repaint(); applyButton.repaint(); cancelButton.repaint();
    }

    void ThemeDialogContent::paint(juce::Graphics& g) {
        auto area = getLocalBounds().toFloat();
        g.setColour(panel); g.fillRoundedRectangle(area.reduced(1.0f), 10.0f);
        g.setColour(accent.withAlpha(0.95f)); g.drawRoundedRectangle(area.reduced(1.0f), 10.0f, 1.35f);

        auto content = getLocalBounds().reduced(34, 26);
        auto titleArea = content.removeFromTop(34);
        g.setColour(text); g.setFont(juce::Font(juce::FontOptions(18.0f, juce::Font::bold)));
        g.drawText(title, titleArea, juce::Justification::centred, false);

        auto descArea = content.removeFromTop(76).reduced(8, 4);
        g.setColour(panel2.withAlpha(0.42f)); g.fillRoundedRectangle(descArea.toFloat(), 7.0f);
        g.setColour(subText); g.setFont(14.0f);
        g.drawFittedText(description, descArea.reduced(18, 10), juce::Justification::centred, 3, 0.90f);

        auto rowArea = content.removeFromTop(58);
        auto labelArea = rowArea.removeFromTop(22);
        g.setColour(text); g.setFont(13.5f);
        g.drawText(themeLabel, labelArea, juce::Justification::centredLeft, false);
    }

    void ThemeDialogContent::resized() {
        auto content = getLocalBounds().reduced(34, 26);
        content.removeFromTop(34); content.removeFromTop(76);
        auto rowArea = content.removeFromTop(58); rowArea.removeFromTop(22);
        themeCombo.setBounds(rowArea.removeFromTop(30));

        content.removeFromTop(16);
        auto buttonRow = content.removeFromTop(42).withSizeKeepingCentre(190, 42);
        applyButton.setBounds(buttonRow.removeFromLeft(84));
        buttonRow.removeFromLeft(22);
        cancelButton.setBounds(buttonRow.removeFromLeft(84));
    }

    void ThemeDialogContent::styleButton(juce::TextButton& target, juce::Colour base, juce::Colour onColour) {
        target.setColour(juce::TextButton::buttonColourId, base);
        target.setColour(juce::TextButton::buttonOnColourId, onColour);
        target.setColour(juce::TextButton::textColourOffId, getReadableTextColour(base));
        target.setColour(juce::TextButton::textColourOnId, getReadableTextColour(onColour));
    }
}