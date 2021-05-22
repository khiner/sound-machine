#include "Connections.h"

static bool canProcessorDefaultConnectTo(const ValueTree &processor, const ValueTree &otherProcessor, ConnectionType connectionType) {
    return !(!Processor::isType(otherProcessor) || processor == otherProcessor) &&
           Connections::isProcessorAProducer(processor, connectionType) && Connections::isProcessorAnEffect(otherProcessor, connectionType);
}

ValueTree Connections::findDefaultDestinationProcessor(const ValueTree &sourceProcessor, ConnectionType connectionType) {
    if (!isProcessorAProducer(sourceProcessor, connectionType)) return {};

    const ValueTree &track = Track::getTrackForProcessor(sourceProcessor);
    const auto &masterTrack = tracks.getMasterTrack();
    if (Processor::isTrackOutputProcessor(sourceProcessor)) {
        if (track == masterTrack) return {};
        if (masterTrack.isValid()) return Track::getInputProcessor(masterTrack);
        return {};
    }

    bool isTrackInputProcessor = Processor::isTrackInputProcessor(sourceProcessor);
    const auto &lane = Track::getProcessorLaneForProcessor(sourceProcessor);
    int siblingDelta = 0;
    ValueTree otherLane;
    while ((otherLane = lane.getSibling(siblingDelta++)).isValid()) {
        for (const auto &otherProcessor : otherLane) {
            if (otherProcessor == sourceProcessor) continue;
            bool isOtherProcessorBelow = isTrackInputProcessor || Processor::getSlot(otherProcessor) > Processor::getSlot(sourceProcessor);
            if (!isOtherProcessorBelow) continue;
            if (canProcessorDefaultConnectTo(sourceProcessor, otherProcessor, connectionType) ||
                // If a non-effect (another producer) is under this processor in the same track, and no effect processors
                // to the right have a lower slot, block this producer's output by the other producer,
                // effectively replacing it.
                lane == otherLane) {
                return otherProcessor;
            }
        }
        // TODO adapt this when there are multiple lanes
        return Track::getOutputProcessor(track);
    }

    return {};
}

bool Connections::isNodeConnected(AudioProcessorGraph::NodeID nodeId) const {
    for (const auto &connection : state) {
        if (fg::Connection::getSourceNodeId(connection) == nodeId)
            return true;
    }
    return false;
}

static bool channelMatchesConnectionType(int channel, ConnectionType connectionType) {
    if (connectionType == all) return true;
    return (connectionType == audio && channel != AudioProcessorGraph::midiChannelIndex) ||
           (connectionType == midi && channel == AudioProcessorGraph::midiChannelIndex);
}

Array<ValueTree> Connections::getConnectionsForNode(const ValueTree &processor, ConnectionType connectionType, bool incoming, bool outgoing, bool includeCustom, bool includeDefault) {
    Array<ValueTree> nodeConnections;
    for (const auto &connection : state) {
        if ((fg::Connection::isCustom(connection) && !includeCustom) || (!fg::Connection::isCustom(connection) && !includeDefault))
            continue;

        auto processorNodeId = Processor::getNodeId(processor);
        if ((incoming && fg::Connection::getDestinationNodeId(connection) == processorNodeId && channelMatchesConnectionType(fg::Connection::getDestinationChannel(connection), connectionType)) ||
            (outgoing && fg::Connection::getSourceNodeId(connection) == processorNodeId && channelMatchesConnectionType(fg::Connection::getSourceChannel(connection), connectionType)))
            nodeConnections.add(connection);
    }

    return nodeConnections;
}

bool Connections::anyNonMasterTrackHasEffectProcessor(ConnectionType connectionType) {
    for (const auto &track : tracks.getState())
        if (!Track::isMaster(track))
            for (const auto &processor : Track::getProcessorLane(track))
                if (isProcessorAnEffect(processor, connectionType))
                    return true;
    return false;
}
