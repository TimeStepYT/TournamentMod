#pragma once

#include <Geode/Geode.hpp>

class LoginPopup : public geode::Popup<> {
protected:
    bool setup() override;
    void onDone(cocos2d::CCObject*);

    geode::TextInput* m_nameInput = nullptr;

public:
    static LoginPopup* create() {
        auto ret = new LoginPopup();
        if (ret->initAnchored(270.f, 121.f)) {
            ret->autorelease();
            return ret;
        }

        delete ret;
        return nullptr;
    }
};