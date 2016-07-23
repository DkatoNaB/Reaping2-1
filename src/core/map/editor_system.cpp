#include "platform/i_platform.h"
#include "editor_system.h"
#include "engine/engine.h"
#include "engine/controllers/controller_system.h"
#include "ui/ui.h"
#include "json/json.h"
#include "map_system.h"
#include "spawn_actor_map_element.h"
#include "editor_grid_system.h"
#include "editor_target_system.h"
#include "editor_brush_system.h"
#include "engine/collision_system.h"
#include "ctf_soldier_spawn_point_map_element.h"
#include "respawn_actor_map_element.h"
#include "respawn_actor_map_element_system.h"
#include "ctf_flag_spawn_point_map_element.h"
#include "../i_renderable_component.h"
#include "input/keyboard_adapter_system.h"
#include <boost/assign/std/vector.hpp>
#include "editor_back_event.h"
#include "room_editor_system.h"
#include "editor_mode_changed_event.h"
#include "editor_hud_state.h"

namespace map {

EditorSystem::EditorSystem()
    : mScene( Scene::Get() )
    , mEditorModel( "editor", &RootModel::Get() )
    , mStartModel( VoidFunc( this, &EditorSystem::Start ), "start", &mEditorModel )
    , mLoadModel( StringFunc( this, &EditorSystem::Load ), "load", &mEditorModel )
    , mModeModel( StringFunc( this, &EditorSystem::ModeSelect ), "mode", &mEditorModel )
    , mSaveModel( VoidFunc( this, &EditorSystem::Save ), "save", &mEditorModel )
    , mLevelModel( (ModelValue::get_string_vec_t) boost::bind(&EditorSystem::LevelNames, this),"levels", &mEditorModel )
    , mX( 0 )
    , mY( 0 )
    , mEditorMode()
    , mCurrentMovement( 0 )
    , mLevelName()
    , mTimer()
    , mAutoSaveOn( false )
{
    mTimer.SetFrequency( 25000 );
}

void EditorSystem::Init()
{
    mKeyboard =::engine::Engine::Get().GetSystem<engine::KeyboardSystem>();
    mOnScreenMouseMove = EventServer< ::ScreenMouseMoveEvent>::Get().Subscribe( boost::bind( &EditorSystem::OnScreenMouseMove, this, _1 ) );
    mWindow = engine::Engine::Get().GetSystem<engine::WindowSystem>();
    mRenderer = engine::Engine::Get().GetSystem<engine::RendererSystem>();
    mKeyId = EventServer<KeyEvent>::Get().Subscribe( boost::bind( &EditorSystem::OnKeyEvent, this, _1 ) );
    mOnEditorBack = EventServer<map::EditorBackEvent>::Get().Subscribe( boost::bind( &EditorSystem::OnEditorBack, this, _1 ) );
    using namespace boost::assign;
    mLevelNames += "level1", "level2", "level3", "level4", "level5";
}

void EditorSystem::OnEditorBack( map::EditorBackEvent const& Evt )
{
    if (mEnabled)
    {
        if (Evt.mBackToBaseHud)
        {
             Ui::Get().Load( "editor_base_hud" );
             EditorHudState::Get().SetHudShown( false );
        }
    }
}

void EditorSystem::Start()
{
    ::engine::Engine::Get().SetEnabled< ::engine::ControllerSystem>( false );
    ::engine::Engine::Get().SetEnabled< ::engine::CollisionSystem>( false );
    ::engine::Engine::Get().SetEnabled< EditorSystem>( true );
    ::engine::Engine::Get().SetEnabled< RoomEditorSystem>( false );
    RespawnActorMapElementSystem::Get()->SetRespawnOnDeath( false ); //to be able to delete target actors
    EditorTargetSystem::Get()->SetNextUID( AutoId( "spawn_at_start" ) );
}

void EditorSystem::Load( std::string const& level )
{
    mLevelName = level;
    mX = 0;
    mY = 0;
    ModelValue& PlayerModel = const_cast<ModelValue&>( RootModel::Get()["player"] );
    mPlayerModels.clear();
    mPlayerModels.push_back( new ModelValue( GetDoubleFunc( this, &EditorSystem::GetX ), "x", &PlayerModel ) );
    mPlayerModels.push_back( new ModelValue( GetDoubleFunc( this, &EditorSystem::GetY ), "y", &PlayerModel ) );

    mScene.Load( level );
    Ui::Get().Load( "editor_base_hud" );
    EditorHudState::Get().SetHudShown( false );
    mAutoSaveOn = true;
}

double const& EditorSystem::GetX() const
{
    return mX;
}

double const& EditorSystem::GetY() const
{
    return mY;
}

EditorSystem::~EditorSystem()
{
    mPlayerModels.clear();
}



void EditorSystem::Update( double DeltaTime )
{
    mTimer.Update( DeltaTime );
    if ( mAutoSaveOn && mTimer.IsTime() )
    {
        Save();
    }
    glm::vec2 cameraCenter = mRenderer->GetCamera().GetCenter();
    mX = cameraCenter.x;
    mY = cameraCenter.y;
    uint32_t currentKeyMovement = 0;
    if( mKeyboard->GetKey( GLFW_KEY_W ).State == KeyState::Down )
    {
        currentKeyMovement |= engine::KeyboardAdapterSystem::MF_Up;
    }
    if( mKeyboard->GetKey( GLFW_KEY_A ).State == KeyState::Down )
    {
        currentKeyMovement |= engine::KeyboardAdapterSystem::MF_Left;
    }
    if( mKeyboard->GetKey( GLFW_KEY_S ).State == KeyState::Down )
    {
        currentKeyMovement |= engine::KeyboardAdapterSystem::MF_Down;
    }
    if( mKeyboard->GetKey( GLFW_KEY_D ).State == KeyState::Down )
    {
        currentKeyMovement |= engine::KeyboardAdapterSystem::MF_Right;
    }
    currentKeyMovement |= mCurrentMovement;
    if ( !EditorHudState::Get().IsHudShown() )
    {
        mX += 1000 * DeltaTime * ( ( ( currentKeyMovement & engine::KeyboardAdapterSystem::MF_Left ) ? -1 : 0 ) + ( ( currentKeyMovement & engine::KeyboardAdapterSystem::MF_Right ) ? 1 : 0 ) );
        mY += 1000 * DeltaTime * ( ( ( currentKeyMovement & engine::KeyboardAdapterSystem::MF_Up ) ? 1 : 0 ) + ( ( currentKeyMovement & engine::KeyboardAdapterSystem::MF_Down ) ? -1 : 0 ) );
    }
    if ( mKeyboard->GetKey(GLFW_KEY_M).State == KeyState::Typed )
    {
        if (EditorHudState::Get().IsHudShown())
        {
            EventServer<EditorBackEvent>::Get().SendEvent( EditorBackEvent( true ) );
        }
        else
        {
            EditorHudState::Get().SetHudShown( true );
            Ui::Get().Load( "editor_hud" );
            EventServer<EditorModeChangedEvent>::Get().SendEvent( EditorModeChangedEvent( "mode_select" ) );
        }
    }
}

void EditorSystem::OnScreenMouseMove( ::ScreenMouseMoveEvent const& Evt )
{
    int w, h;
    mWindow->GetWindowSize( w, h );
    mCurrentMovement = 0;
    if( Evt.Pos.y < 100 )
    {
        mCurrentMovement |= engine::KeyboardAdapterSystem::MF_Up;
    }
    if( Evt.Pos.x < 100 )
    {
        mCurrentMovement |= engine::KeyboardAdapterSystem::MF_Left;
    }
    if( Evt.Pos.y > h - 150 )
    {
        mCurrentMovement |= engine::KeyboardAdapterSystem::MF_Down;
    }
    if( Evt.Pos.x > w - 100 )
    {
        mCurrentMovement |= engine::KeyboardAdapterSystem::MF_Right;
    }
}

void EditorSystem::Save()
{
    {
        Json::Value Root( Json::arrayValue );
        Json::Value RootBackground( Json::arrayValue );
        MapElementList_t& mapElementList = MapSystem::Get()->GetMapElementList();
        for ( MapElementList_t::iterator i = mapElementList.begin(), e = mapElementList.end(); i != e; ++i )
        {
            if ( ( *i )->GetType() == SpawnActorMapElement::GetType_static() )
            {
                Json::Value Element( Json::objectValue );
                Opt<Actor> actor( mScene.GetActor( ( *i )->GetSpawnedActorGUID() ) );
                if ( actor.IsValid() )
                {
                    Opt<IRenderableComponent> renderableC( actor->Get<IRenderableComponent>() );
                    if ( renderableC.IsValid() )
                    {
                        if( renderableC->GetLayer() == RenderableLayer::Background )
                        {
                            ( *i )->Save( Element );
                            if ( Element.size() > 0 )
                            {
                                RootBackground.append( Element );
                            }
                            continue;
                        }
                    }
                }
                ( *i )->Save( Element );
                if ( Element.size() > 0 )
                {
                    Root.append( Element );
                }
            }
        }

        {
            Json::StyledWriter Writer;
            std::string const& JString = Writer.write( Root );
            {
                OsFile OutJson( "data/map/" + mLevelName + "/saved.json", std::ios_base::out );
                OutJson.Write( JString );
            }
        }
        {
            Json::StyledWriter Writer;
            std::string const& JString = Writer.write( RootBackground );
            {
                OsFile OutJson( "data/map/" + mLevelName + "/saved_background.json", std::ios_base::out );
                OutJson.Write( JString );
            }
        }
    }
    {
        Json::Value Root( Json::arrayValue );
        MapElementList_t& mapElementList = MapSystem::Get()->GetMapElementList();
        for ( MapElementList_t::iterator i = mapElementList.begin(), e = mapElementList.end(); i != e; ++i )
        {
            if ( ( *i )->GetType() == ctf::CtfSoldierSpawnPointMapElement::GetType_static()
                 || ( *i )->GetType() == ctf::CtfFlagSpawnPointMapElement::GetType_static() )
            {
                Json::Value Element( Json::objectValue );
                ( *i )->Save( Element );
                if ( Element.size() > 0 )
                {
                    Root.append( Element );
                }
            }
        }

        Json::StyledWriter Writer;
        std::string const& JString = Writer.write( Root );
        {
            OsFile OutJson( "data/map/" + mLevelName + "/saved_start_points.json", std::ios_base::out );
            OutJson.Write( JString );
        }
    }
    {
        Json::Value Root( Json::arrayValue );
        MapElementList_t& mapElementList = MapSystem::Get()->GetMapElementList();
        for ( MapElementList_t::iterator i = mapElementList.begin(), e = mapElementList.end(); i != e; ++i )
        {
            if ( ( *i )->GetType() == RespawnActorMapElement::GetType_static() )
            {
                Json::Value Element( Json::objectValue );
                ( *i )->Save( Element );
                if ( Element.size() > 0 )
                {
                    Root.append( Element );
                }
            }
        }

        Json::StyledWriter Writer;
        std::string const& JString = Writer.write( Root );
        {
            OsFile OutJson( "data/map/" + mLevelName + "/saved_pickups.json", std::ios_base::out );
            OutJson.Write( JString );
        }
    }
}

void EditorSystem::OnKeyEvent( const KeyEvent& Event )
{
    if (!mEnabled)
    {
        return;
    }
}

Opt<EditorSystem> EditorSystem::Get()
{
    return engine::Engine::Get().GetSystem<EditorSystem>();
}

std::vector<std::string> EditorSystem::LevelNames()
{
    return mLevelNames;
}

void EditorSystem::ModeSelect( std::string const& mode )
{
    mEditorMode = mode;
    EventServer<EditorModeChangedEvent>::Get().SendEvent( EditorModeChangedEvent( mode ) );
}


} // namespace map

