#include "UpdateAllDefaultConnections.h"

UpdateAllDefaultConnections::UpdateAllDefaultConnections(bool makeInvalidDefaultsIntoCustom, bool resetDefaultExternalInputConnections, Tracks &tracks, Connections &connections, Input &input,
                                                         Output &output, ProcessorGraph &processorGraph, Track *trackToTreatAsFocused)
        : CreateOrDeleteConnections(connections) {
    for (const auto *track : tracks.getChildren()) {
        for (const auto *processor : track->getAllProcessors())
            coalesceWith(UpdateProcessorDefaultConnections(processor, makeInvalidDefaultsIntoCustom, connections, output, processorGraph));
    }

    if (resetDefaultExternalInputConnections) {
        perform();
        auto resetAction = ResetDefaultExternalInputConnectionsAction(connections, tracks, input, processorGraph, trackToTreatAsFocused);
        undo();
        coalesceWith(resetAction);
    }
}