#include "OutputState.h"

#include "actions/CreateProcessorAction.h"
#include "processors/MidiOutputProcessor.h"

OutputState::OutputState(PluginManager &pluginManager, UndoManager &undoManager, AudioDeviceManager &deviceManager)
        : pluginManager(pluginManager), undoManager(undoManager), deviceManager(deviceManager) {
    output = ValueTree(TracksStateIDs::OUTPUT);
    output.addListener(this);
}

OutputState::~OutputState() {
    output.removeListener(this);
}

Array<ValueTree> OutputState::syncOutputDevicesWithDeviceManager() {
    Array<ValueTree> outputProcessorsToDelete;
    for (const auto &outputProcessor : output) {
        if (outputProcessor.hasProperty(TracksStateIDs::deviceName)) {
            const String &deviceName = outputProcessor[TracksStateIDs::deviceName];
            if (!MidiOutput::getDevices().contains(deviceName) || !deviceManager.isMidiOutputEnabled(deviceName)) {
                outputProcessorsToDelete.add(outputProcessor);
            }
        }
    }
    for (const auto &deviceName : MidiOutput::getDevices()) {
        if (deviceManager.isMidiOutputEnabled(deviceName) &&
            !output.getChildWithProperty(TracksStateIDs::deviceName, deviceName).isValid()) {
            auto midiOutputProcessor = CreateProcessorAction::createProcessor(MidiOutputProcessor::getPluginDescription());
            midiOutputProcessor.setProperty(TracksStateIDs::deviceName, deviceName, nullptr);
            output.addChild(midiOutputProcessor, -1, &undoManager);
        }
    }
    return outputProcessorsToDelete;
}

void OutputState::initializeDefault() {
    auto outputProcessor = CreateProcessorAction::createProcessor(pluginManager.getAudioOutputDescription());
    output.appendChild(outputProcessor, &undoManager);
}
