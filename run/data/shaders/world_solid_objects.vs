#version 330

//---global uniforms
layout(std140) uniform GlobalMatrices
{
    mat4 cameraToClipMatrix;
    mat4 worldToCameraMatrix;
    mat4 uiProjection;
    mat4 inverseProjection;
};

//---shader spec
layout(location=0) in vec2 position;
layout(location=1) in vec2 texcoord;
smooth out vec2 inTexCoord;
smooth out mat4 unproject;
void main()
{
    inTexCoord=texcoord;
    unproject=inverseProjection;
    gl_Position=vec4(position,0,1);
}
