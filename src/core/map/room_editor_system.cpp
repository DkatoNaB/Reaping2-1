#include "platform/i_platform.h"
#include "room_editor_system.h"
#include "engine/engine.h"
#include "engine/controllers/controller_system.h"
#include "ui/ui.h"
#include "json/json.h"
#include "map_system.h"
#include "spawn_actor_map_element.h"
#include "editor_target_system.h"
#include "engine/collision_system.h"
#include "ctf_soldier_spawn_point_map_element.h"
#include "respawn_actor_map_element.h"
#include "respawn_actor_map_element_system.h"
#include "ctf_flag_spawn_point_map_element.h"
#include "core/i_renderable_component.h"
#include "core/actor_factory.h"
#include "input/keyboard_adapter_system.h"
#include <boost/assign/std/vector.hpp>
#include "level_generator/room_repo.h"
#include "level_generator/level_generated_event.h"
#include "spawn_actor_map_element_system.h"
#include "map_start_map_element.h"
#include "spawn_soldiers_map_element.h"
#include "level_generator/level_generated_map_element.h"
#include "link_map_element.h"
#include "level_generator/json_room.h"
#include "editor_back_event.h"
#include "editor_system.h"
#include "editor_mode_changed_event.h"
#include "editor_hud_state.h"
#include "group_map_element.h"
#include "editor_group_system.h"
#include "editor_mode.h"
#include "room_editor_loaded_event.h"
#include "core/i_cell_component.h"
#include "level_generator/spawn_property.h"
#include "property_editor.h"
#include "editor_camera_system.h"
#include <imgui.h>

namespace map {

RoomEditorSystem::RoomEditorSystem()
    : mScene( Scene::Get() )
    , mEditorModel( "room_editor", &RootModel::Get() )
    , mLevelModel( (ModelValue::get_string_vec_t) RefTo(mLevelNames),"levels", &mEditorModel )
    , mStartModel( VoidFunc( this, &RoomEditorSystem::Start ), "start", &mEditorModel )
    , mLoadModel( StringFunc( this, &RoomEditorSystem::Load ), "load", &mEditorModel )
    , mModeModel( StringFunc( this, &RoomEditorSystem::ModeSelect ), "mode", &mEditorModel )
    , mSaveModel( VoidFunc( this, &RoomEditorSystem::Save ), "save", &mEditorModel )
    , mNewRoomModel( VoidFunc( this, &RoomEditorSystem::NewRoom ), "new_room", &mEditorModel )
    , mRoomName()
    , mEditorMode()
    , mTimer()
    , mAutoSaveOn( false )
{
    mTimer.SetFrequency( 25 );
}

void RoomEditorSystem::Init()
{
    mKeyboard =::engine::Engine::Get().GetSystem<engine::KeyboardSystem>();
    mMouseClickId = EventServer<WorldMouseReleaseEvent>::Get().Subscribe( boost::bind( &RoomEditorSystem::OnMouseClickEvent, this, _1 ) );
    mWindow = engine::Engine::Get().GetSystem<engine::WindowSystem>();
    mRenderer = engine::Engine::Get().GetSystem<engine::RendererSystem>();
    mKeyId = EventServer<KeyEvent>::Get().Subscribe( boost::bind( &RoomEditorSystem::OnKeyEvent, this, _1 ) );
    mOnPhaseChanged = EventServer<PhaseChangedEvent>::Get().Subscribe( boost::bind( &RoomEditorSystem::OnPhaseChanged, this, _1 ) );
    using namespace boost::assign;
    auto& idStorage = IdStorage::Get();
    for (auto&& elem : RoomRepo::Get().GetElements())
    {
        if (dynamic_cast<JsonRoom const*>(elem.second) != nullptr)
        {
            std::string name;
            if (idStorage.GetName( elem.second->GetId(), name ))
            {
                mLevelNames += name;
            }
        }
    }
}

void RoomEditorSystem::RemoveCells()
{
    for (auto cellGUID : mCellGUIDs)
    {
        mScene.RemoveActor( cellGUID );
    }
    mCellGUIDs.clear();
}

void RoomEditorSystem::AddCells()
{
    if( mPrevCellSize == mCellSize &&
        mPrevCellCount == mCellCount &&
        !mCellGUIDs.empty() )
    {
        return;
    }
    mPrevCellSize = mCellSize;
    mPrevCellCount = mCellCount;

    static ActorFactory& actorFactory = ActorFactory::Get();
    static int32_t cellId = AutoId( "cell" );
    RemoveCells();
    for (int32_t y = 0; y < mRoomDesc.GetCellCount(); ++y)
    {
        for (int32_t x = 0; x < mRoomDesc.GetCellCount(); ++x)
        {
            std::auto_ptr<Actor> cellActor( actorFactory( cellId ) );
            Opt<IPositionComponent> positionC( cellActor->Get<IPositionComponent>() );
            if (positionC.IsValid())
            {
                positionC->SetX( mRoomDesc.GetCellSize() * x + mRoomDesc.GetCellSize() / 2 );
                positionC->SetY( mRoomDesc.GetCellSize() * y + mRoomDesc.GetCellSize() / 2 );
            }
            auto CellC( cellActor->Get<ICellComponent>() );
            mCellGUIDs.push_back( cellActor->GetGUID() );
            if (CellC.IsValid())
            {
                CellC->SetRoomDesc( &mRoomDesc );
                CellC->SetX( x );
                CellC->SetY( y );
            }
            auto collisionC( cellActor->Get<ICollisionComponent>() );
            if (collisionC.IsValid())
            {
                collisionC->SetRadius( mRoomDesc.GetCellSize() / 2 );
            }
            mScene.AddActor( cellActor.release() );
        }
    }
}


void RoomEditorSystem::Start()
{
    ::engine::Engine::Get().SetEnabled< ::engine::ControllerSystem>( false );
    ::engine::Engine::Get().SetEnabled< ::engine::CollisionSystem>( false );
    ::engine::Engine::Get().SetEnabled< EditorSystem>( false );
    ::engine::Engine::Get().SetEnabled< RoomEditorSystem>( true );
    RespawnActorMapElementSystem::Get()->SetRespawnOnDeath( false ); //to be able to delete target actors
    EditorTargetSystem::Get()->SetNextUID( AutoId( "spawn_at_level_generated" ) );
}

void RoomEditorSystem::Load( std::string const& room )
{
    engine::Engine::Get().GetSystem<SpawnActorMapElementSystem>()->SetRemoveMapElementWhenUsed( false );
    mRoomName = room;
    auto&& editorCameraS = EditorCameraSystem::Get();
    editorCameraS->SetEnabled( true );

    mRoomId = AutoId( room );
    auto& aroom = RoomRepo::Get()( mRoomId );
    mRoomDesc = aroom.GetRoomDesc();
    mScene.Load( "room_editor" );
    Opt<engine::System> spawnActorMES( engine::Engine::Get().GetSystem<SpawnActorMapElementSystem>() );
    spawnActorMES->Update( 0 );

    // removing temporary big_backgrounds
        for (MapElementList_t::iterator it = MapSystem::Get()->GetMapElementList().begin(); it != MapSystem::Get()->GetMapElementList().end(); )
        {
            if ((*it)->GetType() == SpawnActorMapElement::GetType_static())
            {
                delete (*it).Get();
                it = MapSystem::Get()->GetMapElementList().erase( it );
            }
            else
            {
                ++it;
            }
        }
    // removing temporary big_backgrounds

    aroom.Generate( mRoomDesc, glm::vec2(0), true );
    std::vector<std::string> groupNames;
    auto& idStorage = IdStorage::Get();
    for (Opt<GroupMapElement> groupMapElement : MapElementListFilter<MapSystem::All>( MapSystem::Get()->GetMapElementList(), GroupMapElement::GetType_static() ))
    {
        std::string groupName;
        if (idStorage.GetName( groupMapElement->GetIdentifier(), groupName ))
        {
            groupNames.push_back( groupName );
        }
    }
    EditorGroupSystem::Get()->SetGroupNames( groupNames );
    Ui::Get().Load( "room_editor_base_hud" );
    mAutoSaveOn = true;
    EventServer<LevelGeneratedEvent>::Get().SendEvent( LevelGeneratedEvent( LevelGeneratedEvent::TerrainGenerated ) );
    EventServer<RoomEditorLoadedEvent>::Get().SendEvent( RoomEditorLoadedEvent(&mRoomDesc) );
    mCellSize = mRoomDesc.GetCellSize() / 100;
    mCellCount = mRoomDesc.GetCellCount();
    PropertyEditor::Get().SetRoomDesc( &mRoomDesc );
}

RoomEditorSystem::~RoomEditorSystem()
{
    mPlayerModels.clear();
}



void RoomEditorSystem::Update( double DeltaTime )
{
    mTimer.Update( DeltaTime );
    if ( mAutoSaveOn && mTimer.IsTime() )
    {
        Save();
    }
    if (mKeyboard->GetKey( GLFW_KEY_M ).State == KeyState::Typed)
    {
        SwitchToModeSelect();
    }

    {
        mRoomDesc.SetCellCount( mCellCount );
        mRoomDesc.SetCellSize( mCellSize * 100 );
        if( mShowCells )
        {
            AddCells();
        }
        else
        {
            RemoveCells();
        }
    }


    bool shown = true;
    ImGui::Begin( "Room Editor", &shown );
    ImGui::Checkbox( "show cells", &mShowCells );
    ImGui::Separator();
    ImGui::DragInt( "cell size", &mCellSize, 0.05f, 4, 20 );
    ImGui::DragInt( "cell count", &mCellCount, 0.05f, 1, 20 );
    ImGui::Separator();
    ImGui::Checkbox( "end", &mEnd );
    ImGui::Checkbox( "start", &mStart );
    ImGui::Checkbox( "key", &mKey );
    ImGui::End();

    PropertyEditor::Get().DrawUI();
}

void RoomEditorSystem::Save()
{
    {
        Json::Value Root( Json::arrayValue );
        Json::Value RoomObj( Json::objectValue );
        Json::Value MapElementArray( Json::arrayValue );

        for ( auto& mapElement : MapSystem::Get()->GetMapElementList() )
        {
            if (mapElement->GetType() != MapStartMapElement::GetType_static()
                 && mapElement->GetType() != LevelGeneratedMapElement::GetType_static()
                 && mapElement->GetType() != LinkMapElement::GetType_static()
                 && mapElement->GetType() != SpawnSoldiersMapElement::GetType_static() )
            {
                Json::Value Element( Json::objectValue );
                mapElement->Save( Element );
                if ( Element.size() > 0 )
                {
                    MapElementArray.append( Element );
                }
            }
        }
        RoomObj["map_elements"] = MapElementArray;
        auto& jsonRoom = static_cast<JsonRoom&>(RoomRepo::Get()(mRoomId));
        jsonRoom.Save( RoomObj, mRoomDesc );
        Root.append( RoomObj );
        {
            Json::StyledWriter Writer;
            std::string const& JString = Writer.write( Root );
            {
                OsFile OutJson( "data/rooms/" + mRoomName + "/room.json", std::ios_base::out );
//                OsFile OutJson( "data/rooms/test_save/test_save.json", std::ios_base::out );
                OutJson.Write( JString );
            }
        }
    }
}

void RoomEditorSystem::OnKeyEvent( const KeyEvent& Event )
{
    if (!mEnabled)
    {
        return;
    }
}

Opt<RoomEditorSystem> RoomEditorSystem::Get()
{
    return engine::Engine::Get().GetSystem<RoomEditorSystem>();
}

std::vector<std::string> RoomEditorSystem::LevelNames()
{
    return mLevelNames;
}

void RoomEditorSystem::ModeSelect( std::string const& mode )
{
    EventServer<EditorModeChangedEvent>::Get().SendEvent( EditorModeChangedEvent( mode, mEditorMode ) );
    mEditorMode = mode;
}

void RoomEditorSystem::OnPhaseChanged( PhaseChangedEvent const& Evt )
{
    if (Evt.CurrentPhase==ProgramPhase::Running)
    {
        EventServer<EditorModeChangedEvent>::Get().SendEvent( EditorModeChangedEvent( "mode_select", mEditorMode ) );
    }
}

void RoomEditorSystem::SwitchToModeSelect()
{
    EditorHudState::Get().SetHudShown( true );
    Ui::Get().Load( "room_editor_hud" );
    EventServer<EditorModeChangedEvent>::Get().SendEvent( EditorModeChangedEvent( "mode_select", mEditorMode ) );
    mEditorMode = "mode_select";
}

void RoomEditorSystem::NewRoom()
{
    static std::string const NEW_ROOM = "new_room";
    int max = 0;
    for (auto& levelName : mLevelNames)
    {
        try
        {
            if (boost::starts_with( levelName, NEW_ROOM ))
            {
                max = std::max( std::stoi( levelName.substr( NEW_ROOM.size() ) ), max );
            }
        }
        catch (...) {}
    }
    ++max;
    std::string newRoomName = NEW_ROOM + std::to_string( max );
    auto& roomBase = RoomRepo::Get()(AutoId( "room_base" ));
    std::unique_ptr<IRoom> jsonRoom( new JsonRoom( AutoId( newRoomName ) ) );

    jsonRoom->_SetRoomDesc( roomBase.GetRoomDesc() );
    {
        static auto& propertyFactory = PropertyFactory::Get();
        static int32_t propertyId = AutoId( "spawn" );
        auto prop = propertyFactory( propertyId );
        Opt<SpawnProperty> spawnProp( dynamic_cast<SpawnProperty*>(prop.get()) );
        spawnProp->SetTargets( { AutoId( "spawn_at_level_generated" ) } );
        jsonRoom->AddProperty( prop );
    }
    RoomRepo::Get().AddRoom( std::move(jsonRoom) );
    auto& fs = Filesys::Get();
    boost::filesystem::path dir( "data/rooms/"+newRoomName );
    if (boost::filesystem::create_directory( dir ))
    {
        Load( newRoomName );
        Save();
    }
}

void RoomEditorSystem::OnMouseClickEvent( const WorldMouseReleaseEvent& Event )
{
    if( !mEnabled )
    {
        return;
    }
    if( EditorMode::Get().GetMode() == EditorMode::Entrances )
    {
        SwitchEntranceState( Event.Pos, GetEntranceType(Event.Pos) );
    }
    if( EditorMode::Get().GetMode() == EditorMode::CellFill )
    {
        SwitchCellFilledState( Event.Pos );
    }
}

Opt<Cell> RoomEditorSystem::GetCellFromScene( glm::vec2 pos )
{
    Opt<Cell> r;
    if (pos.x < 0 || pos.y < 0)
    {
        return r;
    }
    int32_t x = pos.x / mRoomDesc.GetCellSize();
    int32_t y = pos.y / mRoomDesc.GetCellSize();
    if (x >= mRoomDesc.GetCellCount() || y >= mRoomDesc.GetCellCount())
    {
        return r;
    }
    r = &mRoomDesc.GetCell( x, y );
    return r;
}

void RoomEditorSystem::SwitchEntranceState( glm::vec2 pos, EntranceType::Type entrance )
{
    Opt<Cell> cell = GetCellFromScene( pos );
    if (cell.IsValid())
    {
        if (cell->HasEntrance( entrance ))
        {
            cell->RemoveEntrance( entrance );
        }
        else
        {
            cell->AddEntrance( entrance );
        }
    }
}

void RoomEditorSystem::SwitchCellFilledState( glm::vec2 pos )
{
    Opt<Cell> cell = GetCellFromScene( pos );
    if (cell.IsValid())
    {
        cell->SetFilled( !cell->IsFilled() );
    }
}

EntranceType::Type RoomEditorSystem::GetEntranceType( glm::vec2 pos )
{
    int32_t x = (int)pos.x%mRoomDesc.GetCellSize();
    int32_t y = (int)pos.y%mRoomDesc.GetCellSize();
    if (x < mRoomDesc.GetCellSize()*0.3)
    {
        return EntranceType::Left;
    }
    else if (x > mRoomDesc.GetCellSize()*0.7)
    {
        return  EntranceType::Right;
    }
    if (y < mRoomDesc.GetCellSize()*0.3)
    {
        return  EntranceType::Bottom;
    }
    else if (y > mRoomDesc.GetCellSize()*0.7)
    {
        return  EntranceType::Top;
    }

    return EntranceType::Left;
}


} // namespace map

