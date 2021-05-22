#include "BaseGraphEditorProcessor.h"

BaseGraphEditorProcessor::BaseGraphEditorProcessor(const ValueTree &state, View &view, Tracks &tracks,
                                                   ProcessorGraph &processorGraph, ConnectorDragListener &connectorDragListener)
        : state(state), view(view), tracks(tracks), processorGraph(processorGraph), connectorDragListener(connectorDragListener) {
    this->state.addListener(this);

    for (auto child : state) {
        if (InputChannels::isType(child) || OutputChannels::isType(child)) {
            for (auto channel : child) {
                // TODO shouldn't have to do this kind of thing
                valueTreeChildAdded(child, channel);
            }
        }
    }
}

BaseGraphEditorProcessor::~BaseGraphEditorProcessor() {
    state.removeListener(this);
}

void BaseGraphEditorProcessor::paint(Graphics &g) {
    auto boxColour = findColour(TextEditor::backgroundColourId);
    if (Processor::isBypassed(state))
        boxColour = boxColour.brighter();
    else if (isSelected())
        boxColour = boxColour.brighter(0.02f);

    g.setColour(boxColour);
    g.fillRect(getBoxBounds());
}

bool BaseGraphEditorProcessor::hitTest(int x, int y) {
    for (auto *child : getChildren())
        if (child->getBounds().contains(x, y))
            return true;

    return x >= 3 && x < getWidth() - 6 && y >= channelSize && y < getHeight() - channelSize;
}

void BaseGraphEditorProcessor::resized() {
    if (auto *processor = processorGraph.getAudioProcessorForState(state)) {
        for (auto *channel : channels)
            layoutChannel(processor, channel);
        repaint();
        connectorDragListener.update();
    }
}

juce::Point<float> BaseGraphEditorProcessor::getConnectorDirectionVector(bool isInput) const {
    bool isLeftToRight = Tracks::isProcessorLeftToRightFlowing(getState());
    if (isInput) {
        if (isLeftToRight) return {-1, 0};
        return {0, -1};
    }
    if (isLeftToRight) return {1, 0};
    return {0, 1};
}

Rectangle<int> BaseGraphEditorProcessor::getBoxBounds() const {
    auto r = getLocalBounds().reduced(1);
    bool isLeftToRight = Tracks::isProcessorLeftToRightFlowing(getState());
    if (isLeftToRight) {
        if (getNumInputChannels() > 0)
            r.setLeft(channelSize);
        if (getNumOutputChannels() > 0)
            r.removeFromRight(channelSize);
    } else {
        if (getNumInputChannels() > 0)
            r.setTop(channelSize);
        if (getNumOutputChannels() > 0)
            r.removeFromBottom(channelSize);
    }
    return r;
}

void BaseGraphEditorProcessor::layoutChannel(AudioProcessor *processor, GraphEditorChannel *channel) const {
    const auto boxBounds = getBoxBounds();
    const bool isInput = channel->isInput();
    auto channelIndex = channel->getChannelIndex();
    int busIndex = 0;
    processor->getOffsetInBusBufferForAbsoluteChannelIndex(isInput, channelIndex, busIndex);
    int total = isInput ? getNumInputChannels() : getNumOutputChannels();
    const int index = channel->isMidi() ? (total - 1) : channelIndex;
    auto indexPosition = index + busIndex / 2;

    int x = boxBounds.getX() + static_cast<int>(indexPosition * channelSize);
    int y = boxBounds.getY() + static_cast<int>(indexPosition * channelSize);
    if (Tracks::isProcessorLeftToRightFlowing(getState()))
        channel->setSize(static_cast<int>(boxBounds.getWidth() / 2), channelSize);
    else
        channel->setSize(channelSize, static_cast<int>(boxBounds.getHeight() / 2));

    const auto &connectionDirection = getConnectorDirectionVector(channel->isInput());
    if (connectionDirection == juce::Point(-1.0f, 0.0f))
        channel->setTopLeftPosition(static_cast<int>(boxBounds.getX()), y);
    else if (connectionDirection == juce::Point(1.0f, 0.0f))
        channel->setTopRightPosition(static_cast<int>(boxBounds.getRight()), y);
    else if (connectionDirection == juce::Point(0.0f, -1.0f))
        channel->setTopLeftPosition(x, boxBounds.getY());
    else
        channel->setTopLeftPosition(x, boxBounds.getBottom() - channel->getHeight());
}

void BaseGraphEditorProcessor::valueTreeChildAdded(ValueTree &parent, ValueTree &child) {
    if (child.hasType(ChannelIDs::CHANNEL)) {
        auto *channel = new GraphEditorChannel(child, connectorDragListener, Processor::isIoProcessor(state));
        addAndMakeVisible(channel);
        channels.add(channel);
        resized();
    }
}

void BaseGraphEditorProcessor::valueTreeChildRemoved(ValueTree &parent, ValueTree &child, int) {
    if (child.hasType(ChannelIDs::CHANNEL)) {
        auto *channelToRemove = findChannelWithState(child);
        channels.removeObject(channelToRemove);
        resized();
    }
}

void BaseGraphEditorProcessor::valueTreePropertyChanged(ValueTree &v, const Identifier &i) {
    if (v != state) return;

    if (i == ProcessorIDs::processorInitialized)
        resized();
}
