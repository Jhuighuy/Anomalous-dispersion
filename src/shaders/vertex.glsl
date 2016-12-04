#version 140

attribute vec4 in_VertexCoordMS;
attribute vec2 in_TexCoord;
attribute vec3 in_NormalMS;
attribute vec4 in_Color;

uniform mat4 un_ModelMatrix;
uniform mat4 un_ViewProjectionMatrix;

varying vec4 vr_VertexCoordWS;
varying vec2 vr_TexCoord;
varying vec3 vr_NormalWS;
varying vec4 vr_Color;

void main(void)
{
    vr_VertexCoordWS = un_ModelMatrix * in_VertexCoordMS;
    vr_TexCoord = in_TexCoord;
    vr_NormalWS = mat3(inverse(transpose(un_ModelMatrix))) * in_NormalMS;
    vr_Color = in_Color;

    gl_Position = un_ViewProjectionMatrix * vr_VertexCoordWS;
}
