#include <Geode/Geode.hpp>

#include <Geode/utils/cocos.hpp>

#include "../include/NetworkManager.hpp"
#include "../include/ConnectionLabel.hpp"
#include "../include/hooks.hpp"

using namespace geode::prelude;

void MyEndLevelLayer::showLayer(bool p0) {
    EndLevelLayer::showLayer(p0);

    int id = this->m_playLayer->m_level->m_levelID;

    auto& nm = NetworkManager::get();
    if (!nm.isConnected || !nm.isLoggedIn || !nm.m_levelID == id)
        return;

    nm.send(fmt::format("/completed {}", id));

    Loader::get()->queueInMainThread([this]() {
        auto buttonMenu = this->m_mainLayer->getChildByType<CCMenu>(0);
        auto completeMessage = this->m_mainLayer->getChildByType<TextArea>(0);

        if (buttonMenu)
            buttonMenu->setVisible(false);

        if (completeMessage)
            completeMessage->setPositionY(completeMessage->getPositionY() - 9);
        });
}
void MyEndLevelLayer::onReplay(CCObject* sender) {
    auto& nm = NetworkManager::get();
    if (nm.isConnected && nm.isLoggedIn)
        return;

    EndLevelLayer::onReplay(sender);
}
void MyEndLevelLayer::onMenu(CCObject* sender) {
    auto& nm = NetworkManager::get();
    if (nm.isConnected && nm.isLoggedIn)
        return;

    EndLevelLayer::onMenu(sender);
}

void MyEndLevelLayer::onEdit(CCObject* sender) {
    auto& nm = NetworkManager::get();
    if (nm.isConnected && nm.isLoggedIn)
        return;

    EndLevelLayer::onEdit(sender);
}

bool MyMenuLayer::init() {
    if (!MenuLayer::init())
        return false;

    auto cl = ConnectionLabel::create();
    cl->setAnchorPoint(ccp(1, 1));
    cl->setPositionX(this->getContentWidth());
    cl->setPositionY(this->getContentHeight());
    this->addChild(cl);

    NetworkManager::get().connect([]() {
        NetworkManager::get().send("/getclients");
        });

    return true;
}

bool MyLevelInfoLayer::init(GJGameLevel* level, bool challenge) {
    if (!LevelInfoLayer::init(level, challenge)) return false;

    int id = !NetworkManager::get().m_levelID;

    if (!this->m_fields->forced || id == this->m_level->m_levelID)
        return true;



    return true;
}

void MyLevelInfoLayer::onBack(CCObject* sender) {
    int id = NetworkManager::get().m_levelID;

    if (this->m_fields->forced && id == this->m_level->m_levelID)
        return;

    LevelInfoLayer::onBack(sender);
}