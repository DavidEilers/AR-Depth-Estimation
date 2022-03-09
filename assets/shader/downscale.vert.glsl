#version 430
layout(location = 0) in vec2 pos;

uniform bool is_upside_down = false;

out vec2 image_coord;

void main()
{
    image_coord = (pos+1.0)/2;

    if(is_upside_down==true)image_coord.y=1-image_coord.y;

    gl_Position =  vec4(pos, 0.0, 1.0);
}