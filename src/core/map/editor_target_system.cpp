#include "platform/i_platform.h"
#include "editor_target_system.h"
#include "engine/engine.h"
#include "core/i_position_component.h"
#include "ui/ui.h"
#include "core/i_collision_component.h"
#include "editor_back_event.h"
#include "target_repo.h"
#include "pickup_target.h"
#include "wall_target.h"
#include "editor_hud_state.h"
#include "grid_repo.h"
#include "brush_repo.h"
#include "core/renderable_layer.h"
#include "map_system.h"
#include <imgui.h>
#include <boost/assign/std/vector.hpp>

namespace {

    // helper function to collect autoids of targets based on target type
    template <typename T>
    static void CollectAutoids(std::vector<int32_t>& autoids)
    {
        using namespace map;
        static TargetRepo::const_iterator cbegin = TargetRepo::Get().begin();
        static TargetRepo::const_iterator cend = TargetRepo::Get().end();
        for ( auto it = cbegin; it != cend; ++it )
        {
            if ( dynamic_cast<const T*>(it->second) )
            {
                autoids.push_back(it->first);
            }
        }
    }
    // helper function specialized for pickups, because pickups have further subtypes
    // collects autoids of pickups based on pickup type
    static void CollectPickupAutoids(const std::string& type, std::vector<int32_t> &autoids)
    {
        using namespace map;
        IdStorage & idstorage = IdStorage::Get();
        static TargetRepo::const_iterator cbegin = TargetRepo::Get().begin();
        static TargetRepo::const_iterator cend = TargetRepo::Get().end();
        for ( auto it = cbegin; it != cend; ++it )
        {
            if ( const PickupTarget *p = dynamic_cast<const PickupTarget*>(it->second) )
            {
                int32_t typeId = p->GetTypeId();
                std::string typeName;
                idstorage.GetName(typeId, typeName);
                if ( typeName == type )
                {
                    autoids.push_back(it->first);
                }
            }
        }
    }

    static bool GetTargets( void* data, int idx, char const** out )
    {
        if( data == nullptr )
        {
            return false;
        }
        std::vector<int32_t>* vec = static_cast<std::vector<int32_t>* >( data );
        if( idx >= 0 && idx < vec->size() )
        {
            IdStorage & idstorage = IdStorage::Get();
            static std::string name; // static: we need the pointer valid after return, but the same data can be reused
            if( idstorage.GetName( vec->at( idx ), name ) )
            {
                *out = &name[0];
                return true;
            }
        }
        return false;
    }
}

namespace map {

EditorTargetSystem::EditorTargetSystem()
    : mScene( Scene::Get() )
    , mTargetRepo( TargetRepo::Get() )
    , mTargetId( -1 )
    , mCursorPosition( 0.0, 0.0 )
    , mCursorGuid( -1 )
    , mNextUID( AutoId( "spawn_at_start" ) )
{
}


void EditorTargetSystem::Init()
{
    mMouseClickId = EventServer<WorldMouseReleaseEvent>::Get().Subscribe( boost::bind( &EditorTargetSystem::OnMouseClickEvent, this, _1 ) );
    mOnWorldMouseMove = EventServer< ::WorldMouseMoveEvent>::Get().Subscribe( boost::bind( &EditorTargetSystem::OnWorldMouseMoveEvent, this, _1 ) );
    mKeyId = EventServer<KeyEvent>::Get().Subscribe( boost::bind( &EditorTargetSystem::OnKeyEvent, this, _1 ) );

    /// ------ Pickups ------
    // guns
    CollectPickupAutoids("weapon", mGunActorIds);
    // mapping the visual ids to the actor ids
    mGunVisualIds = mGunActorIds;

    // buffs
    CollectPickupAutoids("buff", mBuffActorIds);
    // mapping the visual ids to the actor ids
    mBuffVisualIds = mBuffActorIds;

    // items
    CollectPickupAutoids("normal", mItemActorIds);
    // mapping the visual ids to the actor ids
    mItemVisualIds = mItemActorIds;

    /// ------ Targets ------
    using namespace boost::assign;
    // mapitems: wall types
    CollectAutoids<WallTarget>(mMapitemActorIds);
    mMapitemVisualIds = mMapitemActorIds;
    // spawnpoints
    mSpawnpointVisualIds += mTargetRepo( AutoId("ctf_flag_spawn_red")).GetCursorId(),
                            mTargetRepo( AutoId("ctf_soldier_spawn_red")).GetCursorId(),
                            mTargetRepo( AutoId("ctf_flag_spawn_blue")).GetCursorId(),
                            mTargetRepo( AutoId("ctf_soldier_spawn_blue")).GetCursorId(),
                            mTargetRepo( AutoId( "soldier_spawn" ) ).GetCursorId();
    mSpawnpointActorIds +=  AutoId("ctf_flag_spawn_red"),
                            AutoId("ctf_soldier_spawn_red"),
                            AutoId("ctf_flag_spawn_blue"),
                            AutoId("ctf_soldier_spawn_blue"),
                            AutoId( "soldier_spawn" );
    mSpawnpointVisualBackground +=  0xaa0000ee,
                                    0xaa0000ee,
                                    0x0000aaee,
                                    0x0000aaee,
                                    0xeeeeeeee;

    mTargetActorIdsMap["spawnpoint"] = mSpawnpointActorIds;
    mTargetActorIdsMap["mapitem"] = mMapitemActorIds;
    mTargetActorIdsMap["gun"] = mGunActorIds;
    mTargetActorIdsMap["buff"] = mBuffActorIds;
    mTargetActorIdsMap["item"] = mItemActorIds;

}

void EditorTargetSystem::Update( double DeltaTime )
{
    GetTarget().Update( DeltaTime );
    auto Cursor = mScene.GetActor( mCursorGuid );
    if ( Cursor.IsValid() )
    {
        Opt<IPositionComponent> positionC( Cursor->Get<IPositionComponent>() );
        if ( positionC.IsValid() )
        {
            positionC->SetX( mCursorPosition.x );
            positionC->SetY( mCursorPosition.y );
        }
    }

    if (!mEnabled)
    {
        return;
    }

    bool shown = true;
    ImGui::Begin( "Editor", &shown );
    char const* targettypes[] = {
        "spawnpoint",
        "mapitem",
        "gun",
        "buff",
        "item",
    };
    ImGui::Combo( "type", &mTargetType, targettypes, 5 );
    auto& tgt = mTargetActorIdsMap[ targettypes[mTargetType] ];
    if( mTarget >= tgt.size() )
    {
        mTarget = 0;
    }
    auto id = mTargetId;
    if( 0 <= mTarget && mTarget < tgt.size() )
    {
        id = tgt.at( mTarget );
    }
    if( id != mTargetId )
    {
        RemoveCursor();
        mTargetId = id;
        AddCursor();
    }
    ImGui::Combo( "target", &mTarget, &GetTargets, &tgt, tgt.size() );
    char const* brush[] = {
        "normal",
        "border",
    };
    ImGui::Combo( "brush", &mBrush, brush, 2 );
    char const* pos[] = {
        "free",
        "snap to grid",
    };
    ImGui::Combo( "positioning", &mPositioning, pos, 2 );
    ImGui::Separator();
    auto& rl( RenderableLayer::Get() );
    auto const& layers = rl.GetNameToPriorityMap();
    if( mLayers.size() != layers.size() )
    {
        mLayers.resize( layers.size() );
    }
    int cnt = 0;
    for( auto const& ly : layers )
    {
        ImGui::Checkbox( ly.first.c_str(), &mLayers[ cnt ].b );
        ++cnt;
    }
    ImGui::End();

}

EditorTargetSystem::~EditorTargetSystem()
{
}

Opt<EditorTargetSystem> EditorTargetSystem::Get()
{
    return engine::Engine::Get().GetSystem<EditorTargetSystem>();
}

ITarget& EditorTargetSystem::GetTarget()
{
    return mTargetRepo( mTargetId );
}

glm::vec2 EditorTargetSystem::GetCursorPosition() const
{
    return mCursorPosition;
}

void EditorTargetSystem::SetCursorPosition( double x, double y )
{
    mCursorPosition.x = x;
    mCursorPosition.y = y;
}

Opt<Actor> EditorTargetSystem::GetCursor() const
{
    return mScene.GetActor( mCursorGuid );
}

double EditorTargetSystem::GetCursorRadius() const
{
    double r = 1.0;
    Opt<Actor> cursor( GetCursor() );
    if ( !cursor.IsValid() )
    {
        return r;
    }
    Opt<ICollisionComponent> collisionC( cursor->Get<ICollisionComponent>() );
    if ( !collisionC.IsValid() )
    {
        return r;
    }
    return collisionC->GetRadius();
}

std::vector<int32_t> EditorTargetSystem::Guns()
{
    return mGunVisualIds;
}

std::vector<int32_t> EditorTargetSystem::Buffs()
{
    return mBuffVisualIds;
}

std::vector<int32_t> EditorTargetSystem::Items()
{
    return mItemVisualIds;
}

std::vector<int32_t> EditorTargetSystem::MapItems()
{
    return mMapitemVisualIds;
}

std::vector<int32_t> EditorTargetSystem::Spawnpoints()
{
    return mSpawnpointVisualIds;
}

std::vector<int32_t> EditorTargetSystem::SpawnpointBackground()
{
    return mSpawnpointVisualBackground;
}

int32_t EditorTargetSystem::GetNextUID() const
{
    return mNextUID;
}

void EditorTargetSystem::SetNextUID( int32_t uid )
{
    mNextUID = uid;
}

void EditorTargetSystem::PutTarget( glm::vec2 position )
{
    GetTarget().PutTarget( position );
}

void EditorTargetSystem::PutTarget( glm::vec2 position, IBorderComponent::Borders_t& borders, IBorderComponent::Borders_t& outerBorders )
{
    GetTarget().PutTarget( position, borders, outerBorders );
}

void EditorTargetSystem::RemoveCursor()
{
    if (mCursorGuid>0)
    {
        mScene.RemoveActor( mCursorGuid );
    }
    mCursorGuid = -1;
}

void EditorTargetSystem::AddCursor()
{
    if (mTargetId == -1 || mCursorGuid > 0)
    {
        return;
    }
    std::auto_ptr<Actor> cursor( GetTarget().GetCursor() );
    Opt<IPositionComponent> positionC( cursor->Get<IPositionComponent>() );
    if (positionC.IsValid())
    {
        positionC->SetX( mCursorPosition.x );
        positionC->SetY( mCursorPosition.y );
    }
    Opt<IRenderableComponent> renderableC( cursor->Get<IRenderableComponent>() );
    if (renderableC.IsValid())
    {
        auto& rl( RenderableLayer::Get() );
        auto const& layers = rl.GetNameToPriorityMap();
        auto it = layers.find( "editor_objects" );
        renderableC->SetLayerPriority( it == layers.end() ? 0 : it->second );
    }
    mCursorGuid = cursor->GetGUID();
    mScene.AddActor( cursor.release() );
}

void EditorTargetSystem::OnMouseClickEvent( const WorldMouseReleaseEvent& Event )
{
    if( !mEnabled )
    {
        return;
    }
    auto& brush = BrushRepo::Get()( mBrush == 0 ? AutoId( "normal" ) : AutoId( "border" ) );
    if( Event.Button == engine::MouseSystem::Button_Left )
    {
        LL() << "Create target";
        brush.CreateTarget();
    }
    else if( Event.Button == engine::MouseSystem::Button_Right )
    {
        static Opt<engine::KeyboardSystem> keyboard = ::engine::Engine::Get().GetSystem<engine::KeyboardSystem>();
        EditorSelection::Mode m = EditorSelection::ClearAndSelect;
        if (keyboard->GetKey( GLFW_KEY_LEFT_CONTROL ).State == KeyState::Down
            || keyboard->GetKey( GLFW_KEY_RIGHT_CONTROL ).State == KeyState::Down)
        {
            m = EditorSelection::Remove;
        }
        else if (keyboard->GetKey( GLFW_KEY_LEFT_SHIFT ).State == KeyState::Down
            || keyboard->GetKey( GLFW_KEY_RIGHT_SHIFT ).State == KeyState::Down)
        {
            m = EditorSelection::Add;
        }
        std::vector<int32_t> lys;
        auto& rl( RenderableLayer::Get() );
        auto const& layers = rl.GetNameToPriorityMap();
        int cnt = 0;
        std::for_each( layers.begin(), layers.end(), [&]( std::pair<std::string,int32_t> const& p )
                {
                    if( mLayers[cnt].b )
                    {
                        lys.push_back( p.second );
                    }
                    ++cnt;
                });
        mSelection.SelectActors( Event.Pos, Event.Pos, lys, m );
    }
}

void EditorTargetSystem::OnWorldMouseMoveEvent( ::WorldMouseMoveEvent const& Evt )
{
    auto& grid = GridRepo::Get()( mPositioning == 0 ? AutoId("absolute") : AutoId("matrix") );
    grid.SetMousePosition( Evt.Pos.x, Evt.Pos.y );
    SetCursorPosition( grid.GetProcessedPosition().x, grid.GetProcessedPosition().y );
}

void EditorTargetSystem::OnKeyEvent( const KeyEvent& Event )
{
    if (!mEnabled)
    {
        return;
    }
    if( Event.Key == GLFW_KEY_DELETE )
    {
        auto const& selected = mSelection.GetSelectedActors();
        mSelection.ClearSelection();
        auto mapSystem = MapSystem::Get();
        for (auto&& actorid : selected)
        {
            mScene.RemoveActor( actorid );
            mapSystem->RemoveMapElement( actorid );
        }
    }
}




} // namespace map

