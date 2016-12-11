
attribute vec3 aPosition;
attribute vec4 aColor;

uniform mat4 uProjectionMatrix;

varying vec4 vColor;
varying vec4 vPosition;

void main()
{
    vec4 position = uProjectionMatrix * vec4(aPosition, 1.0);
    vPosition = position;
    vColor = aColor;
    gl_Position = position;
}
