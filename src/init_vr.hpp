#pragma once

#include <stdexcept>

#include <openvr.h>

namespace arDepthEstimation
{
class Vr
{

    vr::IVRSystem *m_pHMD;

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
            std::runtime_error("OpenVR couldn't initiliaze!");
        }
    }

    ~Vr()
    {
        // Shutdown OpenVR instance
        vr::VR_Shutdown();
    }
};

} // namespace arDepthEstimation