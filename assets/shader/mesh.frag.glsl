#version 330

in float depth;

out vec4 color;

void main()
{
    vec3 color_taint = vec3(1.0,0.0,0.0);
    color = vec4(depth*color_taint, 1.0);
}
 