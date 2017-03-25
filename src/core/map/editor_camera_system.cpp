#include "platform/i_platform.h"
#include "editor_camera_system.h"
#include "engine/engine.h"
#include "input/keyboard_adapter_system.h"
#include "main/window.h"
#include "editor_target_system.h"

namespace map {

EditorCameraSystem::EditorCameraSystem()
{
}


void EditorCameraSystem::Init()
{
    mOnScreenMouseMove = EventServer< ::ScreenMouseMoveEvent>::Get().Subscribe( std::bind( &EditorCameraSystem::OnScreenMouseMove, this, std::placeholders::_1 ) );
}

void EditorCameraSystem::SetEnabled( bool enabled )
{
    System::SetEnabled( enabled );
    auto&& rs = engine::Engine::Get().GetSystem<engine::RendererSystem>();
    if( !rs.IsValid() )
    {
        return;
    }
    if( enabled && !mCamera )
    {
        mCamera.reset( new Camera( rs->GetCamera().GetProjection() ) );
    }
    rs->SetOverrideCamera( enabled ? mCamera.get() : nullptr );
}

void EditorCameraSystem::Update(double DeltaTime)
{
    if(!mCamera)
    {
        return;
    }
    auto&& keyboard =::engine::Engine::Get().GetSystem<engine::KeyboardSystem>();
    auto&& renderer = engine::Engine::Get().GetSystem<engine::RendererSystem>();
    uint32_t currentKeyMovement = 0;
    if( keyboard->GetKey( GLFW_KEY_W ).State == KeyState::Down )
    {
        currentKeyMovement |= engine::KeyboardAdapterSystem::MF_Up;
    }
    if( keyboard->GetKey( GLFW_KEY_A ).State == KeyState::Down )
    {
        currentKeyMovement |= engine::KeyboardAdapterSystem::MF_Left;
    }
    if( keyboard->GetKey( GLFW_KEY_S ).State == KeyState::Down )
    {
        currentKeyMovement |= engine::KeyboardAdapterSystem::MF_Down;
    }
    if( keyboard->GetKey( GLFW_KEY_D ).State == KeyState::Down )
    {
        currentKeyMovement |= engine::KeyboardAdapterSystem::MF_Right;
    }
    currentKeyMovement |= mCurrentMovement;
    auto center = mCamera->GetCenter();
    center.x += 1000 * DeltaTime * ( ( ( currentKeyMovement & engine::KeyboardAdapterSystem::MF_Left ) ? -1 : 0 ) + ( ( currentKeyMovement & engine::KeyboardAdapterSystem::MF_Right ) ? 1 : 0 ) );
    center.y += 1000 * DeltaTime * ( ( ( currentKeyMovement & engine::KeyboardAdapterSystem::MF_Up ) ? 1 : 0 ) + ( ( currentKeyMovement & engine::KeyboardAdapterSystem::MF_Down ) ? -1 : 0 ) );
    mCamera->SetCenter( center );
}

void EditorCameraSystem::OnScreenMouseMove( ::ScreenMouseMoveEvent const& Evt )
{
    mCurrentMovement = 0;
    if (!EditorTargetSystem::Get()->EdgeScrollAllowed())
    {
        return;
    }
    int w, h;
    auto&& window = engine::Engine::Get().GetSystem<engine::WindowSystem>();
    window->GetWindowSize( w, h );
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

Opt<EditorCameraSystem> EditorCameraSystem::Get()
{
    return engine::Engine::Get().GetSystem<EditorCameraSystem>();
}

REGISTER_SYSTEM( EditorCameraSystem );

} // namespace map

