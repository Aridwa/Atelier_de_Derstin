#pragma once
#include <JuceHeader.h>
#include <functional>
#include <cmath>
#include <algorithm>
#include <vector>
#include <utility>
#include <limits>

#include "../MmlData/MmlData.h"
#include "../MmlData/MmlLogic.h"
#include "../CustomUI/CustomUI.h"

class PianoRollComponent : public juce::Component
{
public:
    PianoRollComponent()
    {
        setWantsKeyboardFocus(true);
    }

    static constexpr int minMidi = 12;
    static constexpr int maxMidi = 108;
    static constexpr double defaultPixelsPerBeat = 50.0;

    std::function<bool(int, int)> isPartActiveForBank;
    std::function<bool(int)> isPartEditable;
    std::function<void(double)> onSeekBeat;
    std::function<void(double)> onTimelineTempoRequested;             // mabbiico timeline right-click tempo
    std::function<void(int, int, int, int)> onNoteDoubleClicked; // part, noteIndex, textStart, textEnd
    std::function<void(int, int, int, double)> onNoteEdited;     // part, noteIndex, midi, lengthBeats
    std::function<void(int, int, double, double)> onNoteInserted; // part, midi, startBeat, lengthBeats
    std::function<void(int, int)> onNoteSelected;                // part, noteIndex
    std::function<void()> onDeleteSelectedNote;                   // Del/Backspace
    std::function<void(const std::vector<std::pair<int, int>>&)> onNotesSelected; // multi-select changed
    std::function<void()> onCopySelectedNotes;                    // Ctrl+C
    std::function<void()> onPasteCopiedNotes;                     // Ctrl+V
    std::function<void()> onDeleteSelectedNotes;                  // Del/Backspace with multi-select
    std::function<bool(int, double)> onSelectedNotesMove;          // selected notes: absolute move from mouse-down; returns true when movement was applied
    std::function<void()> onMoveSnapshotRequested;                  // capture exact current notes before an object-move drag
    std::function<void()> onUndoRequested;                         // Ctrl+Z
    std::function<void()> onEditGestureStarted;                    // snapshot before drag/insert gesture
    std::function<void()> onEditGestureFinished;                   // end undo transaction
    std::function<void(double)> onHorizontalScrollRequested;
    std::function<void(double)> onVerticalScrollRequested;
    std::function<void(float, float)> onTimelineZoomRequested;             // Ctrl+wheel horizontal timeline zoom in mabbiico mode
    std::function<void(double, double)> onHorizontalRangeChanged;
    std::function<void(int)> onPreviewMidiChanged;                 // -1 = stop preview

    void setModel(InstrumentBank* bankArray,
                  int* activeBankCount,
                  int* selectedBankIndex,
                  const std::vector<EventItem>* events,
                  const std::vector<TempoChange>* tempos)
    {
        banks = bankArray;
        numActiveTracks = activeBankCount;
        currentBankIndex = selectedBankIndex;
        cachedEventList = events;
        tempoMap = tempos;
        repaint();
    }

    void setMeterChanges(const std::vector<MeterChange>* changes)
    {
        meterChanges = changes;
        repaint();
    }

    void setTheme(int newThemeId, int newLanguageId, int newBeatsPerMeasure)
    {
        themeId = juce::jlimit(1, 12, newThemeId);
        languageId = (newLanguageId == 1 || newLanguageId == 2) ? newLanguageId : 2;
        beatsPerMeasure = newBeatsPerMeasure > 0 ? newBeatsPerMeasure : 4;
        repaint();
    }

    void setScroll(double newScrollX, double newScrollY, float newRowHeight)
    {
        scrollX = juce::jmax(0.0, newScrollX);
        scrollY = juce::jmax(0.0, newScrollY);
        rowHeight = juce::jmax(6.0f, newRowHeight);
        repaint();
    }

    void setTimelineZoom(double newPixelsPerBeat)
    {
        pixelsPerBeat = juce::jlimit(18.0, 260.0, newPixelsPerBeat);
        repaint();
    }

    double getPixelsPerBeat() const
    {
        return pixelsPerBeat;
    }

    void setTransport(double newSampleRate, const int64_t* sampleCounterPtr, const bool* playingPtr)
    {
        sampleRate = newSampleRate;
        globalSampleCount = sampleCounterPtr;
        isPlaying = playingPtr;
        repaint();
    }

    void setResizeQuantizeStepBeats(double stepBeats)
    {
        resizeQuantizeStepBeats = juce::jlimit(0.0625, 4.0, stepBeats);
    }

    void setEventListVisible(bool shouldShowEventList)
    {
        if (showEventList == shouldShowEventList)
            return;

        showEventList = shouldShowEventList;
        repaint();
    }

    bool isEventListVisible() const
    {
        return showEventList;
    }

    juce::Rectangle<int> getRollAreaBounds() const
    {
        return getLayout().rollArea;
    }

    void setSeekOnlyFromTimeline(bool shouldSeekOnlyFromTimeline)
    {
        seekOnlyFromTimeline = shouldSeekOnlyFromTimeline;
    }

    void setSelectedNote(int partIdx, int noteIdx)
    {
        selectedPartIdx = partIdx;
        selectedNoteIdx = noteIdx;
        selectedNotes.clear();
        if (partIdx >= 0 && noteIdx >= 0)
            selectedNotes.push_back({ partIdx, noteIdx });
        repaint();
    }

    void setSelectedNotes(const std::vector<std::pair<int, int>>& notes)
    {
        selectedNotes = notes;
        if (selectedNotes.size() == 1)
        {
            selectedPartIdx = selectedNotes.front().first;
            selectedNoteIdx = selectedNotes.front().second;
        }
        else
        {
            selectedPartIdx = selectedNotes.empty() ? -1 : selectedNotes.front().first;
            selectedNoteIdx = -1;
        }
        repaint();
    }

    double getTotalWidthPixels() const
    {
        double maxBeat = 16.0;
        if (banks != nullptr && numActiveTracks != nullptr)
        {
            for (int i = 0; i < *numActiveTracks; ++i)
            {
                for (int j = 0; j < 4; ++j)
                {
                    if (isPartActiveForBank && !isPartActiveForBank(i, j))
                        continue;
                    if (!banks[i].tracks[j].sequence.empty())
                        maxBeat = std::max(maxBeat, banks[i].tracks[j].sequence.back().endBeat);
                }
            }
        }
        return maxBeat * pixelsPerBeat + 400.0;
    }

    void paint(juce::Graphics& g) override
    {
        const auto backgroundTop = CustomUI::getThemeColour(themeId, "backgroundTop");
        const auto backgroundBottom = CustomUI::getThemeColour(themeId, "backgroundBottom");
        juce::ColourGradient bgGradient(backgroundTop, 0.0f, 0.0f, backgroundBottom, 0.0f, static_cast<float>(getHeight()), false);
        g.setGradientFill(bgGradient);
        g.fillRect(getLocalBounds().toFloat());

        if (banks == nullptr || numActiveTracks == nullptr || currentBankIndex == nullptr)
            return;

        auto layout = getLayout();
        if (showEventList)
            drawEventList(g, layout.leftPanel, layout.leftTimeline);
        drawPianoAndGrid(g, layout.pianoArea, layout.rollArea, layout.pianoTimeline, layout.gridTimeline);
        drawNotesAndPlayhead(g, layout.rollArea, layout.gridTimeline);

        if (lassoState.active || pendingGridSelection)
            drawLassoSelection(g);

        if (dragState.active)
            drawDragHint(g, layout.rollArea);
    }

    void mouseDown(const juce::MouseEvent& e) override
    {
        grabKeyboardFocus();

        if (banks == nullptr || currentBankIndex == nullptr)
            return;

        const auto layout = getLayout();

        seekDragActive = false;
        if (seekOnlyFromTimeline && layout.gridTimeline.contains(e.getPosition()))
        {
            const double timelineBeat = pointToSeekBeat(e.position.x, layout.rollArea);

            if (e.mods.isRightButtonDown())
            {
                if (onTimelineTempoRequested)
                    onTimelineTempoRequested(timelineBeat);
                return;
            }

            seekDragActive = true;
            if (onSeekBeat)
                onSeekBeat(timelineBeat);
            return;
        }

        if (!layout.rollArea.contains(e.getPosition()))
            return;

        if (e.getNumberOfClicks() >= 2)
        {
            auto hit = hitTestCurrentBankNote(e.getPosition(), false);
            if (hit.valid && onNoteDoubleClicked)
            {
                const auto& note = banks[*currentBankIndex].tracks[hit.partIdx].sequence[static_cast<size_t>(hit.noteIdx)];
                onNoteDoubleClicked(hit.partIdx, hit.noteIdx, note.textStart, note.textEnd);
                return;
            }

            // mabbiico 스타일 직접 찍기: 빈 그리드를 더블클릭하면 현재 선택 파트에
            // 선택된 음표 길이만큼 새 노트를 만든다.
            if (!hit.valid && onNoteInserted)
            {
                const int editablePart = getFirstEditablePart();
                if (editablePart >= 0)
                {
                    const double step = juce::jlimit(0.0625, 4.0, resizeQuantizeStepBeats);
                    const double rawBeat = pointToBeat(e.position.x, layout.rollArea);
                    const double startBeat = juce::jmax(0.0, std::floor(rawBeat / step) * step);
                    const int midi = juce::jlimit(minMidi, maxMidi, pointToMidi(e.position.y, layout.rollArea));

                    selectedPartIdx = editablePart;
                    selectedNoteIdx = -1;
                    if (onNoteSelected)
                        onNoteSelected(editablePart, -1);

                    beginEditGestureIfNeeded();
                    onNoteInserted(editablePart, midi, startBeat, step);
                    beginDragForInsertedNote(editablePart, midi, startBeat, step);
                    return;
                }
            }
        }

        auto hit = hitTestCurrentBankNote(e.getPosition(), true);

        // mabbiico 편집 감각 정리:
        // - 왼쪽 마우스 빈 공간: 즉시 노트를 생성하고, 버튼을 누른 상태로 드래그하면
        //   생성된 노트의 음높이 + 위치가 바로 바뀐다.
        // - 오른쪽 마우스 빈 공간: 노트 생성 없이 선택 박스 전용.
        if (!hit.valid && e.mods.isLeftButtonDown() && onNoteInserted)
        {
            const int editablePart = getFirstEditablePart();
            if (editablePart >= 0)
            {
                const double step = juce::jlimit(0.0625, 4.0, resizeQuantizeStepBeats);
                const double rawBeat = pointToBeat(e.position.x, layout.rollArea);
                const double startBeat = juce::jmax(0.0, std::floor(rawBeat / step) * step);
                const int midi = juce::jlimit(minMidi, maxMidi, pointToMidi(e.position.y, layout.rollArea));

                selectedPartIdx = editablePart;
                selectedNoteIdx = -1;
                selectedNotes.clear();
                if (onNoteSelected)
                    onNoteSelected(editablePart, -1);
                if (onNotesSelected)
                    onNotesSelected(selectedNotes);

                beginEditGestureIfNeeded();
                onNoteInserted(editablePart, midi, startBeat, step);
                beginDragForInsertedNote(editablePart, midi, startBeat, step);
                return;
            }
        }

        if (!hit.valid && e.mods.isRightButtonDown())
        {
            const int editablePart = getFirstEditablePart();
            if (editablePart >= 0)
            {
                pendingGridSelection = true;
                pendingInsertPartIdx = -1;
                pendingInsertCancelled = false;
                pendingInsertMouseDown = e.getPosition();
                lassoState.start = e.getPosition();
                lassoState.current = e.getPosition();
                repaint();
                return;
            }
        }

        if (hit.valid && isEditable(hit.partIdx))
        {
            // 오른쪽 마우스로 노트를 클릭하면 범위 선택 메뉴를 띄운다.
            // 빈 공간 오른쪽 드래그는 기존처럼 선택 박스 전용이다.
            if (e.mods.isRightButtonDown())
            {
                showNoteRangeSelectMenu(hit.partIdx, hit.noteIdx, e);
                return;
            }

            const bool leftButtonEdit = e.mods.isLeftButtonDown();
            const bool multiSelectedNoteDrag = leftButtonEdit
                                            && selectedNotes.size() > 1
                                            && isSelectedNote(hit.partIdx, hit.noteIdx);

            if (!multiSelectedNoteDrag)
            {
                selectedPartIdx = hit.partIdx;
                selectedNoteIdx = hit.noteIdx;
                selectedNotes.clear();
                selectedNotes.push_back({ hit.partIdx, hit.noteIdx });
                if (onNoteSelected)
                    onNoteSelected(hit.partIdx, hit.noteIdx);
                if (onNotesSelected)
                    onNotesSelected(selectedNotes);
            }

            if (!leftButtonEdit)
            {
                repaint();
                return;
            }

            dragState.active = true;
            // 왼쪽 노트 본문 드래그: 위/아래는 음높이 이동, 좌/우는 현재 음표 길이 단위로 위치 이동.
            // 더 이상 노트 본문 드래그로 길이를 늘리거나 줄이지 않는다.
            // 노트 오른쪽 끝 드래그는 기존처럼 길이 조절만 담당한다.
            // 여러 노트가 선택된 상태에서도 같은 규칙으로 묶음 전체가 이동하며, 각 노트의 길이는 유지된다.
            const bool bodyMoveDrag = !hit.onRightEdge;
            dragState.groupPitchOnly = bodyMoveDrag;
            dragState.resizing = hit.onRightEdge;
            dragState.pitchAndLength = false;
            dragState.partIdx = hit.partIdx;
            dragState.noteIdx = hit.noteIdx;
            dragState.startMidi = hit.midiNote;
            dragState.startLengthBeats = hit.endBeat - hit.startBeat;
            dragState.noteStartBeat = hit.startBeat;
            dragState.dragStartBeat = pointToBeatClampedToRollArea(e.position.x, layout.rollArea);
            dragState.lastSelectedMoveBeatDelta = 0.0;
            dragState.lastAppliedSemitoneDelta = 0;
            dragState.lastAppliedBeatDelta = 0.0;
            dragState.previewSemitoneDelta = 0;
            dragState.previewBeatDelta = 0.0;
            dragState.minStartBeatAtDragStart = getCurrentSelectionMinStartBeat();
            dragState.moveSnapshotPrepared = false;
            dragState.lastMidi = hit.midiNote;
            dragState.lastLengthBeats = hit.endBeat - hit.startBeat;

            // 노트 이동/길이 조절을 시작할 때는 현재 음을 1번만 미리듣기한다.
            // 특히 오른쪽 끝을 잡고 길이를 늘리거나 줄일 때, 드래그 중에는 반복 재생하지 않는다.
            setActivePreviewMidi(hit.midiNote);

            setMouseCursor(hit.onRightEdge ? juce::MouseCursor::LeftRightResizeCursor
                                           : juce::MouseCursor::DraggingHandCursor);
            repaint();
            return;
        }

        clearSelectedNotesAndNotify();

        if (!seekOnlyFromTimeline && onSeekBeat)
            onSeekBeat(pointToSeekBeat(e.position.x, layout.rollArea));
    }

    void mouseDrag(const juce::MouseEvent& e) override
    {
        const auto layout = getLayout();

        if (seekDragActive)
        {
            if (onSeekBeat)
                onSeekBeat(pointToSeekBeat(e.position.x, layout.rollArea));
            return;
        }

        if (pendingGridSelection)
        {
            const int dragDistance = e.getDistanceFromDragStart();
            if (dragDistance >= 5)
            {
                // 오른쪽 버튼 드래그만 선택 박스를 만든다.
                lassoState.active = true;
                lassoState.current = e.getPosition();
                updateLassoSelection();
                repaint();
            }
            return;
        }

        if (!layout.rollArea.intersects(juce::Rectangle<int>(e.x - 2, e.y - 2, 4, 4)) && !dragState.active)
            return;

        if (!dragState.active)
        {
            if (!seekOnlyFromTimeline && onSeekBeat)
                onSeekBeat(pointToSeekBeat(e.position.x, layout.rollArea));
            return;
        }

        if (!onNoteEdited || !isEditable(dragState.partIdx))
            return;

        if (dragState.groupPitchOnly)
        {
            const int newMidi = juce::jlimit(minMidi, maxMidi, pointToMidi(e.position.y, layout.rollArea));
            const int semitoneDelta = newMidi - dragState.startMidi;
            const double rawBeatDelta = pointToBeatClampedToRollArea(e.position.x, layout.rollArea) - dragState.dragStartBeat;
            const double beatDelta = clampDragBeatDeltaToZero(quantizeBeatDelta(rawBeatDelta, resizeQuantizeStepBeats));

            // 왼쪽 마우스 노트 이동은 드래그 중 실제 MML/노트 데이터를 건드리지 않는다.
            // 여기서는 화면 미리보기용 delta만 저장하고, 마우스를 뗄 때 한 번만 최종 적용한다.
            dragState.previewSemitoneDelta = semitoneDelta;
            dragState.previewBeatDelta = beatDelta;
            dragState.lastMidi = newMidi;

            // 좌/우 이동만 할 때는 같은 음높이 미리듣기를 계속 재생하지 않는다.
            // 마우스 다운 시 1회 재생하고, 드래그 중에는 음높이가 실제로 바뀔 때만 다시 재생한다.
            if (newMidi != activePreviewMidi)
                setActivePreviewMidi(newMidi);

            repaint();
            return;
        }

        // 노트 오른쪽 끝 길이 조절도 실시간으로 MML을 바꾸지 않는다.
        // 드래그 중에는 화면 미리보기용 길이만 저장하고, 마우스를 뗄 때 한 번만 적용한다.
        if (dragState.pitchAndLength)
        {
            const int newMidi = juce::jlimit(minMidi, maxMidi, pointToMidi(e.position.y, layout.rollArea));
            const double endBeat = pointToBeatClampedToRollArea(e.position.x, layout.rollArea);
            const double newLength = quantizeBeats(juce::jmax(0.0625, endBeat - dragState.noteStartBeat), resizeQuantizeStepBeats);

            dragState.lastMidi = newMidi;
            dragState.lastLengthBeats = newLength;
            if (newMidi != activePreviewMidi)
                setActivePreviewMidi(newMidi);
        }
        else if (dragState.resizing)
        {
            const double endBeat = pointToBeatClampedToRollArea(e.position.x, layout.rollArea);
            const double newLength = quantizeBeats(juce::jmax(0.0625, endBeat - dragState.noteStartBeat), resizeQuantizeStepBeats);
            dragState.lastLengthBeats = newLength;
        }
        else
        {
            const int newMidi = juce::jlimit(minMidi, maxMidi, pointToMidi(e.position.y, layout.rollArea));
            dragState.lastMidi = newMidi;
            if (newMidi != activePreviewMidi)
                setActivePreviewMidi(newMidi);
        }
        repaint();
    }

    void mouseUp(const juce::MouseEvent& e) override
    {
        if (pendingGridSelection)
        {
            pendingGridSelection = false;
            pendingInsertPartIdx = -1;
            pendingInsertCancelled = false;
            lassoState = LassoState{};
        }

        seekDragActive = false;

        if (dragState.active && dragState.groupPitchOnly)
        {
            const bool hasMove = (dragState.previewSemitoneDelta != 0)
                              || (std::abs(dragState.previewBeatDelta) > 0.0001);
            if (hasMove && onSelectedNotesMove)
            {
                beginEditGestureIfNeeded();
                if (!dragState.moveSnapshotPrepared && onMoveSnapshotRequested)
                    onMoveSnapshotRequested();
                dragState.moveSnapshotPrepared = true;
                onSelectedNotesMove(dragState.previewSemitoneDelta, dragState.previewBeatDelta);
            }
        }
        else if (dragState.active && (dragState.resizing || dragState.pitchAndLength))
        {
            const bool changed = dragState.lastMidi != dragState.startMidi
                              || std::abs(dragState.lastLengthBeats - dragState.startLengthBeats) > 0.0001;
            if (changed && onNoteEdited && isEditable(dragState.partIdx))
            {
                beginEditGestureIfNeeded();
                onNoteEdited(dragState.partIdx, dragState.noteIdx, dragState.lastMidi, dragState.lastLengthBeats);
            }
        }

        clearActivePreviewMidi();
        finishEditGestureIfNeeded();
        dragState = DragState{};
        setMouseCursor(juce::MouseCursor::NormalCursor);
        repaint();
    }

    void mouseMove(const juce::MouseEvent& e) override
    {
        const auto layout = getLayout();
        if (seekOnlyFromTimeline && layout.gridTimeline.contains(e.getPosition()))
        {
            setMouseCursor(juce::MouseCursor::PointingHandCursor);
            return;
        }

        const auto hit = hitTestCurrentBankNote(e.getPosition(), true);
        if (hit.valid && isEditable(hit.partIdx))
            setMouseCursor(hit.onRightEdge ? juce::MouseCursor::LeftRightResizeCursor : juce::MouseCursor::PointingHandCursor);
        else if (layout.rollArea.contains(e.getPosition()) && getFirstEditablePart() >= 0 && onNoteInserted)
            setMouseCursor(juce::MouseCursor::CrosshairCursor);
        else
            setMouseCursor(juce::MouseCursor::NormalCursor);
    }

    bool keyPressed(const juce::KeyPress& key) override
    {
        const int keyCode = key.getKeyCode();
        const juce::juce_wchar ch = juce::CharacterFunctions::toLowerCase(key.getTextCharacter());
        const bool commandDown = key.getModifiers().isCommandDown() || key.getModifiers().isCtrlDown();
        const bool copyShortcut = commandDown && (keyCode == 'c' || keyCode == 'C' || ch == 'c' || ch == 3);
        const bool pasteShortcut = commandDown && (keyCode == 'v' || keyCode == 'V' || ch == 'v' || ch == 22);
        const bool selectAllShortcut = commandDown && (keyCode == 'a' || keyCode == 'A' || ch == 'a' || ch == 1);
        const bool undoShortcut = commandDown && (keyCode == 'z' || keyCode == 'Z' || ch == 'z' || ch == 26);

        if (undoShortcut)
        {
            if (onUndoRequested)
            {
                onUndoRequested();
                return true;
            }
        }

        if (selectAllShortcut)
        {
            selectAllEditableNotes();
            return true;
        }

        if (copyShortcut)
        {
            if (onCopySelectedNotes)
            {
                onCopySelectedNotes();
                return true;
            }
        }

        if (pasteShortcut)
        {
            if (onPasteCopiedNotes)
            {
                onPasteCopiedNotes();
                return true;
            }
        }

        if (key == juce::KeyPress::deleteKey || key == juce::KeyPress::backspaceKey)
        {
            if (!selectedNotes.empty() && onDeleteSelectedNotes)
            {
                onDeleteSelectedNotes();
                return true;
            }

            if (onDeleteSelectedNote)
            {
                onDeleteSelectedNote();
                return true;
            }
        }
        return false;
    }

    void mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel) override
    {
        if (seekOnlyFromTimeline && (event.mods.isCtrlDown() || event.mods.isCommandDown()))
        {
            if (onTimelineZoomRequested)
                onTimelineZoomRequested(event.position.x, wheel.deltaY);
            return;
        }

        if (wheel.deltaX != 0.0f || event.mods.isShiftDown())
        {
            const double delta = (wheel.deltaX != 0.0f) ? wheel.deltaX : wheel.deltaY;
            const double maxScroll = std::max(0.0, getTotalWidthPixels() - static_cast<double>(getLayout().rollArea.getWidth()));
            const double newScroll = juce::jlimit(0.0, maxScroll, scrollX - delta * 300.0);
            if (onHorizontalScrollRequested) onHorizontalScrollRequested(newScroll);
        }
        else
        {
            const double totalHeight = static_cast<double>(maxMidi - minMidi + 1) * rowHeight;
            const double maxScroll = std::max(0.0, totalHeight - static_cast<double>(getLayout().rollArea.getHeight()));
            const double newScroll = juce::jlimit(0.0, maxScroll, scrollY - static_cast<double>(wheel.deltaY) * 300.0);
            if (onVerticalScrollRequested) onVerticalScrollRequested(newScroll);
        }
    }

private:
    void beginEditGestureIfNeeded()
    {
        if (editGestureActive)
            return;
        editGestureActive = true;
        if (onEditGestureStarted)
            onEditGestureStarted();
    }

    void finishEditGestureIfNeeded()
    {
        if (!editGestureActive)
            return;
        editGestureActive = false;
        if (onEditGestureFinished)
            onEditGestureFinished();
    }

    struct Layout
    {
        juce::Rectangle<int> leftPanel;
        juce::Rectangle<int> leftTimeline;
        juce::Rectangle<int> pianoArea;
        juce::Rectangle<int> pianoTimeline;
        juce::Rectangle<int> gridTimeline;
        juce::Rectangle<int> rollArea;
    };

    struct NoteHit
    {
        bool valid = false;
        bool onRightEdge = false;
        int partIdx = -1;
        int noteIdx = -1;
        int midiNote = 0;
        double startBeat = 0.0;
        double endBeat = 0.0;
    };

    struct DragState
    {
        bool active = false;
        bool resizing = false;
        bool pitchAndLength = false;
        bool groupPitchOnly = false;
        int partIdx = -1;
        int noteIdx = -1;
        int startMidi = 0;
        int lastMidi = 0;
        double startLengthBeats = 0.0;
        double lastLengthBeats = 0.0;
        double noteStartBeat = 0.0;
        double dragStartBeat = 0.0;
        double lastSelectedMoveBeatDelta = 0.0;
        int lastAppliedSemitoneDelta = 0;
        double lastAppliedBeatDelta = 0.0;
        int previewSemitoneDelta = 0;
        double previewBeatDelta = 0.0;
        double minStartBeatAtDragStart = 0.0;
        bool moveSnapshotPrepared = false;
    } dragState;

    struct LassoState
    {
        bool active = false;
        juce::Point<int> start;
        juce::Point<int> current;
    } lassoState;

    InstrumentBank* banks = nullptr;
    int* numActiveTracks = nullptr;
    int* currentBankIndex = nullptr;
    const std::vector<EventItem>* cachedEventList = nullptr;
    const std::vector<TempoChange>* tempoMap = nullptr;
    const std::vector<MeterChange>* meterChanges = nullptr;
    const int64_t* globalSampleCount = nullptr;
    const bool* isPlaying = nullptr;

    int themeId = 3;
    int languageId = 2;
    int beatsPerMeasure = 4;
    double sampleRate = 44100.0;
    double scrollX = 0.0;
    double scrollY = 600.0;
    float rowHeight = 12.0f;
    double pixelsPerBeat = defaultPixelsPerBeat;
    bool showEventList = true;
    bool seekOnlyFromTimeline = false;
    bool seekDragActive = false;
    double resizeQuantizeStepBeats = 1.0; // default: quarter note
    int selectedPartIdx = -1;
    int selectedNoteIdx = -1;
    std::vector<std::pair<int, int>> selectedNotes;
    bool editGestureActive = false;
    bool pendingGridSelection = false;
    bool pendingInsertCancelled = false;
    int pendingInsertPartIdx = -1;
    juce::Point<int> pendingInsertMouseDown;
    int activePreviewMidi = -1;

    juce::String T(const juce::String& en, const juce::String& ko) const { return languageId == 2 ? ko : en; }

    bool isEditable(int partIdx) const
    {
        if (partIdx < 0 || partIdx >= 4) return false;
        return !isPartEditable || isPartEditable(partIdx);
    }

    int getFirstEditablePart() const
    {
        for (int part = 0; part < 4; ++part)
            if (isEditable(part))
                return part;
        return -1;
    }

    Layout getLayout() const
    {
        auto bounds = getLocalBounds();
        auto leftPanel = bounds.removeFromLeft(showEventList ? 180 : 0);
        auto rollArea = bounds;
        auto fullPianoArea = rollArea.removeFromLeft(30);
        rollArea.removeFromBottom(15);
        rollArea.removeFromRight(15);
        auto leftTimeline = leftPanel.removeFromTop(20);
        auto pianoArea = fullPianoArea.withTrimmedBottom(15);
        auto pianoTimeline = pianoArea.removeFromTop(20);
        auto gridTimeline = rollArea.removeFromTop(20);
        return { leftPanel, leftTimeline, pianoArea, pianoTimeline, gridTimeline, rollArea };
    }

    double pointToBeat(float x, const juce::Rectangle<int>& rollArea) const
    {
        return juce::jmax(0.0, (scrollX + static_cast<double>(x - rollArea.getX())) / pixelsPerBeat);
    }

    double pointToBeatClampedToRollArea(float x, const juce::Rectangle<int>& rollArea) const
    {
        // During a drag, the mouse can move over the piano keyboard or outside the roll.
        // Do not let that out-of-grid X coordinate create an unstable beat delta.
        const float safeX = juce::jlimit(static_cast<float>(rollArea.getX()),
                                        static_cast<float>(rollArea.getRight()),
                                        x);
        return juce::jmax(0.0, (scrollX + static_cast<double>(safeX - rollArea.getX())) / pixelsPerBeat);
    }

    double pointToSeekBeat(float x, const juce::Rectangle<int>& rollArea) const
    {
        const double rawBeat = pointToBeat(x, rollArea);

        // mabbiico mode: the red playhead line moves on the currently selected
        // note-length grid. Whole=4 beats, Half=2, Quarter=1, ... 64th=1/16.
        if (seekOnlyFromTimeline)
        {
            const double step = juce::jlimit(0.0625, 4.0, resizeQuantizeStepBeats);
            return juce::jmax(0.0, std::round(rawBeat / step) * step);
        }

        // 3mle mode keeps the old free seek behaviour.
        return rawBeat;
    }

    int pointToMidi(float y, const juce::Rectangle<int>& rollArea) const
    {
        return maxMidi - static_cast<int>(std::floor((static_cast<double>(y - rollArea.getY()) + scrollY) / static_cast<double>(rowHeight)));
    }

    static double quantizeBeats(double beatLength, double stepBeats)
    {
        // Note-length grid. 1 beat = quarter note, 4 beats = whole note.
        const double safeStep = juce::jlimit(0.0625, 4.0, stepBeats);
        return juce::jmax(safeStep, std::round(beatLength / safeStep) * safeStep);
    }

    static double quantizeBeatDelta(double beatDelta, double stepBeats)
    {
        const double safeStep = juce::jlimit(0.0625, 4.0, stepBeats);
        return std::round(beatDelta / safeStep) * safeStep;
    }

    double getCurrentSelectionMinStartBeat() const
    {
        if (banks == nullptr || currentBankIndex == nullptr)
            return 0.0;

        const int bankIdx = *currentBankIndex;
        if (bankIdx < 0)
            return 0.0;

        double minStart = std::numeric_limits<double>::max();
        for (const auto& selected : selectedNotes)
        {
            const int partIdx = selected.first;
            const int noteIdx = selected.second;
            if (partIdx < 0 || partIdx >= 4)
                continue;

            const auto& seq = banks[bankIdx].tracks[partIdx].sequence;
            if (noteIdx < 0 || noteIdx >= static_cast<int>(seq.size()))
                continue;

            const auto& note = seq[static_cast<size_t>(noteIdx)];
            if (note.frequency <= 0.0)
                continue;

            minStart = std::min(minStart, note.startBeat);
        }

        return minStart == std::numeric_limits<double>::max() ? 0.0 : minStart;
    }

    double clampDragBeatDeltaToZero(double beatDelta) const
    {
        // Use the selection's mouse-down min start, not the live regenerated sequence.
        // This makes dragging left over the piano keyboard behave like moving one object.
        if (dragState.minStartBeatAtDragStart + beatDelta < 0.0)
            return -dragState.minStartBeatAtDragStart;

        return beatDelta;
    }

    static bool sameSelection(const std::vector<std::pair<int, int>>& a, const std::vector<std::pair<int, int>>& b)
    {
        return a == b;
    }

    void selectAllEditableNotes()
    {
        selectedNotes.clear();

        if (banks != nullptr && currentBankIndex != nullptr)
        {
            const int bankIdx = *currentBankIndex;
            if (bankIdx >= 0)
            {
                for (int part = 0; part < 4; ++part)
                {
                    if (!isEditable(part))
                        continue;
                    if (isPartActiveForBank && !isPartActiveForBank(bankIdx, part))
                        continue;

                    const auto& seq = banks[bankIdx].tracks[part].sequence;
                    for (int i = 0; i < static_cast<int>(seq.size()); ++i)
                    {
                        if (seq[static_cast<size_t>(i)].frequency <= 0.0)
                            continue;
                        selectedNotes.push_back({ part, i });
                    }
                }
            }
        }

        if (selectedNotes.size() == 1)
        {
            selectedPartIdx = selectedNotes.front().first;
            selectedNoteIdx = selectedNotes.front().second;
            if (onNoteSelected)
                onNoteSelected(selectedPartIdx, selectedNoteIdx);
        }
        else
        {
            selectedPartIdx = selectedNotes.empty() ? -1 : selectedNotes.front().first;
            selectedNoteIdx = -1;
            if (onNoteSelected)
                onNoteSelected(selectedPartIdx, -1);
        }

        if (onNotesSelected)
            onNotesSelected(selectedNotes);

        repaint();
    }

    bool isSelectedNote(int partIdx, int noteIdx) const
    {
        for (const auto& n : selectedNotes)
            if (n.first == partIdx && n.second == noteIdx)
                return true;
        return false;
    }

    void clearSelectedNotesAndNotify()
    {
        selectedPartIdx = -1;
        selectedNoteIdx = -1;
        selectedNotes.clear();
        if (onNoteSelected)
            onNoteSelected(-1, -1);
        if (onNotesSelected)
            onNotesSelected(selectedNotes);
        repaint();
    }

    void selectNotesFromAnchorToEdge(int partIdx, int noteIdx, bool toLeft)
    {
        if (banks == nullptr || currentBankIndex == nullptr)
            return;

        const int bankIdx = *currentBankIndex;
        if (bankIdx < 0 || partIdx < 0 || partIdx >= 4 || !isEditable(partIdx))
            return;
        if (isPartActiveForBank && !isPartActiveForBank(bankIdx, partIdx))
            return;

        const auto& seq = banks[bankIdx].tracks[partIdx].sequence;
        if (noteIdx < 0 || noteIdx >= static_cast<int>(seq.size()))
            return;

        const double anchorBeat = seq[static_cast<size_t>(noteIdx)].startBeat;
        constexpr double eps = 0.0001;

        selectedNotes.clear();
        for (int i = 0; i < static_cast<int>(seq.size()); ++i)
        {
            const auto& note = seq[static_cast<size_t>(i)];
            if (note.frequency <= 0.0)
                continue;

            const bool inRange = toLeft ? (note.startBeat <= anchorBeat + eps)
                                        : (note.startBeat >= anchorBeat - eps);
            if (inRange)
                selectedNotes.push_back({ partIdx, i });
        }

        selectedPartIdx = selectedNotes.empty() ? -1 : partIdx;
        selectedNoteIdx = (selectedNotes.size() == 1) ? selectedNotes.front().second : -1;

        if (onNoteSelected)
            onNoteSelected(selectedPartIdx, selectedNoteIdx);
        if (onNotesSelected)
            onNotesSelected(selectedNotes);

        repaint();
    }

    void showNoteRangeSelectMenu(int partIdx, int noteIdx, const juce::MouseEvent& e)
    {
        juce::PopupMenu menu;
        menu.addItem(1, T("Select from this note to left edge", L"해당 노트부터 왼쪽 끝까지 선택"));
        menu.addItem(2, T("Select from this note to right edge", L"해당 노트부터 오른쪽 끝까지 선택"));

        const auto screenPos = e.getScreenPosition();
        auto options = juce::PopupMenu::Options()
            .withTargetComponent(this)
            .withTargetScreenArea(juce::Rectangle<int>(screenPos.x, screenPos.y, 1, 1));

        menu.showMenuAsync(options, [this, partIdx, noteIdx](int result)
        {
            if (result == 1)
                selectNotesFromAnchorToEdge(partIdx, noteIdx, true);
            else if (result == 2)
                selectNotesFromAnchorToEdge(partIdx, noteIdx, false);
        });
    }

    juce::Rectangle<float> getNoteRectangle(const MmlNote& note, int midiNote, const juce::Rectangle<int>& rollArea) const
    {
        const float noteX = static_cast<float>(rollArea.getX()) + static_cast<float>((note.startBeat * pixelsPerBeat) - scrollX);
        const float noteW = juce::jmax(4.0f, static_cast<float>((note.endBeat - note.startBeat) * pixelsPerBeat));
        const float y = static_cast<float>(rollArea.getY()) + static_cast<float>(maxMidi - midiNote) * rowHeight - static_cast<float>(scrollY);
        return { noteX, y + 1.0f, noteW, rowHeight - 2.0f };
    }

    juce::Rectangle<int> getLassoRectangle() const
    {
        return juce::Rectangle<int>::leftTopRightBottom(
            juce::jmin(lassoState.start.x, lassoState.current.x),
            juce::jmin(lassoState.start.y, lassoState.current.y),
            juce::jmax(lassoState.start.x, lassoState.current.x),
            juce::jmax(lassoState.start.y, lassoState.current.y));
    }

    void updateLassoSelection()
    {
        if (banks == nullptr || currentBankIndex == nullptr)
            return;

        const auto layout = getLayout();
        const auto selectionRect = getLassoRectangle().getIntersection(layout.rollArea);
        std::vector<std::pair<int, int>> newSelection;
        if (!selectionRect.isEmpty())
        {
            const int bankIdx = *currentBankIndex;
            for (int part = 0; part < 4; ++part)
            {
                if (!isEditable(part))
                    continue;
                if (isPartActiveForBank && !isPartActiveForBank(bankIdx, part))
                    continue;

                const auto& seq = banks[bankIdx].tracks[part].sequence;
                for (int i = 0; i < static_cast<int>(seq.size()); ++i)
                {
                    const auto& note = seq[static_cast<size_t>(i)];
                    if (note.frequency <= 0.0)
                        continue;
                    const int midi = juce::jlimit(minMidi, maxMidi, static_cast<int>(std::round(69.0 + 12.0 * std::log2(note.frequency / 440.0))));
                    if (getNoteRectangle(note, midi, layout.rollArea).intersects(selectionRect.toFloat()))
                        newSelection.push_back({ part, i });
                }
            }
        }

        if (!sameSelection(selectedNotes, newSelection))
        {
            selectedNotes = newSelection;
            if (selectedNotes.size() == 1)
            {
                selectedPartIdx = selectedNotes.front().first;
                selectedNoteIdx = selectedNotes.front().second;
                if (onNoteSelected)
                    onNoteSelected(selectedPartIdx, selectedNoteIdx);
            }
            else
            {
                selectedPartIdx = selectedNotes.empty() ? -1 : selectedNotes.front().first;
                selectedNoteIdx = -1;
                if (onNoteSelected)
                    onNoteSelected(selectedPartIdx, -1);
            }
            if (onNotesSelected)
                onNotesSelected(selectedNotes);
        }
    }

    NoteHit hitTestCurrentBankNote(juce::Point<int> pos, bool allowEdge) const
    {
        NoteHit best;
        if (banks == nullptr || currentBankIndex == nullptr)
            return best;

        const auto layout = getLayout();
        if (!layout.rollArea.contains(pos))
            return best;

        double bestDistance = 1.0e18;
        const int bankIdx = *currentBankIndex;
        for (int part = 0; part < 4; ++part)
        {
            if (isPartActiveForBank && !isPartActiveForBank(bankIdx, part))
                continue;

            const auto& seq = banks[bankIdx].tracks[part].sequence;
            for (int i = 0; i < static_cast<int>(seq.size()); ++i)
            {
                const auto& note = seq[static_cast<size_t>(i)];
                if (note.frequency <= 0.0)
                    continue;

                const int midi = juce::jlimit(minMidi, maxMidi, static_cast<int>(std::round(69.0 + 12.0 * std::log2(note.frequency / 440.0))));
                juce::Rectangle<float> r = getNoteRectangle(note, midi, layout.rollArea);
                auto hitRect = r.expanded(2.0f, 2.0f);
                if (!hitRect.contains(pos.toFloat()))
                    continue;

                const double dist = std::abs(pointToBeat(static_cast<float>(pos.x), layout.rollArea) - (note.startBeat + note.endBeat) * 0.5);
                if (dist < bestDistance)
                {
                    bestDistance = dist;
                    best.valid = true;
                    best.partIdx = part;
                    best.noteIdx = i;
                    best.midiNote = midi;
                    best.startBeat = note.startBeat;
                    best.endBeat = note.endBeat;
                    best.onRightEdge = allowEdge && std::abs(static_cast<float>(pos.x) - r.getRight()) <= 7.0f;
                }
            }
        }
        return best;
    }

    void setActivePreviewMidi(int midiNote)
    {
        const int safeMidi = juce::jlimit(minMidi, maxMidi, midiNote);
        activePreviewMidi = safeMidi;
        if (onPreviewMidiChanged)
            onPreviewMidiChanged(activePreviewMidi);
        repaint();
    }

    void clearActivePreviewMidi()
    {
        if (activePreviewMidi < 0)
            return;

        activePreviewMidi = -1;
        if (onPreviewMidiChanged)
            onPreviewMidiChanged(-1);
        repaint();
    }

    void beginDragForInsertedNote(int partIdx, int midiNote, double startBeat, double lengthBeats)
    {
        if (!isEditable(partIdx) || banks == nullptr || currentBankIndex == nullptr)
            return;

        const int bankIdx = *currentBankIndex;
        if (bankIdx < 0)
            return;

        const auto& seq = banks[bankIdx].tracks[partIdx].sequence;
        int insertedIdx = -1;
        double bestScore = 1.0e18;
        const double endBeat = startBeat + lengthBeats;

        for (int i = 0; i < static_cast<int>(seq.size()); ++i)
        {
            const auto& note = seq[static_cast<size_t>(i)];
            if (note.frequency <= 0.0)
                continue;

            const int noteMidi = juce::jlimit(minMidi, maxMidi, static_cast<int>(std::round(69.0 + 12.0 * std::log2(note.frequency / 440.0))));
            const double score = std::abs(note.startBeat - startBeat)
                               + std::abs(note.endBeat - endBeat)
                               + (noteMidi == midiNote ? 0.0 : 1000.0);
            if (score < bestScore)
            {
                bestScore = score;
                insertedIdx = i;
            }
        }

        if (insertedIdx < 0)
            return;

        const auto& inserted = seq[static_cast<size_t>(insertedIdx)];
        const int insertedMidi = juce::jlimit(minMidi, maxMidi, static_cast<int>(std::round(69.0 + 12.0 * std::log2(inserted.frequency / 440.0))));
        const double insertedLength = juce::jmax(0.0625, inserted.endBeat - inserted.startBeat);

        selectedPartIdx = partIdx;
        selectedNoteIdx = insertedIdx;
        selectedNotes.clear();
        selectedNotes.push_back({ partIdx, insertedIdx });
        if (onNoteSelected)
            onNoteSelected(partIdx, insertedIdx);
        if (onNotesSelected)
            onNotesSelected(selectedNotes);

        dragState.active = true;
        // 생성 직후 마우스를 떼지 않고 드래그하면,
        // 음높이 이동 + 현재 음표 길이 단위의 좌/우 위치 이동을 수행한다.
        // 이때 첫 drag에서 추가된 노트를 포함한 상태를 스냅샷으로 다시 캡처한다.
        dragState.groupPitchOnly = true;
        dragState.resizing = false;
        dragState.pitchAndLength = false;
        dragState.partIdx = partIdx;
        dragState.noteIdx = insertedIdx;
        dragState.startMidi = insertedMidi;
        dragState.startLengthBeats = insertedLength;
        dragState.noteStartBeat = inserted.startBeat;
        dragState.dragStartBeat = startBeat;
        dragState.lastSelectedMoveBeatDelta = 0.0;
        dragState.lastAppliedSemitoneDelta = 0;
        dragState.lastAppliedBeatDelta = 0.0;
        dragState.previewSemitoneDelta = 0;
        dragState.previewBeatDelta = 0.0;
        dragState.minStartBeatAtDragStart = inserted.startBeat;
        dragState.moveSnapshotPrepared = false;
        dragState.lastMidi = insertedMidi;
        dragState.lastLengthBeats = insertedLength;
        setActivePreviewMidi(insertedMidi);
        setMouseCursor(juce::MouseCursor::DraggingHandCursor);
        repaint();
    }

    void drawEventList(juce::Graphics& g, juce::Rectangle<int> leftPanel, juce::Rectangle<int> leftTimeline)
    {
        g.setColour(CustomUI::getThemeColour(themeId, "panel"));
        g.fillRect(leftPanel.toFloat());
        g.setColour(CustomUI::getThemeColour(themeId, "timeline"));
        g.fillRect(leftTimeline.toFloat());
        g.setColour(CustomUI::getReadableTextColour(CustomUI::getThemeColour(themeId, "timeline")));
        g.setFont(11.0f);
        g.drawText(T("Trk Time        Step  Event", L"트랙 시간          스텝   이벤트"), leftTimeline.withTrimmedLeft(5), juce::Justification::centredLeft, true);

        if (cachedEventList == nullptr)
            return;

        double viewStartBeat = scrollX / pixelsPerBeat;
        g.saveState();
        g.reduceClipRegion(leftPanel);
        float eventY = static_cast<float>(leftPanel.getY()) + 4.0f;
        constexpr float lineHeight = 18.0f;
        g.setFont(12.0f);

        for (const auto& ev : *cachedEventList)
        {
            if (ev.endBeat < viewStartBeat) continue;
            if (eventY > leftPanel.getBottom()) break;

            if (ev.trackIdx < 0 && ev.frequency < -1.0)
            {
                const auto leftBg = CustomUI::getThemeColour(themeId, "panel");
                const auto readableText = CustomUI::getReadableTextColour(leftBg);
                const auto tempoBadge = readableText.withAlpha(readableText == juce::Colours::black ? 0.10f : 0.18f);
                const auto tempoOutline = readableText.withAlpha(0.34f);
                auto labelBadge = juce::Rectangle<float>(static_cast<float>(leftPanel.getX() + 4), eventY + 2.0f, 58.0f, lineHeight - 4.0f);
                auto valueBadge = juce::Rectangle<float>(static_cast<float>(leftPanel.getX() + 76), eventY + 2.0f, 42.0f, lineHeight - 4.0f);
                g.setColour(tempoBadge); g.fillRoundedRectangle(labelBadge, 3.0f); g.fillRoundedRectangle(valueBadge, 3.0f);
                g.setColour(tempoOutline); g.drawRoundedRectangle(labelBadge, 3.0f, 1.0f); g.drawRoundedRectangle(valueBadge, 3.0f, 1.0f);
                g.setColour(readableText.withAlpha(0.92f)); g.drawText(ev.timeStr, leftPanel.getX() + 6, static_cast<int>(eventY), 58, static_cast<int>(lineHeight), juce::Justification::centredLeft, false);
                g.setColour(readableText.withAlpha(0.90f)); g.drawText(ev.stepStr, leftPanel.getX() + 78, static_cast<int>(eventY), 40, static_cast<int>(lineHeight), juce::Justification::centredLeft, false);
                if (ev.eventType.isNotEmpty()) { g.setColour(readableText.withAlpha(0.70f)); g.drawText(ev.eventType, leftPanel.getX() + 130, static_cast<int>(eventY), 45, static_cast<int>(lineHeight), juce::Justification::centredLeft, false); }
                eventY += lineHeight;
                continue;
            }

            juce::Colour tColor = CustomUI::getTrackThemeColour(themeId, ev.trackIdx);
            g.setColour(tColor.withAlpha(0.3f)); g.fillRoundedRectangle(leftPanel.getX() + 4.0f, eventY + 2.0f, 18.0f, lineHeight - 4.0f, 3.0f);
            g.setColour(tColor); g.drawText(juce::String("T") + juce::String(ev.trackIdx + 1), leftPanel.getX() + 4, static_cast<int>(eventY), 18, static_cast<int>(lineHeight), juce::Justification::centred, false);
            g.setColour(CustomUI::getThemeColour(themeId, "mutedText")); g.drawText(ev.timeStr, leftPanel.getX() + 26, static_cast<int>(eventY), 70, static_cast<int>(lineHeight), juce::Justification::centredLeft, false);
            g.setColour(CustomUI::getThemeColour(themeId, "textOnDark").withAlpha(0.65f)); g.drawText(ev.stepStr, leftPanel.getX() + 95, static_cast<int>(eventY), 35, static_cast<int>(lineHeight), juce::Justification::centredLeft, false);
            g.setColour(ev.frequency > 0 ? tColor : CustomUI::getThemeColour(themeId, "mutedText")); g.drawText(ev.eventType, leftPanel.getX() + 130, static_cast<int>(eventY), 45, static_cast<int>(lineHeight), juce::Justification::centredLeft, false);
            eventY += lineHeight;
        }
        g.restoreState();
    }


    std::vector<std::pair<double, int>> buildVisibleMeasureStarts(double maxBeatToBuild) const
    {
        std::vector<MeterChange> changes;
        if (meterChanges != nullptr)
            changes = *meterChanges;

        if (changes.empty() || std::abs(changes.front().beatPosition) > 0.0001)
        {
            MeterChange base;
            base.beatPosition = 0.0;
            base.timeSignatureId = 1;
            base.beatsPerMeasure = beatsPerMeasure > 0 ? beatsPerMeasure : 4;
            base.displayText = juce::String(base.beatsPerMeasure) + "/4";
            changes.insert(changes.begin(), base);
        }

        std::sort(changes.begin(), changes.end(), [](const MeterChange& a, const MeterChange& b)
        {
            return a.beatPosition < b.beatPosition;
        });

        std::vector<std::pair<double, int>> result;
        double beat = 0.0;
        int meterBeats = juce::jmax(1, changes.front().beatsPerMeasure);
        int changeIndex = 0;
        int measureNumber = 0;

        while (changeIndex + 1 < static_cast<int>(changes.size()) && changes[changeIndex + 1].beatPosition <= 0.0001)
        {
            ++changeIndex;
            meterBeats = juce::jmax(1, changes[changeIndex].beatsPerMeasure);
        }

        const int guardMax = 20000;
        for (int guard = 0; guard < guardMax && beat <= maxBeatToBuild + 0.0001; ++guard)
        {
            result.push_back({ beat, measureNumber });

            double nextBeat = beat + static_cast<double>(meterBeats);
            if (changeIndex + 1 < static_cast<int>(changes.size()))
            {
                const double changeBeat = changes[changeIndex + 1].beatPosition;
                if (changeBeat > beat + 0.0001 && changeBeat < nextBeat - 0.0001)
                    nextBeat = changeBeat;
            }

            if (nextBeat <= beat + 0.0001)
                nextBeat = beat + static_cast<double>(juce::jmax(1, meterBeats));

            beat = nextBeat;
            ++measureNumber;

            while (changeIndex + 1 < static_cast<int>(changes.size())
                   && changes[changeIndex + 1].beatPosition <= beat + 0.0001)
            {
                ++changeIndex;
                meterBeats = juce::jmax(1, changes[changeIndex].beatsPerMeasure);
            }
        }

        return result;
    }

    void drawPianoAndGrid(juce::Graphics& g, juce::Rectangle<int> pianoArea, juce::Rectangle<int> rollArea, juce::Rectangle<int> pianoTimeline, juce::Rectangle<int> gridTimeline)
    {
        g.setColour(CustomUI::getThemeColour(themeId, "timeline")); g.fillRect(pianoTimeline.toFloat());
        g.setColour(CustomUI::getThemeColour(themeId, "timeline2")); g.fillRect(gridTimeline.toFloat());

        const int numNotes = maxMidi - minMidi + 1;
        g.saveState();
        g.reduceClipRegion(pianoArea.getUnion(rollArea));

        for (int i = 0; i < numNotes; ++i)
        {
            const float y = static_cast<float>(rollArea.getY()) + static_cast<float>(i) * rowHeight - static_cast<float>(scrollY);
            if (y + rowHeight < rollArea.getY() || y > rollArea.getBottom()) continue;
            const int midiNote = maxMidi - i;
            const int noteInOctave = midiNote % 12;
            const bool isBlack = (noteInOctave == 1 || noteInOctave == 3 || noteInOctave == 6 || noteInOctave == 8 || noteInOctave == 10);
            g.setColour(isBlack ? CustomUI::getThemeColour(themeId, "gridBlack") : CustomUI::getThemeColour(themeId, "gridWhite"));
            g.fillRect(static_cast<float>(rollArea.getX()), y, static_cast<float>(rollArea.getWidth()), rowHeight);
            g.setColour(CustomUI::getThemeColour(themeId, "gridLine"));
            g.drawLine(static_cast<float>(rollArea.getX()), y, static_cast<float>(rollArea.getRight()), y, 1.0f);
        }

        const float keysTop = static_cast<float>(pianoArea.getY()) - static_cast<float>(scrollY);
        const float keysHeight = static_cast<float>(numNotes) * rowHeight;
        g.setColour(CustomUI::getThemeColour(themeId, "pianoBlack")); g.fillRect(pianoArea.toFloat());
        g.setColour(CustomUI::getThemeColour(themeId, "pianoWhite")); g.fillRect(static_cast<float>(pianoArea.getX()), keysTop, static_cast<float>(pianoArea.getWidth()), keysHeight);

        for (int i = 0; i < numNotes; ++i)
        {
            const float y = static_cast<float>(pianoArea.getY()) + static_cast<float>(i) * rowHeight - static_cast<float>(scrollY);
            if (y + rowHeight < pianoArea.getY() || y > pianoArea.getBottom()) continue;
            const int midiNote = maxMidi - i;
            const int noteInOctave = midiNote % 12;
            const bool isBlack = (noteInOctave == 1 || noteInOctave == 3 || noteInOctave == 6 || noteInOctave == 8 || noteInOctave == 10);
            const bool isPreviewKey = (midiNote == activePreviewMidi);
            if (isBlack)
            {
                g.setColour(isPreviewKey ? juce::Colours::yellow.withAlpha(0.96f)
                                          : CustomUI::getThemeColour(themeId, "pianoBlack"));
                g.fillRect(static_cast<float>(pianoArea.getX()), y, static_cast<float>(pianoArea.getWidth()) * 0.62f, rowHeight);
                if (isPreviewKey)
                {
                    g.setColour(juce::Colours::black.withAlpha(0.80f));
                    g.drawRect(juce::Rectangle<float>(static_cast<float>(pianoArea.getX()), y, static_cast<float>(pianoArea.getWidth()) * 0.62f, rowHeight), 1.5f);
                }
            }
            else
            {
                if (isPreviewKey)
                {
                    g.setColour(juce::Colours::yellow.withAlpha(0.86f));
                    g.fillRect(static_cast<float>(pianoArea.getX()), y, static_cast<float>(pianoArea.getWidth()), rowHeight);
                }
                g.setColour(isPreviewKey ? juce::Colours::black.withAlpha(0.70f)
                                          : CustomUI::getThemeColour(themeId, "gridLine"));
                g.drawLine(static_cast<float>(pianoArea.getX()), y + rowHeight, static_cast<float>(pianoArea.getRight()), y + rowHeight, isPreviewKey ? 1.5f : 1.0f);
                if (noteInOctave == 0 || isPreviewKey)
                {
                    static const char* noteNames[12] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
                    g.setColour(isPreviewKey ? juce::Colours::black
                                             : CustomUI::getReadableTextColour(CustomUI::getThemeColour(themeId, "pianoWhite")));
                    g.setFont(rowHeight * 0.85f);
                    const juce::String keyText = juce::String(noteNames[noteInOctave]) + juce::String((midiNote / 12) - 1);
                    g.drawText(keyText, static_cast<int>(pianoArea.getX() + 2), static_cast<int>(y), static_cast<int>(pianoArea.getWidth()), static_cast<int>(rowHeight), juce::Justification::centredLeft, false);
                }
            }
        }
        g.setColour(CustomUI::getThemeColour(themeId, "accent").withAlpha(0.6f));
        g.drawRect(pianoArea.toFloat(), 1.0f);
        g.restoreState();

        const double totalWidthPixels = getTotalWidthPixels();
        if (onHorizontalRangeChanged) onHorizontalRangeChanged(totalWidthPixels, rollArea.getWidth());

        const double viewStartBeat = scrollX / pixelsPerBeat;
        const double viewEndBeat = (scrollX + rollArea.getWidth()) / pixelsPerBeat;
        const int64_t firstBeat = static_cast<int64_t>(std::floor(viewStartBeat));
        const int64_t lastBeat = static_cast<int64_t>(std::ceil(viewEndBeat));

        // Thin beat grid stays on every beat. Measure starts are drawn separately
        // so variable time signatures can change the bold bar lines mid-song.
        for (int64_t beat = firstBeat; beat <= lastBeat; ++beat)
        {
            const float x = static_cast<float>(rollArea.getX()) + static_cast<float>((beat * pixelsPerBeat) - scrollX);
            if (x < rollArea.getX() || x > rollArea.getRight()) continue;
            g.setColour(CustomUI::getThemeColour(themeId, "gridLine").withAlpha(0.35f));
            g.drawLine(x, static_cast<float>(rollArea.getY()), x, static_cast<float>(rollArea.getBottom()), 1.0f);
            g.setColour(CustomUI::getThemeColour(themeId, "textOnDark").withAlpha(0.8f));
            g.drawLine(x, static_cast<float>(gridTimeline.getBottom() - 4), x, static_cast<float>(gridTimeline.getBottom()), 1.0f);
        }

        const auto measureStarts = buildVisibleMeasureStarts(viewEndBeat + 32.0);
        for (const auto& m : measureStarts)
        {
            const double beat = m.first;
            if (beat < viewStartBeat - 0.0001 || beat > viewEndBeat + 0.0001) continue;
            const float x = static_cast<float>(rollArea.getX()) + static_cast<float>((beat * pixelsPerBeat) - scrollX);
            if (x < rollArea.getX() || x > rollArea.getRight()) continue;
            g.setColour(CustomUI::getThemeColour(themeId, "textOnDark").withAlpha(0.68f));
            g.drawLine(x, static_cast<float>(rollArea.getY()), x, static_cast<float>(rollArea.getBottom()), 2.0f);
            g.setColour(CustomUI::getThemeColour(themeId, "textOnDark"));
            g.setFont(12.0f);
            g.drawText(juce::String(m.second), static_cast<int>(x + 4), static_cast<int>(gridTimeline.getY()), 50, static_cast<int>(gridTimeline.getHeight()), juce::Justification::centredLeft, false);
            g.setColour(CustomUI::getThemeColour(themeId, "textOnDark").withAlpha(0.85f));
            g.drawLine(x, static_cast<float>(gridTimeline.getBottom() - 8), x, static_cast<float>(gridTimeline.getBottom()), 1.0f);
        }

        if (meterChanges != nullptr)
        {
            for (const auto& change : *meterChanges)
            {
                if (change.beatPosition < viewStartBeat - 0.0001 || change.beatPosition > viewEndBeat + 0.0001) continue;
                const float x = static_cast<float>(rollArea.getX()) + static_cast<float>((change.beatPosition * pixelsPerBeat) - scrollX);
                if (x < rollArea.getX() || x > rollArea.getRight()) continue;
                g.setColour(CustomUI::getThemeColour(themeId, "accent2").withAlpha(0.92f));
                g.drawLine(x, static_cast<float>(gridTimeline.getY()), x, static_cast<float>(rollArea.getBottom()), 1.5f);
                g.setFont(11.0f);
                g.drawText(change.displayText, static_cast<int>(x + 4), static_cast<int>(gridTimeline.getY()), 60, static_cast<int>(gridTimeline.getHeight()), juce::Justification::centredLeft, false);
            }
        }


        if (tempoMap != nullptr)
        {
            g.setFont(12.0f);
            for (const auto& tChange : *tempoMap)
            {
                const float tX = static_cast<float>(rollArea.getX()) + static_cast<float>((tChange.beatPosition * pixelsPerBeat) - scrollX);
                if (tX < rollArea.getX() || tX > rollArea.getRight()) continue;
                g.setColour(CustomUI::getThemeColour(themeId, "accent2").withAlpha(0.85f));
                g.drawLine(tX, static_cast<float>(gridTimeline.getY()), tX, static_cast<float>(gridTimeline.getBottom()), 2.0f);
                const juce::String tText = juce::String("T") + juce::String(static_cast<int>(tChange.bpm));
                const int tempoLabelY = juce::jmax(0, static_cast<int>(gridTimeline.getY()) - 4);
                const auto timelineReadable = CustomUI::getReadableTextColour(CustomUI::getThemeColour(themeId, "timeline"));
                auto tempoBadge = juce::Rectangle<float>(static_cast<float>(tX + 2), static_cast<float>(tempoLabelY + 2), 34.0f, static_cast<float>(gridTimeline.getHeight() - 4));
                g.setColour(timelineReadable.withAlpha(timelineReadable == juce::Colours::black ? 0.10f : 0.20f));
                g.fillRoundedRectangle(tempoBadge, 3.0f);
                g.setColour(timelineReadable.withAlpha(0.94f));
                g.drawText(tText, static_cast<int>(tX + 4), tempoLabelY, 50, static_cast<int>(gridTimeline.getHeight()), juce::Justification::centredLeft, false);
            }
        }
    }

    void drawNotesAndPlayhead(juce::Graphics& g, juce::Rectangle<int> rollArea, juce::Rectangle<int>)
    {
        if (sampleRate <= 0.0)
            return;

        const double viewStartBeat = scrollX / pixelsPerBeat;
        const double viewEndBeat = (scrollX + rollArea.getWidth()) / pixelsPerBeat;
        bool anySolo = false;
        for (int i = 0; i < *numActiveTracks; ++i)
            for (int j = 0; j < 4; ++j)
                if ((!isPartActiveForBank || isPartActiveForBank(i, j)) && banks[i].tracks[j].solo)
                    anySolo = true;

        g.saveState();
        g.reduceClipRegion(rollArea);

        auto drawNotes = [&](const std::vector<MmlNote>& seq, int partIdx, juce::Colour color, size_t currentIndex, bool muted, float alpha, bool editableTrack)
        {
            if (muted) return;

            auto volumeFloatToDisplayValue = [](float volume) -> int
            {
                return juce::jlimit(0, 15, static_cast<int>(std::round(juce::jlimit(0.0f, 1.0f, volume) * 15.0f)));
            };

            auto drawVolumeLabel = [&](int volumeValue, float noteLeftX, float noteTopY)
            {
                if (alpha < 0.25f)
                    return;

                const juce::String label = juce::String("V") + juce::String(volumeValue);
                const auto gridBase = CustomUI::getThemeColour(themeId, "gridWhite")
                                    .interpolatedWith(CustomUI::getThemeColour(themeId, "gridBlack"), 0.5f);
                const auto readable = CustomUI::getReadableTextColour(gridBase);
                const bool textIsDark = readable.getBrightness() < 0.5f;

                #if JUCE_MAJOR_VERSION >= 8
                g.setFont(juce::Font(juce::FontOptions(10.5f, juce::Font::bold)));
#else
                g.setFont(juce::Font(10.5f, juce::Font::bold));
#endif
                const int labelW = 28;
                const int labelH = 13;

                float labelX = noteLeftX + 2.0f;
                float labelY = noteTopY - static_cast<float>(labelH) + 1.0f;
                if (labelY < static_cast<float>(rollArea.getY()) + 1.0f)
                    labelY = noteTopY + rowHeight + 1.0f;
                if (labelY + static_cast<float>(labelH) > static_cast<float>(rollArea.getBottom()) - 1.0f)
                    labelY = noteTopY + 1.0f;

                labelX = juce::jlimit(static_cast<float>(rollArea.getX()) + 1.0f,
                                      static_cast<float>(rollArea.getRight() - labelW - 1),
                                      labelX);

                const auto badgeRect = juce::Rectangle<float>(labelX, labelY, static_cast<float>(labelW), static_cast<float>(labelH));
                const auto badgeFill = textIsDark ? juce::Colours::white.withAlpha(0.68f * alpha)
                                                  : juce::Colours::black.withAlpha(0.38f * alpha);
                const auto badgeOutline = CustomUI::getThemeColour(themeId, "gridLine").withAlpha(0.45f * alpha);

                g.setColour(badgeFill);
                g.fillRoundedRectangle(badgeRect, 3.0f);
                g.setColour(badgeOutline);
                g.drawRoundedRectangle(badgeRect, 3.0f, 1.0f);
                g.setColour(readable.withAlpha(0.95f * alpha));
                g.drawText(label, badgeRect.toNearestInt(), juce::Justification::centred, false);
            };

            int lastDisplayedVolume = -1;
            bool hasDisplayedVolumeBasis = false;

            for (size_t i = 0; i < seq.size(); ++i)
            {
                const auto& note = seq[i];
                if (note.frequency <= 0.0) continue;

                const int currentDisplayVolume = volumeFloatToDisplayValue(note.volume);
                const bool shouldDrawVolumeLabel = !hasDisplayedVolumeBasis || currentDisplayVolume != lastDisplayedVolume;
                hasDisplayedVolumeBasis = true;
                lastDisplayedVolume = currentDisplayVolume;

                const bool selected = alpha >= 0.9f && isSelectedNote(partIdx, static_cast<int>(i));
                const bool previewMoving = selected
                                         && dragState.active
                                         && dragState.groupPitchOnly
                                         && (dragState.previewSemitoneDelta != 0 || std::abs(dragState.previewBeatDelta) > 0.0001);

                double visualStartBeat = note.startBeat;
                double visualEndBeat = note.endBeat;
                int midiNoteInt = juce::jlimit(minMidi, maxMidi, static_cast<int>(std::round(69.0 + 12.0 * std::log2(note.frequency / 440.0))));

                if (previewMoving)
                {
                    visualStartBeat = juce::jmax(0.0, note.startBeat + dragState.previewBeatDelta);
                    visualEndBeat = juce::jmax(visualStartBeat + 0.0625, note.endBeat + dragState.previewBeatDelta);
                    midiNoteInt = juce::jlimit(minMidi, maxMidi, midiNoteInt + dragState.previewSemitoneDelta);
                }

                const bool previewResizing = alpha >= 0.9f
                                           && dragState.active
                                           && (dragState.resizing || dragState.pitchAndLength)
                                           && dragState.partIdx == partIdx
                                           && dragState.noteIdx == static_cast<int>(i);
                if (previewResizing)
                {
                    visualEndBeat = juce::jmax(visualStartBeat + 0.0625, visualStartBeat + dragState.lastLengthBeats);
                    if (dragState.pitchAndLength)
                        midiNoteInt = juce::jlimit(minMidi, maxMidi, dragState.lastMidi);
                }

                if (visualEndBeat < viewStartBeat || visualStartBeat > viewEndBeat) continue;
                const float noteX = static_cast<float>(rollArea.getX()) + static_cast<float>((visualStartBeat * pixelsPerBeat) - scrollX);
                const float noteW = juce::jmax(4.0f, static_cast<float>((visualEndBeat - visualStartBeat) * pixelsPerBeat));
                const float drawX = std::max(static_cast<float>(rollArea.getX()), noteX);
                const float drawRight = std::min(static_cast<float>(rollArea.getRight()), noteX + noteW);
                const float drawW = drawRight - drawX;
                if (drawW <= 0.0f) continue;
                const float y = static_cast<float>(rollArea.getY()) + static_cast<float>(maxMidi - midiNoteInt) * rowHeight - static_cast<float>(scrollY);
                if (y + rowHeight < rollArea.getY() || y > rollArea.getBottom()) continue;
                const float visualAlpha = alpha * (0.3f + 0.7f * note.volume);
                const bool nowPlaying = (isPlaying != nullptr && *isPlaying && i == currentIndex);
                const bool leftMouseHeldNote = alpha >= 0.9f
                                             && dragState.active
                                             && dragState.partIdx == partIdx
                                             && dragState.noteIdx == static_cast<int>(i);

                auto noteFillColour = nowPlaying ? color.brighter(0.5f).withAlpha(std::min(1.0f, visualAlpha + 0.3f))
                                                 : color.withAlpha(visualAlpha);
                if (leftMouseHeldNote)
                    noteFillColour = juce::Colours::yellow.withAlpha(0.96f);

                g.setColour(noteFillColour);
                g.fillRoundedRectangle(drawX, y + 1.0f, drawW, rowHeight - 2.0f, 2.0f);
                g.setColour(leftMouseHeldNote ? juce::Colours::black.withAlpha(0.65f)
                                               : CustomUI::getThemeColour(themeId, "backgroundBottom").withAlpha(0.55f));
                g.drawRoundedRectangle(drawX, y + 1.0f, drawW, rowHeight - 2.0f, 2.0f, 1.0f);

                if (selected)
                {
                    g.setColour(leftMouseHeldNote ? juce::Colours::black.withAlpha(0.95f)
                                                   : CustomUI::getThemeColour(themeId, "accent2").withAlpha(0.95f));
                    g.drawRoundedRectangle(drawX - 1.5f, y - 0.5f, drawW + 3.0f, rowHeight + 1.0f, 3.0f, 2.0f);
                }

                if (shouldDrawVolumeLabel)
                    drawVolumeLabel(currentDisplayVolume, noteX, y);

                if (editableTrack && alpha >= 0.9f)
                {
                    g.setColour(CustomUI::getReadableTextColour(color).withAlpha(0.65f));
                    g.drawLine(noteX + noteW - 3.0f, y + 2.0f, noteX + noteW - 3.0f, y + rowHeight - 3.0f, 1.0f);
                }
            }
        };

        for (int i = 0; i < *numActiveTracks; ++i)
        {
            if (i == *currentBankIndex) continue;
            const juce::Colour bgC = CustomUI::getBankColor(i);
            for (int j = 0; j < 4; ++j)
            {
                if (isPartActiveForBank && !isPartActiveForBank(i, j)) continue;
                drawNotes(banks[i].tracks[j].sequence, j, bgC, banks[i].tracks[j].noteIndex, anySolo ? !banks[i].tracks[j].solo : banks[i].tracks[j].mute, 0.3f, false);
            }
        }

        const juce::Colour fgC = CustomUI::getBankColor(*currentBankIndex);
        for (int j = 0; j < 4; ++j)
        {
            if (isPartActiveForBank && !isPartActiveForBank(*currentBankIndex, j)) continue;
            drawNotes(banks[*currentBankIndex].tracks[j].sequence, j, fgC, banks[*currentBankIndex].tracks[j].noteIndex, anySolo ? !banks[*currentBankIndex].tracks[j].solo : banks[*currentBankIndex].tracks[j].mute, 1.0f, isEditable(j));
        }

        if (globalSampleCount != nullptr && tempoMap != nullptr)
        {
            const double playheadBeat = MmlLogic::getBeatFromSample(*globalSampleCount, *tempoMap, sampleRate);
            const float playheadX = static_cast<float>(rollArea.getX()) + static_cast<float>((playheadBeat * pixelsPerBeat) - scrollX);
            if (playheadX >= rollArea.getX() && playheadX <= rollArea.getRight())
            {
                g.setColour(CustomUI::getThemeColour(themeId, "playhead").withAlpha(0.9f));
                g.drawLine(playheadX, static_cast<float>(rollArea.getY()), playheadX, static_cast<float>(rollArea.getBottom()), 2.5f);
            }
        }
        g.restoreState();
    }

    void drawLassoSelection(juce::Graphics& g)
    {
        if (!lassoState.active && !pendingGridSelection)
            return;

        auto r = getLassoRectangle().toFloat();
        if (r.getWidth() < 1.0f || r.getHeight() < 1.0f)
            return;

        g.setColour(CustomUI::getThemeColour(themeId, "accent2").withAlpha(0.16f));
        g.fillRect(r);
        g.setColour(CustomUI::getThemeColour(themeId, "accent2").withAlpha(0.85f));
        g.drawRect(r, 1.5f);
    }

    void drawDragHint(juce::Graphics& g, juce::Rectangle<int> rollArea)
    {
        g.setColour(CustomUI::getThemeColour(themeId, "accent2").withAlpha(0.85f));
        g.setFont(12.0f);
        const juce::String text = dragState.groupPitchOnly ? T("Move note position", L"노트 위치 이동")
                                                            : (dragState.pitchAndLength ? T("Move pitch / Resize note", L"음높이 이동 / 길이 조절")
                                                                                        : (dragState.resizing ? T("Resize note", L"노트 길이 조절")
                                                                                                              : T("Move pitch", L"노트 음높이 이동")));
        g.drawText(text, rollArea.withSizeKeepingCentre(180, 22).translated(0, -rollArea.getHeight() / 2 + 18), juce::Justification::centred, false);
    }
};
