#version 330
uniform mat4 mvp;
layout(location=0) in vec3 vPos;

out float depth;

void main()
{
    vec4 pos = mvp * vec4(vPos, 1.0);
    depth = pos.z;
    gl_Position = pos;
}