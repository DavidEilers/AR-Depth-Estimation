#include <application.hpp>
#define PROJECT_NAME "arDepthEstimation"

int main(int argc, char **argv)
{
    MainApplication myApp{};
    ContextManager{&myApp};
    return 0;
}
