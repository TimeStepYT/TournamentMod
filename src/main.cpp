#include <Geode/Geode.hpp>

#include <Geode/utils/cocos.hpp>

#include "../include/NetworkManager.hpp"
#include "../include/ConnectionLabel.hpp"
#include "../include/LoadLevelPopup.hpp"
#include "../include/hooks.hpp"

using namespace geode::prelude;

void MyEndLevelLayer::showLayer(bool p0) {
    EndLevelLayer::showLayer(p0);

    int id = this->m_playLayer->m_level->m_levelID;
    bool inPracticeMode = this->m_playLayer->m_isPracticeMode;

    auto& nm = NetworkManager::get();
    if (!nm.isConnected || !nm.isLoggedIn || nm.m_levelID != id || inPracticeMode)
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
    int id = this->m_playLayer->m_level->m_levelID;
    if (nm.isConnected && nm.isLoggedIn && nm.m_levelID == id) {
        return;
    }

    EndLevelLayer::onReplay(sender);
}
void MyEndLevelLayer::onMenu(CCObject* sender) {
    auto& nm = NetworkManager::get();
    int id = this->m_playLayer->m_level->m_levelID;
    if (nm.isConnected && nm.isLoggedIn && nm.m_levelID == id)
        return;

    EndLevelLayer::onMenu(sender);
}

void MyEndLevelLayer::onEdit(CCObject* sender) {
    auto& nm = NetworkManager::get();
    int id = this->m_playLayer->m_level->m_levelID;
    if (nm.isConnected && nm.isLoggedIn && nm.m_levelID == id)
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

    auto& nm = NetworkManager::get();

    this->m_fields->layer = this;
    nm.m_levelInfoLayers.emplace_back(this);

    int id = nm.m_levelID;

    if (id != this->m_level->m_levelID) {
        return true;
    }

    auto children = this->getChildrenExt();

    for (auto* child : children) {
        auto menu = typeinfo_cast<CCMenu*>(child);

        if (!menu)
            continue;

        if (menu == this->m_playBtnMenu)
            continue;

        if (menu->getChildrenCount() == 1) {
            auto firstItem = menu->getChildByType<CCMenuItemSpriteExtra>(0);
            bool hasLabel = !!firstItem->getChildByType<CCLabelBMFont>(0);
            bool containsOneChild = firstItem->getChildrenCount() == 1;

            if (firstItem && containsOneChild && hasLabel) {
                menu->setEnabled(false);
                continue;
            }
        }

        menu->setVisible(false);
    }

    return true;
}

void MyLevelInfoLayer::onBack(CCObject* sender) {
    auto& nm = NetworkManager::get();
    int id = nm.m_levelID;

    if (!nm.isConnected || !nm.isLoggedIn) {
        LevelInfoLayer::onBack(sender);
        return;
    }

    if (id == this->m_level->m_levelID)
        return;

    LevelInfoLayer::onBack(sender);
}

void MyLevelInfoLayer::confirmDelete(CCObject* p0) {
    auto& nm = NetworkManager::get();
    int id = nm.m_levelID;

    if (!nm.isConnected || !nm.isLoggedIn) {
        LevelInfoLayer::confirmDelete(p0);
        return;
    }
    if (id == this->m_level->m_levelID)
        return;

    LevelInfoLayer::confirmDelete(p0);
}

void MyLevelInfoLayer::levelIDChanged() {
    int newID = NetworkManager::get().m_levelID;
    int currentID = this->m_level->m_levelID;

    bool onChosenLevel = newID == currentID;

    auto children = this->getChildrenExt();

    for (auto* child : children) {
        auto menu = typeinfo_cast<CCMenu*>(child);

        if (!menu)
            continue;

        if (menu == this->m_playBtnMenu)
            continue;

        if (menu->getChildrenCount() == 1) {
            auto firstItem = menu->getChildByType<CCMenuItemSpriteExtra>(0);
            bool hasLabel = !!firstItem->getChildByType<CCLabelBMFont>(0);
            bool containsOneChild = firstItem->getChildrenCount() == 1;

            if (firstItem && containsOneChild && hasLabel) {
                menu->setEnabled(!onChosenLevel);
                continue;
            }
        }

        menu->setVisible(!onChosenLevel);
    }
}

void MyLevelInfoLayer::onEnterTransitionDidFinish() {
    LevelInfoLayer::onEnterTransitionDidFinish();
    LoadLevelPopup::m_loading = false;
}

MyLevelInfoLayer::Fields::~Fields() {
    std::vector<MyLevelInfoLayer*> newVec;
    auto& nm = NetworkManager::get();
    auto& layers = nm.m_levelInfoLayers;

    for (auto* layer : layers) {
        if (layer == this->layer) {
            continue;
        }
        newVec.emplace_back(layer);
    }
    layers = newVec;
}

void MyPlayLayer::onExit() {
    PlayLayer::onExit();
    Loader::get()->queueInMainThread([]() {
        auto& nm = NetworkManager::get();
        for (auto& callback : nm.m_openNextLevelQueue) {
            callback();
        }
        nm.m_openNextLevelQueue.clear();
        });
}

void MyPlayLayer::togglePracticeMode(bool practiceMode) {
    auto& nm = NetworkManager::get();
    if (nm.m_levelID == this->m_level->m_levelID)
        return;

    PlayLayer::togglePracticeMode(practiceMode);
}

constexpr auto VERTEX_SHADER = R"(
    attribute vec4 a_position;
    attribute vec4 a_color;
    attribute vec2 a_texCoord;

    #ifdef GL_ES
    varying lowp vec4 v_fragmentColor;
    varying mediump vec2 v_texCoord;
    #else
    varying vec4 v_fragmentColor;
    varying vec2 v_texCoord;
    #endif

    void main() {
        gl_Position = CC_MVPMatrix * a_position;
        v_fragmentColor = a_color;
        v_texCoord = a_texCoord;
    })";

constexpr auto FRAGMENT_SHADER = R"(
    #ifdef GL_ES
    precision mediump float;
    #endif

    varying vec4 v_fragmentColor;
    varying vec2 v_texCoord;
    uniform sampler2D u_texture;

    uniform vec3 u_lum;

    void main(void) {
        vec4 color = texture2D(u_texture, v_texCoord);
        vec3 tintedColor = v_fragmentColor.rgb * color.rgb;
        float gray = dot(tintedColor, u_lum);
        float normalization = (v_fragmentColor.r + v_fragmentColor.g + v_fragmentColor.b) / 3.0;
        gray /= normalization;  // Adjust for tint brightness
        gl_FragColor = vec4(v_fragmentColor.rgb * vec3(gray), color.a * v_fragmentColor.a);
    })";

static CCGLProgram* getShader() {
    auto ccsc = CCShaderCache::sharedShaderCache();
    auto program = ccsc->programForKey("gradient-shader"_spr);

    if (program)
        return program;

    auto vsh = VERTEX_SHADER;
    auto fsh = FRAGMENT_SHADER;

    program = new CCGLProgram();
    program->initWithVertexShaderByteArray(vsh, fsh);
    program->addAttribute("a_position", kCCVertexAttrib_Position);
    program->addAttribute("a_texCoord", kCCVertexAttrib_TexCoords);
    program->addAttribute("a_color", kCCVertexAttrib_Color);

    if (!program->link()) {
        log::error("Shader link failed!");
        program->release();
        return nullptr;
    }

    program->updateUniforms();

    program->setUniformLocationWith3f(program->getUniformLocationForName("u_lum"),
        0.299f, 0.587f, 0.114f);

    ccsc->addProgram(program, "gradient-shader"_spr);
    program->release();

    return program;
}

void TournamentPauseLayer::customSetup() {
    PauseLayer::customSetup();
    int pausedLevelID = GameManager::get()->m_currentLevelID;

    NodeIDs::provideFor(this);

    auto& nm = NetworkManager::get();

    if (!nm.isConnected || !nm.isLoggedIn || nm.m_levelID != pausedLevelID)
        return;

    auto centerButtonMenu = this->getChildByID("center-button-menu");
    if (!centerButtonMenu)
        return;

    auto practiceButtonUncasted = centerButtonMenu->getChildByID("practice-button");
    if (!practiceButtonUncasted)
        return;

    auto practiceButton = static_cast<CCMenuItemSpriteExtra*>(practiceButtonUncasted);

    practiceButton->setEnabled(false);

    Loader::get()->queueInMainThread([this]() {
        auto globedPlayerListMenu = this->getChildByID("dankmeme.globed2/playerlist-menu");
        if (!globedPlayerListMenu)
            return;
        globedPlayerListMenu->setVisible(false);
        });

    auto program = getShader();
    if (!program)
        return;

    auto practiceButtonSprite = practiceButton->getChildByType<CCSprite>(0);
    practiceButtonSprite->setShaderProgram(program);
}