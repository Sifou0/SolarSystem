#version 130
precision mediump float;

attribute vec3 vPosition;
attribute vec3 vNormal;
attribute vec2 vUV;
uniform mat4 uMVP;
uniform mat4 uModel;
uniform mat3 uInvModel3x3;

varying vec3 vary_normal;
varying vec4 vary_world_position;

varying vec2 vary_uv;

void main()
{
    gl_Position = uMVP*vec4(vPosition, 1.0);
    vary_normal = vec3(uModel*vec4(vNormal,0.0));

    vary_world_position = uModel * vec4(vPosition, 1.0);
    vary_world_position = vary_world_position / vary_world_position.w; //Normalization from w
    vary_uv = vUV;
}