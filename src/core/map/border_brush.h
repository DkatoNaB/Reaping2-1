#ifndef INCLUDED_CORE_MAP_BORDER_BRUSH_H
#define INCLUDED_CORE_MAP_BORDER_BRUSH_H
#include "i_brush.h"
#include "neighbors.h"

namespace map {

class BorderBrush: public IBrush
{
public:
    BorderBrush( int32_t Id );
    void UpdateBorders( Neighbors& neighbors );
    virtual void CreateTarget();
    virtual void RemoveTarget();
};

} // namespace map
#endif//INCLUDED_CORE_MAP_BORDER_BRUSH_H
