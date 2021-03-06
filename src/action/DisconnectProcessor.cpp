#include "DisconnectProcessor.h"

DisconnectProcessor::DisconnectProcessor(Connections &connections, const Processor *processor, ConnectionType connectionType, bool defaults, bool custom, bool incoming, bool outgoing,
                                         AudioProcessorGraph::NodeID excludingRemovalTo)
        : CreateOrDeleteConnections(connections) {
    const auto nodeConnections = connections.getConnectionsForNode(processor, connectionType, incoming, outgoing);
    for (const auto *connection : nodeConnections) {
        const auto destinationNodeId = connection->getDestinationNodeId();
        if (excludingRemovalTo != destinationNodeId) {
            coalesceWith(DeleteConnection(connection, custom, defaults, connections));
        }
    }
}
