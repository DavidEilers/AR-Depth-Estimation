#version 330
layout(location = 0) in vec2 pos;

uniform mat4 transform;
uniform mat4 cam_unproj;
uniform bool is_upside_down=false;

out vec2 image_coord;
out vec2 disparity_coord;

void main()
{
    image_coord = pos;
    gl_Position = vec4(pos,0.0,1.0);;
   /* image_coord = (pos+1.0)/2.0;
    //image_coord = (transform*vec4(pos,0,1)).xy;
    disparity_coord = image_coord;
    if(is_upside_down)image_coord.y=1-image_coord.y;
    vec4 unproj_pos = cam_unproj*vec4(pos,0.0,1.0);
    unproj_pos = unproj_pos*unproj_pos.w;
    unproj_pos.z = -unproj_pos.z;
    gl_Position =  transform*vec4(unproj_pos.xyz, 1.0);*/
}