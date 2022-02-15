#include <application_vr.hpp>
#define PROJECT_NAME "arDepthEstimation"

int main(int argc, char **argv)
{
    arDepthEstimation::MainApplication myApp{};
    arDepthEstimation::ContextManager{&myApp};
    return 0;
}
