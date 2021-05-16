#pragma once

#include "model/Connections.h"
#include "model/Tracks.h"
#include "ResetDefaultExternalInputConnectionsAction.h"

struct SelectAction : public UndoableAction {
    SelectAction(Tracks &tracks, Connections &connections, View &view, Input &input,
                 ProcessorGraph &processorGraph);

    SelectAction(SelectAction *coalesceLeft, SelectAction *coalesceRight,
                 Tracks &tracks, Connections &connections, View &view, Input &input,
                 ProcessorGraph &processorGraph);

    void setNewFocusedSlot(juce::Point<int> newFocusedSlot, bool resetDefaultExternalInputs = true);

    ValueTree getNewFocusedTrack() { return tracks.getTrack(newFocusedSlot.x); }

    bool perform() override;

    bool undo() override;

    int getSizeInUnits() override { return (int) sizeof(*this); }

    UndoableAction *createCoalescedAction(UndoableAction *nextAction) override;

protected:
    bool canCoalesceWith(SelectAction *otherAction);

    void updateViewFocus(juce::Point<int> focusedSlot);

    bool changed();

    Tracks &tracks;
    Connections &connections;
    View &view;
    Input &input;
    ProcessorGraph &processorGraph;

    Array<String> oldSelectedSlotsMasks, newSelectedSlotsMasks;
    Array<bool> oldTrackSelections, newTrackSelections;
    juce::Point<int> oldFocusedSlot, newFocusedSlot;

    std::unique_ptr<ResetDefaultExternalInputConnectionsAction> resetInputsAction;
};
