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

    void start_frame(){
        vr::VRActiveActionSet_t actionSet = { 0 };
        vr::VRCompositor()->WaitGetPoses(m_rTrackedDevicePose, vr::k_unTrackedDeviceIndex_Hmd, NULL,0);
        vr::VRInput()->UpdateActionState(&actionSet, sizeof(actionSet),1);
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

    GLuint get_left_framebuffer_texture_id(){
        return m_left_eye_framebuffer->get_framebuffer_resolve_texture_id();
    }

    GLuint get_right_framebuffer_texture_id(){
        return m_right_eye_framebuffer->get_framebuffer_resolve_texture_id();
    }

    void blit_frame_left(){
         size_t width = m_left_eye_framebuffer->get_width();
        size_t height = m_left_eye_framebuffer->get_height();
        glBindFramebuffer(GL_READ_FRAMEBUFFER, m_left_eye_framebuffer->get_framebuffer_id());
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_left_eye_framebuffer->get_framebuffer_resolve_id());
        glViewport(0,0,width,height);
        glBlitFramebuffer(0,0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glFlush();glFinish();
    }

    void blit_frame_right(){
        size_t width = m_right_eye_framebuffer->get_width();
        size_t height = m_right_eye_framebuffer->get_height();

        glBindFramebuffer(GL_READ_FRAMEBUFFER, m_right_eye_framebuffer->get_framebuffer_id());
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_right_eye_framebuffer->get_framebuffer_resolve_id()); //m_right_eye_framebuffer->get_framebuffer_resolve_id());
        glViewport(0,0,width,height);
        glBlitFramebuffer(0,0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
        
        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glFlush();glFinish();
    }

    
    std::string compo_err_to_str(vr::EVRCompositorError err){
        std::string out{};
        switch(err){
            case vr::EVRCompositorError::VRCompositorError_RequestFailed : out +="VRCompositorError_RequestFailed";break;
            case vr::EVRCompositorError::VRCompositorError_None : out +="VRCompositorError_None";break;
            case vr::EVRCompositorError::VRCompositorError_IncompatibleVersion : out +="VRCompositorError_IncompatibleVersion"; break;
            case vr::EVRCompositorError::VRCompositorError_DoNotHaveFocus	 : out +="VRCompositorError_DoNotHaveFocus";break;
            case vr::EVRCompositorError::VRCompositorError_InvalidTexture : out +="VRCompositorError_InvalidTexture";break;
            case vr::EVRCompositorError::VRCompositorError_IsNotSceneApplication : out +="VRCompositorError_IsNotSceneApplication";break;
            case vr::EVRCompositorError::VRCompositorError_TextureIsOnWrongDevice : out +="VRCompositorError_TextureIsOnWrongDevice";break;
            case vr::EVRCompositorError::VRCompositorError_TextureUsesUnsupportedFormat : out +="VRCompositorError_TextureUsesUnsupportedFormat";break;
            case vr::EVRCompositorError::VRCompositorError_SharedTexturesNotSupported : out +="VRCompositorError_SharedTexturesNotSupported";break;
            case vr::EVRCompositorError::VRCompositorError_IndexOutOfRange : out +="VRCompositorError_IndexOutOfRange";break;
            case vr::EVRCompositorError::VRCompositorError_AlreadySubmitted : out +="VRCompositorError_AlreadySubmitted";break;
            case vr::EVRCompositorError::VRCompositorError_InvalidBounds : out +="VRCompositorError_InvalidBounds";break;
            case vr::EVRCompositorError::VRCompositorError_AlreadySet : out +="VRCompositorError_AlreadySet";break;
            default: out +="UNKNOWN ERROR";break;
        }
        return out;
    }

    void submit_frames(){


        vr::Texture_t left_eye = {(void*)(uintptr_t)(m_left_eye_framebuffer->get_framebuffer_resolve_texture_id()), vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
        vr::EVRCompositorError err =  vr::VRCompositor()->Submit(vr::Eye_Left, &left_eye);
        if(err != vr::EVRCompositorError::VRCompositorError_None){
            logger_warn << "Compositor ERRROR in submit" << compo_err_to_str(err);
        }

        
        vr::Texture_t right_eye = {(void*)(uintptr_t)(m_right_eye_framebuffer->get_framebuffer_resolve_texture_id()), vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
        err = vr::VRCompositor()->Submit(vr::Eye_Right, &right_eye);
        glFinish();

        if(err != vr::EVRCompositorError::VRCompositorError_None){
            logger_warn << "Compositor ERRROR in submit" << compo_err_to_str(err);
        }
        
        //vr::VRCompositor()->PostPresentHandoff();
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