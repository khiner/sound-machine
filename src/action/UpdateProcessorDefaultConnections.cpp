#include "UpdateProcessorDefaultConnections.h"

#include "DefaultConnectProcessor.h"
#include "DisconnectProcessor.h"

UpdateProcessorDefaultConnections::UpdateProcessorDefaultConnections(const Processor *processor, bool makeInvalidDefaultsIntoCustom,
                                                                     Connections &connections, Output &output, AllProcessors &allProcessors, ProcessorGraph &processorGraph)
        : CreateOrDeleteConnections(connections) {
    for (auto connectionType : {audio, midi}) {
        auto customOutgoingConnections = connections.getConnectionsForNode(processor, connectionType, false, true, true, false);
        if (!customOutgoingConnections.isEmpty()) continue;

        auto *processorToConnectTo = connections.findDefaultDestinationProcessor(processor, connectionType);
        auto nodeIdToConnectTo = processorToConnectTo == nullptr ? Processor::getNodeId(output.getAudioOutputProcessorState()) : processorToConnectTo->getNodeId();
        auto disconnectDefaultsAction = DisconnectProcessor(connections, processor, connectionType, true, false, false, true, nodeIdToConnectTo);
        coalesceWith(disconnectDefaultsAction);
        if (makeInvalidDefaultsIntoCustom) {
            if (!disconnectDefaultsAction.connectionsToDelete.isEmpty()) {
                for (const auto *connectionToConvert : disconnectDefaultsAction.connectionsToDelete) {
                    auto customConnection = std::make_unique<Connection>(connectionToConvert);
                    customConnection->setCustom(true);
                    connectionsToCreate.add(std::move(customConnection));
                }
            }
        } else {
            coalesceWith(DefaultConnectProcessor(processor, nodeIdToConnectTo, connectionType, connections, allProcessors, processorGraph));
        }
    }
}
