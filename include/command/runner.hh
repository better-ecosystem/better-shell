#pragma once
#include <filesystem>
#include <string>
#include <unordered_map>


namespace cmd
{
    inline std::unordered_map<std::string, std::filesystem::path>
        BINARY_PATH_LIST;


    void fill_binary_path_list();
}
