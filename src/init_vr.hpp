#pragma once

#include <stdexcept>

#include <openvr.h>

#include "texture.hpp"

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
    }

    void update_texture(){
        vr::EVRTrackedCameraError nCameraError = m_pVRTrackedCamera->GetVideoStreamFrameBuffer( m_hTrackedCamera, vr::VRTrackedCameraFrameType_Undistorted, m_framebuffer_data, m_framebuffer_data_size , &frameHeader, sizeof( frameHeader ) );
        if(nCameraError != vr::VRTrackedCameraError_None){
            throw std::runtime_error("update texture: VRTrackedCameraError");
        }
        texture->upload_texture(m_framebuffer_data);
    }

    ~Vr()
    {
        // Shutdown OpenVR instance
        m_pVRTrackedCamera->ReleaseVideoStreamingService(m_hTrackedCamera);
        delete texture;
        delete[] m_framebuffer_data;
        vr::VR_Shutdown();
    }
};

} // namespace arDepthEstimation