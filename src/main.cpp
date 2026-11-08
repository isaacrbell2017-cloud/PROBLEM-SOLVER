#include <Geode/Geode.hpp>
#include <Geode/loader/SettingV3.hpp>
#include <Geode/modify/EditorPauseLayer.hpp>
#include <Geode/modify/EditorUI.hpp>
#include <Geode/modify/MenuLayer.hpp>

#include <algorithm>
#include <cmath>
#include <vector>

#include "BusterBob.hpp"
#include "Fixes.hpp"
#include "Shared.hpp"

using namespace geode::prelude;

namespace {

// Bob's face in a blue circle, sized like the other main menu buttons.
CCNode* bobButtonSprite(CircleBaseSize size, float bobHeight, float scale = 1.f) {
    auto bobSpr = bob::createBob(bobHeight);
    auto circle = CircleButtonSprite::create(bobSpr, CircleBaseColor::Blue, size);
    CCNode* look = circle ? static_cast<CCNode*>(circle) : static_cast<CCNode*>(bobSpr);
    look->setScale(look->getScale() * scale);
    return look;
}

// Rects (in world space) of buttons and labels already on a layer.
void collectTaken(CCNode* node, std::vector<CCRect>& out, CCSize const& win) {
    if (!node || !node->isVisible()) return;
    auto children = node->getChildren();
    if (!children) return;
    for (auto child : CCArrayExt<CCNode*>(children)) {
        if (!child || !child->isVisible()) continue;
        if (typeinfo_cast<CCMenuItem*>(child) || typeinfo_cast<CCLabelBMFont*>(child)) {
            auto bb = child->boundingBox();
            auto bl = node->convertToWorldSpace(bb.origin);
            auto tr = node->convertToWorldSpace(CCPoint(bb.getMaxX(), bb.getMaxY()));
            CCRect rect(std::min(bl.x, tr.x), std::min(bl.y, tr.y), std::abs(tr.x - bl.x), std::abs(tr.y - bl.y));
            if (rect.size.width < win.width * 0.6f && rect.size.height < win.height * 0.6f) out.push_back(rect);
        }
        collectTaken(child, out, win);
    }
}

// First free spot on the editor pause menu, so Bob never covers another button.
CCPoint findFreeSpot(CCNode* layer) {
    auto win = CCDirector::sharedDirector()->getWinSize();
    std::vector<CCRect> taken;
    collectTaken(layer, taken, win);

    std::vector<CCPoint> candidates = {
        { win.width - 38.f, 46.f },
        { 38.f, 46.f },
        { win.width / 2.f, 34.f },
        { 38.f, win.height - 40.f },
        { win.width / 2.f, win.height - 34.f },
        { 38.f, win.height / 2.f },
        { win.width - 38.f, win.height / 2.f },
        { win.width - 38.f, win.height - 90.f },
    };
    for (auto const& c : candidates) {
        CCRect area(c.x - 26.f, c.y - 36.f, 52.f, 64.f); // button + label underneath
        bool clear = true;
        for (auto const& t : taken) {
            if (area.intersectsRect(t)) {
                clear = false;
                break;
            }
        }
        if (clear) return c;
    }
    return candidates.front();
}

} // namespace

// Main menu: Bob's blue robot button sits in the bottom row next to the Geode button.
class $modify(BobMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) return false;

        if (Mod::get()->getSettingValue<bool>("menu-button")) {
            auto btn = CCMenuItemSpriteExtra::create(
                bobButtonSprite(CircleBaseSize::MediumAlt, 34.f), this, menu_selector(BobMenuLayer::onProblemBob)
            );
            btn->setID("problem-bob-button"_spr);

            if (auto bottom = this->getChildByID("bottom-menu")) {
                bottom->addChild(btn);
                bottom->updateLayout();
            }
            else {
                auto win = CCDirector::sharedDirector()->getWinSize();
                auto menu = CCMenu::create();
                menu->setID("problem-bob-menu"_spr);
                menu->addChild(btn);
                btn->setPosition({ 0.f, 0.f });
                menu->setPosition({ win.width / 2.f + 150.f, 45.f });
                this->addChild(menu);
            }
        }

        // Tell the player about any new bug fixes once the menu has faded in.
        this->runAction(CCSequence::create(
            CCDelayTime::create(1.2f),
            CCCallFunc::create(this, callfunc_selector(BobMenuLayer::announceFixes)),
            nullptr
        ));
        return true;
    }

    void onProblemBob(CCObject*) {
        bob::sfx("counter003.ogg");
        bob::openHub();
    }

    void announceFixes() {
        bob::announceFixes();
    }
};

// Editor pause menu (the "||" button): Buster Bob's button. This is how mobile opens him.
class $modify(BusterPauseLayer, EditorPauseLayer) {
    bool init(LevelEditorLayer* lel) {
        if (!EditorPauseLayer::init(lel)) return false;
        if (!Mod::get()->getSettingValue<bool>("editor-button")) return true;

        auto btn = CCMenuItemSpriteExtra::create(
            bobButtonSprite(CircleBaseSize::Medium, 30.f, 0.85f), this, menu_selector(BusterPauseLayer::onBusterBob)
        );
        btn->setID("buster-bob-button"_spr);

        auto menu = CCMenu::create();
        menu->setID("buster-bob-menu"_spr);
        menu->addChild(btn);
        btn->setPosition({ 0.f, 0.f });

        auto label = CCLabelBMFont::create("Buster Bob", "goldFont.fnt");
        label->setScale(0.38f);
        label->setPosition({ 0.f, -28.f });
        label->setID("buster-bob-label"_spr);
        menu->addChild(label);

        menu->setPosition(findFreeSpot(this));
        this->addChild(menu, 50);
        return true;
    }

    void onBusterBob(CCObject*) {
        bob::sfx("counter003.ogg");
        bob::openGuide();
    }
};

// PC: press the Buster Bob key (F by default) in the editor.
class $modify(BusterEditorUI, EditorUI) {
    bool init(LevelEditorLayer* lel) {
        if (!EditorUI::init(lel)) return false;

        this->addEventListener(
            KeybindSettingPressedEventV3(Mod::get(), "editor-key"),
            [](Keybind const& keybind, bool down, bool repeat, double timestamp) {
                if (!down || repeat) return;
                if (BusterBobPopup::isOpen()) return;
                bob::sfx("counter003.ogg");
                bob::openGuide();
            }
        );
        return true;
    }
};
