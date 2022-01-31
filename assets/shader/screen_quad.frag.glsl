#version 330

in vec2 image_coord;
layout (binding = 0) uniform sampler2D image_sampler;
out vec4 color;

void main()
{
    color = vec4(texture(image_sampler,image_coord).rgb, 1.0);
}
 