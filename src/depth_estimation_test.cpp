#include <depth_estimation_test_application.hpp>
#define PROJECT_NAME "arDepthEstimation"

int main(int argc, char **argv)
{
    arDepthEstimation::MainApplication myApp{};
    // arDepthEstimation::ContextManager{&myApp};
    arDepthEstimation::run_app(&myApp);
    return 0;
}
