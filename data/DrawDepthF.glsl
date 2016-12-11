
precision mediump float;

varying vec4 vColor;

// http://stackoverflow.com/questions/9882716/packing-float-into-vec4-how-does-this-code-work
vec4 packFloat(float depth)
{
    const vec4 bit_shift = vec4(256.0*256.0*256.0, 256.0*256.0, 256.0, 1.0);
    const vec4 bit_mask  = vec4(0.0, 1.0/256.0, 1.0/256.0, 1.0/256.0);
    vec4 res = fract(depth * bit_shift);
    res -= res.xxyz * bit_mask;
    return res;
}

void main()
{
    gl_FragColor = packFloat(gl_FragCoord.z);
}
