#pragma once

#include <Utilities.h>
#include "JuceHeader.h"

class Stateful {
public:
    virtual void loadFromState(const ValueTree& state) = 0;

    virtual ValueTree& getState() = 0;

    virtual void clear() {
        getState().removeAllChildren(nullptr);
    }

    void addListener(ValueTree::Listener *listener) {
        getState().addListener(listener);
    }

    void removeListener(ValueTree::Listener *listener) {
        getState().removeListener(listener);
    }

    static void resetVarToInt(ValueTree& tree, const Identifier& id, ValueTree::Listener* listenerToExclude) {
        tree.setPropertyExcludingListener(listenerToExclude, id, int(tree.getProperty(id)), nullptr);
    };
    static void resetVarToBool(ValueTree& tree, const Identifier& id, ValueTree::Listener* listenerToExclude) {
        tree.setPropertyExcludingListener(listenerToExclude, id, bool(tree.getProperty(id)), nullptr);
    };

    static void resetVarAllowingNoopUndo(ValueTree& tree, const Identifier& id, UndoManager *undoManager) {
        tree.setProperty(id, tree.getProperty(id), undoManager, true);
    }
};