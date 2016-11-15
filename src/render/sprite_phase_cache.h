// ====================================================================================================================
#pragma once
#ifndef INCLUDED_SPRITE_PHASE_CACHE_H
#define INCLUDED_SPRITE_PHASE_CACHE_H

#include "sprite_phase.h"
#include "platform/singleton.h"
#include <map>
#include <vector>

namespace render {

class SpritePhaseCache : public platform::Singleton<SpritePhaseCache>
{
    std::map<SpritePhase const*, SpritePhase> mOriginal;
    std::vector<SpritePhase const*> mPending;
    uint32_t mTarget;
    glm::vec2 mTargetSize;
    static const int mMaxCellSize = 256;
    int mCacheIndex = 0;
    int mRowSize = 0;
    GLuint mTargetTexId;
    friend class platform::Singleton<SpritePhaseCache>;
    SpritePhaseCache();
    glm::vec4 FindFreeRegion( SpritePhase const& sprphase );
public:
    void ProcessPending();
    void Request( SpritePhase const& sprphase );
};

}

#endif // INCLUDED_SPRITE_PHASE_CACHE_H

