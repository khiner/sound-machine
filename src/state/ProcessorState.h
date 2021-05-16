#pragma once

#include "Stateful.h"
#include "InputChannelsState.h"
#include "OutputChannelsState.h"
#include "ParamState.h"

namespace ProcessorStateIDs {
#define ID(name) const juce::Identifier name(#name);
    ID(PROCESSOR)
    ID(id)
    ID(name)
    ID(processorInitialized)
    ID(processorSlot)
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

class ProcessorState : public Stateful<ProcessorState> {
    static Identifier getIdentifier() { return ProcessorStateIDs::PROCESSOR; }
};
