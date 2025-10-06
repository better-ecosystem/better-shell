#include <filesystem>

#include "command/built_in.hh"
#include "print.hh"


namespace cmd::built_in
{
    void
    cd(const std::vector<std::string> &args)
    {
        std::filesystem::current_path(args[1]);
    }


    void
    exit(const std::vector<std::string> &/* args */)
    {
        SHOULD_EXIT = true;
    }


    void
    pwd(const std::vector<std::string> &/* args */)
    {
        io::println("{}", std::filesystem::current_path().string());
    }


    void
    calc(const std::vector<std::string> &args)
    {
        /* im not implementing a whole damn calculator here */
    }
}
