#version 330
layout(location = 0) in vec2 pos;

out vec2 image_coord;

void main()
{
    image_coord = (pos+1.0)/2;
    gl_Position =  vec4(pos, 0.0, 1.0);
}