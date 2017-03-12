#ifndef INCLUDED_CORE_MAP_EDITOR_MODE_H
#define INCLUDED_CORE_MAP_EDITOR_MODE_H

#include "platform/singleton.h"

namespace map {

class EditorMode : public platform::Singleton<EditorMode>
{
    friend class platform::Singleton<EditorMode>;
    EditorMode();
public:
    enum Mode : int {
        Actors = 0,
        Entrances,
        CellFill,
    };
    Mode GetMode() const;
    void SetMode( Mode mode );
    void DrawSelector();
private:
    Mode mMode = Actors;
};

}

#endif // INCLUDED_CORE_MAP_EDITOR_MODE_H

