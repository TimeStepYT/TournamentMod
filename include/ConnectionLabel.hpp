#pragma once

#include <Geode/Geode.hpp>

class ConnectionLabel : public cocos2d::CCNode {
public:
    void connectedChanged(bool connected);
    void recreate();

    bool init();
    CREATE_FUNC(ConnectionLabel);

protected:
    cocos2d::CCLabelBMFont* m_connectedLabel;
    bool m_connected = false;
    std::mutex m_connectedChangedMutex;
};