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


void EditorCameraSystem::Update(double DeltaTime)
{
    auto&& keyboard =::engine::Engine::Get().GetSystem<engine::KeyboardSystem>();
    auto&& renderer = engine::Engine::Get().GetSystem<engine::RendererSystem>();
    glm::vec2 cameraCenter = renderer->GetCamera().GetCenter();
    mX = cameraCenter.x;
    mY = cameraCenter.y;
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
    mX += 1000 * DeltaTime * ( ( ( currentKeyMovement & engine::KeyboardAdapterSystem::MF_Left ) ? -1 : 0 ) + ( ( currentKeyMovement & engine::KeyboardAdapterSystem::MF_Right ) ? 1 : 0 ) );
    mY += 1000 * DeltaTime * ( ( ( currentKeyMovement & engine::KeyboardAdapterSystem::MF_Up ) ? 1 : 0 ) + ( ( currentKeyMovement & engine::KeyboardAdapterSystem::MF_Down ) ? -1 : 0 ) );
}

void EditorCameraSystem::OnScreenMouseMove( ::ScreenMouseMoveEvent const& Evt )
{
    if (!EditorTargetSystem::Get()->EdgeScrollAllowed())
    {
        return;
    }
    int w, h;
    auto&& window = engine::Engine::Get().GetSystem<engine::WindowSystem>();
    window->GetWindowSize( w, h );
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

double const& EditorCameraSystem::GetX() const
{
    return mX;
}

double const& EditorCameraSystem::GetY() const
{
    return mY;
}

Opt<EditorCameraSystem> EditorCameraSystem::Get()
{
    return engine::Engine::Get().GetSystem<EditorCameraSystem>();
}

REGISTER_SYSTEM( EditorCameraSystem );

} // namespace map

