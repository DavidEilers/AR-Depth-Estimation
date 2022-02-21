#version 330
layout(location = 0) in vec2 pos;

out vec2 image_coord;

void main()
{
    image_coord = (pos+1.0)/2.0;
    image_coord = vec2(image_coord.x, 1.0-image_coord.y);
    gl_Position =  vec4(pos, 0.0, 1.0);
}