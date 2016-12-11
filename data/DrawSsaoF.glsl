
precision mediump float;

varying vec2 vTexCoord;

uniform sampler2D uRandomizerTexture;
uniform sampler2D uAuxTexture0; // depth map
uniform sampler2D uAuxTexture1; // color map

// http://stackoverflow.com/questions/9882716/packing-float-into-vec4-how-does-this-code-work
float unpackFloat(vec4 rgba_depth)
{
    const vec4 bit_shift = vec4(1.0/(256.0*256.0*256.0), 1.0/(256.0*256.0), 1.0/256.0, 1.0);
    float depth = dot(rgba_depth, bit_shift);
    return depth;
}

const float radius = 0.1;
const float distScale = 5.0;

float getDepth(vec2 texCoord)
{
    const float zFar = 100.0;
    const float zNear = 0.1;
    float value = unpackFloat(texture2D(uAuxTexture0, texCoord));
    return zFar * zNear / (value * (zFar - zNear) - zFar);
}

float pass(vec3 plane, float z, vec3 rnd)
{
    vec3 sample = reflect(rnd, plane);
    float zSample = getDepth(vTexCoord + radius * sample.xy / z);

    float dist = max(zSample - z, 0.0) / distScale;
    float occl = 15.0 * max(dist * (2.0 - dist), 0.0);
    return 1.0 / (1.0 + occl * occl);
}

// http://steps3d.narod.ru/tutorials/ssao-tutorial.html
void main()
{
    float z = getDepth(vTexCoord);
    vec3 plane = 2.0 * texture2D(uRandomizerTexture, vTexCoord * 512.0f).rgb - vec3(1.0);

    float att;
    att  = pass(plane, z, vec3(-0.5, -0.5, -0.5));
    att += pass(plane, z, vec3( 0.5, -0.5, -0.5));
    att += pass(plane, z, vec3(-0.5,  0.5, -0.5));
    att += pass(plane, z, vec3( 0.5,  0.5, -0.5));
    att += pass(plane, z, vec3(-0.5, -0.5,  0.5));
    att += pass(plane, z, vec3( 0.5, -0.5,  0.5));
    att += pass(plane, z, vec3(-0.5,  0.5,  0.5));
    att += pass(plane, z, vec3( 0.5,  0.5,  0.5));

    att = clamp((att / 8.0 + 0.25), 0.0, 1.0);
    gl_FragColor = vec4(vec3(att), 1.0);
}
