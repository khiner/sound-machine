#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include "model/Project.h"
#include "BaseGraphEditorProcessor.h"

class TrackInputGraphEditorProcessor : public BaseGraphEditorProcessor, private ChangeListener {
public:
    TrackInputGraphEditorProcessor(Processor *processor, Track *track, View &view, Project &project, StatefulAudioProcessorWrappers &processorWrappers, ConnectorDragListener &connectorDragListener);

    ~TrackInputGraphEditorProcessor() override;

    void setTrackName(const String &trackName) { nameLabel.setText(trackName, dontSendNotification); }

    void mouseDown(const MouseEvent &e) override;
    void mouseUp(const MouseEvent &e) override;
    void resized() override;
    void paint(Graphics &g) override;

    bool isInView() override { return true; }

    Rectangle<int> getBoxBounds() const override { return getLocalBounds(); }

private:
    Project &project;
    Label nameLabel;
    std::unique_ptr<ImageButton> audioMonitorToggle, midiMonitorToggle;
    StatefulAudioProcessorWrapper::Parameter *audioMonitorParameter;
    StatefulAudioProcessorWrapper::Parameter *midiMonitorParameter;

    void colourChanged() override { repaint(); }

    void valueTreePropertyChanged(ValueTree &v, const Identifier &i) override;

    void changeListenerCallback(ChangeBroadcaster *source) override;
};
