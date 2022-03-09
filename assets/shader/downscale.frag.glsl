#version 430


in vec2 image_coord;
layout (binding = 0) uniform sampler2D left_eye_sampler;
layout (binding = 1) uniform sampler2D right_eye_sampler;
layout (binding = 2) uniform sampler2D both_eye_sampler;
uniform ivec2 texture_size;
uniform bool is_single_texture = false;
uniform float gamma = 1.0;
layout(location = 0)out vec4 color_left;
layout(location = 1)out vec4 color_right;
vec2 texel_size;




float luminance_kernel_filter55(sampler2D sampler_obj, vec2 coord,float kernel[25], vec2 offset){
    float filter_color = 0.0;
    for(int i=0; i<5; i++){
        for(int j=0; j<5;j++){
            vec2 offset_coord = coord + vec2((i-2),(j-2))*texel_size;
            filter_color += kernel[j*5+i] * dot(pow(texture(sampler_obj,offset_coord+offset).rgb,vec3(1/gamma)),vec3(0.299,0.587,0.114));
        }
    }
    return filter_color;
}

float luminance_kernel_filter33(sampler2D sampler_obj, vec2 coord,float kernel[9], vec2 offset){
    float filter_color = 0.0;
    for(int i=0; i<3; i++){
        for(int j=0; j<3;j++){
            vec2 offset_coord = coord + vec2((i-1),(j-1))*texel_size;
            filter_color += kernel[j*5+i] * dot(texture(sampler_obj,offset_coord+offset).rgb,vec3(0.299,0.587,0.114));
        }
    }
    return filter_color;
}

vec2 get_texel_size(){
    return vec2(1.0)/texture_size;
}

    float gaussian_kernel33[9]={     
        1.0/16, 1.0/8, 1.0/16, 
        1.0/8, 1.0/4, 1.0/8,
        1.0/16, 1.0/8, 1.0/16};

    float gaussian_kernel55[25]={     
        1.0/273, 4.0/273, 7.0/273, 4.0/273, 1.0/273,
        4.0/273, 16.0/273, 26.0/273, 16.0/273, 4.0/273,
        7.0/273, 26.0/273, 41.0/273, 26.0/273, 7.0/273,
        4.0/273, 16.0/273, 26.0/273, 16.0/273, 4.0/273,
        1.0/273, 4.0/273, 7.0/273, 4.0/273, 1.0/273
    };

float luminance_gaussian55(sampler2D sampler_obj, vec2 coord, vec2 offset){
    return luminance_kernel_filter55(sampler_obj, coord, gaussian_kernel55, offset);
}

float luminance_gaussian33(sampler2D sampler_obj, vec2 coord, vec2 offset){

    return luminance_kernel_filter33(sampler_obj, coord, gaussian_kernel33, offset);
}



void main()
{
    texel_size = get_texel_size();

    if(is_single_texture== true){ // Left and right image are in one texture an o
        float left_luminance = luminance_gaussian55(both_eye_sampler,image_coord, vec2(0.0,0.0));
        color_left = vec4(vec3(left_luminance),1.0);
        float right_luminance = luminance_gaussian55(both_eye_sampler,image_coord,vec2(0.5,0.0));
        color_right = vec4(vec3(right_luminance),1.0);
    } else{
        float left_luminance = luminance_gaussian55(left_eye_sampler,image_coord, vec2(0.0,0.0));
        color_left = vec4(vec3(left_luminance),1.0);
        float right_luminance = luminance_gaussian55(right_eye_sampler,image_coord,vec2(0.0,0.0));
        color_right = vec4(vec3(right_luminance),1.0);
    }
}
 