#version 330

uniform vec2 resolution;
uniform vec2 lightPosition;
uniform vec2 lightSize;
uniform sampler2D texture;
smooth in vec2 inTexCoord;
out vec4 outputColor;
float pi = 3.141592654;
float threshold = 0.75;
float lightSourceRadius = 0.001;

void main()
{
    float distance = 1.0;

    float rays = 256.0;
    for (float y=0.0; y<rays-0.1; y+=1.0)
    {
        float theta = inTexCoord.x * 2 * pi;
        float r = y / rays;

        //coord which we will sample from occlude map
        vec2 coord = vec2( cos( theta ), sin( theta ) ) * r / 2.0 + lightPosition;
        if( coord.x < 0.0 || coord.y < 0.0 ||
            coord.x > 1.0 || coord.y > 1.0 )
        {
            continue;
        }
        //sample the occlusion map
        vec4 data = texture2D(texture, coord);

        //the current distance is how far from the top we've come
        float dst = y/rays;

        //if we've hit an opaque fragment (occluder), then get new distance
        //if the new distance is below the current, then we'll use that for our ray
        float caster = data.a;
        if (caster > threshold && dst > lightSourceRadius) {
            distance = min(distance, dst);
            //NOTE: we could probably use "break" or "return" here
        }
    }
    outputColor = vec4( distance, distance, distance, 1 );
}
