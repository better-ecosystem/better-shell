#pragma once
#include <functional>
#include <string>
#include <vector>


namespace cmd::built_in
{
    using method_signature
        = std::function<void(const std::vector<std::string> &args)>;

    void cd(const std::vector<std::string> &args);
    void exit(const std::vector<std::string> &args);
    void pwd(const std::vector<std::string> &args);
    void calc(const std::vector<std::string> &args);


    const inline std::unordered_map<std::string, method_signature> COMMANDS {
        { "cd",   cd   },
        { "exit", exit },
        { "pwd",  pwd  },
        { "calc", calc },
    };


    inline bool SHOULD_EXIT { false };
}
