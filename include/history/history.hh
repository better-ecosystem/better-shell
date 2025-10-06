#pragma once
#include <filesystem>
#include <optional>
#include <vector>


namespace history
{
    class Handler
    {
    public:
        /**
         * creates a history handler with the history list file pointing at
         * @p history_file
         * ----------------------------------------------------------------
         *
         * if @p history_file is empty, the constructor will use the default
         * path given by history::Handler::get_default_history_path()
         */
        Handler(const std::filesystem::path &history_file);


        /**
         * adds @p text to the end to the history list
         */
        void push_back(const std::string &text);


        /**
         * get the next text in the history list and increment the line index
         * -------------------------------------
         *
         * the function will return an std::nullopt if the internal
         * line index points at the end of the list
         */
        [[nodiscard]]
        auto get_next() -> std::optional<std::string>;


        /**
         * get the previous text in the history list
         * and decrement the line index
         * -----------------------------------------
         *
         * the function will never return std::nullopt
         * instead, the function will always return the line pointed
         * by the index even though it cannot decrement the index anymore
         */
        [[nodiscard]]
        auto get_prev() -> std::optional<std::string>;


        /**
         * resets the index, i.e., makes it point to the last text in the list
         */
        void reset();

    private:
        std::filesystem::path m_history_file;

        std::vector<std::string> m_lines;
        size_t                   m_idx;


        [[nodiscard]]
        static auto get_default_history_path() -> std::filesystem::path;
    };
}
