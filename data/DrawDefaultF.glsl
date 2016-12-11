
precision mediump float;

varying vec2 vTexCoord;
varying vec4 vColor;

uniform sampler2D uTexture;

void main()
{
    gl_FragColor = texture2D(uTexture, vTexCoord) * vColor;
}
