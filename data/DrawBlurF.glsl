
precision mediump float;

uniform vec2 uViewportSize;

varying vec2 vTexCoord;

uniform sampler2D uAuxTexture1; // color map
uniform sampler2D uAuxTexture2; // blur map

void main()
{
    float offsets[4];
    offsets[0] = -1.5;
    offsets[1] = -0.5;
    offsets[2] = 0.5;
    offsets[3] = 1.5;

    vec3 color = vec3(0.0);
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            vec2 texCoord;
            texCoord.x = vTexCoord.x + offsets[j] / uViewportSize.x;
            texCoord.y = vTexCoord.y + offsets[i] / uViewportSize.y;
            color += texture2D(uAuxTexture2, texCoord).rgb;
        }
    }

    color /= 16.0;
    gl_FragColor = texture2D(uAuxTexture1, vTexCoord) * vec4(color, 1.0);
}
