#pragma once

#include <Geode/Geode.hpp>

class ConnectionLabel : public cocos2d::CCNode {
public:
    ~ConnectionLabel();

    void connectedChanged(bool connected);

    bool init();
    void onClick(cocos2d::CCObject*);
    static ConnectionLabel* create() {
        auto res = new ConnectionLabel();
        if (res && res->init()) {
            res->autorelease();
            return res;
        }
        CC_SAFE_DELETE(res);
        return nullptr;
    }

protected:
    cocos2d::CCLabelBMFont* m_connectedLabel = nullptr;
    CCMenuItemSpriteExtra* m_loginButton = nullptr;
    bool m_connected = false;
    bool m_childAdded = false;
};