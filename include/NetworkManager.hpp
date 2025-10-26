#pragma once
#undef _WINSOCKAPI_

#define _WEBSOCKETPP_CPP11_STL_
#define _WEBSOCKETPP_CPP11_INTERNAL_
#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>

#include "ConnectionLabel.hpp"

#include <Geode/loader/Mod.hpp>
using namespace geode;

using Client = websocketpp::client<websocketpp::config::asio_client>;
using Handle = websocketpp::connection_hdl;

class NetworkManager {
protected:
    NetworkManager();

    Client client;
    Handle hdl;

    std::thread runThread;

    std::condition_variable heartbeatCondition;
    std::mutex heartbeatMutex;
    std::mutex connectMutex;
    std::mutex sendMutex;
    std::thread heartbeatThread;

    std::optional<std::string> m_userName = std::nullopt;

    std::function<void()> connCallback = nullptr;

    void init();

    void onOpen(Handle hdl);
    void onMessage(Handle hdl, Client::message_ptr msg);
    void onFail(Handle hdl);
    void onClose(Handle hdl);

    void run();
    void heartbeat();

    void tryReconnect(int);
public:
    static NetworkManager& get() {
        static NetworkManager instance;
        return instance;
    }

    std::atomic_bool isConnecting = false;
    std::atomic_bool isConnected = false;
    std::atomic_bool isRunning = false;
    std::atomic_bool isLoggedIn = false;
    std::atomic_int m_levelID = -1;
    ConnectionLabel* m_connectionLabel = nullptr;

    void connect();
    void connect(std::function<void()> callback);
    void disconnect();
    void close();
    void setUserName(std::string_view);
    std::optional<std::string> getUserName();

    void send(std::string const& msg) {
        std::lock_guard<std::mutex> lock(this->sendMutex);
        auto error = client.get_con_from_hdl(hdl)->send(msg, websocketpp::frame::opcode::text);
        if (error) {
            log::error("no message sent :( {}", error.message());
        }
    }

    void login(std::string_view);
};