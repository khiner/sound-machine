#pragma once

#include <Utilities.h>
#include <Identifiers.h>
#include <view/ColourChangeButton.h>
#include "JuceHeader.h"
#include "GraphEditorProcessors.h"

class GraphEditorTrack : public Component, public Utilities::ValueTreePropertyChangeListener, public GraphEditorProcessorContainer {
public:
    explicit GraphEditorTrack(Project& project, ValueTree v, ConnectorDragListener &connectorDragListener, ProcessorGraph& graph)
            : state(std::move(v)), connectorDragListener(connectorDragListener), graph(graph) {
        addAndMakeVisible(*(processors = std::make_unique<GraphEditorProcessors>(project, state, connectorDragListener, graph)));
        state.addListener(this);
    }

    Colour getColour() const {
        return Colour::fromString(state[IDs::colour].toString());
    }

    void resized() override {
        auto r = getLocalBounds();
        processors->setBounds(r);
    }

    void paint(Graphics &g) override {
//        g.setColour(getUIColourIfAvailable(LookAndFeel_V4::ColourScheme::UIColour::defaultText, Colours::black));
//        g.setFont(jlimit(8.0f, 15.0f, getHeight() * 0.9f));
//        g.drawText(state[IDs::name].toString(), r.removeFromLeft(clipX).reduced(4, 0), Justification::centredLeft, true);
    }

    GraphEditorProcessor *getProcessorForNodeId(const AudioProcessorGraph::NodeID nodeId) const override {
        if (auto *currentlyMovingProcessor = processors->getCurrentlyMovingProcessor()) {
            if (currentlyMovingProcessor->getNodeId() == nodeId) {
                return currentlyMovingProcessor;
            }
        }
        return processors->getProcessorForNodeId(nodeId);
    }

    PinComponent *findPinAt(const MouseEvent &e) const {
        return processors->findPinAt(e);
    }

    void updateProcessors() {
        processors->update();
    }

    void setCurrentlyMovingProcessor(GraphEditorProcessor *currentlyMovingProcessor) {
        processors->setCurrentlyMovingProcessor(currentlyMovingProcessor);
    }

    ValueTree state;
private:
    std::unique_ptr<GraphEditorProcessors> processors;
    ConnectorDragListener &connectorDragListener;
    ProcessorGraph &graph;

    void valueTreePropertyChanged(juce::ValueTree &v, const juce::Identifier &i) override {
        if (v == state)
            if (i == IDs::name || i == IDs::colour)
                repaint();
    }
};
