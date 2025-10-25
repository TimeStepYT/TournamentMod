#pragma once

#include <Geode/Geode.hpp>

#include <Geode/modify/LevelInfoLayer.hpp>
#include <Geode/modify/EndLevelLayer.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/GameManager.hpp>

class $modify(MyEndLevelLayer, EndLevelLayer) {
    void showLayer(bool p0) override;
    void onReplay(CCObject* sender);
    void onMenu(CCObject* sender);
    void onEdit(CCObject* sender);
};

class $modify(MyMenuLayer, MenuLayer) {
    bool init();
};

class $modify(MyLevelInfoLayer, LevelInfoLayer) {
    struct Fields {
        bool forced = false;
    };
    bool init(GJGameLevel* level, bool challenge);
    void onBack(CCObject* sender);
};