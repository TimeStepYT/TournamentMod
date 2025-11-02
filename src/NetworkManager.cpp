#include "../include/NetworkManager.hpp"
#include "../include/MessageHandler.hpp"
#include "../include/ConnectionLabel.hpp"

const std::string getServerUrl() {
    auto url = Mod::get()->getSettingValue<std::string>("server-url");
    auto port = Mod::get()->getSettingValue<int64_t>("server-port");
    auto str = fmt::format("ws://{}:{}", url, port);
    if (str.ends_with("/")) {
        if (str.empty()) {
            log::error("server URL is empty... uh oh");
            return "";
        }
        str.pop_back();
    }
    if (str.empty()) {
        log::error("server URL is empty... uh oh");
    }

    return str;
}

NetworkManager::NetworkManager() {
    init();
}

void NetworkManager::init() {
    client.init_asio();
    client.clear_access_channels(websocketpp::log::alevel::all);
    client.clear_error_channels(websocketpp::log::elevel::all);

    client.set_message_handler([this](Handle hdl, Client::message_ptr msg) {
        this->onMessage(hdl, msg);
        });
    client.set_open_handler([this](Handle hdl) {
        this->onOpen(hdl);
        });
    client.set_close_handler([this](Handle hdl) {
        this->onClose(hdl);
        });
    client.set_fail_handler([this](Handle hdl) {
        this->onFail(hdl);
        });

    client.start_perpetual();

    runThread = std::thread([this]() {
        this->run();
        });
    isRunning = true;
    heartbeatThread = std::thread([this]() {
        this->heartbeat();
        });
}

void NetworkManager::heartbeat() {
    while (isRunning) {
        using namespace std::chrono_literals;
        auto lock = std::unique_lock<std::mutex>(heartbeatMutex);
        if (!heartbeatCondition.wait_for(lock, 10000ms, [this]() { return bool(!isRunning); }) && isConnected) {
            this->send("/ping");
        }
    }
}

void NetworkManager::run() {
    client.run();
}

void NetworkManager::close() {
    client.stop_perpetual();
    isRunning = false;
}

void NetworkManager::connect(std::function<void()> callback) {
    std::lock_guard<std::mutex> lock(this->connectMutex);
    if (this->isConnected) {
        callback();
        return;
    }

    if (this->isConnecting)
        return;

    this->isConnecting = true;

    auto error = std::error_code();

    auto connection = this->client.get_connection(
        getServerUrl(),
        error
    );

    if (error || !connection) {
        log::error("no connection! {}", error.message());
        return;
    }

    this->connCallback = callback;
    this->client.connect(connection);
}

void NetworkManager::connect() {
    connect([]() {});
}

void NetworkManager::disconnect() {
    auto error = std::error_code();
    client.close(hdl, websocketpp::close::status::normal, "User closed connection", error);
    if (error) {
        log::error("no disconnection! {}", error.message());
    }
}

void NetworkManager::tryReconnect(int delay) {
    std::this_thread::sleep_for(std::chrono::seconds(delay));
    Loader::get()->queueInMainThread([]() { // Doing this to prevent thread explosion
        NetworkManager::get().connect();
        });
}

void NetworkManager::login(std::string_view name) {
    this->send(fmt::format("/login {}", name));
    this->m_userName = name;
}

void NetworkManager::setUserName(std::string_view userName) {
    auto& mh = MessageHandler::get();
    
    if (this->m_userName != userName)
        mh.m_showedLoginMessage = false;

    this->m_userName = userName;
}

std::optional<std::string> NetworkManager::getUserName() {
    return this->m_userName;
}

void NetworkManager::onOpen(Handle hdl) {
    this->hdl = hdl;
    this->isConnecting = false;
    isConnected = true;
    connCallback();
    Loader::get()->queueInMainThread([]() {
        if (NetworkManager::get().m_connectionLabel)
            NetworkManager::get().m_connectionLabel->connectedChanged(true);
        }
    );
    this->send("/settype 0"); // I am a player!
    if (this->m_userName.has_value())
        this->login(this->getUserName().value());
}

void NetworkManager::onClose(Handle hdl) {
    this->isConnected = false;
    this->isConnecting = false;
    auto connection = client.get_con_from_hdl(hdl);
    auto const localCode = connection->get_local_close_code();
    auto const remoteCode = connection->get_remote_close_code();
    this->isLoggedIn = false;
    Loader::get()->queueInMainThread([]() {
        if (NetworkManager::get().m_connectionLabel)
            NetworkManager::get().m_connectionLabel->connectedChanged(false);
        }
    );

    this->tryReconnect(0);
}

void NetworkManager::onFail(Handle hdl) {
    this->isConnected = false;
    this->isConnecting = false;
    auto connection = client.get_con_from_hdl(hdl);
    auto const localCode = connection->get_local_close_code();
    auto const remoteCode = connection->get_remote_close_code();
    Loader::get()->queueInMainThread([]() {
        if (NetworkManager::get().m_connectionLabel)
            NetworkManager::get().m_connectionLabel->connectedChanged(false);
        }
    );
    // log::info("FAIL. ({}/{})", localCode, remoteCode);
    // log::info("because: {}/{}", connection->get_local_close_reason(), connection->get_remote_close_reason());
    this->tryReconnect(0);
}

void NetworkManager::onMessage(Handle hdl, Client::message_ptr msg) {
    Loader::get()->queueInMainThread([this, msg] {
        MessageHandler::get().onMessage(msg);
        });
}