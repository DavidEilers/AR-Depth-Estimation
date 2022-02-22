#version 330


in vec2 image_coord;
layout (binding = 0) uniform sampler2D left_eye_sampler;
layout (binding = 1) uniform sampler2D right_eye_sampler;
uniform ivec2 texture_size;
out vec4 color;


vec4 box_filter33(sampler2D sampler_obj, vec2 coord, ivec2 size){
    vec4 filter_color = vec4(0.0);
    for(int i=0; i<3; i++){
        for(int j=0; j<3;j++){
            float x_size = 1.0/size.x;
            float y_size = 1.0/size.y;
            vec2 offset_coord = coord + vec2((i-1)*x_size,(j-1)*y_size);
            filter_color += (1.0/9) * texture(sampler_obj,offset_coord);
        }
    }
    return filter_color;
}

vec4 box_filter55(sampler2D sampler_obj, vec2 coord, ivec2 size){
    vec4 filter_color = vec4(0.0);
    for(int i=0; i<5; i++){
        for(int j=0; j<5;j++){
            float x_size = 1.0/size.x;
            float y_size = 1.0/size.y;
            vec2 offset_coord = coord + vec2((i-2)*x_size,(j-2)*y_size);
            filter_color += (1.0/25) * texture(sampler_obj,offset_coord);
        }
    }
    return filter_color;
}

vec3 rgb_to_yuv(vec3 rgb_vec ){
    vec3 yuv_vec = vec3(0.0,0.0,0.0);
    yuv_vec.r = (0.299*rgb_vec.r) + (0.587* rgb_vec.g) + (0.114*rgb_vec.b);
    yuv_vec.g = 0.493*(rgb_vec.b - yuv_vec.r);
    yuv_vec.b = 0.877*(rgb_vec.r - yuv_vec.r);
    return yuv_vec;
}


vec4 kernel_filter55(sampler2D sampler_obj, vec2 coord, ivec2 size,float kernel[25]){
    vec4 filter_color = vec4(0.0);
    for(int i=0; i<5; i++){
        for(int j=0; j<5;j++){
            float texel_size_x = 1.0/size.x;
            float texel_size_y = 1.0/size.y;
            vec2 offset_coord = coord + vec2((i-2)*texel_size_x,(j-2)*texel_size_y);
            filter_color += kernel[j*5+i] * texture(sampler_obj,offset_coord);
        }
    }
    return filter_color;
}

float luminance_kernel_filter55(sampler2D sampler_obj, vec2 coord, ivec2 size,float kernel[25]){
    float filter_color = 0.0;
    for(int i=0; i<5; i++){
        for(int j=0; j<5;j++){
            float texel_size_x = 1.0/size.x;
            float texel_size_y = 1.0/size.y;
            vec2 offset_coord = coord + vec2((i-2)*texel_size_x,(j-2)*texel_size_y);
            filter_color += kernel[j*5+i] * dot(texture(sampler_obj,offset_coord).rgb,vec3(0.299,0.587,0.114));
        }
    }
    return filter_color;
}

void main()
{
    float gaussian_kernel[25]={     1.0/273, 4.0/273, 7.0/273, 4.0/273, 1.0/273,
                                    4.0/273, 16.0/273, 26.0/273, 16.0/273, 4.0/273,
                                    7.0/273, 26.0/273, 41.0/273, 26.0/273, 7.0/273,
                                    4.0/273, 16.0/273, 26.0/273, 16.0/273, 4.0/273,
                                    1.0/273, 4.0/273, 7.0/273, 4.0/273, 1.0/273};
    vec3 original_color = texture(left_eye_sampler,my_coord).rgb;
    vec3 yuv_original_color = rgb_to_yuv(original_color);
    float filter_color_luminance = (1.0)*luminance_kernel_filter55(left_eye_sampler,image_coord,texture_size,gaussian_kernel);
    float final_luminace = yuv_original_color.r - filter_color_luminance;
    vec3 final_color = vec3(final_luminace*10);
    color = vec4(original_color*0.3+final_color,1.0);
}
 