#pragma once

// Thank you undefined!

class LoadLevelPopup : public geode::Popup<unsigned int>, public LevelManagerDelegate {
public:
    static LoadLevelPopup* create(unsigned int levelID);
    bool setup(unsigned int levelID);

    cocos2d::CCLabelBMFont* m_message;
    bool m_allowClose;
    unsigned int m_id;

    void loadLevelsFinished(cocos2d::CCArray* levels, char const* p1, int p2);
    void loadLevelsFinished(cocos2d::CCArray* levels, char const* p1);
    void loadLevelsFailed(char const* p0, int p1);
    void loadLevelsFailed(char const* p0);

    void onClose(cocos2d::CCObject* sender);
};