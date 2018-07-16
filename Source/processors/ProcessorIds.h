#pragma once

#include "JuceHeader.h"
#include "GainProcessor.h"
#include "MixerChannelProcessor.h"
#include "SineBank.h"
#include "BalanceProcessor.h"
#include "InternalPluginFormat.h"

const static StringArray processorIdsWithoutMixer {GainProcessor::getIdentifier(), BalanceProcessor::getIdentifier(),
                                                   SineBank::getIdentifier() };
static StringArray allProcessorIds {MixerChannelProcessor::getIdentifier(), GainProcessor::getIdentifier(),
                                     BalanceProcessor::getIdentifier(), SineBank::getIdentifier() };

static const StringArray getAvailableProcessorIdsForTrack(const ValueTree& track) {
    if (!track.isValid()) {
        return StringArray();
    } else if (track.getChildWithProperty(IDs::name, MixerChannelProcessor::getIdentifier()).isValid()) {
        // at most one MixerChannel per track
        return processorIdsWithoutMixer;
    } else {
        return allProcessorIds;
    }
}

static AudioPluginInstance *createStatefulAudioProcessorFromId(const String &id) {
    if (id == MixerChannelProcessor::getIdentifier()) return new MixerChannelProcessor(MixerChannelProcessor::getPluginDescription());
    if (id == GainProcessor::getIdentifier()) return new GainProcessor(GainProcessor::getPluginDescription());
    if (id == BalanceProcessor::getIdentifier()) return new BalanceProcessor(BalanceProcessor::getPluginDescription());
    if (id == SineBank::getIdentifier()) return new SineBank(SineBank::getPluginDescription());
    return nullptr;
}

class ProcessorIds : private ChangeListener {
public:
    ProcessorIds() {
        PropertiesFile::Options options;
        options.applicationName = ProjectInfo::projectName;
        options.filenameSuffix = "settings";
        options.osxLibrarySubFolder = "Preferences";
        appProperties.setStorageParameters(options);

        InternalPluginFormat internalFormat;
        internalFormat.getAllTypes(internalTypes);
        std::unique_ptr<XmlElement> savedPluginList(appProperties.getUserSettings()->getXmlValue("pluginList"));

        if (savedPluginList != nullptr)
            knownPluginList.recreateFromXml(*savedPluginList);

        for (auto* pluginType : internalTypes)
            knownPluginList.addType(*pluginType);

        pluginSortMethod = (KnownPluginList::SortMethod) appProperties.getUserSettings()->getIntValue("pluginSortMethod", KnownPluginList::sortByManufacturer);
        knownPluginList.addChangeListener(this);

        formatManager.addDefaultFormats();
        formatManager.addFormat(new InternalPluginFormat());
    }

    ApplicationProperties& getApplicationProperties() {
        return appProperties;
    }

    PluginListComponent* makePluginListComponent() {
        const File &deadMansPedalFile = appProperties.getUserSettings()->getFile().getSiblingFile("RecentlyCrashedPluginsList");
        return new PluginListComponent(formatManager, knownPluginList, deadMansPedalFile, appProperties.getUserSettings(), true);
    }

    PluginDescription *getTypeForIdentifier(const String &identifier) {
        return knownPluginList.getTypeForIdentifierString(identifier);
    }

    KnownPluginList& getKnownPluginList() {
        return knownPluginList;
    }

    const KnownPluginList::SortMethod getPluginSortMethod() const {
        return pluginSortMethod;
    }

    AudioPluginFormatManager& getFormatManager() {
        return formatManager;
    }

    void setPluginSortMethod(const KnownPluginList::SortMethod pluginSortMethod) {
        this->pluginSortMethod = pluginSortMethod;
    }

    void addPluginsToMenu(PopupMenu& m) const {
        int i = 0;
        for (auto* t : internalTypes)
            m.addItem (++i, t->name + " (" + t->pluginFormatName + ")", true);

        m.addSeparator();

        knownPluginList.addToMenu(m, pluginSortMethod);
    }

    const PluginDescription* getChosenType(const int menuId) const {
        if (menuId >= 1 && menuId < 1 + internalTypes.size())
            return internalTypes[menuId - 1];

        return knownPluginList.getType(knownPluginList.getIndexChosenByMenu(menuId));
    }

private:
    OwnedArray<PluginDescription> internalTypes;
    KnownPluginList knownPluginList;
    KnownPluginList::SortMethod pluginSortMethod;
    AudioPluginFormatManager formatManager;
    ApplicationProperties appProperties;

    void changeListenerCallback(ChangeBroadcaster* changed) override {
        if (changed == &knownPluginList) {
            // save the plugin list every time it gets changed, so that if we're scanning
            // and it crashes, we've still saved the previous ones
            std::unique_ptr<XmlElement> savedPluginList(knownPluginList.createXml());

            if (savedPluginList != nullptr) {
                appProperties.getUserSettings()->setValue("pluginList", savedPluginList.get());
                appProperties.saveIfNeeded();
            }
        }
    }
};
