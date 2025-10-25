#include "../include/MessageHandler.hpp"
#include "../include/LoadLevelPopup.hpp"

using namespace geode::prelude;

// std::vector<std::string> MessageHandler::split(std::string_view str, std::string_view delimeter, int limit) {
//     std::vector<std::string> res;

//     int prevDelimeterIndex = 0;
//     int itemCount = 1;

//     for (int i = 0; i < str.size(); ++i) {
//         char c = str.at(i);
//         int delimLength = delimeter.size();
//         bool onDelimeter = str.substr(i, delimLength) == delimeter;
//         bool onLastChar = i == str.size() - 1;
//         int charOffset = 0;

//         if (onLastChar)
//             charOffset = 1;

//         if (onDelimeter || onLastChar) {
//             ++itemCount;
//             std::string_view part = str.substr(prevDelimeterIndex, i - prevDelimeterIndex + charOffset);
//             res.emplace_back(part);

//             if (limit > 0 && itemCount > limit)
//                 break;

//             prevDelimeterIndex = i + delimLength;
//         }
//     }

//     return res;
// }

void MessageHandler::openAlert(std::string_view alertString) {
    std::array<std::string, 3> params;

    auto sanStr = geode::utils::string::replace(std::string{ alertString }, "ö", "oe");
    sanStr = geode::utils::string::replace(sanStr, "ä", "ae");
    sanStr = geode::utils::string::replace(sanStr, "ü", "ue");
    sanStr = geode::utils::string::replace(sanStr, "ß", "ss");
    sanStr = geode::utils::string::replace(sanStr, "Ä", "AE");
    sanStr = geode::utils::string::replace(sanStr, "Ö", "OE");
    sanStr = geode::utils::string::replace(sanStr, "Ü", "UE");
    auto splitStr = geode::utils::string::split(sanStr, "~|~");
    //auto splitStr = split(alertString, "~|~", 3);


    if (splitStr.size() >= 2) {
        params[0] = splitStr[0];
        params[1] = splitStr[1];
    }
    else {
        params[0] = "Alert!";
        params[1] = splitStr[0];
    }
    if (splitStr.size() >= 3 && !splitStr[2].empty())
        params[2] = splitStr[2];
    else
        params[2] = "OK";

    FLAlertLayer::create(params[0].c_str(), params[1], params[2].c_str())->show();
}

void MessageHandler::openDialog(std::string_view dialogString) {
    std::array<std::string, 4> params;

    auto sanStr = geode::utils::string::replace(std::string{ dialogString }, "ö", "oe");
    sanStr = geode::utils::string::replace(sanStr, "ä", "ae");
    sanStr = geode::utils::string::replace(sanStr, "ü", "ue");
    sanStr = geode::utils::string::replace(sanStr, "ß", "ss");
    sanStr = geode::utils::string::replace(sanStr, "Ä", "AE");
    sanStr = geode::utils::string::replace(sanStr, "Ü", "UE");
    sanStr = geode::utils::string::replace(sanStr, "Ö", "OE");
    sanStr = geode::utils::string::replace(sanStr, "ß", "ss");
    auto splitStr = geode::utils::string::split(sanStr, "~|~");
    //auto splitStr = split(dialogString, "~|~", 4);

    if (splitStr.size() >= 2) {
        params[0] = splitStr[0];
        params[1] = splitStr[1];
    }
    else {
        params[0] = "Alert!";
        params[1] = splitStr[0];
    }
    if (splitStr.size() >= 3 && !splitStr[2].empty())
        params[2] = splitStr[2];
    else
        params[2] = "28";
    if (splitStr.size() >= 4 && !splitStr[3].empty())
        params[3] = splitStr[3];
    else
        params[3] = "4";

    try {
        std::string const& bgStr = params[3];
        std::string const& characterStr = params[2];
        int bg = std::stoi(bgStr);
        int character = std::stoi(characterStr);

        if (bg < 0 || bg > 7 || character < 1 || character > 56) {
            return;
        }

        auto obj = DialogObject::create(params[0], params[1], character, 1.f, true, ccc3(255, 255, 255));
        auto layer = DialogLayer::createDialogLayer(obj, nullptr, bg);
        layer->animateInRandomSide();
        layer->addToMainScene();
    }
    catch (...) {}
}

void MessageHandler::handleSuccess(std::string_view content) {
    if (content == "login") {
        NetworkManager::get().isLoggedIn = true;
        auto label = NetworkManager::get().m_connectionLabel;

        label->connectedChanged(true);

        if (this->m_showedLoginMessage)
            return;

        this->m_showedLoginMessage = true;

        FLAlertLayer::create(
            "Success!",
            fmt::format("Playing as <cg>{}</c>", NetworkManager::get().getUserName().value()),
            "OK"
        )->show();
    }
}

void MessageHandler::levelKick(std::string_view content) {
    auto pl = PlayLayer::get();

    if (!pl)
        return;

    pl->onQuit();
}
void MessageHandler::playLevel(std::string_view content) {
    this->levelKick(content);

    auto& nm = NetworkManager::get();
    if (!nm.isConnected || !nm.isLoggedIn)
        return;

    int id = 0;

    try {
        id = std::stoi(std::string{ content });
    }
    catch (...) {
        log::info("Not an int apparently {}", content);
        return;
    }

    log::info("create {}", id);

    LoadLevelPopup::create(id)->show();
}

void MessageHandler::onMessage(Client::message_ptr msgPtr) {
    std::string msg = msgPtr->get_payload();

    this->handleCommand(msg, "alert", [this](std::string_view c) { this->openAlert(c); });
    this->handleCommand(msg, "dialog", [this](std::string_view c) { this->openDialog(c); });
    this->handleCommand(msg, "success", [this](std::string_view c) { this->handleSuccess(c); });
    this->handleCommand(msg, "levelkick", [this](std::string_view c) { this->levelKick(c); });
    this->handleCommand(msg, "play", [this](std::string_view c) { this->playLevel(c); });
}

void MessageHandler::handleCommand(std::string fullCommand, std::string_view commandName, std::function<void(std::string_view)> callback) {
    std::string command = fmt::format("/{}", commandName);

    auto params = geode::utils::string::split(fullCommand, " ");

    if (params[0] != command)
        return;

    if (params.size() > 1) {
        int contentStart = params[0].size() + 1;
        std::string content = fullCommand.substr(contentStart, fullCommand.length() - contentStart);
        callback(content);
        return;
    }

    callback("");
}