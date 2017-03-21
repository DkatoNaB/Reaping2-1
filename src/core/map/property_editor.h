#ifndef INCLUDED_MAP_PROPERTY_EDITOR_H
#define INCLUDED_MAP_PROPERTY_EDITOR_H

#include "platform/singleton.h"

namespace map {
class PropertyEditor : public platform::Singleton<PropertyEditor>
{
    friend class platform::Singleton<PropertyEditor>;
    PropertyEditor();
public:
    /// Draws ui for
    /// - creating a new property: type
    ///     note: implement in propertyfactory!
    ///     start, end, spawn, cell_entrance
    /// - selecting the active prop from a combo box of existing props
    ///     room spec, implement here!
    /// - modify prop properties
    ///     prop-specific ui, implemented in the prop itself (dirs,pos,actor list)
    ///     (pick cell)
    void DrawUI();
private:
    /// var for keeping track of the edited room
    //Opt<IRoom> mRoom;
    /// cache for existing properties in the room (?)
    void DrawExistingProperties();
    /// the currently selected property (index?)
    int mCurrentProperty = 0;
    int mNewPropType = 0;
};
}

#endif // INCLUDED_MAP_PROPERTY_EDITOR_H
