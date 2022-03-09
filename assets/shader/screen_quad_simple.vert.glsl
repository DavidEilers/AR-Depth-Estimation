#version 330
layout(location = 0) in vec2 pos;

out vec2 image_coord;
uniform bool is_upside_down=false;

void main()
{
    image_coord = (pos+1.0)/2;
    if(is_upside_down)image_coord.y=1-image_coord.y;
    gl_Position =  vec4(pos, 0.0, 1.0);
}