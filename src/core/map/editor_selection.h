#ifndef INCLUDED_EDITOR_SELECTION_H
#define INCLUDED_EDITOR_SELECTION_H

#include "platform/i_platform.h"

namespace map {

typedef std::map<int32_t, glm::vec4> ActorColors_t;
class EditorSelection
{
public:
    enum Mode : int32_t {
        ClearAndSelect = 0,
        Add,
        Remove,
    };
    void SelectActors( glm::vec2 begin, glm::vec2 end, std::vector<int32_t> const& levels, Mode = Add );
    void ClearSelection();
    std::vector<int32_t> GetSelectedActors() const;
private:
    ActorColors_t mSelectedActors;
};

}

#endif // INCLUDED_EDITOR_SELECTION_H

