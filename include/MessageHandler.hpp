#pragma once
#include <Geode/Geode.hpp>
#include "NetworkManager.hpp"

class MessageHandler {
    public:
    static MessageHandler& get() {
        static MessageHandler instance;
        return instance;
    }

    // static std::vector<std::string> split(std::string_view, std::string_view, int limit = 0);
    void openAlert(std::string_view);
    void onMessage(Client::message_ptr);
};