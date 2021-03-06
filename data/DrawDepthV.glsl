
attribute vec3 aPosition;
attribute vec4 aColor;

uniform mat4 uProjectionMatrix;

varying vec4 vColor;

void main()
{
    vec4 position = uProjectionMatrix * vec4(aPosition, 1.0);
    vColor = aColor;
    gl_Position = position;
}
