#version 140

uniform sampler2D un_Texture;
varying vec2 vr_TexCoord;
varying vec4 vr_Color;

void main(void)
{
    gl_FragColor = vr_Color * texture2D(un_Texture, vr_TexCoord);
}
