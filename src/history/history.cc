#include <format>
#include <fstream>

#include "history/history.hh"
#include "utils.hh"

using history::Handler;


Handler::Handler(const std::filesystem::path &history_file) : m_first_run(true)
{
    if (history_file.empty()) { m_history_file = get_default_history_path(); }
    else
    {
        if (!std::filesystem::exists(history_file)
            || !std::filesystem::is_regular_file(history_file))
            m_history_file = get_default_history_path();

        m_history_file = history_file;
    }

    std::ifstream file { m_history_file };

    for (std::string line; std::getline(file, line);)
        m_lines.emplace_back(utils::str::trim(line));

    if (!m_lines.empty())
        m_idx = m_lines.size() - 1;
    else
        m_idx = 0; /* default to 0 if there no history */
}


auto
Handler::get_default_history_path() -> std::filesystem::path
{
    std::string cache_path { utils::getenv("XDG_HOME_CACHE",
                                           utils::getenv("HOME") + "/.cache") };

    if (cache_path.empty()) throw std::runtime_error("$HOME is not set");

    std::filesystem::path dir { std::format("{}/better/better-shell",
                                            cache_path) };
    std::filesystem::create_directories(dir);

    return (dir / "history");
}


void
Handler::push_back(const std::string &text)
{
    if (text.empty() || text[0] == '\n') return;
    if (!m_lines.empty() && m_lines.back() == text) return;

    std::string trimmed { utils::str::trim(text) };

    m_lines.emplace_back(trimmed);

    std::ofstream file { m_history_file, std::ios_base::app };
    if (file.is_open())
    {
        /* std::endl puts a newline and flush. */
        file << trimmed << std::endl; /* NOLINT */
    }
    file.close();
}


auto
Handler::get_next() -> std::optional<std::string>
{
    if (m_idx >= m_lines.size() - 1) return std::nullopt;

    m_idx++;
    return m_lines[m_idx];
}


auto
Handler::get_prev() -> std::optional<std::string>
{
    if (m_idx == 0) return m_lines[m_idx];

    if (!m_first_run) m_idx--;

    m_first_run = false;
    return m_lines[m_idx];
}


void
Handler::reset()
{
    m_idx       = m_lines.size() - 1;
    m_first_run = true;
}
