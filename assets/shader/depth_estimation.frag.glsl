#version 330


in vec2 image_coord;
layout (binding = 0) uniform sampler2D left_eye_sampler;
layout (binding = 1) uniform sampler2D right_eye_sampler;
out vec4 color;

void main()
{
    vec2 my_coord = image_coord;
    color = vec4(texture(left_eye_sampler,my_coord).rgb, 1.0);
}
 