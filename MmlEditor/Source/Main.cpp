#include <JuceHeader.h>
#include "MainComponent.h"

class MmlEditorApplication : public juce::JUCEApplication
{
public:
    MmlEditorApplication() {}
    const juce::String getApplicationName() override { return ProjectInfo::projectName; }
    const juce::String getApplicationVersion() override { return ProjectInfo::versionString; }
    bool moreThanOneInstanceAllowed() override { return true; }

    void initialise(const juce::String& commandLine) override
    {
        mainWindow.reset(new MainWindow(getApplicationName()));
    }

    void shutdown() override { mainWindow = nullptr; }
    void systemRequestedQuit() override
    {
        if (mainWindow != nullptr)
        {
            if (auto* main = dynamic_cast<MainComponent*>(mainWindow->getContentComponent()))
            {
                main->requestCloseWithSavePrompt();
                return;
            }
        }

        quit();
    }
    void anotherInstanceStarted(const juce::String& commandLine) override {}

    class MainWindow : public juce::DocumentWindow
    {
    public:
        MainWindow(juce::String name)
            : DocumentWindow(name,
                juce::Desktop::getInstance().getDefaultLookAndFeel()
                .findColour(juce::ResizableWindow::backgroundColourId),
                DocumentWindow::allButtons)
        {
            setUsingNativeTitleBar(true);
            setWindowIconFromAssets();
            setContentOwned(new MainComponent(), true);

#if JUCE_IOS || JUCE_ANDROID
            setFullScreen(true);
#else
            setResizable(true, true);
            centreWithSize(getWidth(), getHeight());
#endif

            setVisible(true);
        }
        void closeButtonPressed() override
        {
            if (auto* main = dynamic_cast<MainComponent*>(getContentComponent()))
            {
                main->requestCloseWithSavePrompt();
                return;
            }

            juce::JUCEApplication::getInstance()->systemRequestedQuit();
        }
    private:
        void setWindowIconFromAssets()
        {
            juce::Array<juce::File> iconCandidates;
            const auto exeFolder = juce::File::getSpecialLocation(juce::File::currentExecutableFile).getParentDirectory();

            iconCandidates.add(exeFolder.getChildFile("Assets").getChildFile("app_icon.png"));
            iconCandidates.add(exeFolder.getChildFile("app_icon.png"));
            iconCandidates.add(juce::File::getCurrentWorkingDirectory().getChildFile("Assets").getChildFile("app_icon.png"));
            iconCandidates.add(juce::File::getCurrentWorkingDirectory().getChildFile("app_icon.png"));

            // Backward-compatible fallback.
            iconCandidates.add(exeFolder.getChildFile("Assets").getChildFile("Atelier de Derstin.png"));
            iconCandidates.add(exeFolder.getChildFile("Atelier de Derstin.png"));
            iconCandidates.add(juce::File::getCurrentWorkingDirectory().getChildFile("Assets").getChildFile("Atelier de Derstin.png"));
            iconCandidates.add(juce::File::getCurrentWorkingDirectory().getChildFile("Atelier de Derstin.png"));

            for (const auto& candidate : iconCandidates)
            {
                if (candidate.existsAsFile())
                {
                    const auto icon = juce::ImageFileFormat::loadFrom(candidate);
                    if (icon.isValid())
                    {
                        setIcon(icon);
                        return;
                    }
                }
            }
        }



        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
    };

private:
    std::unique_ptr<MainWindow> mainWindow;
};

START_JUCE_APPLICATION(MmlEditorApplication)