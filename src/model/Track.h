#pragma once

#include "Stateful.h"
#include "ProcessorLanes.h"

namespace TrackIDs {
#define ID(name) const juce::Identifier name(#name);
ID(TRACK)
ID(uuid)
ID(colour)
ID(name)
ID(selected)
ID(isMaster)
#undef ID
}

struct Track : public Stateful<Track>, private ProcessorLanes::Listener, private ProcessorLane::Listener, private ValueTree::Listener {
    struct Listener {
        virtual void trackPropertyChanged(Track *track, const Identifier &i) {}
        virtual void processorAdded(Processor *) = 0;
        virtual void processorRemoved(Processor *, int oldIndex) = 0;
        virtual void processorPropertyChanged(Processor *track, const Identifier &i) {}
    };

    void addTrackListener(Listener *listener) {
        listeners.add(listener);
    }
    void removeTrackListener(Listener *listener) { listeners.remove(listener); }

    Track(): Stateful<Track>() {
        state.appendChild(processorLanes.getState(), nullptr);
        processorLanes.addProcessorLanesListener(this);
        for (auto *lane : processorLanes.getChildren()) {
            processorLaneAdded(lane);
        }
        state.addListener(this);
    }
    explicit Track(ValueTree state): Stateful<Track>(std::move(state)), processorLanes(this->state.getChildWithName(ProcessorLanesIDs::PROCESSOR_LANES).isValid() ? this->state.getChildWithName(ProcessorLanesIDs::PROCESSOR_LANES) : ValueTree(ProcessorLanesIDs::PROCESSOR_LANES)) {
        processorLanes.addProcessorLanesListener(this);
        for (auto *lane : processorLanes.getChildren()) {
            processorLaneAdded(lane);
        }
        this->state.addListener(this);
    }

    ~Track() override {
        state.removeListener(this);
        processorLanes.removeProcessorLanesListener(this);
    }

    Track copy() const { return Track(state); }

    static Identifier getIdentifier() { return TrackIDs::TRACK; }

    static BigInteger createFullSelectionBitmask(int numSlots) {
        BigInteger selectedSlotsMask;
        selectedSlotsMask.setRange(0, numSlots, true);
        return selectedSlotsMask;
    }

    int getIndex() const { return state.getParent().indexOf(state); }
    String getUuid() const { return state[TrackIDs::uuid]; }
    Colour getColour() const { return Colour::fromString(state[TrackIDs::colour].toString()); }
    Colour getDisplayColour() const { return isSelected() ? getColour().brighter(0.25) : getColour(); }
    String getName() const { return state[TrackIDs::name]; }
    bool isSelected() const { return state[TrackIDs::selected]; }
    bool isMaster() const { return state[TrackIDs::isMaster]; }
    BigInteger getSlotMask() const { return getProcessorLane()->getSelectedSlotsMask(); }
    bool isSlotSelected(int slot) const { return getSlotMask()[slot]; }
    int firstSelectedSlot() const { return getSlotMask().getHighestBit(); }
    bool hasAnySlotSelected() const { return firstSelectedSlot() != -1; }
    bool hasSelections() const { return isSelected() || hasAnySlotSelected(); }
    bool hasProducerProcessor() const {
        for (const auto &processor : getProcessorLane()->getState())
            if (Processor::isProcessorAProducer(processor, audio) || Processor::isProcessorAProducer(processor, midi))
                return true;
        return false;
    }

    Processor *getProcessorAtSlot(int slot) const {
        const auto *lane = getProcessorLane();
        if (lane == nullptr) return nullptr;
        for (auto *processor : getProcessorLane()->getChildren())
            if (processor->getSlot() == slot)
                return processor;
        return nullptr;
    }
    ValueTree getProcessorStateAtSlot(int slot) const {
        const auto *lane = getProcessorLane();
        return lane != nullptr ? lane->getState().getChildWithProperty(ProcessorIDs::slot, slot) : ValueTree();
    }
    int getInsertIndexForProcessor(Processor *processor, const ProcessorLane *lane, int insertSlot) const;
    int getNumProcessors() const {
        const auto *lane = getProcessorLane();
        return lane != nullptr ? lane->size() : 0;
    }
    Processor *findProcessorNearestToSlot(int slot) const;
    Processor *findFirstSelectedProcessor() const;
    Processor *findLastSelectedProcessor() const;
    Array<Processor *> findSelectedProcessors() const;
    Array<Processor *> getAllProcessors() const;

    ProcessorLanes &getProcessorLanes() { return processorLanes; }
    ProcessorLane *getProcessorLane() const { return processorLanes.getChild(0); }
    ProcessorLane *getProcessorLaneAt(int index) const { return processorLanes.getChild(index); }
    Processor *getInputProcessor() const { return audioInputProcessor.get(); }
    Processor *getOutputProcessor() const { return audioOutputProcessor.get(); }

    Processor *getFirstProcessor() const {
        const auto *lane = getProcessorLane();
        auto *processor = lane->getChild(0);
        if (processor == nullptr) return nullptr;
        return processor;
    }
    ValueTree getFirstProcessorState() const {
        const auto *lane = getProcessorLane();
        const auto *processor = lane->getChild(0);
        if (processor == nullptr) return {};
        return processor->getState();
    }
    bool isProcessorSelected(const Processor *processor) const {
        return processor != nullptr && isSlotSelected(processor->getSlot());
    }
    bool isProcessorLeftToRightFlowing(const Processor *processor) const {
        return processor != nullptr && isMaster() && !processor->isTrackIOProcessor();
    }

    void setUuid(const String &uuid) { state.setProperty(TrackIDs::uuid, uuid, nullptr); }
    void setName(const String &name, UndoManager *undoManager = nullptr) {
        if (undoManager) undoManager->beginNewTransaction();
        state.setProperty(TrackIDs::name, name, undoManager);
    }
    void setColour(const Colour &colour, UndoManager *undoManager = nullptr) { state.setProperty(TrackIDs::colour, colour.toString(), undoManager); }
    void setSelected(bool selected) { state.setProperty(TrackIDs::selected, selected, nullptr); }
    void setMaster(bool isMaster) { state.setProperty(TrackIDs::isMaster, isMaster, nullptr); }

private:
    ProcessorLanes processorLanes;
    ListenerList<Listener> listeners;
    std::unique_ptr<Processor> audioInputProcessor, audioOutputProcessor, midiInputProcessor, midiOutputProcessor;

    void processorLaneAdded(ProcessorLane *processorLane) override {
        processorLane->addProcessorLaneListener(this);
    }
    void processorLaneRemoved(ProcessorLane *processorLane, int oldIndex) override {
        processorLane->removeProcessorLaneListener(this);
    }

    void processorAdded(Processor *processor) override {
        if (processor->isTrackInputProcessor()) audioInputProcessor.reset(processor);
        else if (processor->isTrackOutputProcessor()) audioOutputProcessor.reset(processor);
        else if (processor->isMidiInputProcessor()) midiInputProcessor.reset(processor);
        else if (processor->isMidiOutputProcessor()) midiOutputProcessor.reset(processor);
        listeners.call([processor](Listener &l) { l.processorAdded(processor); });
    }
    void processorRemoved(Processor *processor, int oldIndex) override {
        if (processor == audioInputProcessor.get()) audioInputProcessor.reset();
        else if (processor == audioOutputProcessor.get()) audioOutputProcessor.reset();
        else if (processor == midiInputProcessor.get()) midiInputProcessor.reset();
        else if (processor == midiOutputProcessor.get()) midiOutputProcessor.reset();
        listeners.call([processor, oldIndex](Listener &l) { l.processorRemoved(processor, oldIndex); });
    }
    void processorPropertyChanged(Processor *processor, const Identifier &i) override {
        listeners.call([processor, i](Listener &l) { l.processorPropertyChanged(processor, i); });
    }

    void valueTreeChildAdded(ValueTree &parent, ValueTree &tree) override {
        if (Processor::isType(tree) && parent == getState()) {
            processorAdded(new Processor(tree));
        }
    }
    void valueTreeChildRemoved(ValueTree &exParent, ValueTree &tree, int oldIndex) override {
        if (Processor::isType(tree) && exParent == getState()) {
            if (audioInputProcessor != nullptr && audioInputProcessor->getState() == tree) processorRemoved(audioInputProcessor.get(), oldIndex);
            else if (audioOutputProcessor != nullptr && audioOutputProcessor->getState() == tree) processorRemoved(audioOutputProcessor.get(), oldIndex);
            else if (midiInputProcessor != nullptr && midiInputProcessor->getState() == tree) processorRemoved(midiInputProcessor.get(), oldIndex);
            else if (midiOutputProcessor != nullptr && midiOutputProcessor->getState() == tree) processorRemoved(midiOutputProcessor.get(), oldIndex);
        }
    }
    void valueTreePropertyChanged(ValueTree &tree, const Identifier &i) override {
        if (isType(tree)) {
            listeners.call([this, i](Listener &l) { l.trackPropertyChanged(this, i); });
        } else if (Processor::isType(tree) && tree.getParent() == getState()) {
            if (audioInputProcessor != nullptr && audioInputProcessor->getState() == tree) processorPropertyChanged(audioInputProcessor.get(), i);
            else if (audioOutputProcessor != nullptr && audioOutputProcessor->getState() == tree) processorPropertyChanged(audioOutputProcessor.get(), i);
            else if (midiInputProcessor != nullptr && midiInputProcessor->getState() == tree) processorPropertyChanged(midiInputProcessor.get(), i);
            else if (midiOutputProcessor != nullptr && midiOutputProcessor->getState() == tree) processorPropertyChanged(midiOutputProcessor.get(), i);
        }
    }
};