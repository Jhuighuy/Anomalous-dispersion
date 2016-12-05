#version 120

uniform sampler2D un_DiffuseTexture;
varying vec2 vr_TexCoord;
varying vec4 vr_Color;

void main(void)
{
    gl_FragColor = vr_Color * texture2D(un_DiffuseTexture, vr_TexCoord);
}
