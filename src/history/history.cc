#include <format>

#include "history/history.hh"
#include "utils.hh"

using history::Handler;


Handler::Handler(const std::filesystem::path &history_file)
{
    if (history_file.empty()) { m_history_file = get_default_history_path(); }
    else
    {
        if (!std::filesystem::exists(history_file)
            || !std::filesystem::is_regular_file(history_file))
            m_history_file = get_default_history_path();

        m_history_file = history_file;
    }

    m_file = std::fstream { m_history_file };
}


auto
Handler::get_default_history_path() -> std::filesystem::path
{
    std::string cache_path { utils::getenv("XDG_HOME_CACHE",
                                           utils::getenv("HOME") + "/.cache") };

    if (cache_path.empty()) throw std::runtime_error("$HOME is not set");
    return std::format("{}/better/better-shell/history.json", cache_path);
}
