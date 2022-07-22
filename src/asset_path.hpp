#include <filesystem>
#include <initializer_list>
#include <string>

class AssetPath{
    const std::filesystem::path asset_path;

    public:    
    AssetPath():asset_path{std::filesystem::current_path()/"assets"}{}
    
    AssetPath(const std::filesystem::path asset_path_): asset_path{asset_path_}{}

    ~AssetPath(){}

    std::filesystem::path get_path(std::initializer_list<std::string> list){
        std::filesystem::path outPath = asset_path;
        for(auto e :list){
            outPath /= e;
        }
        return outPath;
    }

};
