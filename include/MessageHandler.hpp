#pragma once
#include <Geode/Geode.hpp>
#include "NetworkManager.hpp"

class MessageHandler {
    protected:
    
    public:
    std::atomic_bool m_showedLoginMessage = false;

    static MessageHandler& get() {
        static MessageHandler instance;
        return instance;
    }

    // static std::vector<std::string> split(std::string_view, std::string_view, int limit = 0);
    static bool isInt(std::string_view);

    void openAlert(std::string_view);
    void openDialog(std::string_view);
    void handleSuccess(std::string_view);
    void levelKick(std::string_view);
    void playLevel(std::string_view);
    void openToast(std::string_view);
    void rickRoll(std::string_view);
    void onMessage(Client::message_ptr);
    void handleCommand(std::string, std::string_view, std::function<void(std::string_view)>);
};