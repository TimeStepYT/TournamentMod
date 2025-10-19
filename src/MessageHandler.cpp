#include "../include/MessageHandler.hpp"

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

    auto splitStr = geode::utils::string::split(std::string(alertString), "~|~");
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

void MessageHandler::onMessage(Client::message_ptr msgPtr) {
    std::string msg = msgPtr->get_payload();

    if (msg.starts_with("/alert") && msg.size() > 7) {
        std::string content = msg.substr(7, msg.size() - 7);
        this->openAlert(content);
    }
    else if (msg == "/ping")
        return;
}