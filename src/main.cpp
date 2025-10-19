#include <Geode/Geode.hpp>

#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/GameManager.hpp>
#include "../include/NetworkManager.hpp"
#include "../include/ConnectionLabel.hpp"
#include "../include/PointsManager.hpp"

using namespace geode::prelude;

/*
class $modify(MyPlayLayer, PlayLayer) {
    bool init(GJGameLevel * gameLevel, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(gameLevel, useReplay, dontCreateObjects))
            return false;

        auto savedLabel = PointsManager::get()->getLabel();
        auto newLabel = CCLabelBMFont::create("", "bigFont.fnt");
        newLabel->setAnchorPoint(ccp(1, 0));
        newLabel->setPositionX(this->getContentWidth());
        newLabel->setPositionY(0);
        newLabel->setScale(0.5f);

        constexpr uint8_t halfByte = 255 * 0.5;
        newLabel->setOpacity(halfByte);

        PointsManager::get()->setLabel(newLabel);

        this->addChild(newLabel);

        return true;
    }

    void showEndLayer() {
        this->receivePoints();
        PlayLayer::showEndLayer();
    }

    void receivePoints() {
        float deathPercent = this->getCurrentPercent();
        PointsManager::get()->setLastDeath(deathPercent);

        float points = PointsManager::get()->getPoints();
        std::string pointsString = fmt::format("{} Points", static_cast<int>(points));

        CCLabelBMFont* label = PointsManager::get()->getLabel();

        if (label) {
            label->setString(pointsString.c_str());
        }
    }

    void destroyPlayer(PlayerObject * playerObject, GameObject * gameObject) {
        PlayLayer::destroyPlayer(playerObject, gameObject);

        if (gameObject == this->m_anticheatSpike)
            return;

        this->receivePoints();
    }
};
*/
class $modify(MyMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init())
            return false;


        auto cl = ConnectionLabel::create();
        cl->setPositionX(this->getContentWidth());
        cl->setPositionY(this->getContentHeight());
        this->addChild(cl);

        NetworkManager::get().connect([](){
            NetworkManager::get().send("/getclients");
        });

        return true;
    }
};