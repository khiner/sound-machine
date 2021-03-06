#pragma once

#include "processors/InternalPluginFormat.h"

class PluginManager : private ChangeListener {
public:
    PluginManager();

    std::unique_ptr<PluginDescription> getDescriptionForIdentifier(const String &identifier);
    PluginDescription &getAudioInputDescription() { return internalFormat.audioInDesc; }
    PluginDescription &getAudioOutputDescription() { return internalFormat.audioOutDesc; }
    Array<PluginDescription> &getInternalPluginDescriptions() { return internalFormat.getInternalPluginDescriptions(); }
    Array<PluginDescription> &getExternalPluginDescriptions() { return externalPluginDescriptions; }
    KnownPluginList::SortMethod getPluginSortMethod() const { return pluginSortMethod; }
    AudioPluginFormatManager &getFormatManager() { return formatManager; }
    PluginDescription getChosenType(int menuId);

    PluginListComponent *makePluginListComponent();
    void setPluginSortMethod(const KnownPluginList::SortMethod pluginSortMethod) { this->pluginSortMethod = pluginSortMethod; }
    void addPluginsToMenu(PopupMenu &menu);

    static bool isGeneratorOrInstrument(const PluginDescription *description) {
        return description->isInstrument || description->category.equalsIgnoreCase("generator") || description->category.equalsIgnoreCase("synth");
    }

private:
    const String PLUGIN_LIST_FILE_NAME = "pluginList";

    InternalPluginFormat internalFormat;
    KnownPluginList knownPluginListExternal;
    KnownPluginList knownPluginListInternal;
    KnownPluginList userCreatablePluginListInternal;

    KnownPluginList::SortMethod pluginSortMethod;
    AudioPluginFormatManager formatManager;

    Array<PluginDescription> externalPluginDescriptions;
    Array<PluginDescription> userCreatableInternalPluginDescriptions;

    void changeListenerCallback(ChangeBroadcaster *changed) override;
};
