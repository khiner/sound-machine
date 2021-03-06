#pragma once

#include "model/Input.h"
#include "CreateOrDeleteConnections.h"
#include "ProcessorGraph.h"

// For both audio and midi connection types:
//   * Find the topmost effect processor (receiving audio/midi) in the focused track
//   * Connect external device inputs to its most-upstream connected processor (including itself) that doesn't already have incoming connections
// (Note that it is possible for the same focused track to have a default audio-input processor different
// from its default midi-input processor.)

struct ResetDefaultExternalInputConnectionsAction : public CreateOrDeleteConnections {
    ResetDefaultExternalInputConnectionsAction(Connections &connections, Tracks &tracks, Input &input, AllProcessors &allProcessors, ProcessorGraph &processorGraph, Track *trackToTreatAsFocused = nullptr);

private:
    // Find the upper-right-most effect processor that flows into the given processor
    // which doesn't already have incoming node connections.
    const Processor *findMostUpstreamAvailableProcessorConnectedTo(const Processor *processor, ConnectionType connectionType, Tracks &tracks, Input &input);
    bool isAvailableForExternalInput(const Processor *processor, ConnectionType connectionType, Input &input);
    bool areProcessorsConnected(AudioProcessorGraph::NodeID upstreamNodeId, AudioProcessorGraph::NodeID downstreamNodeId);
};
