#include "platform/i_platform.h"
#include "editor_system.h"
#include "engine/engine.h"
#include "engine/controllers/controller_system.h"
#include "ui/ui.h"
#include "engine/controllers/player_controller_sub_system.h"
#include "json/json.h"
#include "map_system.h"
#include "spawn_actor_map_element.h"
#include "editor_grid_system.h"
#include "editor_target_system.h"
#include "editor_brush_system.h"
#include "engine/collision_system.h"
#include "ctf_soldier_spawn_point_map_element.h"
#include "respawn_actor_map_element.h"

namespace map {

EditorSystem::EditorSystem()
    : mScene( Scene::Get() )
    , mEditorModel( "editor", &RootModel::Get() )
    , mStartModel( VoidFunc( this, &EditorSystem::Start ), "start", &mEditorModel )
    , mLoadModel( StringFunc( this, &EditorSystem::Load ), "load", &mEditorModel )
    , mSaveModel( VoidFunc( this, &EditorSystem::Save ), "save", &mEditorModel )
    , mX(0)
    , mY(0)
    , mCurrentMovement(0)
    , mLevelName()
    , mHudState(false)
    , mSpaceTyped(false)
{
}


void EditorSystem::Init()
{
    mKeyboard=::engine::Engine::Get().GetSystem<engine::KeyboardSystem>();
    mOnScreenMouseMove=EventServer< ::ScreenMouseMoveEvent>::Get().Subscribe( boost::bind( &EditorSystem::OnScreenMouseMove, this, _1 ) );
    mWindow=engine::Engine::Get().GetSystem<engine::WindowSystem>();
    mRenderer=engine::Engine::Get().GetSystem<engine::RendererSystem>();
    mKeyId = EventServer<KeyEvent>::Get().Subscribe( boost::bind( &EditorSystem::OnKeyEvent, this, _1 ) );

}



void EditorSystem::Start()
{
    ::engine::Engine::Get().SetEnabled< ::engine::ControllerSystem>(false);
    ::engine::Engine::Get().SetEnabled< ::engine::CollisionSystem>(false);
}

void EditorSystem::Load(std::string const& level)
{
    mLevelName=level;
    mX=0;
    mY=0;
    ModelValue& PlayerModel = const_cast<ModelValue&>(RootModel::Get()["player"]);
    mPlayerModels.clear();
    mPlayerModels.push_back( new ModelValue( GetX(), "x", &PlayerModel ) );
    mPlayerModels.push_back( new ModelValue( GetY(), "y", &PlayerModel ) );

    mScene.Load(level);
    Ui::Get().Load("editor_base_hud");
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



void EditorSystem::Update(double DeltaTime)
{
    glm::vec2 cameraCenter=mRenderer->GetCamera().GetCenter();
    mX=cameraCenter.x;
    mY=cameraCenter.y;
    uint32_t currentKeyMovement = 0;
    if( mKeyboard->GetKey(GLFW_KEY_W).State==KeyState::Down)
    {
        currentKeyMovement |= MF_Up;
    }
    if( mKeyboard->GetKey(GLFW_KEY_A).State==KeyState::Down )
    {
        currentKeyMovement |= MF_Left;
    }
    if( mKeyboard->GetKey(GLFW_KEY_S).State==KeyState::Down )
    {
        currentKeyMovement |= MF_Down;
    }
    if( mKeyboard->GetKey(GLFW_KEY_D).State==KeyState::Down )
    {
        currentKeyMovement |= MF_Right;
    }
    currentKeyMovement |= mCurrentMovement;
    if (!mHudState)
    {
        mX += 1000*DeltaTime*(( ( currentKeyMovement & MF_Left ) ? -1 : 0 ) + ( ( currentKeyMovement & MF_Right ) ? 1 : 0 ));
        mY += 1000*DeltaTime*(( ( currentKeyMovement & MF_Up ) ? 1 : 0 ) + ( ( currentKeyMovement & MF_Down ) ? -1 : 0 ));
    }
    if( mKeyboard->GetKey(GLFW_KEY_SPACE).State==KeyState::Down )
    {
        currentKeyMovement |= MF_Right;
    }
    if (mSpaceTyped)
    {
        mSpaceTyped=false;
        if (mHudState)
        {
             Ui::Get().Load("editor_base_hud");
             ::engine::Engine::Get().SetEnabled<EditorGridSystem>(true);
             ::engine::Engine::Get().SetEnabled<EditorTargetSystem>(true);
             ::engine::Engine::Get().SetEnabled<EditorBrushSystem>(true);
             mHudState=false;
        }
        else
        {
             Ui::Get().Load("editor_hud");
             ::engine::Engine::Get().SetEnabled<EditorGridSystem>(false);
             ::engine::Engine::Get().SetEnabled<EditorTargetSystem>(false);
             ::engine::Engine::Get().SetEnabled<EditorBrushSystem>(false);
             mHudState=true;
        }
    }
}
void EditorSystem::OnScreenMouseMove(::ScreenMouseMoveEvent const& Evt)
{
    int w, h;
    mWindow->GetWindowSize( w, h );
    mCurrentMovement = 0;
    if( Evt.Pos.y<100)
    {
        mCurrentMovement |= MF_Up;
    }
    if( Evt.Pos.x<100 )
    {
        mCurrentMovement |= MF_Left;
    }
    if(Evt.Pos.y>h-150 )
    {
        mCurrentMovement |= MF_Down;
    }
    if( Evt.Pos.x>w-100 )
    {
        mCurrentMovement |= MF_Right;
    }
}

void EditorSystem::Save()
{
    {
        Json::Value Root(Json::arrayValue);
        MapElementList_t& mapElementList=MapSystem::Get()->GetMapElementList();
        for (MapElementList_t::iterator i=mapElementList.begin(),e=mapElementList.end();i!=e;++i)
        {
            if ((*i)->GetType()==SpawnActorMapElement::GetType_static())
            {
                Json::Value Element(Json::objectValue);
                (*i)->Save(Element);
                if (Element.size()>0)
                {
                    Root.append(Element);
                }
            }
        }

        Json::StyledWriter Writer;
        std::string const& JString=Writer.write(Root);
        {
	        OsFile OutJson("data/map/"+mLevelName+"/saved.json",std::ios_base::out);
	        OutJson.Write(JString);
        }
        {
	        OsFile OutJson("saved.json",std::ios_base::out);
	        OutJson.Write(JString);
        }
    }
    {
        Json::Value Root(Json::arrayValue);
        MapElementList_t& mapElementList=MapSystem::Get()->GetMapElementList();
        for (MapElementList_t::iterator i=mapElementList.begin(),e=mapElementList.end();i!=e;++i)
        {
            if ((*i)->GetType()==ctf::CtfSoldierSpawnPointMapElement::GetType_static())
            {
                Json::Value Element(Json::objectValue);
                (*i)->Save(Element);
                if (Element.size()>0)
                {
                    Root.append(Element);
                }
            }
        }

        Json::StyledWriter Writer;
        std::string const& JString=Writer.write(Root);
        {
	        OsFile OutJson("data/map/"+mLevelName+"/saved_start_points.json",std::ios_base::out);
	        OutJson.Write(JString);
        }
        {
	        OsFile OutJson("saved_start_points.json",std::ios_base::out);
	        OutJson.Write(JString);
        }
    }
    {
        Json::Value Root(Json::arrayValue);
        MapElementList_t& mapElementList=MapSystem::Get()->GetMapElementList();
        for (MapElementList_t::iterator i=mapElementList.begin(),e=mapElementList.end();i!=e;++i)
        {
            if ((*i)->GetType()==RespawnActorMapElement::GetType_static())
            {
                Json::Value Element(Json::objectValue);
                (*i)->Save(Element);
                if (Element.size()>0)
                {
                    Root.append(Element);
                }
            }
        }

        Json::StyledWriter Writer;
        std::string const& JString=Writer.write(Root);
        {
	        OsFile OutJson("data/map/"+mLevelName+"/saved_pickups.json",std::ios_base::out);
	        OutJson.Write(JString);
        }
        {
	        OsFile OutJson("saved_pickups.json",std::ios_base::out);
	        OutJson.Write(JString);
        }
    }
}

void EditorSystem::OnKeyEvent(const KeyEvent& Event)
{
    if( Event.Key == GLFW_KEY_SPACE && Event.State == KeyState::Up )
    {
        mSpaceTyped=true;
    }
}

} // namespace map

