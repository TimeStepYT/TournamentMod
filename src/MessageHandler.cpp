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

bool MessageHandler::isInt(std::string_view string) {
    int value;
    auto [ptr, ec] = std::from_chars(string.data(), string.data() + string.size(), value);
    return ec == std::errc() && ptr == string.data() + string.size();
}

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

    std::string const& bgStr = params[3];
    std::string const& characterStr = params[2];

    if (!this->isInt(bgStr) || !this->isInt(characterStr))
        return;

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

    auto& nm = NetworkManager::get();
    nm.m_levelID = -1;


    for (auto* lil : nm.m_levelInfoLayers) {
        lil->levelIDChanged();
    }

    if (!pl)
        return;

    pl->onQuit();
}
void MessageHandler::playLevel(std::string_view content) {
    bool wasPlaying = !!PlayLayer::get();

    std::string contentStr{ content };

    auto callback = [contentStr]() {
        auto& nm = NetworkManager::get();

        if (!nm.isLoggedIn)
            return;

        if (!MessageHandler::isInt(contentStr)) {
            log::info("Not an int apparently {}", contentStr);
            return;
        }

        int id = 0;
        id = std::stoi(std::string{ contentStr });

        nm.m_levelID = id;

        LoadLevelPopup::create(nm.m_levelID)->show();

        for (auto* lil : nm.m_levelInfoLayers) {
            lil->levelIDChanged();
        }
        };


    if (wasPlaying) {
        NetworkManager::get().m_openNextLevelQueue.push_back(callback);
        this->levelKick(content);
    }
    else {
        callback();
    }
}

void MessageHandler::openToast(std::string_view content) {
    std::string contentStr{ content };

    auto sanStr = geode::utils::string::replace(contentStr, "ö", "oe");
    sanStr = geode::utils::string::replace(sanStr, "ä", "ae");
    sanStr = geode::utils::string::replace(sanStr, "ü", "ue");
    sanStr = geode::utils::string::replace(sanStr, "ß", "ss");
    sanStr = geode::utils::string::replace(sanStr, "Ä", "AE");
    sanStr = geode::utils::string::replace(sanStr, "Ü", "UE");
    sanStr = geode::utils::string::replace(sanStr, "Ö", "OE");
    sanStr = geode::utils::string::replace(sanStr, "ß", "ss");

    auto params = geode::utils::string::split(sanStr, "~|~");

    std::string const& text = params[0];

    if (params.size() != 3 && params.size() != 1) {
        return;
    }

    if (params.size() == 3) {
        std::string const& isFromSheetStr = params[1];
        std::string const& spriteName = params[2];

        if (isFromSheetStr == "0" || isFromSheetStr == "1") {

            bool isFromSheet = !!std::stoi(isFromSheetStr);

            CCSprite* icon = nullptr;

            auto spriteNameC = spriteName.c_str();

            if (isFromSheet) {
                icon = CCSprite::createWithSpriteFrameName(spriteNameC);
            }
            else {
                icon = CCSprite::create(spriteNameC);
            }

            if (icon) {
                geode::Notification::create(text, icon, 3.f)->show();
                return;
            }
        }
    }

    auto notif = geode::Notification::create(text);
    notif->setTime(3.f);
    notif->show();
    return;
}

void MessageHandler::rickRoll(std::string_view content) {
    geode::utils::web::openLinkInBrowser("https://www.youtube.com/watch?v=dQw4w9WgXcQ");
}

#define REG_COMMAND(name, func) this->handleCommand(msg, name, [this](std::string_view c) { this->func(c); })

void MessageHandler::onMessage(Client::message_ptr msgPtr) {
    std::string msg = msgPtr->get_payload();

    REG_COMMAND("alert", openAlert);
    REG_COMMAND("dialog", openDialog);
    REG_COMMAND("success", handleSuccess);
    REG_COMMAND("levelkick", levelKick);
    REG_COMMAND("play", playLevel);
    REG_COMMAND("toast", openToast);
    REG_COMMAND("rickroll", rickRoll);
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