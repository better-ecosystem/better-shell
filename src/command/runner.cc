#include "command/runner.hh"
#include "utils.hh"


namespace cmd
{
    void
    fill_binary_path_list()
    {
        std::string path { utils::getenv(
            "PATH", "/usr/local/sbin:/usr/local/bin:/usr/bin") };

        std::istringstream iss { path };
        for (std::string dir; std::getline(iss, dir, ':');)
        {
            if (dir.empty()) continue;

            std::filesystem::recursive_directory_iterator it { dir };
            for (const auto &entry : it)
            {
                if (!entry.is_regular_file()) continue;

                const auto &file { entry.path() };
                if (access(file.c_str(), X_OK) == 0)
                    BINARY_PATH_LIST[file.filename().string()] = file;
            }
        }
    }
}
