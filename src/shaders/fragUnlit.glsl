#version 120

uniform vec4 un_Color;
varying vec4 vr_Color;

void main(void)
{
    gl_FragColor = un_Color * vr_Color;
}
