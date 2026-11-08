#include "BobHub.hpp"
#include "BusterBob.hpp"
#include "Fixes.hpp"
#include "LevelBeater.hpp"

#include <Geode/ui/GeodeUI.hpp>

using namespace geode::prelude;

bool BobHubPopup::s_open = false;
bool BobHubPopup::s_freed = false;

namespace {
constexpr float kW = 420.f;
constexpr float kH = 260.f;

constexpr char const* kThanks = "THANK YOU FOR FREEING ME! NO-ONE WANTS TO KNOW WHAT IS UNDER THERE...";
constexpr char const* kHello =
    "I'm Problem Bob! I squash bugs, help you build levels as Buster Bob, "
    "and help you beat them as Level Beater Bob. What do you need?";
constexpr char const* kBackAgain = "Back again? Good thing I'm already out of that hole. What do you need?";

CCMenuItemSpriteExtra* textButton(char const* text, char const* bg, float scale, CCObject* target, SEL_MenuHandler sel) {
    auto spr = ButtonSprite::create(text, "goldFont.fnt", bg, 0.9f);
    spr->setScale(scale);
    return CCMenuItemSpriteExtra::create(spr, target, sel);
}
}

namespace bob {
void openHub() {
    if (BobHubPopup::isOpen()) return;
    if (auto popup = BobHubPopup::create()) popup->show();
}
}

BobHubPopup* BobHubPopup::create() {
    auto ret = new BobHubPopup();
    if (ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

BobHubPopup::~BobHubPopup() {
    s_open = false;
}

bool BobHubPopup::init() {
    if (!Popup::init(kW, kH)) return false;
    s_open = true;
    this->setID("problem-bob-popup"_spr);
    this->setTitle("Problem Solver");

    // Bob's patch of ground on the left.
    m_stage = BobStage::create({ 132.f, 196.f }, 112.f);
    m_stage->setID("bob-stage"_spr);
    m_mainLayer->addChildAtPosition(m_stage, Anchor::BottomLeft, { 14.f, 14.f });

    // His name tag and the box his words go in.
    m_name = CCLabelBMFont::create(bob::roleName(bob::Role::Problem), "bigFont.fnt");
    m_name->setScale(0.42f);
    m_name->setAnchorPoint({ 0.f, 0.5f });
    m_name->setColor(bob::roleColor(bob::Role::Problem));
    m_name->setID("name"_spr);
    m_mainLayer->addChildAtPosition(m_name, Anchor::BottomLeft, { 154.f, 212.f });

    auto box = bob::darkBox({ 254.f, 118.f }, 80);
    box->setID("speech-box"_spr);
    m_mainLayer->addChildAtPosition(box, Anchor::BottomLeft, { 281.f, 143.f });

    m_speech = BobSpeech::create("bigFont.fnt", 0.36f, 236.f, 14.f);
    m_speech->setID("speech"_spr);
    m_mainLayer->addChildAtPosition(m_speech, Anchor::BottomLeft, { 163.f, 195.f });

    // Main choices (shown after Bob has talked).
    m_menu = CCMenu::create();
    m_menu->setID("choices"_spr);
    m_menu->ignoreAnchorPointForPosition(false);
    m_menu->setContentSize({ kW, kH });
    m_mainLayer->addChildAtPosition(m_menu, Anchor::Center);

    auto fixesBtn = textButton("Bug Fixes", "GJ_button_01.png", 0.62f, this, menu_selector(BobHubPopup::onFixes));
    fixesBtn->setID("fixes-button"_spr);
    fixesBtn->setPosition({ 218.f, 62.f });
    m_menu->addChild(fixesBtn);

    auto levelBtn = textButton("Level Help", "GJ_button_02.png", 0.62f, this, menu_selector(BobHubPopup::onLevelHelp));
    levelBtn->setID("level-help-button"_spr);
    levelBtn->setPosition({ 344.f, 62.f });
    m_menu->addChild(levelBtn);

    auto guideBtn = textButton("Editor Guide", "GJ_button_05.png", 0.62f, this, menu_selector(BobHubPopup::onGuide));
    guideBtn->setID("guide-button"_spr);
    guideBtn->setPosition({ 218.f, 30.f });
    m_menu->addChild(guideBtn);

    auto settingsBtn = textButton("Settings", "GJ_button_04.png", 0.62f, this, menu_selector(BobHubPopup::onSettings));
    settingsBtn->setID("settings-button"_spr);
    settingsBtn->setPosition({ 344.f, 30.f });
    m_menu->addChild(settingsBtn);

    m_menu->setVisible(false);

    // Back and Skip.
    auto ctrl = CCMenu::create();
    ctrl->setID("controls"_spr);
    ctrl->ignoreAnchorPointForPosition(false);
    ctrl->setContentSize({ kW, kH });
    m_mainLayer->addChildAtPosition(ctrl, Anchor::Center);

    m_backBtn = textButton("Back", "GJ_button_04.png", 0.6f, this, menu_selector(BobHubPopup::onBack));
    m_backBtn->setID("back-button"_spr);
    m_backBtn->setPosition({ 281.f, 46.f });
    m_backBtn->setVisible(false);
    ctrl->addChild(m_backBtn);

    m_skipBtn = textButton("Skip", "GJ_button_04.png", 0.4f, this, menu_selector(BobHubPopup::onSkip));
    m_skipBtn->setID("skip-button"_spr);
    m_skipBtn->setPosition({ 386.f, 96.f });
    m_skipBtn->setVisible(false);
    ctrl->addChild(m_skipBtn);

    this->schedule(schedule_selector(BobHubPopup::tickSkip));

    // Give the popup a moment to open before the ground starts shaking.
    this->runAction(CCSequence::create(
        CCDelayTime::create(0.35f),
        CCCallFunc::create(this, callfunc_selector(BobHubPopup::startIntro)),
        nullptr
    ));
    return true;
}

void BobHubPopup::startIntro() {
    bool dig = Mod::get()->getSettingValue<bool>("always-intro") || !s_freed;
    if (dig) {
        m_stage->digOut([this]() { this->sayThanks(); });
    }
    else {
        m_stage->standUp();
        this->sayHello(kBackAgain);
    }
}

void BobHubPopup::sayThanks() {
    s_freed = true;
    m_stage->cheer();
    m_stage->startTalking();
    m_speech->setTextColor({ 255, 230, 120 });
    m_speech->say(kThanks, [this]() {
        m_stage->stopTalking();
        this->runAction(CCSequence::create(
            CCDelayTime::create(1.1f),
            CCCallFunc::create(this, callfunc_selector(BobHubPopup::thanksDone)),
            nullptr
        ));
    });
}

void BobHubPopup::thanksDone() {
    this->sayHello(kHello);
}

void BobHubPopup::sayHello(char const* line) {
    this->setRole(bob::Role::Problem);
    this->say(line, true);
}

void BobHubPopup::say(std::string const& text, bool menuAfter) {
    m_speech->setTextColor({ 255, 255, 255 });
    m_stage->startTalking();
    m_speech->say(text, [this, menuAfter]() {
        m_stage->stopTalking();
        if (menuAfter) this->showMenu();
    });
}

void BobHubPopup::showMenu() {
    m_backBtn->setVisible(false);
    m_menu->setVisible(true);
    int i = 0;
    for (auto child : CCArrayExt<CCNode*>(m_menu->getChildren())) {
        child->stopAllActions();
        child->setScale(0.f);
        child->runAction(CCSequence::create(
            CCDelayTime::create(0.06f * static_cast<float>(i++)),
            CCEaseBackOut::create(CCScaleTo::create(0.25f, 1.f)),
            nullptr
        ));
    }
}

void BobHubPopup::hideMenu() {
    m_menu->setVisible(false);
}

void BobHubPopup::setRole(bob::Role role) {
    m_name->setString(bob::roleName(role));
    m_name->setColor(bob::roleColor(role));
}

void BobHubPopup::tickSkip(float) {
    if (m_skipBtn && m_speech) m_skipBtn->setVisible(m_speech->isTyping());
}

void BobHubPopup::onFixes(CCObject*) {
    bob::sfx("counter003.ogg");
    this->hideMenu();
    this->setRole(bob::Role::Problem);

    auto const& list = bob::fixes();
    std::string text;
    if (list.empty()) {
        text = "My fix list is empty right now! Tell Litchin which bugs keep bugging you, "
               "and I'll squash them one by one.";
    }
    else {
        text = fmt::format("Bugs I've squashed ({}):", list.size());
        size_t shown = 0;
        for (auto const& fix : list) {
            if (shown == 5) {
                text += fmt::format("\n...and {} more in Settings.", list.size() - shown);
                break;
            }
            text += fmt::format("\n- {} ({})", fix.name, bob::fixOn(fix.id) ? "ON" : "OFF");
            shown++;
        }
    }
    this->say(text, false);
    m_backBtn->setVisible(true);
}

void BobHubPopup::onLevelHelp(CCObject*) {
    bob::sfx("counter003.ogg");
    this->hideMenu();
    this->setRole(bob::Role::LevelBeater);
    m_stage->cheer();
    std::string text =
        "Level Beater Bob here! When one part keeps getting you, I'll pop in with a tip. "
        "I never play for you, so your runs stay legit.\nTip: " +
        bob::levelTip(m_tip++, 0, false);
    this->say(text, false);
    m_backBtn->setVisible(true);
}

void BobHubPopup::onGuide(CCObject*) {
    bob::sfx("counter003.ogg");
    // Swap to Buster Bob's guide. Nothing below touches this popup after it closes.
    this->onClose(nullptr);
    bob::openGuide();
}

void BobHubPopup::onSettings(CCObject*) {
    bob::sfx("counter003.ogg");
    openSettingsPopup(Mod::get());
}

void BobHubPopup::onBack(CCObject*) {
    bob::sfx("counter003.ogg");
    m_backBtn->setVisible(false);
    this->setRole(bob::Role::Problem);
    this->say("What else can I fix for you?", true);
}

void BobHubPopup::onSkip(CCObject*) {
    m_speech->skip();
}

void BobHubPopup::onClose(CCObject* sender) {
    bob::sfx("quitSound_01.ogg");
    Popup::onClose(sender);
}
