#include "GraphEditorProcessorLane.h"

#include "view/graph_editor/processor/LabelGraphEditorProcessor.h"
#include "view/graph_editor/processor/ParameterPanelGraphEditorProcessor.h"

GraphEditorProcessorLane::GraphEditorProcessorLane(const ValueTree &state, View &view, Tracks &tracks, ProcessorGraph &processorGraph, ConnectorDragListener &connectorDragListener)
        : ValueTreeObjectList<BaseGraphEditorProcessor>(state),
          state(state), view(view), tracks(tracks),
          processorGraph(processorGraph), connectorDragListener(connectorDragListener) {
    rebuildObjects();
    view.addListener(this);
    // TODO shouldn't need to do this
    valueTreePropertyChanged(view.getState(), Track::isMaster(getTrack()) ? ViewIDs::numMasterProcessorSlots : ViewIDs::numProcessorSlots);
    addAndMakeVisible(laneDragRectangle);
    setInterceptsMouseClicks(false, false);
}

GraphEditorProcessorLane::~GraphEditorProcessorLane() {
    view.removeListener(this);
    freeObjects();
}

void GraphEditorProcessorLane::resized() {
    auto slotOffset = getSlotOffset();
    auto processorSlotSize = tracks.getProcessorSlotSize(getTrack());
    auto r = getLocalBounds();
    if (Track::isMaster(getTrack())) {
        r.setWidth(processorSlotSize);
        r.setX(-slotOffset * processorSlotSize - View::TRACK_LABEL_HEIGHT);
    } else {
        auto laneHeaderBounds = r.removeFromTop(View::LANE_HEADER_HEIGHT).reduced(0, 2);
        laneDragRectangle.setRectangle(laneHeaderBounds.toFloat());

        r.setHeight(processorSlotSize);
        r.setY(r.getY() - slotOffset * processorSlotSize - View::TRACK_LABEL_HEIGHT);
    }

    bool isMaster = Track::isMaster(getTrack());
    for (int slot = 0; slot < processorSlotRectangles.size(); slot++) {
        if (slot == slotOffset) {
            if (isMaster)
                r.setX(r.getX() + View::TRACK_LABEL_HEIGHT);
            else
                r.setY(r.getY() + View::TRACK_LABEL_HEIGHT);
        } else if (slot == slotOffset + View::NUM_VISIBLE_NON_MASTER_TRACK_SLOTS + (isMaster ? 1 : 0)) {
            if (isMaster)
                r.setX(r.getRight());
            else
                r.setY(r.getBottom());
        }
        processorSlotRectangles.getUnchecked(slot)->setRectangle(r.reduced(1).toFloat());
        if (auto *processor = findProcessorAtSlot(slot))
            processor->setBounds(r);
        if (isMaster)
            r.setX(r.getRight());
        else
            r.setY(r.getBottom());
    }
}

void GraphEditorProcessorLane::updateProcessorSlotColours() {
    const static auto &baseColour = findColour(ResizableWindow::backgroundColourId).brighter(0.4f);
    const auto &track = getTrack();

    laneDragRectangle.setFill(Track::getColour(track));
    for (int slot = 0; slot < processorSlotRectangles.size(); slot++) {
        auto fillColour = baseColour;
        if (Track::hasSelections(track))
            fillColour = fillColour.brighter(0.2f);
        if (Track::isSlotSelected(track, slot))
            fillColour = Track::getColour(track);
        if (tracks.isSlotFocused(track, slot))
            fillColour = fillColour.brighter(0.16f);
        if (!tracks.isProcessorSlotInView(track, slot))
            fillColour = fillColour.darker(0.3f);
        processorSlotRectangles.getUnchecked(slot)->setFill(fillColour);
    }
}

BaseGraphEditorProcessor *GraphEditorProcessorLane::createEditorForProcessor(const ValueTree &processor) {
    if (Processor::getName(processor) == InternalPluginFormat::getMixerChannelProcessorName()) {
        return new ParameterPanelGraphEditorProcessor(processor, view, tracks, processorGraph, connectorDragListener);
    }
    return new LabelGraphEditorProcessor(processor, view, tracks, processorGraph, connectorDragListener);
}

BaseGraphEditorProcessor *GraphEditorProcessorLane::createNewObject(const ValueTree &processor) {
    auto *graphEditorProcessor = createEditorForProcessor(processor);
    addAndMakeVisible(graphEditorProcessor);
    return graphEditorProcessor;
}

// TODO only instantiate 64 slot rects (and maybe another set for the boundary perimeter)
//  might be an over-early optimization though
void GraphEditorProcessorLane::valueTreePropertyChanged(ValueTree &tree, const Identifier &i) {
    bool isMaster = Track::isMaster(getTrack());
    if (isSuitableType(tree) && i == ProcessorIDs::slot) {
        resized();
    } else if (i == TrackIDs::selected || i == TrackIDs::colour ||
               i == ProcessorLaneIDs::selectedSlotsMask || i == ViewIDs::focusedTrackIndex || i == ViewIDs::focusedProcessorSlot ||
               i == ViewIDs::gridViewTrackOffset) {
        updateProcessorSlotColours();
    } else if (i == ViewIDs::gridViewSlotOffset || (i == ViewIDs::masterViewSlotOffset && isMaster)) {
        resized();
        updateProcessorSlotColours();
    } else if (i == ViewIDs::numProcessorSlots || (i == ViewIDs::numMasterProcessorSlots && isMaster)) {
        auto numSlots = getNumSlots();
        while (processorSlotRectangles.size() < numSlots) {
            auto *rect = new DrawableRectangle();
            processorSlotRectangles.add(rect);
            addAndMakeVisible(rect);
            rect->setCornerSize({3, 3});
            rect->toBack();
        }
        processorSlotRectangles.removeLast(processorSlotRectangles.size() - numSlots, true);
        resized();
        updateProcessorSlotColours();
    }
    ValueTreeObjectList<BaseGraphEditorProcessor>::valueTreePropertyChanged(tree, i);
}

void GraphEditorProcessorLane::valueTreeChildAdded(ValueTree &parent, ValueTree &child) {
    ValueTreeObjectList::valueTreeChildAdded(parent, child);
    if (this->parent == parent && isSuitableType(child))
        resized();
}
