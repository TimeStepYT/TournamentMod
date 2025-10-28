#include "../include/LoginPopup.hpp"
#include "../include/NetworkManager.hpp"

using namespace geode::prelude;

bool LoginPopup::setup() {
    this->setTitle("Change player");
    this->setID("login-popup"_spr);

    auto mainLayer = this->m_mainLayer;
    auto layout = ColumnLayout::create();
    layout->setAxisReverse(true);
    layout->setAutoScale(false);
    layout->setAxisAlignment(AxisAlignment::End);

    constexpr float yOffset = 40;

    auto inputsNode = CCNode::create();
    inputsNode->setID("input-fields");
    inputsNode->setAnchorPoint(ccp(0.5f, 0.5f));
    inputsNode->setLayout(layout);
    inputsNode->setContentWidth(mainLayer->getContentWidth());
    inputsNode->setContentHeight(mainLayer->getContentHeight() - yOffset);
    inputsNode->setPositionX(mainLayer->getContentWidth() / 2);
    inputsNode->setPositionY(mainLayer->getContentHeight() / 2 - yOffset / 2 + 5);

    auto inputLayout = ColumnLayout::create();
    inputLayout->setAxisReverse(true);
    inputLayout->setAxisAlignment(AxisAlignment::End);
    inputLayout->setAutoScale(false);
    inputLayout->setGap(0.f);

    auto nameNode = CCNode::create();
    nameNode->setLayout(inputLayout);
    auto nameLabel = CCLabelBMFont::create("Name", "bigFont.fnt");
    auto nameInput = TextInput::create(150, "Name", "bigFont.fnt");
    nameLabel->setScale(0.5f);

    nameNode->setContentWidth(nameInput->getContentWidth());
    nameNode->setContentHeight(nameInput->getContentHeight() + 20);
    nameNode->addChild(nameLabel);
    nameNode->addChild(nameInput);
    nameNode->updateLayout();

    this->m_nameInput = nameInput;

    inputsNode->addChild(nameNode);
    inputsNode->updateLayout();

    auto doneMenu = CCMenu::create();
    doneMenu->setID("done-menu");
    doneMenu->setPositionX(mainLayer->getContentWidth() / 2);
    doneMenu->setPositionY(20);

    auto doneButtonSprite = ButtonSprite::create("Done");
    doneButtonSprite->setScale(0.75f);

    auto doneButton = CCMenuItemSpriteExtra::create(doneButtonSprite, this, menu_selector(LoginPopup::onDone));

    doneMenu->addChild(doneButton);

    mainLayer->addChild(inputsNode);
    mainLayer->addChild(doneMenu);

    return true;
}

void LoginPopup::onDone(CCObject* sender) {
    auto name = this->m_nameInput->getString();

    auto& nm = NetworkManager::get();
    nm.setUserName(name);
    nm.send(fmt::format("/login {}", name));

    this->onClose(sender);
}