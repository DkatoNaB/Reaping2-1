#version 330

uniform sampler2D spriteTexture;
smooth in vec3 inTexCoord;
layout( location = 0 ) out vec4 outputColor;
void main()
{
    outputColor = texture2D(spriteTexture,inTexCoord.st);
    outputColor.w=outputColor.w*inTexCoord.z;
}
