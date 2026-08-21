#pragma once
#include <JuceHeader.h>

namespace CustomUI
{
    // --------------------------------------------------------
    // 테마 및 색상 유틸리티
    // --------------------------------------------------------
    juce::Colour getThemeColour(int themeId, const juce::String& role);
    juce::Colour getTrackThemeColour(int themeId, int trackIdx);
    juce::Colour getReadableTextColour(juce::Colour background);
    juce::Colour getBankColor(int index);

    // --------------------------------------------------------
    // 로딩 오버레이 컴포넌트 (WorkLoadingOverlayComponent)
    // --------------------------------------------------------
    class WorkLoadingOverlayComponent final : public juce::Component
    {
    public:
        WorkLoadingOverlayComponent();
        void setVisuals(juce::Image newImage, juce::String newText,
                        juce::Colour newBackgroundTop, juce::Colour newBackgroundBottom,
                        juce::Colour newPanel, juce::Colour newPanel2,
                        juce::Colour newAccent, juce::Colour newAccent2,
                        juce::Colour newTextColour, juce::Colour newMutedTextColour);

        void paint(juce::Graphics& g) override;
        
        void mouseDown(const juce::MouseEvent&) override {}
        void mouseDrag(const juce::MouseEvent&) override {}
        void mouseUp(const juce::MouseEvent&) override {}
        void mouseWheelMove(const juce::MouseEvent&, const juce::MouseWheelDetails&) override {}
        bool keyPressed(const juce::KeyPress&) override { return true; }

    private:
        juce::Image image;
        juce::String message = "Working...";
        juce::Colour backgroundTop = juce::Colours::black;
        juce::Colour backgroundBottom = juce::Colours::black;
        juce::Colour panel = juce::Colour(0xff202020);
        juce::Colour panel2 = juce::Colour(0xff101010);
        juce::Colour accent = juce::Colours::gold;
        juce::Colour accent2 = juce::Colours::orange;
        juce::Colour textColour = juce::Colours::white;
        juce::Colour mutedTextColour = juce::Colours::grey;
    };

    // --------------------------------------------------------
    // 테마가 적용된 커스텀 메시지 박스 (ThemedMessageContent)
    // --------------------------------------------------------
    class ThemedMessageContent final : public juce::Component
    {
    public:
        ThemedMessageContent(const juce::String& titleText, const juce::String& bodyText, const juce::String& okText, const juce::String& iconText,
                             juce::Colour panelColour, juce::Colour panel2Colour, juce::Colour accentColour, juce::Colour accent2Colour,
                             juce::Colour textColour, juce::Colour mutedTextColour, juce::Colour buttonTextColour);
        void paint(juce::Graphics& g) override;
        void resized() override;

    private:
        juce::String title;
        juce::String body;
        juce::String icon;
        juce::Colour panel, panel2, accent, accent2, text, mutedText, buttonText;
        juce::TextButton okButton;
    };

    // --------------------------------------------------------
    // 테마 설정 다이얼로그 컴포넌트 (ThemeDialogContent)
    // --------------------------------------------------------
    class ThemeDialogContent final : public juce::Component
    {
    public:
        ThemeDialogContent(const juce::String& titleText, const juce::String& descriptionText,
                           const juce::String& themeLabelText, const juce::String& applyText, const juce::String& cancelText,
                           const juce::StringArray& themeItems, int selectedThemeId);

        int getSelectedThemeId() const { return themeCombo.getSelectedId(); }
        void applyDialogTheme(juce::Colour panelColour, juce::Colour panel2Colour, juce::Colour accentColour, juce::Colour accent2Colour, juce::Colour buttonColour);
        
        void paint(juce::Graphics& g) override;
        void resized() override;

        // MainComponent에서 이벤트를 연결할 수 있도록 public으로 노출
        juce::ComboBox themeCombo;
        juce::TextButton applyButton;
        juce::TextButton cancelButton;

    private:
        void styleButton(juce::TextButton& target, juce::Colour base, juce::Colour onColour);

        juce::String title, description, themeLabel;
        juce::Colour panel{ juce::Colours::white }, panel2{ juce::Colours::lightgrey };
        juce::Colour accent{ juce::Colours::cornflowerblue }, accent2{ juce::Colours::orange }, button{ juce::Colours::grey };
        juce::Colour text{ juce::Colours::black }, subText{ juce::Colours::darkgrey }, comboText{ juce::Colours::black };
    };
}