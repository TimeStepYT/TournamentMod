#pragma once

#include <Geode/Geode.hpp>

#include <Geode/modify/LevelInfoLayer.hpp>
#include <Geode/modify/EndLevelLayer.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/GameManager.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>

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
        cocos2d::CCMenu* backMenu = nullptr;
        cocos2d::CCMenu* rightMenu = nullptr;
        MyLevelInfoLayer* layer = nullptr;
        ~Fields();
    };
    bool init(GJGameLevel* level, bool challenge);
    void onBack(CCObject* sender);
    void confirmDelete(CCObject* p0);
    void levelIDChanged();
    void onEnterTransitionDidFinish() override;
};

class $modify(MyPlayLayer, PlayLayer) {
    void onExit() override;
    void togglePracticeMode(bool practiceMode);
};

class $modify(TournamentPauseLayer, PauseLayer) {
    void customSetup();
};