#pragma once

#include <stdexcept>

#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <openvr.h>
//#include <glm/gtc/matrix_transform.hpp>

#include "framebuffer.hpp"
#include "log.hpp"
#include "texture.hpp"

namespace arDepthEstimation
{
class Vr
{

    vr::IVRSystem *m_pHMD;
    vr::IVRTrackedCamera *m_pVRTrackedCamera;
    arDepthEstimation::LinearSampler m_sampler{};
    vr::CameraVideoStreamFrameHeader_t m_frameHeader;
    std::byte *m_framebuffer_data;
    size_t m_framebuffer_data_size = 0;
    vr::TrackedCameraHandle_t m_hTrackedCamera;
    arDepthEstimation::Framebuffer *m_left_eye_framebuffer;
    arDepthEstimation::Framebuffer *m_right_eye_framebuffer;
    vr::TrackedDevicePose_t m_rTrackedDevicePose[vr::k_unMaxTrackedDeviceCount];

  public:
    arDepthEstimation::Texture *m_texture;
    glm::mat4 m_eye_to_cam_left_mat[2];
    glm::mat4 m_eye_to_cam_right_mat[2];
    glm::mat4 m_eye_unproj[2];
    glm::mat4 m_hmd_to_cam[2];
    glm::mat4 m_view_to_eye_mat[2];
    glm::mat4 m_cam_proj_mat[2];;
    void init_video_texture()
    {

        m_pVRTrackedCamera->AcquireVideoStreamingService(vr::k_unTrackedDeviceIndex_Hmd, &m_hTrackedCamera);
        if (m_hTrackedCamera == INVALID_TRACKED_CAMERA_HANDLE)
        {
            throw std::runtime_error("couldn't acquire video stream");
        }
        vr::EVRTrackedCameraError nCameraError =
            m_pVRTrackedCamera->GetVideoStreamFrameBuffer(m_hTrackedCamera, vr::VRTrackedCameraFrameType_Distorted,
                                                          nullptr, 0, &m_frameHeader, sizeof(m_frameHeader));
        if (nCameraError != vr::VRTrackedCameraError_None)
        {
            throw std::runtime_error("couldn't aquire camera frameHeader");
        }
        m_sampler.initialize_sampler();
        m_texture = new arDepthEstimation::Texture{
            m_frameHeader.nWidth, m_frameHeader.nHeight, GL_RGBA8, GL_UNSIGNED_BYTE, nullptr, &m_sampler, 0};
        m_framebuffer_data_size = m_frameHeader.nWidth * m_frameHeader.nHeight * m_frameHeader.nBytesPerPixel * 4;
        m_framebuffer_data = new std::byte[m_framebuffer_data_size];
    }

    void init_framebuffers()
    {
        uint32_t width, height;
        m_pHMD->GetRecommendedRenderTargetSize(&width, &height);
        logger_info << "Got recommended RenderTargetSize";
        m_left_eye_framebuffer = new Framebuffer{width, height};
        logger_info << "Created left eye framebuffer";
        m_right_eye_framebuffer = new Framebuffer{width, height};
        logger_info << "Created right eye framebuffer";

        if (!vr::VRCompositor())
        {
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
        vr::EVRTrackedCameraError camera_error =
            m_pVRTrackedCamera->HasCamera(vr::k_unTrackedDeviceIndex_Hmd, &has_camera);
        if (camera_error != vr::VRTrackedCameraError_None || has_camera == false)
        {
            throw std::runtime_error("no tracked camera available");
        }
        init_video_texture();
        logger_info << "trying to load framebuffers";
        init_framebuffers();
        logger_info << "loaded framebuffers";
    }

    void update_texture()
    {
        vr::EVRTrackedCameraError nCameraError = m_pVRTrackedCamera->GetVideoStreamFrameBuffer(
            m_hTrackedCamera, vr::VRTrackedCameraFrameType_Undistorted, m_framebuffer_data, m_framebuffer_data_size,
            &m_frameHeader, sizeof(m_frameHeader));
        if (nCameraError != vr::VRTrackedCameraError_None)
        {
            throw std::runtime_error("update texture: VRTrackedCameraError");
        }
        m_texture->upload_texture(m_framebuffer_data);
    }

    void start_frame()
    {
        vr::VRActiveActionSet_t actionSet = {0};
        vr::VRCompositor()->WaitGetPoses(m_rTrackedDevicePose, vr::k_unTrackedDeviceIndex_Hmd, NULL, 0);
        vr::VRInput()->UpdateActionState(&actionSet, sizeof(actionSet), 1);
    }

    inline glm::mat4 ovr34_to_glm44(vr::HmdMatrix34_t &m)
    {
        glm::mat4 result = glm::mat4(m.m[0][0], m.m[1][0], m.m[2][0], 0.0, m.m[0][1], m.m[1][1], m.m[2][1], 0.0,
                                     m.m[0][2], m.m[1][2], m.m[2][2], 0.0, m.m[0][3], m.m[1][3], m.m[2][3], 1.0f);
        return result;
    }

    inline glm::mat4 ovr44_to_glm44(vr::HmdMatrix44_t &m)
    {
        glm::mat4 result =
            glm::mat4(m.m[0][0], m.m[1][0], m.m[2][0], m.m[3][0], m.m[0][1], m.m[1][1], m.m[2][1], m.m[3][1], m.m[0][2],
                      m.m[1][2], m.m[2][2], m.m[3][2], m.m[0][3], m.m[1][3], m.m[2][3], m.m[3][3]);
        return result;
    }

    void update_camera_transform_matrix()
    {

        logger_info << "get camera projection";
        vr::HmdMatrix44_t ovr_cameraProjection[2];
        m_pVRTrackedCamera->GetCameraProjection(vr::k_unTrackedDeviceIndex_Hmd, vr::Eye_Left,
                                                vr::VRTrackedCameraFrameType_Undistorted, 0.1, 10,
                                                &(ovr_cameraProjection[0]));
        m_pVRTrackedCamera->GetCameraProjection(vr::k_unTrackedDeviceIndex_Hmd, vr::Eye_Right,
                                                vr::VRTrackedCameraFrameType_Undistorted, 0.1 , 10,
                                                &(ovr_cameraProjection[1]));
        glm::mat4 camera_projection[2];
        camera_projection[0] = ovr44_to_glm44(ovr_cameraProjection[0]);
        camera_projection[1] = ovr44_to_glm44(ovr_cameraProjection[1]);

        logger_info << "get camera to head";
        vr::HmdMatrix34_t ovr_camera_to_head_mat[2];
        vr::VRSystem()->GetArrayTrackedDeviceProperty(
            vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_CameraToHeadTransforms_Matrix34_Array, vr::k_unHmdMatrix34PropertyTag,
            &ovr_camera_to_head_mat, sizeof(ovr_camera_to_head_mat));
        glm::mat4 camera_to_head_mat[2];
        camera_to_head_mat[0] = ovr34_to_glm44(ovr_camera_to_head_mat[0]);
        camera_to_head_mat[1] = ovr34_to_glm44(ovr_camera_to_head_mat[1]);//camera_to_head_mat[0];//(camera_to_head_mat[0]);//ovr34_to_glm44(ovr_camera_to_head_mat[1]);
        //camera_to_head_mat[1][3][0] = -camera_to_head_mat[1][3][0];
        logger_info << "camera left to head x y z offset = " << camera_to_head_mat[0][3][0] << camera_to_head_mat[0][3][1] <<camera_to_head_mat[0][3][2] ;
        logger_info << "camera right to head x offset = " << camera_to_head_mat[1][3][0] << camera_to_head_mat[1][3][1] << camera_to_head_mat[1][3][2];

        logger_info << "get eye to head transform";
        vr::HmdMatrix34_t ovr_eye_to_head_mat[2];

        typedef void (vr::IVRSystem::*fn_e2h_ptr)(vr::HmdMatrix34_t *, vr::EVREye);
        fn_e2h_ptr get_eye_to_head_transform = reinterpret_cast<fn_e2h_ptr>(&vr::IVRSystem::GetEyeToHeadTransform);

        (m_pHMD->*get_eye_to_head_transform)(&(ovr_eye_to_head_mat[0]), vr::Eye_Left);
        (m_pHMD->*get_eye_to_head_transform)(&(ovr_eye_to_head_mat[1]), vr::Eye_Right);

        glm::mat4 eye_to_head_transform[2];
        eye_to_head_transform[0] = ovr34_to_glm44(ovr_eye_to_head_mat[0]);
        eye_to_head_transform[1] = ovr34_to_glm44(ovr_eye_to_head_mat[1]);

        logger_info << "get eye projection matrix";
        vr::HmdMatrix44_t ovr_eye_proj[2];

        typedef void (vr::IVRSystem::*fn_proj_ptr)(vr::HmdMatrix44_t *, vr::EVREye, float, float);
        fn_proj_ptr get_projection_matrix = reinterpret_cast<fn_proj_ptr>(&vr::IVRSystem::GetProjectionMatrix);
        (m_pHMD->*get_projection_matrix)(&(ovr_eye_proj[0]), vr::Eye_Left, 0.1, 10);
        (m_pHMD->*get_projection_matrix)(&(ovr_eye_proj[1]), vr::Eye_Right, 0.1, 10);
        logger_info << "convert to glm mat4";
        glm::mat4 eye_proj[2];
        eye_proj[0] = ovr44_to_glm44(ovr_eye_proj[0]);
        eye_proj[1] = ovr44_to_glm44(ovr_eye_proj[1]);

        m_eye_unproj[0]= glm::inverse(eye_proj[0]);
        m_eye_unproj[1]= glm::inverse(eye_proj[1]);
        m_hmd_to_cam[0]= glm::inverse(camera_to_head_mat[0]);
        m_hmd_to_cam[1]= glm::inverse(camera_to_head_mat[1]);

        //m_cam_to_eye_mat[0] =eye_proj[0] * eye_to_head_transform[1] * camera_to_head_mat[0] * glm::inverse(camera_projection[0]) ;

        //eye_to_head_transform[1]*camera_to_head_mat[1]
        m_cam_proj_mat[0]= camera_projection[0];
        m_cam_proj_mat[1]= camera_projection[1];
        m_eye_to_cam_left_mat[0] = glm::inverse(camera_to_head_mat[0])*eye_to_head_transform[0];
        m_eye_to_cam_left_mat[1] = glm::inverse(camera_to_head_mat[1])*eye_to_head_transform[0];
        m_eye_to_cam_right_mat[0] = glm::inverse(camera_to_head_mat[0])*eye_to_head_transform[1];
        m_eye_to_cam_right_mat[1] = glm::inverse(camera_to_head_mat[1])*eye_to_head_transform[1];

        //m_cam_to_eye_mat[0] = (glm::inverse(camera_projection[0])) * camera_to_head_mat[0] *
        //                      (glm::inverse(eye_to_head_transform[0])) * eye_proj[0];
       // m_cam_to_eye_mat[1] = (glm::inverse(camera_projection[1])) * camera_to_head_mat[1] *
        //                      (glm::inverse(eye_to_head_transform[1])) * eye_proj[1];
        m_view_to_eye_mat[0] = eye_proj[0]*eye_to_head_transform[1];// * eye_proj[0] ; 
        m_view_to_eye_mat[1] = eye_proj[1]*eye_to_head_transform[0];// * eye_proj[1] ; 

        logger_info << "finished update camera transform matrix";
    }

    void bind_left_eye()
    {
        m_left_eye_framebuffer->bind();
    }

    void bind_right_eye()
    {
        m_right_eye_framebuffer->bind();
    }
    void bind_window()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    GLuint get_left_framebuffer_texture_id()
    {
        return m_left_eye_framebuffer->get_framebuffer_resolve_texture_id();
    }

    GLuint get_right_framebuffer_texture_id()
    {
        return m_right_eye_framebuffer->get_framebuffer_resolve_texture_id();
    }

    void blit_frame_left()
    {
        size_t width = m_left_eye_framebuffer->get_width();
        size_t height = m_left_eye_framebuffer->get_height();
        glBindFramebuffer(GL_READ_FRAMEBUFFER, m_left_eye_framebuffer->get_framebuffer_id());
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_left_eye_framebuffer->get_framebuffer_resolve_id());
        glViewport(0, 0, width, height);
        glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glFlush();
        glFinish();
    }

    void blit_frame_right()
    {
        size_t width = m_right_eye_framebuffer->get_width();
        size_t height = m_right_eye_framebuffer->get_height();

        glBindFramebuffer(GL_READ_FRAMEBUFFER, m_right_eye_framebuffer->get_framebuffer_id());
        glBindFramebuffer(
            GL_DRAW_FRAMEBUFFER,
            m_right_eye_framebuffer
                ->get_framebuffer_resolve_id()); // m_right_eye_framebuffer->get_framebuffer_resolve_id());
        glViewport(0, 0, width, height);
        glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_LINEAR);

        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glFlush();
        glFinish();
    }

    std::string compo_err_to_str(vr::EVRCompositorError err)
    {
        std::string out{};
        switch (err)
        {
        case vr::EVRCompositorError::VRCompositorError_RequestFailed:
            out += "VRCompositorError_RequestFailed";
            break;
        case vr::EVRCompositorError::VRCompositorError_None:
            out += "VRCompositorError_None";
            break;
        case vr::EVRCompositorError::VRCompositorError_IncompatibleVersion:
            out += "VRCompositorError_IncompatibleVersion";
            break;
        case vr::EVRCompositorError::VRCompositorError_DoNotHaveFocus:
            out += "VRCompositorError_DoNotHaveFocus";
            break;
        case vr::EVRCompositorError::VRCompositorError_InvalidTexture:
            out += "VRCompositorError_InvalidTexture";
            break;
        case vr::EVRCompositorError::VRCompositorError_IsNotSceneApplication:
            out += "VRCompositorError_IsNotSceneApplication";
            break;
        case vr::EVRCompositorError::VRCompositorError_TextureIsOnWrongDevice:
            out += "VRCompositorError_TextureIsOnWrongDevice";
            break;
        case vr::EVRCompositorError::VRCompositorError_TextureUsesUnsupportedFormat:
            out += "VRCompositorError_TextureUsesUnsupportedFormat";
            break;
        case vr::EVRCompositorError::VRCompositorError_SharedTexturesNotSupported:
            out += "VRCompositorError_SharedTexturesNotSupported";
            break;
        case vr::EVRCompositorError::VRCompositorError_IndexOutOfRange:
            out += "VRCompositorError_IndexOutOfRange";
            break;
        case vr::EVRCompositorError::VRCompositorError_AlreadySubmitted:
            out += "VRCompositorError_AlreadySubmitted";
            break;
        case vr::EVRCompositorError::VRCompositorError_InvalidBounds:
            out += "VRCompositorError_InvalidBounds";
            break;
        case vr::EVRCompositorError::VRCompositorError_AlreadySet:
            out += "VRCompositorError_AlreadySet";
            break;
        default:
            out += "UNKNOWN ERROR";
            break;
        }
        return out;
    }

    void submit_frames()
    {

        vr::Texture_t left_eye = {(void *)(uintptr_t)(m_left_eye_framebuffer->get_framebuffer_resolve_texture_id()),
                                  vr::TextureType_OpenGL, vr::ColorSpace_Gamma};
        vr::EVRCompositorError err = vr::VRCompositor()->Submit(vr::Eye_Left, &left_eye);
        if (err != vr::EVRCompositorError::VRCompositorError_None)
        {
            logger_warn << "Compositor ERRROR in submit" << compo_err_to_str(err);
        }

        vr::Texture_t right_eye = {(void *)(uintptr_t)(m_right_eye_framebuffer->get_framebuffer_resolve_texture_id()),
                                   vr::TextureType_OpenGL, vr::ColorSpace_Gamma};
        err = vr::VRCompositor()->Submit(vr::Eye_Right, &right_eye);
        glFinish();

        if (err != vr::EVRCompositorError::VRCompositorError_None)
        {
            logger_warn << "Compositor ERRROR in submit" << compo_err_to_str(err);
        }

        // vr::VRCompositor()->PostPresentHandoff();
    }

    ~Vr()
    {
        // Shutdown OpenVR instance
        delete m_left_eye_framebuffer;
        delete m_right_eye_framebuffer;
        m_pVRTrackedCamera->ReleaseVideoStreamingService(m_hTrackedCamera);
        delete m_texture;
        delete[] m_framebuffer_data;
        vr::VR_Shutdown();
    }
};

} // namespace arDepthEstimation