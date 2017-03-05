#ifndef INCLUDED_CORE_MAP_I_BRUSH_H
#define INCLUDED_CORE_MAP_I_BRUSH_H
#include "platform/i_platform.h"
#include "../scene.h"

namespace map {

class IBrush
{
public:
    IBrush( int32_t Id );
    virtual ~IBrush();
    virtual void CreateTarget() = 0;
    virtual void RemoveTarget() = 0;
    std::vector<int32_t> GetActorsToRemove();
protected:
    int32_t mId;
    Scene& mScene;
};

} // namespace map
#endif//INCLUDED_CORE_MAP_I_BRUSH_H
