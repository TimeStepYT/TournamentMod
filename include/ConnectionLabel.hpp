#pragma once

#include <Geode/Geode.hpp>

class ConnectionLabel : public cocos2d::CCNode {
public:
    ~ConnectionLabel();

    void connectedChanged(bool connected);

    bool init();
    void onClick(cocos2d::CCObject*);
    CREATE_FUNC(ConnectionLabel);

protected:
    cocos2d::CCLabelBMFont* m_connectedLabel = nullptr;
    CCMenuItemSpriteExtra* m_loginButton = nullptr;
    bool m_connected = false;
    std::mutex m_connectedChangedMutex;
    bool m_childAdded = false;
};