#version 330

//---global uniforms
layout(std140) uniform GlobalMatrices
{
    mat4 cameraToClipMatrix;
    mat4 worldToCameraMatrix;
};

//---shader spec
layout(location=0) in vec4 TexCoord;
layout(location=1) in vec2 SpriteCenter;
layout(location=2) in float Heading;
layout(location=3) in vec4 Color;
layout(location=4) in float Radius;
layout(location=5) in float Lifetime;
smooth out vec2 inTexCoord;
smooth out vec4 inColor;

vec2[4] corners=vec2[4](vec2(-1,-1),
        vec2(1,-1),
        vec2(-1,1),
        vec2(1,1));

// A single iteration of Bob Jenkins' One-At-A-Time hashing algorithm.
uint hash( uint x ) {
    x += ( x << 10u );
    x ^= ( x >>  6u );
    x += ( x <<  3u );
    x ^= ( x >> 11u );
    x += ( x << 15u );
    return x;
}

// Compound versions of the hashing algorithm I whipped together.
uint hash( uvec2 v ) { return hash( v.x ^ hash(v.y) ); }
uint hash( uvec3 v ) { return hash( v.x ^ hash(v.y) ^ hash(v.z) ); }
uint hash( uvec4 v ) { return hash( v.x ^ hash(v.y) ^ hash(v.z) ^ hash(v.w) ); }

// Construct a float with half-open range [0:1] using low 23 bits.
// All zeroes yields 0.0, all ones yields the next smallest representable value below 1.0.
float floatConstruct( uint m ) {
    const uint ieeeMantissa = 0x007FFFFFu; // binary32 mantissa bitmask
    const uint ieeeOne= 0x3F800000u; // 1.0 in IEEE binary32

    m &= ieeeMantissa; // Keep only mantissa bits (fractional part)
    m |= ieeeOne;      // Add fractional part to 1.0

    float  f = uintBitsToFloat( m );   // Range [1:2]
    return f - 1.0;                    // Range [0:1]
}

// Pseudo-random value in half-open range [0:1].
float random( float x ) { return floatConstruct(hash(floatBitsToUint(x))); }
float random( vec2  v ) { return floatConstruct(hash(floatBitsToUint(v))); }
float random( vec3  v ) { return floatConstruct(hash(floatBitsToUint(v))); }
float random( vec4  v ) { return floatConstruct(hash(floatBitsToUint(v))); }

void main()
{
    inColor=vec4( Color.r, Color.g, Color.b, Color.a * Lifetime );
    inTexCoord=vec2(TexCoord[int(floor(mod(gl_VertexID,2.0)))],TexCoord[2+int(floor(gl_VertexID/2.0))]);
    vec2 position=corners[gl_VertexID];
    mat2 ScaleMatrix=Radius*mat2(cos(Heading),sin(Heading),-sin(Heading),cos(Heading));
    gl_Position=cameraToClipMatrix*worldToCameraMatrix*vec4(ScaleMatrix*position+SpriteCenter,0,1);
}

