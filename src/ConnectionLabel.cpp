#include "../include/ConnectionLabel.hpp"
#include "../include/NetworkManager.hpp"

using namespace geode::prelude;

void ConnectionLabel::connectedChanged(bool connected) {
    std::lock_guard<std::mutex> lock(this->m_connectedChangedMutex);

    auto& label = this->m_connectedLabel;

    if (!label) {
        label = CCLabelBMFont::create("", "bigFont.fnt");
    }

    this->m_connected = connected;

    if (!connected) {
        label->setString("No connection");
        label->setColor(ccc3(255, 0, 0));
    }
    else {
        label->setString("Connected");
        label->setColor(ccc3(0, 255, 0));
    }

    label->setAnchorPoint(ccp(1, 1));
    label->setScale(0.25f);
}

void ConnectionLabel::recreate() {
    this->m_connectedLabel = nullptr;
    this->connectedChanged(NetworkManager::get().isConnected);
}

bool ConnectionLabel::init() {
    if (!CCNode::init())
        return false;

    this->recreate();
    this->addChild(this->m_connectedLabel);

    NetworkManager::get().m_connectionLabel = this;

    return true;
}