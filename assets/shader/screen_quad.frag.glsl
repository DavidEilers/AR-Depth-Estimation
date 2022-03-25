#version 330

uniform float offset=0.0;

in vec2 image_coord;
in vec2 disparity_coord;
layout (binding = 0) uniform sampler2D image_sampler;
layout (binding = 1) uniform sampler2D disparity_sampler;
uniform mat4 transform;
uniform mat4 eye_unproj;
uniform mat4 eye_to_cam;
uniform mat4 cam_proj;
uniform mat4 cam_unproj;
uniform mat4 hmd_to_cam;
uniform bool is_left = true;
out vec4 color;
out float gl_FragDepth;

/*vec4 unproj(){
    vec4 unproj_pos = cam_unproj*vec4(image_coord,0.75,1.0);
    unproj_pos = unproj_pos/abs(unproj_pos.w);
    //unproj_pos.z = -unproj_pos.z;
    return transform*vec4(unproj_pos.xyz, 1.0);
}*/

vec4 eye_nds_to_cam_nds(){
    vec4 unproj_pos = eye_unproj* vec4(image_coord,1.0,1.0);
    unproj_pos.z = -1;
    unproj_pos.w = 1;
    //vec4 unproj_pos = vec4(image_coord,-2.5,1.0);
    vec4 eye_nds = cam_proj*eye_to_cam*vec4(unproj_pos.xyzw);
    eye_nds = eye_nds/eye_nds.w;
    return eye_nds;
    
}

vec2 nds_to_0_1(vec4 coord){
    vec2 o = ((coord+1.0)/2.0).xy;
    o.y = 1-o.y;
    return o;
}

vec2 get_coord_left(vec2 coord){
    float factor = 1.0;
    return vec2(coord.x-0.25,coord.y*factor);
}

vec2 get_coord_right(vec2 coord){
    //-0.056
    float factor = 1.0;
    return vec2(coord.x+0.25,coord.y*factor);
}

void main()
{

    /*
    image_coord = (pos+1.0)/2.0;
    //image_coord = (transform*vec4(pos,0,1)).xy;
    disparity_coord = image_coord;
    if(is_upside_down)image_coord.y=1-image_coord.y;
    vec4 unproj_pos = cam_unproj*vec4(pos,0.0,1.0);
    unproj_pos = unproj_pos*unproj_pos.w;
    unproj_pos.z = -unproj_pos.z;
    gl_Position =  transform*vec4(unproj_pos.xyz, 1.0);*/


    vec2 coord = nds_to_0_1(eye_nds_to_cam_nds());//(unproj().xy+1.0)/2;//nds_to_0_1(unproj());//,vec2(0.0,0.0),vec2(2.0,1.0));
    //coord.y = 1-coord.y;
    //coord = coord + vec2(-1.0,-1.0);
    //coord = coord * vec2((108.0/112.0), 109.16/112.0);
    //coord.x = pow(coord.x,1/2.5);
    //coord = coord * vec2(2.0/108.02,2.0/109.16);
    //coord = coord + vec2(1.0,1.0);


    vec2 my_coord = clamp(coord,vec2(0,0),vec2(1,1));;
    if(is_left){
        my_coord = get_coord_left(coord);
        coord.y = 1- coord.y;
        gl_FragDepth = clamp(texture(disparity_sampler,coord).r,0.0,0.99);
    }else{
        //coord.x = coord.x+(0.007639*20);
        //coord.y = coord.y+(0.009016*2);
        my_coord = get_coord_right(coord);
        coord.y = 1- coord.y;
        gl_FragDepth = clamp(texture(disparity_sampler,coord).g,0.0,0.99);
    }
    
    color = vec4(texture(image_sampler,my_coord).rgb, 1.0);


  /*  vec2 my_coord = vec2(image_coord.x*0.5+offset, image_coord.y);
    color = vec4(texture(image_sampler,my_coord).rgb, 1.0);
    if(is_left){
    }else{
        gl_FragDepth = -0.01+texture(disparity_sampler,disparity_coord).g;
    }*/
}
 