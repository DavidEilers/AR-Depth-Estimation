#pragma once

#include <stdexcept>

#include <openvr.h>

#include "texture.hpp"
#include "framebuffer.hpp"
#include "log.hpp"

namespace arDepthEstimation
{
class Vr
{

    vr::IVRSystem *m_pHMD;
    vr::IVRTrackedCamera *m_pVRTrackedCamera;
    arDepthEstimation::LinearSampler sampler{};
    vr::CameraVideoStreamFrameHeader_t frameHeader;
    std::byte * m_framebuffer_data;
    size_t m_framebuffer_data_size=0;
    vr::TrackedCameraHandle_t m_hTrackedCamera;
    arDepthEstimation::Framebuffer * m_left_eye_framebuffer;
    arDepthEstimation::Framebuffer * m_right_eye_framebuffer;
	vr::TrackedDevicePose_t m_rTrackedDevicePose[ vr::k_unMaxTrackedDeviceCount ];

    public:
    arDepthEstimation::Texture * texture;
    void init_video_texture(){
        
        m_pVRTrackedCamera->AcquireVideoStreamingService(vr::k_unTrackedDeviceIndex_Hmd, &m_hTrackedCamera);
        if(m_hTrackedCamera == INVALID_TRACKED_CAMERA_HANDLE){
            throw std::runtime_error("couldn't acquire video stream");
        }
        vr::EVRTrackedCameraError nCameraError = m_pVRTrackedCamera->GetVideoStreamFrameBuffer( m_hTrackedCamera, vr::VRTrackedCameraFrameType_Undistorted, nullptr, 0, &frameHeader, sizeof( frameHeader ) );
        if(nCameraError != vr::VRTrackedCameraError_None){
            throw std::runtime_error("couldn't aquire camera frameHeader");
        }
        texture = new arDepthEstimation::Texture{frameHeader.nWidth,frameHeader.nHeight,GL_RGBA8,GL_UNSIGNED_BYTE,nullptr,&sampler,0};
        m_framebuffer_data_size = frameHeader.nWidth*frameHeader.nHeight*frameHeader.nBytesPerPixel*4;
        m_framebuffer_data = new std::byte[m_framebuffer_data_size];
    }

    void init_framebuffers(){
        uint32_t width, height;
        m_pHMD->GetRecommendedRenderTargetSize(&width, &height);
        logger_info << "Got recommended RenderTargetSize";
        m_left_eye_framebuffer = new Framebuffer{width,height};
        logger_info << "Created left eye framebuffer";
        m_right_eye_framebuffer = new Framebuffer{width,height};
        logger_info << "Created right eye framebuffer";

        if( !vr::VRCompositor() ){
            throw std::runtime_error("initialization of VRCompositor failed!");
        }
    }

  public:
    /**
     * @brief Construct and initialize a OpenVR instance
     *
     */
    Vr()
    {
        vr::EVRInitError error = vr::VRInitError_None;
        m_pHMD = vr::VR_Init(&error, vr::VRApplication_Scene);
        if (error != vr::VRInitError_None)
        {
           throw std::runtime_error("OpenVR couldn't initiliaze!");
        }
        bool has_camera = false;
        m_pVRTrackedCamera = vr::VRTrackedCamera();
        vr::EVRTrackedCameraError camera_error = m_pVRTrackedCamera->HasCamera(vr::k_unTrackedDeviceIndex_Hmd, &has_camera);
        if(camera_error != vr::VRTrackedCameraError_None || has_camera == false){
            throw std::runtime_error("no tracked camera available");
        }
        init_video_texture();
        logger_info << "trying to load framebuffers";
        init_framebuffers();
        logger_info << "loaded framebuffers";

    }

    void update_texture(){
        vr::EVRTrackedCameraError nCameraError = m_pVRTrackedCamera->GetVideoStreamFrameBuffer( m_hTrackedCamera, vr::VRTrackedCameraFrameType_Undistorted, m_framebuffer_data, m_framebuffer_data_size , &frameHeader, sizeof( frameHeader ) );
        if(nCameraError != vr::VRTrackedCameraError_None){
            throw std::runtime_error("update texture: VRTrackedCameraError");
        }
        texture->upload_texture(m_framebuffer_data);
    }

    void bind_left_eye(){
        m_left_eye_framebuffer->bind();
    }
    
    void bind_right_eye(){
        m_right_eye_framebuffer->bind();
    }
    void bind_window(){
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void submit_frames(){
        vr::VRActiveActionSet_t actionSet = { 0 };
        vr::VRCompositor()->WaitGetPoses(m_rTrackedDevicePose, vr::k_unTrackedDeviceIndex_Hmd, NULL,0);
        vr::VRInput()->UpdateActionState(&actionSet, sizeof(actionSet),1);
        vr::Texture_t left_eye = {(void*)(uintptr_t)(m_left_eye_framebuffer->get_framebuffer_id()), vr::TextureType_OpenGL, vr::ColorSpace_Auto };
        vr::VRCompositor()->Submit(vr::Eye_Left, &left_eye);

        
        vr::Texture_t right_eye = {(void*)(uintptr_t)(m_right_eye_framebuffer->get_framebuffer_id()), vr::TextureType_OpenGL, vr::ColorSpace_Auto };
        vr::VRCompositor()->Submit(vr::Eye_Right, &right_eye);
        glFlush();
        glFinish();
    }

    ~Vr()
    {
        // Shutdown OpenVR instance
        delete m_left_eye_framebuffer;
        delete m_right_eye_framebuffer;
        m_pVRTrackedCamera->ReleaseVideoStreamingService(m_hTrackedCamera);
        delete texture;
        delete[] m_framebuffer_data;
        vr::VR_Shutdown();
    }
};

} // namespace arDepthEstimation