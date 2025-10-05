#pragma once
#include <filesystem>
#include <fstream>
#include <optional>

#include "parser/tokenizer.hh"


namespace history
{
    class Handler
    {
    public:
        /**
         * If history_file is empty, the _ctor_ will use
         * $XDG_HOME_CACHE/better/better-shell or
         * $HOME/.cache/better/better-shell as the history file.
         */
        Handler(const std::filesystem::path &history_file);


        void push_back(const parser::Token &token);


        /**
         * @brief Get the next token in the history list.
         *
         * Will increment the internal "current-token" index, which
         * starts at the amount of lines in the history file,
         * which means it will point to the last inserted token.
         * When the index reaches the maximum size, and _get_next_ is called
         * again, _get_next_ will return an std::nullopt.
         */
        [[nodiscard]]
        auto get_next() -> std::optional<parser::Token>;


        /**
         * @brief Get the next token in the history list.
         *
         * Will decrement the internal "current-token" index, which starts
         * at the amount of the lines in the history file,
         * which means it will point to the last inserted token.
         * When the index reaches 0, and _get_prev_ is called again,
         * _get_prev_ will return an std::nullopt.
         */
        [[nodiscard]]
        auto get_prev() -> std::optional<parser::Token>;

    private:
        std::filesystem::path m_history_file;
        std::fstream          m_file;

        size_t m_max_idx;
        size_t m_current_token;


        [[nodiscard]]
        static auto get_default_history_path() -> std::filesystem::path;
    };
}
