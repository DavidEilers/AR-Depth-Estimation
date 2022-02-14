#version 330

uniform float offset=0.0;

in vec2 image_coord;
layout (binding = 0) uniform sampler2D image_sampler;
out vec4 color;

void main()
{
    vec2 my_coord = vec2(image_coord.x*0.5+offset, image_coord.y);
    color = vec4(texture(image_sampler,my_coord).rgb, 1.0);
}
 