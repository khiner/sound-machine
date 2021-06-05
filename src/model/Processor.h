#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include "Stateful.h"
#include "InputChannels.h"
#include "OutputChannels.h"
#include "Param.h"
#include "Connection.h"
#include "processors/InternalPluginFormat.h"
#include "ConnectionType.h"
#include "view/PluginWindowType.h"

namespace ProcessorIDs {
#define ID(name) const juce::Identifier name(#name);
ID(PROCESSOR)
ID(id)
ID(name)
ID(initialized)
ID(slot)
ID(nodeId)
ID(bypassed)
ID(acceptsMidi)
ID(producesMidi)
ID(deviceName)
ID(state)
ID(allowDefaultConnections)
ID(pluginWindowType)
ID(pluginWindowX)
ID(pluginWindowY)
#undef ID
}

// TODO override loadFromState
struct Processor : public Stateful<Processor> {
    Processor(): Stateful<Processor>() {}
    explicit Processor(ValueTree state): Stateful<Processor>(std::move(state)) {}

    static ValueTree initState(const PluginDescription &description) {
        ValueTree state(getIdentifier());
        state.setProperty(ProcessorIDs::id, description.createIdentifierString(), nullptr);
        state.setProperty(ProcessorIDs::name, description.name, nullptr);
        state.setProperty(ProcessorIDs::allowDefaultConnections, true, nullptr);
        state.setProperty(ProcessorIDs::pluginWindowType, static_cast<int>(PluginWindowType::none), nullptr);
        return state;
    }

    static Identifier getIdentifier() { return ProcessorIDs::PROCESSOR; }

    void loadFromState(const ValueTree &fromState) override;

    String getId() const { return state[ProcessorIDs::id]; }
    int getIndex() const { return state.getParent().indexOf(state); }
    String getName() const { return state[ProcessorIDs::name]; }
    String getDeviceName() const { return state[ProcessorIDs::deviceName]; }
    int getSlot() const { return state[ProcessorIDs::slot]; }
    AudioProcessorGraph::NodeID getNodeId() const { return state.isValid() ? AudioProcessorGraph::NodeID(static_cast<uint32>(int(state[ProcessorIDs::nodeId]))) : AudioProcessorGraph::NodeID{}; }
    bool hasNodeId() const { return state.hasProperty(ProcessorIDs::nodeId); }
    String getProcessorState() const { return state[ProcessorIDs::state]; }
    bool hasProcessorState() const { return state.hasProperty(ProcessorIDs::state); }
    bool isInitialized() const { return state[ProcessorIDs::initialized]; }
    bool isBypassed() const { return state[ProcessorIDs::bypassed]; }
    bool acceptsMidi() const { return state[ProcessorIDs::acceptsMidi]; }
    bool producesMidi() const { return state[ProcessorIDs::producesMidi]; }
    bool allowsDefaultConnections() const { return state[ProcessorIDs::allowDefaultConnections]; }
    int getPluginWindowType() const { return state[ProcessorIDs::pluginWindowType]; }
    int getPluginWindowX() const { return state[ProcessorIDs::pluginWindowX]; }
    int getPluginWindowY() const { return state[ProcessorIDs::pluginWindowY]; }
    bool hasPluginWindowX() const { return state.hasProperty(ProcessorIDs::pluginWindowX); }
    bool hasPluginWindowY() const { return state.hasProperty(ProcessorIDs::pluginWindowY); }
    bool isTrackInputProcessor() const { return getName() == InternalPluginFormat::getTrackInputProcessorName(); }
    bool isTrackOutputProcessor() const { return getName() == InternalPluginFormat::getTrackOutputProcessorName(); }
    bool isAudioInputProcessor() const { return InternalPluginFormat::isAudioInputProcessor(getName()); }
    bool isAudioOutputProcessor() const { return InternalPluginFormat::isAudioOutputProcessor(getName()); }
    bool isMidiInputProcessor() const { return InternalPluginFormat::isMidiInputProcessor(getName()); }
    bool isMidiOutputProcessor() const { return InternalPluginFormat::isMidiOutputProcessor(getName()); }
    bool isTrackIOProcessor() const { return isTrackInputProcessor() || isTrackOutputProcessor(); }
    bool isIoProcessor() const { return InternalPluginFormat::isIoProcessor(state[ProcessorIDs::name]); }
    int getNumInputChannels() const { return state.getChildWithName(InputChannelsIDs::INPUT_CHANNELS).getNumChildren(); }
    int getNumOutputChannels() const { return state.getChildWithName(OutputChannelsIDs::OUTPUT_CHANNELS).getNumChildren(); }
    bool isProcessorAnEffect(ConnectionType connectionType) const {
        return (connectionType == audio && getNumInputChannels() > 0) || (connectionType == midi && acceptsMidi());
    }
    bool isProcessorAProducer(ConnectionType connectionType) const {
        return (connectionType == audio && getNumOutputChannels() > 0) || (connectionType == midi && producesMidi());
    }

    void setId(const String &id) { state.setProperty(ProcessorIDs::id, id, nullptr); }
    void setNodeId(AudioProcessorGraph::NodeID nodeId) { state.setProperty(ProcessorIDs::nodeId, int(nodeId.uid), nullptr); }
    void setName(const String &name) { state.setProperty(ProcessorIDs::name, name, nullptr); }
    void setDeviceName(const String &deviceName) { state.setProperty(ProcessorIDs::deviceName, deviceName, nullptr); }
    void setDeviceName(const int nodeId) { state.setProperty(ProcessorIDs::nodeId, nodeId, nullptr); }
    void setSlot(int slot) { state.setProperty(ProcessorIDs::slot, slot, nullptr); }
    void setProcessorState(const String &processorState) { state.setProperty(ProcessorIDs::state, processorState, nullptr); }
    void setInitialized(bool initialized) { state.setProperty(ProcessorIDs::initialized, initialized, nullptr); }
    void setBypassed(bool bypassed, UndoManager *undoManager = nullptr) { state.setProperty(ProcessorIDs::bypassed, bypassed, undoManager); }
    void setAcceptsMidi(bool acceptsMidi) { state.setProperty(ProcessorIDs::acceptsMidi, acceptsMidi, nullptr); }
    void setProducesMidi(bool producesMidi) { state.setProperty(ProcessorIDs::producesMidi, producesMidi, nullptr); }
    void setAllowsDefaultConnections(bool allowsDefaultConnections) { state.setProperty(ProcessorIDs::allowDefaultConnections, allowsDefaultConnections, nullptr); }
    void setPluginWindowType(int pluginWindowType, UndoManager *undoManager = nullptr) { state.setProperty(ProcessorIDs::pluginWindowType, pluginWindowType, undoManager); }
    void setPluginWindowX(int pluginWindowX) { state.setProperty(ProcessorIDs::pluginWindowX, pluginWindowX, nullptr); }
    void setPluginWindowY(int pluginWindowY) { state.setProperty(ProcessorIDs::pluginWindowY, pluginWindowY, nullptr); }

    static String getId(const ValueTree &state) { return state[ProcessorIDs::id]; }
    static int getIndex(const ValueTree &state) { return state.getParent().indexOf(state); }
    static String getName(const ValueTree &state) { return state[ProcessorIDs::name]; }
    static String getDeviceName(const ValueTree &state) { return state[ProcessorIDs::deviceName]; }
    static int getSlot(const ValueTree &state) { return state[ProcessorIDs::slot]; }
    static AudioProcessorGraph::NodeID getNodeId(const ValueTree &state) { return state.isValid() ? AudioProcessorGraph::NodeID(static_cast<uint32>(int(state[ProcessorIDs::nodeId]))) : AudioProcessorGraph::NodeID{}; }
    static bool isBypassed(const ValueTree &state) { return state[ProcessorIDs::bypassed]; }
    static bool producesMidi(const ValueTree &state) { return state[ProcessorIDs::producesMidi]; }
    static bool acceptsMidi(const ValueTree &state) { return state[ProcessorIDs::acceptsMidi]; }
    static int getNumInputChannels(const ValueTree &state) { return state.getChildWithName(InputChannelsIDs::INPUT_CHANNELS).getNumChildren(); }
    static int getNumOutputChannels(const ValueTree &state) { return state.getChildWithName(OutputChannelsIDs::OUTPUT_CHANNELS).getNumChildren(); }

    static bool isIoProcessor(const ValueTree &state) { return InternalPluginFormat::isIoProcessor(state[ProcessorIDs::name]); }
    static bool isTrackInputProcessor(const ValueTree &state) { return getName(state) == InternalPluginFormat::getTrackInputProcessorName(); }
    static bool isTrackOutputProcessor(const ValueTree &state) { return getName(state) == InternalPluginFormat::getTrackOutputProcessorName(); }
    static bool isTrackIOProcessor(const ValueTree &state) { return isTrackInputProcessor(state) || isTrackOutputProcessor(state); }
    static bool isMidiInputProcessor(const ValueTree &state) { return InternalPluginFormat::isMidiInputProcessor(getName(state)); }
    static bool isMidiOutputProcessor(const ValueTree &state) { return InternalPluginFormat::isMidiOutputProcessor(getName(state)); }

    static void setId(ValueTree &state, const String &id) { state.setProperty(ProcessorIDs::id, id, nullptr); }
    static void setProcessorState(ValueTree &state, const String &processorState) { state.setProperty(ProcessorIDs::state, processorState, nullptr); }
    static void setNodeId(ValueTree &state, AudioProcessorGraph::NodeID nodeId) { state.setProperty(ProcessorIDs::nodeId, int(nodeId.uid), nullptr); }
    static void setName(ValueTree &state, const String &name) { state.setProperty(ProcessorIDs::name, name, nullptr); }
    static void setDeviceName(ValueTree &state, const String &deviceName) { state.setProperty(ProcessorIDs::deviceName, deviceName, nullptr); }
    static void setSlot(ValueTree &state, int slot) { state.setProperty(ProcessorIDs::slot, slot, nullptr); }
    static void setProducesMidi(ValueTree &state, bool producesMidi) { state.setProperty(ProcessorIDs::producesMidi, producesMidi, nullptr); }
    static void setAcceptsMidi(ValueTree &state, bool acceptsMidi) { state.setProperty(ProcessorIDs::acceptsMidi, acceptsMidi, nullptr); }
    static void setAllowsDefaultConnections(ValueTree &state, bool allowsDefaultConnections) { state.setProperty(ProcessorIDs::allowDefaultConnections, allowsDefaultConnections, nullptr); }
    static void setPluginWindowX(ValueTree &state, int pluginWindowX) { state.setProperty(ProcessorIDs::pluginWindowX, pluginWindowX, nullptr); }
    static void setPluginWindowY(ValueTree &state, int pluginWindowY) { state.setProperty(ProcessorIDs::pluginWindowY, pluginWindowY, nullptr); }

    static AudioProcessorGraph::Connection toProcessorGraphConnection(const ValueTree &state) {
        return {{fg::Connection::getSourceNodeId(state),      fg::Connection::getSourceChannel(state)},
                {fg::Connection::getDestinationNodeId(state), fg::Connection::getDestinationChannel(state)}};
    }

    static bool isProcessorAnEffect(const ValueTree &state, ConnectionType connectionType) {
        return (connectionType == audio && Processor::getNumInputChannels(state) > 0) ||
               (connectionType == midi && Processor::acceptsMidi(state));
    }

    static bool isProcessorAProducer(const ValueTree &state, ConnectionType connectionType) {
        return (connectionType == audio && Processor::getNumOutputChannels(state) > 0) ||
               (connectionType == midi && Processor::producesMidi(state));
    }
};
