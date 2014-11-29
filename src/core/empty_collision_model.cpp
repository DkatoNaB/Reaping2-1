#include "platform/i_platform.h"
#include "core/empty_collision_model.h"

bool EmptyCollisionModel::AreActorsColliding( Actor const& Obj1, Actor const& Obj2, double Dt )const
{
    return false;
}
