
attribute vec2 aPosition;
attribute vec2 aTexCoord;

varying vec2 vTexCoord;

void main()
{
    vec4 position = vec4(aPosition, 0.0, 1.0);
    vTexCoord = aTexCoord;
    gl_Position = position;
}
