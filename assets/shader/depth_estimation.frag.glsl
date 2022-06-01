#version 430


in vec2 image_coord;
layout (binding = 0) uniform sampler2D left_eye_sampler;
layout (binding = 1) uniform sampler2D right_eye_sampler;
uniform ivec2 texture_size;
uniform bool is_rgb = false;
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

float rgb_to_luminance(vec3 rgb_vec){
    return dot(rgb_vec,vec3(0.299,0.587,0.144)); //Y channel of YUV color space => See RGB to YUV Color space conversion
}

vec4 kernel_filter33(sampler2D sampler_obj, vec2 coord, ivec2 size,float kernel[9]){
    vec4 filter_color = vec4(0.0);
    for(int i=0; i<3; i++){
        for(int j=0; j<3;j++){
            float texel_size_x = 1.0/size.x;
            float texel_size_y = 1.0/size.y;
            vec2 offset_coord = coord + vec2((i-1)*texel_size_x,(j-1)*texel_size_y);
            filter_color += kernel[j*3+i] * texture(sampler_obj,offset_coord);
        }
    }
    return filter_color;
}

double luminance_kernel_filter33(sampler2D sampler_obj, vec2 coord, ivec2 size, double kernel[9]){
    double filter_color = 0.0;
    for(int i=0; i<3; i++){
        for(int j=0; j<3;j++){
            float texel_size_x = 1.0/size.x;
            float texel_size_y = 1.0/size.y;
            vec2 offset_coord = coord + vec2((i-1)*texel_size_x,(j-1)*texel_size_y);
            filter_color += kernel[j*3+i] * dot(texture(sampler_obj,offset_coord).rgb,vec3(0.299,0.587,0.114));
        }
    }
    return filter_color;
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

dmat2 calc_tensor(sampler2D sampler_obj, vec2 coord, ivec2 size){
    dmat2 tensor = mat2(0.0);
    double vertical_sobel_kernel[9] = {
        1.0, 0.0, -1.0,
        2.0, 0.0, -2.0,
        1.0, 0.0, -1.0
    };

    double horizontal_sobel_kernel[9] = {
         1.0,  2.0,  1.0,
         0.0,  0.0,  0.0,
        -1.0, -2.0, -1.0
    };
    double x_gradiant = luminance_kernel_filter33(
                        left_eye_sampler,
                        coord,
                        texture_size,
                        vertical_sobel_kernel);
    double y_gradiant = luminance_kernel_filter33(
                        left_eye_sampler,
                        coord,
                        texture_size,
                        horizontal_sobel_kernel);
    tensor[0][0] = x_gradiant * x_gradiant;
    tensor[1][0] = x_gradiant * y_gradiant;
    tensor[0][1] = x_gradiant * y_gradiant;
    tensor[1][1] = y_gradiant * y_gradiant;
    
    return tensor;
}

dvec2 eigenvalues(dmat2 tensor){
    double a = tensor[0][0];
    double b = tensor[1][0];
    double c = tensor[1][1];
    double square_root_part = sqrt((a*a)-(2*a*c)+4*b*b+c*c+a+c);
    return dvec2(1/2 + square_root_part, 1/2 - square_root_part);
}

dvec2 calc_eigenvalues_from_sampler(sampler2D sampler_obj, vec2 pixel_offset, ivec2 size){ 
    vec2 texel_size;// = vec2(1.0)/texture_size;
    texel_size.x = 1.0 / size.x;
    texel_size.y = 1.0 / size.y;
    dmat2 tensor= calc_tensor(sampler_obj,image_coord+pixel_offset*texel_size,size);
    return eigenvalues(tensor);
}

float mat_dot(mat3 first, mat3 second){
    return 
    dot(first[0], second[0]) +
    dot(first[1], second[1]) +
    dot(first[2], second[2]);
}

int maximum(float x, float y){
    if(x>y){
        return 0;
    }
    return 1;
}

ivec2 maximum_mat(dmat3 comp){
    double maximum=comp[0][0];
    ivec2 indices = ivec2(0,0);
    for(int i = 0; i < 3; i++){
        for(int j = 0; j < 3; j++){
            if(comp[i][j]>maximum){
                maximum = comp[i][j];
                indices = ivec2(i,j);
            }
        }
    }
    return indices;
}

vec2 get_texel_size(){
    return vec2(1.0)/texture_size;
}

float sum_of_absolute_differences(int ring_buff_mid, mat3 ring_buffer_luminance_right, mat3 luminance_left){
    return
        dot( abs(abs(luminance_left[1])-abs(ring_buffer_luminance_right[(0+ring_buff_mid)%3])), vec3(1,1,1)) +
        dot( abs(abs(luminance_left[2])-abs(ring_buffer_luminance_right[(1+ring_buff_mid)%3])), vec3(1,1,1)) +
        dot( abs(abs(luminance_left[0])-abs(ring_buffer_luminance_right[(2+ring_buff_mid)%3])), vec3(1,1,1));
}

mat3 luminance_33(sampler2D sampler_obj, vec2 coord){
    vec2 texel_size = get_texel_size();
    mat3 luminance;
    vec2 filter_pixel_offset;
    vec2 filter_offset;
    for(int i = 0; i < 3; i++){
        for(int j = 0; j < 3; j++){
            filter_pixel_offset = vec2(i-1,j-1);
            filter_offset = filter_pixel_offset*texel_size;
            luminance[i][j] = texture(sampler_obj,coord+filter_offset).r;
        }
    }
    return luminance;
}

void fill_buff(in out mat3 buff, int row, sampler2D sampler_obj, vec2 pixel_coord){
    vec2 texel_size = get_texel_size();
    vec2 coord = pixel_coord*texel_size;
    buff[row][0] = texture(sampler_obj,coord+vec2(0,-1)*texel_size).r;
    buff[row][1] = texture(sampler_obj,coord+vec2(0,0)*texel_size).r;
    buff[row][2] = texture(sampler_obj,coord+vec2(0,1)*texel_size).r;
}

float calc_disparity_left_right(sampler2D image_left, sampler2D image_right, vec2 coord, ivec2 size){
    vec2 texel_size = get_texel_size();
    vec2 pos_left = coord/texel_size;
    //float result = 100;
    int max_x_distance= int(size.x*0.6);
    if(pos_left.x + max_x_distance < size.x){
        size.x = int(pos_left.x + max_x_distance);
    }
    mat3 luminance_left = luminance_33(image_left,coord);
    mat3 ring_buffer_texel_fetch;
    fill_buff(ring_buffer_texel_fetch,0,right_eye_sampler,pos_left+vec2(-1,0));
    fill_buff(ring_buffer_texel_fetch,1,right_eye_sampler,pos_left+vec2(0,0));
    int ring_buffer_mid = 1;
    for(vec2 pos_right = pos_left; pos_right.x <=size.x; pos_right.x++){
        fill_buff(ring_buffer_texel_fetch,int(ring_buffer_mid+1)%3,right_eye_sampler,pos_right+vec2(1,0));
        float tmp = sum_of_absolute_differences(ring_buffer_mid, ring_buffer_texel_fetch, luminance_left);
        ring_buffer_mid = int(ring_buffer_mid+1)%3;
        if(tmp<0.1){
            float x_offset = abs(pos_right.x-pos_left.x)*texel_size.x;
            return x_offset;
        }
    }
    return 1.0; //nothing found => maximum disparity
}

float calc_disparity_right_left(sampler2D image_left, sampler2D image_right, vec2 coord, ivec2 size){
    vec2 texel_size = get_texel_size();
    vec2 pos_right = coord/texel_size;
    vec2 pos_left = pos_right;
    //float result = 100;
    int max_x_distance= int(size.x*0.7);
    int min_x_pos = 0;
    if(pos_right.x - max_x_distance > 0){
        min_x_pos = int(pos_right.x - max_x_distance);
    }
    mat3 luminance_right = luminance_33(image_right,coord);
    mat3 ring_buffer_texel_fetch;
    fill_buff(ring_buffer_texel_fetch,2,left_eye_sampler,pos_left+vec2(1,0));
    fill_buff(ring_buffer_texel_fetch,1,left_eye_sampler,pos_left+vec2(0,0));
    int ring_buffer_mid = 1;
    for(; pos_left.x > min_x_pos ; pos_left.x--){
        fill_buff(ring_buffer_texel_fetch,int(ring_buffer_mid+2)%3,left_eye_sampler,pos_left+vec2(-1,0));
        float tmp = sum_of_absolute_differences(ring_buffer_mid, ring_buffer_texel_fetch, luminance_right);
        ring_buffer_mid = int(ring_buffer_mid+2)%3;
        if(tmp<0.1){
            float x_offset = abs(pos_right.x-pos_left.x)*texel_size.x;
            return x_offset;
        }
    }
    return 1.0; //nothing found => maximum disparity
}

void red_fill_buff(in out mat3 buff, int row, sampler2D sampler_obj, vec2 pixel_coord){
    vec2 texel_size = get_texel_size();
    vec2 coord = pixel_coord*texel_size;
    buff[row][0] = texture(sampler_obj,coord+vec2(0,-1)*texel_size).r;
    buff[row][1] = texture(sampler_obj,coord+vec2(0,0)*texel_size).r;
    buff[row][2] = texture(sampler_obj,coord+vec2(0,1)*texel_size).r;
}

void green_fill_buff(in out mat3 buff, int row, sampler2D sampler_obj, vec2 pixel_coord){
    vec2 texel_size = get_texel_size();
    vec2 coord = pixel_coord*texel_size;
    buff[row][0] = texture(sampler_obj,coord+vec2(0,-1)*texel_size).g;
    buff[row][1] = texture(sampler_obj,coord+vec2(0,0)*texel_size).g;
    buff[row][2] = texture(sampler_obj,coord+vec2(0,1)*texel_size).g;
}

void blue_fill_buff(in out mat3 buff, int row, sampler2D sampler_obj, vec2 pixel_coord){
    vec2 texel_size = get_texel_size();
    vec2 coord = pixel_coord*texel_size;
    buff[row][0] = texture(sampler_obj,coord+vec2(0,-1)*texel_size).b;
    buff[row][1] = texture(sampler_obj,coord+vec2(0,0)*texel_size).b;
    buff[row][2] = texture(sampler_obj,coord+vec2(0,1)*texel_size).b;
}

mat3 red_sampling_33(sampler2D sampler_obj, vec2 coord){
    vec2 texel_size = get_texel_size();
    mat3 luminance;
    vec2 filter_pixel_offset;
    vec2 filter_offset;
    for(int i = 0; i < 3; i++){
        for(int j = 0; j < 3; j++){
            filter_pixel_offset = vec2(i-1,j-1);
            filter_offset = filter_pixel_offset*texel_size;
            luminance[i][j] = texture(sampler_obj,coord+filter_offset).r;
        }
    }
    return luminance;
}

mat3 green_sampling_33(sampler2D sampler_obj, vec2 coord){
    vec2 texel_size = get_texel_size();
    mat3 luminance;
    vec2 filter_pixel_offset;
    vec2 filter_offset;
    for(int i = 0; i < 3; i++){
        for(int j = 0; j < 3; j++){
            filter_pixel_offset = vec2(i-1,j-1);
            filter_offset = filter_pixel_offset*texel_size;
            luminance[i][j] = texture(sampler_obj,coord+filter_offset).g;
        }
    }
    return luminance;
}

mat3 blue_sampling_33(sampler2D sampler_obj, vec2 coord){
    vec2 texel_size = get_texel_size();
    mat3 luminance;
    vec2 filter_pixel_offset;
    vec2 filter_offset;
    for(int i = 0; i < 3; i++){
        for(int j = 0; j < 3; j++){
            filter_pixel_offset = vec2(i-1,j-1);
            filter_offset = filter_pixel_offset*texel_size;
            luminance[i][j] = texture(sampler_obj,coord+filter_offset).b;
        }
    }
    return luminance;
}


float calc_disparity_left_right_RGB(sampler2D image_left, sampler2D image_right, vec2 coord, ivec2 size){
    vec2 texel_size = get_texel_size();
    vec2 pos_left = coord/texel_size;
    //float result = 100;
    int max_x_distance= int(size.x*0.6);
    if(pos_left.x + max_x_distance < size.x){
        size.x = int(pos_left.x + max_x_distance);
    }
    mat3 red_left = red_sampling_33(image_left,coord);
    mat3 green_left = green_sampling_33(image_left,coord);
    mat3 blue_left = blue_sampling_33(image_left,coord);
    mat3 red_ring_buffer_texel_fetch;
    mat3 green_ring_buffer_texel_fetch;
    mat3 blue_ring_buffer_texel_fetch;
    red_fill_buff(red_ring_buffer_texel_fetch,0,right_eye_sampler,pos_left+vec2(-1,0));
    red_fill_buff(red_ring_buffer_texel_fetch,1,right_eye_sampler,pos_left+vec2(0,0));
    green_fill_buff(green_ring_buffer_texel_fetch,0,right_eye_sampler,pos_left+vec2(-1,0));
    green_fill_buff(green_ring_buffer_texel_fetch,1,right_eye_sampler,pos_left+vec2(0,0));
    blue_fill_buff(blue_ring_buffer_texel_fetch,0,right_eye_sampler,pos_left+vec2(-1,0));
    blue_fill_buff(blue_ring_buffer_texel_fetch,1,right_eye_sampler,pos_left+vec2(0,0));
    int ring_buffer_mid = 1;
    for(vec2 pos_right = pos_left; pos_right.x <=size.x; pos_right.x++){
        red_fill_buff(red_ring_buffer_texel_fetch,int(ring_buffer_mid+1)%3,right_eye_sampler,pos_right+vec2(1,0));
        green_fill_buff(green_ring_buffer_texel_fetch,int(ring_buffer_mid+1)%3,right_eye_sampler,pos_right+vec2(1,0));
        blue_fill_buff(blue_ring_buffer_texel_fetch,int(ring_buffer_mid+1)%3,right_eye_sampler,pos_right+vec2(1,0));
        vec3 tmp = vec3( 
                        sum_of_absolute_differences(ring_buffer_mid, red_ring_buffer_texel_fetch, red_left),
                        sum_of_absolute_differences(ring_buffer_mid, green_ring_buffer_texel_fetch, green_left),
                        sum_of_absolute_differences(ring_buffer_mid, blue_ring_buffer_texel_fetch, blue_left)
                    );
        float minimum = min(min(tmp.r,tmp.g),tmp.b);
        ring_buffer_mid = int(ring_buffer_mid+1)%3;
        if(minimum<0.1){
            float x_offset = abs(pos_right.x-pos_left.x)*texel_size.x;
            return x_offset;
        }
    }
    return 1.0; //nothing found => maximum disparity
}

float calc_disparity_right_left_RGB(sampler2D image_left, sampler2D image_right, vec2 coord, ivec2 size){
    vec2 texel_size = get_texel_size();
    vec2 pos_right = coord/texel_size;
    vec2 pos_left = pos_right;
    //float result = 100;
    int max_x_distance= int(size.x*0.7);
    int min_x_pos = 0;
    if(pos_right.x - max_x_distance > 0){
        min_x_pos = int(pos_right.x - max_x_distance);
    }
    mat3 red_right = red_sampling_33(image_right,coord);
    mat3 green_right = green_sampling_33(image_right,coord);
    mat3 blue_right = blue_sampling_33(image_right,coord);
    mat3 red_ring_buffer_texel_fetch;
    mat3 green_ring_buffer_texel_fetch;
    mat3 blue_ring_buffer_texel_fetch;
    red_fill_buff(red_ring_buffer_texel_fetch,2,left_eye_sampler,pos_left+vec2(1,0));
    red_fill_buff(red_ring_buffer_texel_fetch,1,left_eye_sampler,pos_left+vec2(0,0));
    green_fill_buff(green_ring_buffer_texel_fetch,2,left_eye_sampler,pos_left+vec2(1,0));
    green_fill_buff(green_ring_buffer_texel_fetch,1,left_eye_sampler,pos_left+vec2(0,0));
    blue_fill_buff(blue_ring_buffer_texel_fetch,2,left_eye_sampler,pos_left+vec2(1,0));
    blue_fill_buff(blue_ring_buffer_texel_fetch,1,left_eye_sampler,pos_left+vec2(0,0));
    mat3 ring_buffer_texel_fetch;
    fill_buff(ring_buffer_texel_fetch,2,left_eye_sampler,pos_left+vec2(1,0));
    fill_buff(ring_buffer_texel_fetch,1,left_eye_sampler,pos_left+vec2(0,0));
    int ring_buffer_mid = 1;
    for(; pos_left.x > min_x_pos ; pos_left.x--){
        red_fill_buff(red_ring_buffer_texel_fetch,int(ring_buffer_mid+2)%3,left_eye_sampler,pos_left+vec2(-1,0));
        green_fill_buff(green_ring_buffer_texel_fetch,int(ring_buffer_mid+2)%3,left_eye_sampler,pos_left+vec2(-1,0));
        blue_fill_buff(blue_ring_buffer_texel_fetch,int(ring_buffer_mid+2)%3,left_eye_sampler,pos_left+vec2(-1,0));
        vec3 tmp = vec3(
                        sum_of_absolute_differences(ring_buffer_mid, red_ring_buffer_texel_fetch, red_right),
                        sum_of_absolute_differences(ring_buffer_mid, green_ring_buffer_texel_fetch, green_right),
                        sum_of_absolute_differences(ring_buffer_mid, blue_ring_buffer_texel_fetch, blue_right)
                    );
        float minimum = min(min(tmp.r,tmp.g),tmp.b);
        ring_buffer_mid = int(ring_buffer_mid+2)%3;
        if(minimum<0.1){
            float x_offset = abs(pos_right.x-pos_left.x)*texel_size.x;
            return x_offset;
        }
    }
    return 1.0; //nothing found => maximum disparity
}




void main()
{
    float gaussian_kernel[25]={     
        1.0/273, 4.0/273, 7.0/273, 4.0/273, 1.0/273,
        4.0/273, 16.0/273, 26.0/273, 16.0/273, 4.0/273,
        7.0/273, 26.0/273, 41.0/273, 26.0/273, 7.0/273,
        4.0/273, 16.0/273, 26.0/273, 16.0/273, 4.0/273,
        1.0/273, 4.0/273, 7.0/273, 4.0/273, 1.0/273
    };


    //vec3 original_color = texture(left_eye_sampler,image_coord).rgb;
    //vec3 yuv_original_color = rgb_to_yuv(original_color);
    //float filter_color_luminance = (1.0)*luminance_kernel_filter55(left_eye_sampler,image_coord,texture_size,gaussian_kernel);
    //float final_luminace = yuv_original_color.r - filter_color_luminance;
    //vec3 sobel_color = kernel_filter33(left_eye_sampler,image_coord,texture_size,horizontal_sobel_kernel).rgb+kernel_filter33(left_eye_sampler,image_coord,texture_size,vertical_sobel_kernel).rgb;
    //float sobel_luminance = luminance_kernel_filter33(left_eye_sampler,image_coord,texture_size,horizontal_sobel_kernel)+luminance_kernel_filter33(left_eye_sampler,image_coord,texture_size,vertical_sobel_kernel);
   
   /* mat2 tensor = calc_tensor(left_eye_sampler,image_coord,texture_size);
    vec2 texel_size = vec2(1.0)/texture_size.x;
    mat2 tensor_offset_minus_x = calc_tensor(left_eye_sampler,image_coord+2*texel_size,texture_size);
    vec2 eigen_values = eigenvalues(tensor);
    vec2 eigen_values_offset = eigenvalues(tensor_offset);*/
    /*dvec2 eigen_values_00 = calc_eigenvalues_from_sampler(left_eye_sampler,vec2(-1,-1),texture_size);
    dvec2 eigen_values_01 = calc_eigenvalues_from_sampler(left_eye_sampler,vec2(0,-1),texture_size);
    dvec2 eigen_values_02 = calc_eigenvalues_from_sampler(left_eye_sampler,vec2(1,-1),texture_size);

    dvec2 eigen_values_10 = calc_eigenvalues_from_sampler(left_eye_sampler,vec2(-1,0),texture_size);
    dvec2 eigen_values_11 = calc_eigenvalues_from_sampler(left_eye_sampler,vec2(0,0),texture_size);
    dvec2 eigen_values_12 = calc_eigenvalues_from_sampler(left_eye_sampler,vec2(1,0),texture_size);

    dvec2 eigen_values_20 = calc_eigenvalues_from_sampler(left_eye_sampler,vec2(-1,1),texture_size);
    dvec2 eigen_values_21 = calc_eigenvalues_from_sampler(left_eye_sampler,vec2(0,1),texture_size);
    dvec2 eigen_values_22 = calc_eigenvalues_from_sampler(left_eye_sampler,vec2(1,1),texture_size);

    dmat3 min_lambda = dmat3(0.0);
    min_lambda[0][0] = min(eigen_values_00.x,eigen_values_00.y);
    min_lambda[0][1] = min(eigen_values_01.x,eigen_values_01.y);
    min_lambda[0][2] = min(eigen_values_02.x,eigen_values_02.y);
    min_lambda[1][0] = min(eigen_values_10.x,eigen_values_10.y);
    min_lambda[1][1] = min(eigen_values_11.x,eigen_values_11.y);
    min_lambda[1][2] = min(eigen_values_12.x,eigen_values_12.y);
    min_lambda[2][0] = min(eigen_values_20.x,eigen_values_20.y);
    min_lambda[2][1] = min(eigen_values_21.x,eigen_values_21.y);
    min_lambda[2][2] = min(eigen_values_22.x,eigen_values_22.y);

    dmat3 vertical_sobel_kernel = dmat3(
         1.0,  2.0,  1.0,
         0.0,  0.0,  0.0,
        -1.0, -2.0, -1.0
    );

    dmat3 horizontal_sobel_kernel = dmat3(
        1.0, 0.0, -1.0,
        2.0, 0.0, -2.0,
        1.0, 0.0, -1.0
    );    
    
    dmat3 test_kernel = dmat3(
        -1.0, -1.0, -1.0,
        -1.0,  8.0, -1.0,
        -1.0, -1.0, -1.0
    );*/

    /*vec2 gradiant_min_lambda = vec2(
        mat_dot(horizontal_sobel_kernel,min_lambda),//dot(horizontal_sobel_kernel[0],min_lambda[0])+dot(horizontal_sobel_kernel[1],min_lambda[1])+dot(horizontal_sobel_kernel[2],min_lambda[2]),
        mat_dot(vertical_sobel_kernel,min_lambda)//dot(vertical_sobel_kernel[0],min_lambda[0])+dot(vertical_sobel_kernel[1],min_lambda[1])+dot(vertical_sobel_kernel[2],min_lambda[2])
    );*/
    vec3 test = vec3(1.0);

   /* if(min_lambda[1][1]>min_lambda[0][0] && min_lambda[1][1]>min_lambda[0][1] && min_lambda[1][1]>min_lambda[0][2] && min_lambda[1][1]>min_lambda[1][0] && min_lambda[1][1]>min_lambda[1][2] && min_lambda[1][1]>min_lambda[2][0] && min_lambda[1][1]>min_lambda[2][1] && min_lambda[1][1]>min_lambda[2][2]){
        test=vec3(0.0);
    }*/
    /*
    if( abs(gradiant_min_lambda.x) == 0.0 && abs(gradiant_min_lambda.y) == 0.0){
        test = vec3(0.0);
    }*/

    /*
    float threshold = 0.1;

    if(abs(min_lambda[1][1])>threshold){
        test= vec3(0.0,0.0,0.0);
     if(maximum_mat(min_lambda)==ivec2(1,1)){
        test = vec3(0.0,1.0,0.0);
        }
    }
    */
    

    /*if(min(eigen_values.x,abs(eigen_values.y)) > threshold){
    }*/
    //test = vec3(0,eigen_values.y*-1,0.0);
    //vec3 final_color = (test);//vec3(sobel_luminance);//final_luminace*10);
    vec3 final_color;
    if(is_rgb == true){
        final_color = vec3(calc_disparity_left_right_RGB(left_eye_sampler,right_eye_sampler,image_coord,texture_size),calc_disparity_right_left_RGB(left_eye_sampler,right_eye_sampler,image_coord,texture_size),0.0);
    }else{
        final_color= vec3(calc_disparity_left_right(left_eye_sampler,right_eye_sampler,image_coord,texture_size),calc_disparity_right_left(left_eye_sampler,right_eye_sampler,image_coord,texture_size),0.0);
        //float left_disp = calc_disparity_left_right(left_eye_sampler,right_eye_sampler,image_coord,texture_size);
        //final_color = vec3(left_disp);
    }
    //vec3 final_color= vec3(1-calc_disparity_left_right(left_eye_sampler,right_eye_sampler,image_coord,texture_size));
    //vec3 final_color= vec3(1-calc_disparity_right_left(left_eye_sampler,right_eye_sampler,image_coord,texture_size));
    //final_color = final_color*5;
    color = vec4(final_color,1.0);
    //color = vec4(vec3(texture(left_eye_sampler,image_coord).r),1.0);
}
 