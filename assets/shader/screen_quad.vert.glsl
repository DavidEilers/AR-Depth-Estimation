#version 330
layout(location = 0) in vec2 pos;

uniform mat4 transform;
uniform bool is_upside_down=false;

out vec2 image_coord;

void main()
{
    image_coord = (pos+1.0)/2.0;
    if(is_upside_down)image_coord.y=1-image_coord.y;
    gl_Position =  transform*vec4(pos, 0.0, 1.0);
}