
precision mediump float;

varying vec4 vPosition;
varying vec4 vColor;

// http://aras-p.info/blog/2009/07/30/encoding-floats-to-rgba-the-final/
vec4 packFloat(float v)
{
    vec4 enc = fract(v * vec4(1.0, 255.0, 65025.0, 160581375.0));
    enc -= enc.yzww * vec4(1.0 / 255.0, 1.0 / 255.0, 1.0 / 255.0, 0.0);
    return enc;
}

void main()
{
    float distance = vPosition.z / vPosition.w;
    float normalizedDistance = (distance + 1.0) / 2.0;
    gl_FragColor = packFloat(normalizedDistance);
}
