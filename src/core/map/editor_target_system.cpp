#include "platform/i_platform.h"
#include "editor_target_system.h"
#include "engine/engine.h"
#include "../i_position_component.h"
#include "ui/ui.h"
#include "../i_collision_component.h"
#include <boost/assign/std/vector.hpp>
#include "editor_back_event.h"
#include "target_repo.h"
#include "pickup_target.h"
#include "wall_target.h"
#include "editor_hud_state.h"
#include <imgui.h>

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
    , mCursor( NULL )
    , mNextUID( AutoId( "spawn_at_start" ) )
{
}


void EditorTargetSystem::Init()
{
    mMouseClickId = EventServer<WorldMouseReleaseEvent>::Get().Subscribe( boost::bind( &EditorTargetSystem::OnMouseClickEvent, this, _1 ) );

    ModelValue& editorModel = const_cast<ModelValue&>( RootModel::Get()["editor"] );
    mEditorModels.push_back( new ModelValue( "target", &editorModel ) );
    ModelValue& targetModel = mEditorModels.back();
    mEditorModels.push_back( new ModelValue( "pickups", &targetModel) );
    ModelValue& pickupModel = mEditorModels.back();
    // for the menus
    // pickups
    mEditorModels.push_back( new ModelValue( (ModelValue::get_int_vec_t) boost::bind( &EditorTargetSystem::Guns, this ), "guns", &pickupModel ) );
    mEditorModels.push_back( new ModelValue( (ModelValue::get_int_vec_t) boost::bind( &EditorTargetSystem::Buffs, this ), "buffs", &pickupModel ) );
    mEditorModels.push_back( new ModelValue( (ModelValue::get_int_vec_t) boost::bind( &EditorTargetSystem::Items, this ), "items", &pickupModel ) );
    // targets
    mEditorModels.push_back( new ModelValue( (ModelValue::get_int_vec_t) boost::bind( &EditorTargetSystem::MapItems, this ), "mapitems", &targetModel ) );
    mEditorModels.push_back( new ModelValue( (ModelValue::get_int_vec_t) boost::bind( &EditorTargetSystem::Spawnpoints, this ), "spawnpoints", &targetModel ) );
    // spawn point teams are ambigous, so use background colors
    mEditorModels.push_back( new ModelValue( (ModelValue::get_int_vec_t) boost::bind( &EditorTargetSystem::SpawnpointBackground, this ), "teamcolor", &targetModel) );
    // for the menu actions
    mEditorModels.push_back( new ModelValue( IntFunc( this, boost::bind(&EditorTargetSystem::TargetChanged,this,"spawnpoint",_2) ), "spawntarget", &editorModel ) );
    mEditorModels.push_back( new ModelValue( IntFunc( this, boost::bind(&EditorTargetSystem::TargetChanged,this,"mapitem",_2) ), "mapitemtarget", &editorModel ) );
    mEditorModels.push_back( new ModelValue( IntFunc( this, boost::bind(&EditorTargetSystem::TargetChanged,this,"gun",_2) ), "guntarget", &editorModel ) );
    mEditorModels.push_back( new ModelValue( IntFunc( this, boost::bind(&EditorTargetSystem::TargetChanged,this,"buff",_2) ), "bufftarget", &editorModel ) );
    mEditorModels.push_back( new ModelValue( IntFunc( this, boost::bind(&EditorTargetSystem::TargetChanged,this,"item",_2) ), "itemtarget", &editorModel ) );
    
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
    mCursor = mScene.GetActor( mCursorGuid );
    if ( mCursor.IsValid() )
    {
        Opt<IPositionComponent> positionC( mCursor->Get<IPositionComponent>() );
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
    char const* layers[] = {
        "any",
        "target",
    };
    ImGui::Combo( "select layer", &mSelectLayer, layers, 2 );
    ImGui::End();

}

void EditorTargetSystem::TargetChanged( std::string const& targetType, int32_t targetIdx )
{
    RemoveCursor();
    auto it = mTargetActorIdsMap.find( targetType );
    if ( it == mTargetActorIdsMap.end() )
    {
        return;
    }
    mTargetId = it->second[targetIdx];
    AddCursor();

    EventServer<EditorBackEvent>::Get().SendEvent( EditorBackEvent() );
}

EditorTargetSystem::~EditorTargetSystem()
{
    mEditorModels.clear();
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
    return mCursor;
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
    if (mCursor.IsValid())
    {
        mScene.RemoveActor( mCursor->GetGUID() );
    }
    mCursor.Reset();
}

void EditorTargetSystem::AddCursor()
{
    if (mTargetId == -1 || mCursor.IsValid())
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
    mCursorGuid = cursor->GetGUID();
    mScene.AddActor( cursor.release() );
    mCursor = mScene.GetActor( mCursorGuid );
}

void EditorTargetSystem::OnMouseClickEvent( const WorldMouseReleaseEvent& Event )
{
    if( !mEnabled )
    {
        return;
    }
    if( Event.Button == engine::MouseSystem::Button_Left )
    {
        PutTarget( Event.Pos );
    }
}

} // namespace map

