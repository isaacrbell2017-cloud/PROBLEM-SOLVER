#include "BusterBob.hpp"
#include "GuideData.hpp"

#include <algorithm>

using namespace geode::prelude;

bool BusterBobPopup::s_open = false;

namespace {
constexpr float kW = 440.f;
constexpr float kH = 290.f;

constexpr char const* kOn = "GJ_button_01.png";
constexpr char const* kOff = "GJ_button_04.png";

CCNode* page(CCNode* parent) {
    auto node = CCNode::create();
    node->setContentSize({ kW, kH });
    node->setAnchorPoint({ 0.f, 0.f });
    node->setPosition({ 0.f, 0.f });
    parent->addChild(node);
    return node;
}

CCMenu* pageMenu(CCNode* parent) {
    auto menu = CCMenu::create();
    menu->ignoreAnchorPointForPosition(false);
    menu->setAnchorPoint({ 0.f, 0.f });
    menu->setContentSize({ kW, kH });
    menu->setPosition({ 0.f, 0.f });
    parent->addChild(menu, 5);
    return menu;
}

CCLabelBMFont* heading(CCNode* parent, char const* text, CCPoint pos) {
    auto label = CCLabelBMFont::create(text, "goldFont.fnt");
    label->setScale(0.42f);
    label->setAnchorPoint({ 0.f, 0.5f });
    label->setPosition(pos);
    parent->addChild(label);
    return label;
}

ButtonSprite* choice(CCMenu* menu, char const* text, int tag, CCPoint pos, CCObject* target, SEL_MenuHandler sel) {
    auto spr = ButtonSprite::create(text, "bigFont.fnt", kOff, 0.7f);
    spr->setScale(0.5f);
    auto btn = CCMenuItemSpriteExtra::create(spr, target, sel);
    btn->setTag(tag);
    btn->setPosition(pos);
    menu->addChild(btn);
    return spr;
}
}

namespace bob {
void openGuide() {
    if (BusterBobPopup::isOpen()) return;
    if (auto popup = BusterBobPopup::create()) popup->show();
}
}

BusterBobPopup* BusterBobPopup::create() {
    auto ret = new BusterBobPopup();
    if (ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

BusterBobPopup::~BusterBobPopup() {
    s_open = false;
}

bool BusterBobPopup::init() {
    if (!Popup::init(kW, kH)) return false;
    s_open = true;
    this->setID("buster-bob-popup"_spr);
    this->setTitle("Buster Bob's Level Builder");

    auto mod = Mod::get();
    m_style = static_cast<int>(std::clamp<int64_t>(mod->getSavedValue<int64_t>("style", 0), 0, guide::styleCount() - 1));
    m_quality = static_cast<int>(std::clamp<int64_t>(mod->getSavedValue<int64_t>("quality", 1), 0, 2));
    m_detail = static_cast<int>(std::clamp<int64_t>(mod->getSavedValue<int64_t>("detail", 1), 0, 2));
    m_step = static_cast<int>(std::clamp<int64_t>(mod->getSavedValue<int64_t>("step", -1), -1, guide::stepCount() - 1));

    m_stage = BobStage::create({ 124.f, 214.f }, 118.f);
    m_stage->setID("bob-stage"_spr);
    m_mainLayer->addChildAtPosition(m_stage, Anchor::BottomLeft, { 14.f, 14.f });

    auto name = CCLabelBMFont::create(bob::roleName(bob::Role::Buster), "bigFont.fnt");
    name->setScale(0.42f);
    name->setAnchorPoint({ 0.f, 0.5f });
    name->setColor(bob::roleColor(bob::Role::Buster));
    name->setID("name"_spr);
    m_mainLayer->addChildAtPosition(name, Anchor::BottomLeft, { 150.f, 250.f });

    auto pages = CCNode::create();
    pages->setContentSize({ kW, kH });
    pages->setAnchorPoint({ 0.f, 0.f });
    pages->setID("pages"_spr);
    m_mainLayer->addChildAtPosition(pages, Anchor::BottomLeft, { 0.f, 0.f });

    m_setupNode = page(pages);
    m_setupNode->setID("setup-page"_spr);
    m_guideNode = page(pages);
    m_guideNode->setID("guide-page"_spr);

    this->buildSetup();
    this->buildGuide();
    this->refreshChoices();

    m_setupNode->setVisible(false);
    m_guideNode->setVisible(false);

    // Quick dig out, then straight to work.
    this->runAction(CCSequence::create(
        CCDelayTime::create(0.3f),
        CCCallFunc::create(this, callfunc_selector(BusterBobPopup::arrived)),
        nullptr
    ));
    return true;
}

void BusterBobPopup::buildSetup() {
    auto box = bob::darkBox({ 280.f, 52.f }, 80);
    box->setPosition({ 290.f, 212.f });
    m_setupNode->addChild(box);

    m_setupSpeech = BobSpeech::create("bigFont.fnt", 0.32f, 262.f, 12.f);
    m_setupSpeech->setPosition({ 159.f, 232.f });
    m_setupNode->addChild(m_setupSpeech, 2);

    auto menu = pageMenu(m_setupNode);

    heading(m_setupNode, "Style", { 152.f, 172.f });
    for (int i = 0; i < guide::styleCount(); i++) {
        CCPoint pos { 185.f + 70.f * static_cast<float>(i % 4), i < 4 ? 152.f : 128.f };
        m_styleBtns.push_back(choice(menu, guide::style(i).name, i, pos, this, menu_selector(BusterBobPopup::onStyle)));
    }

    heading(m_setupNode, "Quality", { 152.f, 102.f });
    heading(m_setupNode, "Detail", { 152.f, 74.f });
    for (int i = 0; i < 3; i++) {
        CCPoint q { 262.f + 64.f * static_cast<float>(i), 102.f };
        CCPoint d { 262.f + 64.f * static_cast<float>(i), 74.f };
        m_qualityBtns.push_back(choice(menu, guide::levelName(i), i, q, this, menu_selector(BusterBobPopup::onQuality)));
        m_detailBtns.push_back(choice(menu, guide::levelName(i), i, d, this, menu_selector(BusterBobPopup::onDetail)));
    }

    auto goSpr = ButtonSprite::create("Let's Build!", "goldFont.fnt", "GJ_button_01.png", 0.9f);
    goSpr->setScale(0.75f);
    auto goBtn = CCMenuItemSpriteExtra::create(goSpr, this, menu_selector(BusterBobPopup::onStart));
    goBtn->setID("start-button"_spr);
    goBtn->setPosition({ 290.f, 32.f });
    menu->addChild(goBtn);
}

void BusterBobPopup::buildGuide() {
    auto box = bob::darkBox({ 280.f, 170.f }, 80);
    box->setPosition({ 290.f, 153.f });
    m_guideNode->addChild(box);

    m_stepTitle = CCLabelBMFont::create("", "goldFont.fnt");
    m_stepTitle->setScale(0.48f);
    m_stepTitle->setAnchorPoint({ 0.f, 0.5f });
    m_stepTitle->setPosition({ 159.f, 226.f });
    m_guideNode->addChild(m_stepTitle, 2);

    m_guideSpeech = BobSpeech::create("bigFont.fnt", 0.3f, 264.f, 11.5f);
    m_guideSpeech->setPosition({ 159.f, 214.f });
    m_guideNode->addChild(m_guideSpeech, 2);

    m_swatches = CCNode::create();
    m_swatches->setContentSize({ kW, kH });
    m_swatches->setAnchorPoint({ 0.f, 0.f });
    m_guideNode->addChild(m_swatches, 2);

    auto menu = pageMenu(m_guideNode);

    auto prevSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
    prevSpr->setScale(0.55f);
    m_prevBtn = CCMenuItemSpriteExtra::create(prevSpr, this, menu_selector(BusterBobPopup::onPrev));
    m_prevBtn->setID("prev-button"_spr);
    m_prevBtn->setPosition({ 170.f, 36.f });
    menu->addChild(m_prevBtn);

    auto nextSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
    nextSpr->setScale(0.55f);
    nextSpr->setFlipX(true);
    m_nextBtn = CCMenuItemSpriteExtra::create(nextSpr, this, menu_selector(BusterBobPopup::onNext));
    m_nextBtn->setID("next-button"_spr);
    m_nextBtn->setPosition({ 410.f, 36.f });
    menu->addChild(m_nextBtn);

    auto doneSpr = ButtonSprite::create("Done!", "goldFont.fnt", "GJ_button_01.png", 0.9f);
    doneSpr->setScale(0.6f);
    m_doneBtn = CCMenuItemSpriteExtra::create(doneSpr, this, menu_selector(BusterBobPopup::onDone));
    m_doneBtn->setID("done-button"_spr);
    m_doneBtn->setPosition({ 402.f, 36.f });
    menu->addChild(m_doneBtn);

    auto styleSpr = ButtonSprite::create("Change Style", "bigFont.fnt", "GJ_button_04.png", 0.7f);
    styleSpr->setScale(0.5f);
    auto styleBtn = CCMenuItemSpriteExtra::create(styleSpr, this, menu_selector(BusterBobPopup::onRestyle));
    styleBtn->setID("restyle-button"_spr);
    styleBtn->setPosition({ 290.f, 36.f });
    menu->addChild(styleBtn);
}

void BusterBobPopup::arrived() {
    m_stage->digOut([this]() {
        if (m_step >= 0) {
            m_guideNode->setVisible(true);
            this->showStep(m_step);
        }
        else {
            this->showSetup(true);
        }
    });
}

void BusterBobPopup::talk(BobSpeech* speech, std::string const& text) {
    m_stage->startTalking();
    speech->say(text, [this]() { m_stage->stopTalking(); });
}

void BusterBobPopup::showSetup(bool greet) {
    m_step = -1;
    this->save();
    m_guideNode->setVisible(false);
    m_setupNode->setVisible(true);
    this->refreshChoices();
    this->talk(m_setupSpeech, greet
        ? "Buster Bob here! Pick a style, then how good and how detailed you want your level."
        : "New look? Pick a style, quality and detail, then hit Let's Build!");
}

void BusterBobPopup::showStep(int step) {
    m_step = std::clamp(step, 0, guide::stepCount() - 1);
    this->save();
    m_setupNode->setVisible(false);
    m_guideNode->setVisible(true);

    bool last = m_step == guide::stepCount() - 1;
    m_prevBtn->setVisible(m_step > 0);
    m_nextBtn->setVisible(!last);
    m_doneBtn->setVisible(last);

    m_stepTitle->setString(fmt::format("Step {}/{}: {}", m_step + 1, guide::stepCount(), guide::stepTitle(m_step)).c_str());
    m_stepTitle->limitLabelWidth(264.f, 0.48f, 0.2f);

    this->buildSwatches();
    m_swatches->setVisible(m_step == guide::kColorStep);

    this->talk(m_guideSpeech, guide::stepText(m_step, m_style, m_quality, m_detail));
}

void BusterBobPopup::buildSwatches() {
    m_swatches->removeAllChildren();
    auto const& colors = guide::style(m_style).colors;
    for (int i = 0; i < 4; i++) {
        float x = 156.f + 68.f * static_cast<float>(i);
        auto const& sw = colors[i];

        auto outline = CCLayerColor::create({ 0, 0, 0, 255 }, 22.f, 22.f);
        outline->setPosition({ x, 76.f });
        m_swatches->addChild(outline);

        auto fill = CCLayerColor::create({ sw.color.r, sw.color.g, sw.color.b, 255 }, 18.f, 18.f);
        fill->setPosition({ x + 2.f, 78.f });
        m_swatches->addChild(fill);

        auto name = CCLabelBMFont::create(sw.name, "goldFont.fnt");
        name->setScale(0.32f);
        name->setAnchorPoint({ 0.f, 0.f });
        name->setPosition({ x + 25.f, 87.f });
        m_swatches->addChild(name);

        auto code = CCLabelBMFont::create(guide::hex(sw.color).c_str(), "bigFont.fnt");
        code->setScale(0.2f);
        code->setAnchorPoint({ 0.f, 0.f });
        code->setPosition({ x + 25.f, 77.f });
        m_swatches->addChild(code);
    }
}

void BusterBobPopup::refreshChoices() {
    for (int i = 0; i < static_cast<int>(m_styleBtns.size()); i++) {
        m_styleBtns[i]->updateBGImage(i == m_style ? kOn : kOff);
    }
    for (int i = 0; i < 3; i++) {
        m_qualityBtns[i]->updateBGImage(i == m_quality ? kOn : kOff);
        m_detailBtns[i]->updateBGImage(i == m_detail ? kOn : kOff);
    }
}

void BusterBobPopup::save() {
    auto mod = Mod::get();
    mod->setSavedValue<int64_t>("style", m_style);
    mod->setSavedValue<int64_t>("quality", m_quality);
    mod->setSavedValue<int64_t>("detail", m_detail);
    mod->setSavedValue<int64_t>("step", m_step);
}

void BusterBobPopup::onStyle(CCObject* sender) {
    m_style = std::clamp(sender->getTag(), 0, guide::styleCount() - 1);
    bob::sfx("counter003.ogg");
    this->refreshChoices();
    this->save();
    this->talk(m_setupSpeech, fmt::format("{}! {}", guide::style(m_style).name, guide::style(m_style).deco));
}

void BusterBobPopup::onQuality(CCObject* sender) {
    m_quality = std::clamp(sender->getTag(), 0, 2);
    bob::sfx("counter003.ogg");
    this->refreshChoices();
    this->save();
}

void BusterBobPopup::onDetail(CCObject* sender) {
    m_detail = std::clamp(sender->getTag(), 0, 2);
    bob::sfx("counter003.ogg");
    this->refreshChoices();
    this->save();
}

void BusterBobPopup::onStart(CCObject*) {
    bob::sfx("playSound_01.ogg");
    m_stage->cheer();
    this->showStep(0);
}

void BusterBobPopup::onNext(CCObject*) {
    if (m_guideSpeech->isTyping()) {
        m_guideSpeech->skip();
        return;
    }
    bob::sfx("counter003.ogg");
    this->showStep(m_step + 1);
}

void BusterBobPopup::onPrev(CCObject*) {
    bob::sfx("counter003.ogg");
    this->showStep(m_step - 1);
}

void BusterBobPopup::onRestyle(CCObject*) {
    bob::sfx("counter003.ogg");
    this->showSetup(false);
}

void BusterBobPopup::onDone(CCObject*) {
    if (m_guideSpeech->isTyping()) {
        m_guideSpeech->skip();
        return;
    }
    bob::sfx("achievement_01.ogg");
    m_stage->cheer();
    // Next time he starts fresh at the style picker.
    m_step = -1;
    this->save();
    m_doneBtn->setVisible(false);
    this->runAction(CCSequence::create(
        CCDelayTime::create(0.7f),
        CCCallFunc::create(this, callfunc_selector(BusterBobPopup::closeNow)),
        nullptr
    ));
}

void BusterBobPopup::closeNow() {
    this->onClose(nullptr);
}

void BusterBobPopup::onClose(CCObject* sender) {
    bob::sfx("quitSound_01.ogg");
    Popup::onClose(sender);
}
