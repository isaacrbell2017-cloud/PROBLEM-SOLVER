#include "Shared.hpp"

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <deque>
#include <sstream>

using namespace geode::prelude;

namespace bob {

char const* roleName(Role role) {
    switch (role) {
        case Role::Buster: return "Buster Bob";
        case Role::LevelBeater: return "Level Beater Bob";
        default: return "Problem Bob";
    }
}

ccColor3B roleColor(Role role) {
    switch (role) {
        case Role::Buster: return { 255, 190, 60 };
        case Role::LevelBeater: return { 255, 120, 120 };
        default: return { 110, 220, 255 };
    }
}

void sfx(char const* file) {
    if (!file || !Mod::get()->getSettingValue<bool>("sfx")) return;
    if (auto engine = FMODAudioEngine::sharedEngine()) {
        engine->playEffect(file);
    }
}

void blip() {
    if (!Mod::get()->getSettingValue<bool>("voice-blips")) return;
    static auto last = std::chrono::steady_clock::time_point {};
    auto now = std::chrono::steady_clock::now();
    if (now - last < std::chrono::milliseconds(80)) return;
    last = now;
    sfx("counter003.ogg");
}

float charsPerSecond() {
    auto speed = Mod::get()->getSettingValue<std::string>("text-speed");
    if (speed == "Instant") return 0.f;
    if (speed == "Slow") return 22.f;
    if (speed == "Fast") return 95.f;
    return 45.f;
}

CCSprite* createBob(float height) {
    // bob.png lives in resources/. Replace that file to give Bob a new look.
    CCSprite* spr = CCSprite::create("bob.png"_spr);
    if (!spr) spr = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
    if (spr && spr->getContentSize().height > 0.f) {
        spr->setScale(height / spr->getContentSize().height);
    }
    return spr;
}

NineSlice* brownBox(CCSize const& size) {
    auto box = NineSlice::create("GJ_square01.png", { 0.f, 0.f, 80.f, 80.f });
    box->setContentSize(size);
    return box;
}

NineSlice* darkBox(CCSize const& size, unsigned char opacity) {
    auto box = NineSlice::create("square02b_001.png", { 0.f, 0.f, 80.f, 80.f });
    box->setContentSize(size);
    box->setColor({ 0, 0, 0 });
    box->setOpacity(opacity);
    return box;
}

std::vector<std::string> wrap(std::string const& text, char const* font, float scale, float width) {
    std::vector<std::string> lines;
    auto measure = CCLabelBMFont::create("", font);
    auto widthOf = [&](std::string const& s) {
        if (!measure) return static_cast<float>(s.size()) * 8.f * scale;
        measure->setString(s.c_str());
        return measure->getContentSize().width * scale;
    };

    size_t start = 0;
    while (true) {
        size_t nl = text.find('\n', start);
        std::string para = text.substr(start, nl == std::string::npos ? std::string::npos : nl - start);

        std::istringstream words(para);
        std::string word;
        std::string line;
        while (words >> word) {
            std::string candidate = line.empty() ? word : line + " " + word;
            if (!line.empty() && widthOf(candidate) > width) {
                lines.push_back(line);
                line = word;
            }
            else {
                line = candidate;
            }
        }
        lines.push_back(line);

        if (nl == std::string::npos) break;
        start = nl + 1;
    }
    return lines;
}

} // namespace bob

// ============================================================================
// BobSpeech: typewriter text
// ============================================================================

BobSpeech* BobSpeech::create(char const* font, float scale, float width, float lineHeight) {
    auto ret = new BobSpeech();
    if (ret->init(font, scale, width, lineHeight)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool BobSpeech::init(char const* font, float scale, float width, float lineHeight) {
    if (!CCNode::init()) return false;
    m_font = font;
    m_scale = scale;
    m_width = width;
    m_lineHeight = lineHeight;
    this->setAnchorPoint({ 0.f, 1.f });
    return true;
}

void BobSpeech::setTextColor(ccColor3B color) {
    m_color = color;
    for (auto label : m_labels) label->setColor(color);
}

void BobSpeech::say(std::string const& text, std::function<void()> onDone) {
    this->unschedule(schedule_selector(BobSpeech::tick));
    for (auto label : m_labels) label->removeFromParent();
    m_labels.clear();

    m_lines = bob::wrap(text, m_font.c_str(), m_scale, m_width);
    float height = m_lineHeight * static_cast<float>(m_lines.size());
    this->setContentSize({ m_width, height });

    m_total = 0;
    for (size_t i = 0; i < m_lines.size(); i++) {
        auto label = CCLabelBMFont::create("", m_font.c_str());
        label->setScale(m_scale);
        label->setAnchorPoint({ 0.f, 1.f });
        label->setPosition({ 0.f, height - m_lineHeight * static_cast<float>(i) });
        label->setColor(m_color);
        this->addChild(label);
        m_labels.push_back(label);
        m_total += static_cast<int>(m_lines[i].size());
    }

    m_shown = 0;
    m_progress = 0.f;
    m_onDone = std::move(onDone);
    m_typing = true;

    if (bob::charsPerSecond() <= 0.f || m_total == 0) {
        this->finish();
        return;
    }
    this->schedule(schedule_selector(BobSpeech::tick));
}

void BobSpeech::tick(float dt) {
    m_progress += dt * std::max(1.f, bob::charsPerSecond());
    int target = std::min(m_total, static_cast<int>(m_progress));
    if (target > m_shown) {
        this->reveal(target);
        bob::blip();
    }
    if (m_shown >= m_total) this->finish();
}

void BobSpeech::reveal(int chars) {
    m_shown = chars;
    int left = chars;
    for (size_t i = 0; i < m_lines.size() && i < m_labels.size(); i++) {
        int len = static_cast<int>(m_lines[i].size());
        int n = std::clamp(left, 0, len);
        m_labels[i]->setString(m_lines[i].substr(0, n).c_str());
        left -= len;
    }
}

void BobSpeech::finish() {
    this->unschedule(schedule_selector(BobSpeech::tick));
    this->reveal(m_total);
    if (!m_typing) return;
    m_typing = false;
    if (m_onDone) {
        auto done = std::move(m_onDone);
        m_onDone = nullptr;
        done();
    }
}

void BobSpeech::skip() {
    if (m_typing) this->finish();
}

// ============================================================================
// BobStage: Bob in his hole
// ============================================================================

BobStage* BobStage::create(CCSize const& size, float bobHeight) {
    auto ret = new BobStage();
    if (ret->init(size, bobHeight)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool BobStage::init(CCSize const& size, float bobHeight) {
    if (!CCNode::init()) return false;
    this->setContentSize(size);
    this->setAnchorPoint({ 0.f, 0.f });
    m_bobHeight = bobHeight;
    m_groundY = 26.f;

    m_shake = CCNode::create();
    m_shake->setContentSize(size);
    this->addChild(m_shake);

    // The ground: dirt with a lighter top edge and a few pebbles.
    auto dirt = CCLayerColor::create({ 86, 52, 28, 255 }, size.width, m_groundY);
    dirt->setPosition({ 0.f, 0.f });
    m_shake->addChild(dirt, 0);

    auto edge = CCLayerColor::create({ 132, 86, 46, 255 }, size.width, 5.f);
    edge->setPosition({ 0.f, m_groundY - 5.f });
    m_shake->addChild(edge, 0);

    constexpr float pebbles[][3] = {
        { 0.12f, 0.35f, 4.f }, { 0.30f, 0.65f, 3.f }, { 0.55f, 0.25f, 5.f },
        { 0.72f, 0.55f, 3.f }, { 0.88f, 0.30f, 4.f }, { 0.43f, 0.72f, 2.f },
    };
    for (auto const& p : pebbles) {
        auto pebble = CCLayerColor::create({ 60, 36, 20, 255 }, p[2], p[2]);
        pebble->setPosition({ size.width * p[0], (m_groundY - 6.f) * p[1] });
        m_shake->addChild(pebble, 0);
    }

    // The hole Bob climbs out of.
    auto hole = bob::darkBox({ bobHeight * 1.6f, 24.f }, 210);
    hole->setScale(0.5f);
    hole->setAnchorPoint({ 0.5f, 0.5f });
    hole->setPosition({ size.width / 2.f, m_groundY - 1.f });
    m_shake->addChild(hole, 1);

    // Everything below the ground line is hidden, so Bob can hide underground.
    auto stencil = CCLayerColor::create({ 255, 255, 255, 255 }, size.width, size.height - m_groundY + 40.f);
    stencil->setPosition({ 0.f, m_groundY });
    auto clip = CCClippingNode::create(stencil);
    clip->setContentSize(size);
    m_shake->addChild(clip, 2);

    m_bob = bob::createBob(bobHeight);
    m_baseScale = m_bob->getScale();
    m_bob->setAnchorPoint({ 0.5f, 0.f });
    m_restPos = { size.width / 2.f, m_groundY - 4.f };
    m_hiddenPos = { size.width / 2.f, m_groundY - bobHeight - 8.f };
    m_bob->setPosition(m_hiddenPos);
    clip->addChild(m_bob);
    return true;
}

void BobStage::digOut(std::function<void()> onDone) {
    m_onDug = std::move(onDone);
    m_bob->stopAllActions();
    m_bob->setPosition(m_hiddenPos);

    // Rumble: the ground shakes and Bob's antenna peeks out.
    bob::sfx("explode_11.ogg");
    auto shake = CCSequence::create(
        CCMoveBy::create(0.04f, { 3.f, 0.f }),
        CCMoveBy::create(0.04f, { -6.f, 1.f }),
        CCMoveBy::create(0.04f, { 6.f, -1.f }),
        CCMoveBy::create(0.04f, { -6.f, 0.f }),
        CCMoveBy::create(0.04f, { 6.f, 1.f }),
        CCMoveBy::create(0.04f, { -3.f, -1.f }),
        nullptr
    );
    m_shake->runAction(CCRepeat::create(shake, 3));

    m_bob->runAction(CCSequence::create(
        CCMoveBy::create(0.25f, { 0.f, 16.f }),
        CCMoveBy::create(0.12f, { -3.f, 0.f }),
        CCMoveBy::create(0.12f, { 6.f, 0.f }),
        CCMoveBy::create(0.12f, { -3.f, -4.f }),
        CCDelayTime::create(0.12f),
        CCCallFunc::create(this, callfunc_selector(BobStage::popUp)),
        nullptr
    ));
}

void BobStage::popUp() {
    bob::sfx("magicExplosion.ogg");
    this->burstDirt();
    m_bob->runAction(CCSequence::create(
        CCEaseBackOut::create(CCMoveTo::create(0.45f, m_restPos)),
        CCCallFunc::create(this, callfunc_selector(BobStage::finishDig)),
        nullptr
    ));
    // A little squash and stretch as he lands.
    m_bob->runAction(CCSequence::create(
        CCScaleTo::create(0.15f, m_baseScale * 0.9f, m_baseScale * 1.12f),
        CCDelayTime::create(0.2f),
        CCScaleTo::create(0.1f, m_baseScale * 1.08f, m_baseScale * 0.92f),
        CCScaleTo::create(0.1f, m_baseScale, m_baseScale),
        nullptr
    ));
}

void BobStage::finishDig() {
    this->startIdle();
    if (m_onDug) {
        auto done = std::move(m_onDug);
        m_onDug = nullptr;
        done();
    }
}

void BobStage::burstDirt() {
    constexpr ccColor4B shades[] = {
        { 86, 52, 28, 255 }, { 120, 78, 40, 255 }, { 60, 36, 20, 255 }, { 150, 100, 55, 255 },
    };
    float cx = this->getContentSize().width / 2.f;
    for (int i = 0; i < 16; i++) {
        float size = 3.f + static_cast<float>(std::rand() % 5);
        auto bit = CCLayerColor::create(shades[i % 4], size, size);
        float startX = cx + static_cast<float>(std::rand() % 40 - 20);
        bit->setPosition({ startX, m_groundY });
        m_shake->addChild(bit, 5);

        float dx = static_cast<float>(std::rand() % 120 - 60);
        float height = 25.f + static_cast<float>(std::rand() % 55);
        float spin = static_cast<float>(std::rand() % 540 - 270);
        bit->runAction(CCSequence::create(
            CCSpawn::create(
                CCJumpBy::create(0.7f, { dx, -m_groundY * 0.6f }, height, 1),
                CCRotateBy::create(0.7f, spin),
                CCSequence::create(CCDelayTime::create(0.4f), CCFadeOut::create(0.3f), nullptr),
                nullptr
            ),
            CCRemoveSelf::create(),
            nullptr
        ));
    }
}

void BobStage::standUp() {
    m_bob->stopAllActions();
    m_bob->setScale(m_baseScale);
    m_bob->setPosition(m_restPos);
    this->startIdle();
}

void BobStage::startIdle() {
    m_bob->stopActionByTag(1);
    m_bob->setPosition(m_restPos);
    auto idle = CCRepeatForever::create(CCSequence::create(
        CCEaseSineInOut::create(CCMoveBy::create(0.9f, { 0.f, 4.f })),
        CCEaseSineInOut::create(CCMoveBy::create(0.9f, { 0.f, -4.f })),
        nullptr
    ));
    idle->setTag(1);
    m_bob->runAction(idle);
}

void BobStage::startTalking() {
    m_bob->stopActionByTag(2);
    auto talk = CCRepeatForever::create(CCSequence::create(
        CCScaleTo::create(0.09f, m_baseScale * 1.02f, m_baseScale * 0.96f),
        CCScaleTo::create(0.09f, m_baseScale * 0.99f, m_baseScale * 1.04f),
        nullptr
    ));
    talk->setTag(2);
    m_bob->runAction(talk);
}

void BobStage::stopTalking() {
    m_bob->stopActionByTag(2);
    m_bob->setScale(m_baseScale);
}

void BobStage::cheer() {
    m_bob->runAction(CCJumpBy::create(0.5f, { 0.f, 0.f }, 16.f, 2));
}

// ============================================================================
// BobToast: Bob slides in with a message
// ============================================================================

namespace {
struct PendingToast {
    bob::Role role;
    std::string text;
    float minSeconds;
    char const* sound;
};
std::deque<PendingToast> s_queue;
bool s_active = false;

constexpr float kToastW = 290.f;
constexpr float kTextW = 206.f;
constexpr float kTextScale = 0.3f;
constexpr float kLineH = 11.f;
}

void BobToast::show(bob::Role role, std::string const& text, float minSeconds, char const* sound) {
    s_queue.push_back({ role, text, minSeconds, sound });
    if (s_queue.size() > 6) s_queue.pop_front();
    if (!s_active) showNext();
}

void BobToast::showNext() {
    if (s_active || s_queue.empty()) return;
    auto scene = CCDirector::sharedDirector()->getRunningScene();
    if (!scene) {
        s_queue.clear();
        return;
    }
    auto next = s_queue.front();
    s_queue.pop_front();

    auto toast = new BobToast();
    if (!toast->init(next.role, next.text, next.minSeconds, next.sound)) {
        delete toast;
        return;
    }
    toast->autorelease();
    s_active = true;
    scene->addChild(toast, 10000);
}

bool BobToast::init(bob::Role role, std::string const& text, float minSeconds, char const* sound) {
    if (!CCNode::init()) return false;
    m_minSeconds = minSeconds;

    auto lines = bob::wrap(text, "bigFont.fnt", kTextScale, kTextW);
    float boxH = std::max(62.f, 30.f + kLineH * static_cast<float>(lines.size()));
    this->setContentSize({ kToastW, boxH + 10.f });
    this->setAnchorPoint({ 0.f, 0.f });

    auto box = bob::brownBox({ kToastW - 52.f, boxH });
    box->setAnchorPoint({ 0.f, 0.f });
    box->setPosition({ 52.f, 4.f });
    this->addChild(box, 0);

    auto bobSpr = bob::createBob(66.f);
    bobSpr->setAnchorPoint({ 0.5f, 0.f });
    bobSpr->setPosition({ 32.f, 2.f });
    bobSpr->setRotation(-6.f);
    this->addChild(bobSpr, 2);
    bobSpr->runAction(CCRepeatForever::create(CCSequence::create(
        CCEaseSineInOut::create(CCRotateBy::create(0.6f, 12.f)),
        CCEaseSineInOut::create(CCRotateBy::create(0.6f, -12.f)),
        nullptr
    )));

    auto name = CCLabelBMFont::create(bob::roleName(role), "bigFont.fnt");
    name->setScale(0.38f);
    name->setColor(bob::roleColor(role));
    name->setAnchorPoint({ 0.f, 1.f });
    name->setPosition({ 64.f, boxH - 4.f });
    this->addChild(name, 1);

    m_speech = BobSpeech::create("bigFont.fnt", kTextScale, kTextW, kLineH);
    m_speech->setPosition({ 64.f, boxH - 18.f });
    this->addChild(m_speech, 1);

    auto win = CCDirector::sharedDirector()->getWinSize();
    float y = win.height - boxH - 44.f;
    this->setPosition({ win.width + 10.f, y });
    this->runAction(CCEaseBackOut::create(CCMoveTo::create(0.35f, { win.width - kToastW - 6.f, y })));

    bob::sfx(sound);
    m_speech->say(text, [this]() { this->typed(); });
    return true;
}

void BobToast::typed() {
    this->runAction(CCSequence::create(
        CCDelayTime::create(m_minSeconds),
        CCCallFunc::create(this, callfunc_selector(BobToast::leave)),
        nullptr
    ));
}

void BobToast::leave() {
    m_finishedNormally = true;
    this->runAction(CCSequence::create(
        CCEaseBackIn::create(CCMoveBy::create(0.3f, { kToastW + 20.f, 0.f })),
        CCRemoveSelf::create(),
        nullptr
    ));
}

void BobToast::onExit() {
    CCNode::onExit();
    s_active = false;
    if (m_finishedNormally) {
        queueInMainThread([]() { BobToast::showNext(); });
    }
    else {
        // The scene changed (like leaving a level): drop the rest.
        s_queue.clear();
    }
}
