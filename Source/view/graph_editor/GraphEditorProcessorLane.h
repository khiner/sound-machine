#pragma once

#include "ValueTreeObjectList.h"
#include "GraphEditorProcessorContainer.h"
#include "ConnectorDragListener.h"
#include "GraphEditorChannel.h"

class GraphEditorProcessorLane : public Component, ValueTreeObjectList<BaseGraphEditorProcessor>, GraphEditorProcessorContainer {
public:
    explicit GraphEditorProcessorLane(const ValueTree &state, ViewState &view, TracksState &tracks, ProcessorGraph &processorGraph, ConnectorDragListener &connectorDragListener);

    ~GraphEditorProcessorLane() override;

    const ValueTree &getState() const { return state; }

    ValueTree getTrack() const { return parent.getParent().getParent(); }

    bool isMasterTrack() const { return TracksState::isMasterTrack(getTrack()); }

    int getNumSlots() const { return tracks.getNumSlotsForTrack(getTrack()); }

    int getSlotOffset() const { return tracks.getSlotOffsetForTrack(getTrack()); }

    void resized() override;

    void updateProcessorSlotColours();

    bool isSuitableType(const ValueTree &v) const override {
        return v.hasType(TracksStateIDs::PROCESSOR);
    }

    BaseGraphEditorProcessor *createEditorForProcessor(const ValueTree &processor);

    BaseGraphEditorProcessor *createNewObject(const ValueTree &processor) override;

    void deleteObject(BaseGraphEditorProcessor *graphEditorProcessor) override { delete graphEditorProcessor; }

    void newObjectAdded(BaseGraphEditorProcessor *processor) override {
        processor->addMouseListener(this, true);
        resized();
    }

    void objectRemoved(BaseGraphEditorProcessor *processor) override {
        processor->removeMouseListener(this);
        resized();
    }

    void objectOrderChanged() override { resized(); }

    BaseGraphEditorProcessor *getProcessorForNodeId(AudioProcessorGraph::NodeID nodeId) const override {
        for (auto *processor : objects)
            if (processor->getNodeId() == nodeId)
                return processor;
        return nullptr;
    }

    GraphEditorChannel *findChannelAt(const MouseEvent &e) const {
        for (auto *processor : objects)
            if (auto *channel = processor->findChannelAt(e))
                return channel;
        return nullptr;
    }

private:
    ValueTree state;

    ViewState &view;
    TracksState &tracks;

    ProcessorGraph &processorGraph;
    ConnectorDragListener &connectorDragListener;

    DrawableRectangle laneDragRectangle;
    OwnedArray<DrawableRectangle> processorSlotRectangles;

    BaseGraphEditorProcessor *findProcessorAtSlot(int slot) const {
        for (auto *processor : objects)
            if (processor->getSlot() == slot)
                return processor;
        return nullptr;
    }

    void valueTreePropertyChanged(ValueTree &tree, const Identifier &i) override;

    void valueTreeChildAdded(ValueTree &parent, ValueTree &child) override;
};
