#version 330
in vec3 color_vs;

out vec4 color;

void main()
{
    color = vec4(color_vs, 1.0);
}
 