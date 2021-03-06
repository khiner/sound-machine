#include "Push2Colours.h"

Push2Colours::Push2Colours(Tracks &tracks) : tracks(tracks) {
    this->tracks.addChildListener(this);
    for (uint8 colourIndex = 1; colourIndex < CHAR_MAX - 1; colourIndex++) {
        availableColourIndexes.add(colourIndex);
    }
}

Push2Colours::~Push2Colours() {
    tracks.removeChildListener(this);
}

uint8 Push2Colours::findIndexForColourAddingIfNeeded(const Colour &colour) {
    auto entry = indexForColour.find(colour.toString());
    if (entry == indexForColour.end()) {
        addColour(colour);
        entry = indexForColour.find(colour.toString());
    }
    jassert(entry != indexForColour.end());
    return entry->second;
}

void Push2Colours::addColour(const Colour &colour) {
    jassert(!availableColourIndexes.isEmpty());
    setColour(availableColourIndexes.removeAndReturn(0), colour);
}

void Push2Colours::setColour(uint8 colourIndex, const Colour &colour) {
    jassert(colourIndex > 0 && colourIndex < CHAR_MAX - 1);
    indexForColour[colour.toString()] = colourIndex;
    listeners.call(&Listener::colourAdded, colour, colourIndex);
}

void Push2Colours::onChildAdded(Track *track) {
    const String &uuid = track->getUuid();
    const auto &colour = track->getColour();
    auto index = findIndexForColourAddingIfNeeded(colour);
    indexForTrackUuid[uuid] = index;
    listeners.call(&Listener::trackColourChanged, uuid, colour);
}
void Push2Colours::onChildRemoved(Track *track, int oldIndex) {
    const auto &uuid = track->getUuid();
    auto index = indexForTrackUuid[uuid];
    availableColourIndexes.add(index);
    indexForTrackUuid.erase(uuid);
}
void Push2Colours::onChildChanged(Track *track, const Identifier &i) {
    if (i == TrackIDs::colour) {
        const String &uuid = track->getUuid();
        auto index = indexForTrackUuid[uuid];
        const auto &colour = track->getColour();
        setColour(index, colour);
        listeners.call(&Listener::trackColourChanged, uuid, colour);
    }
}
