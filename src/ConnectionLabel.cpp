#include "../include/ConnectionLabel.hpp"
#include "../include/NetworkManager.hpp"
#include "../include/LoginPopup.hpp"

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
        if (!NetworkManager::get().isLoggedIn) {
            label->setString("Set player!");
            label->setColor(ccc3(255, 255, 0));
        }
        else {
            label->setString("Connected");
            label->setColor(ccc3(0, 255, 0));
        }
    }

    label->setScale(0.25f);

    if (this->m_loginButton)
        this->m_loginButton->setEnabled(connected);
}

void ConnectionLabel::onClick(CCObject* sender) {
    auto loginPopup = LoginPopup::create();

    loginPopup->show();
}

bool ConnectionLabel::init() {
    if (!CCNode::init())
        return false;

    bool isConnected = NetworkManager::get().isConnected;
    auto& conLabel = this->m_connectedLabel;

    conLabel = nullptr;
    this->connectedChanged(isConnected);

    auto menu = CCMenu::create();
    auto menuItem = CCMenuItemSpriteExtra::create(
        this->m_connectedLabel,
        this,
        menu_selector(ConnectionLabel::onClick)
    );

    menu->setContentSize(menuItem->getContentSize());
    menu->setPositionX(menu->getContentWidth() / 2);
    menu->setPositionY(menu->getContentHeight() / 2);
    menu->addChild(menuItem);

    menuItem->setEnabled(isConnected);
    menuItem->setAnchorPoint(ccp(1, 1));
    menuItem->setPosition(menu->getPosition());
    conLabel->setAnchorPoint(ccp(1, 1));
    conLabel->setPosition(menuItem->getContentSize());

    this->addChild(menu);
    this->setContentSize(menuItem->getContentSize());

    this->m_connectedLabel->setPosition(menuItem->getContentSize());
    this->m_loginButton = menuItem;


    if (!NetworkManager::get().m_connectionLabel)
        NetworkManager::get().m_connectionLabel = this;

    return true;
}

ConnectionLabel::~ConnectionLabel() {
    NetworkManager::get().m_connectionLabel = nullptr;
}