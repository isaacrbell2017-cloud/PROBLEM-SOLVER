#pragma once

#include <Geode/Geode.hpp>
#include <Geode/ui/NineSlice.hpp>
#include <functional>
#include <string>
#include <vector>

namespace bob {
    // Bob's three jobs. Each one has its own name tag colour.
    enum class Role { Problem, Buster, LevelBeater };

    char const* roleName(Role role);
    cocos2d::ccColor3B roleColor(Role role);

    // Plays a GD sound effect if "Sound Effects" is on.
    void sfx(char const* file);
    // Little talking blip (rate-limited so it never spams).
    void blip();
    // Characters per second from the "Text Speed" setting. 0 = instant.
    float charsPerSecond();

    // Bob's sprite, scaled so he is `height` points tall.
    cocos2d::CCSprite* createBob(float height);

    // GD's brown panel (the normal popup look).
    geode::NineSlice* brownBox(cocos2d::CCSize const& size);
    // The see-through dark box GD uses inside popups.
    geode::NineSlice* darkBox(cocos2d::CCSize const& size, unsigned char opacity = 90);

    // Splits text into lines that fit `width` with this font and scale.
    std::vector<std::string> wrap(std::string const& text, char const* font, float scale, float width);

    // Shows Problem Bob's popup (with the dig-out intro).
    void openHub();
    // Shows Buster Bob's level-building guide.
    void openGuide();
}

// Word-wrapped text that types itself out, letter by letter.
// Position is the top-left corner of the text.
class BobSpeech : public cocos2d::CCNode {
public:
    static BobSpeech* create(char const* font, float scale, float width, float lineHeight);

    void say(std::string const& text, std::function<void()> onDone = nullptr);
    // Finishes the current line instantly.
    void skip();
    bool isTyping() const { return m_typing; }
    void setTextColor(cocos2d::ccColor3B color);
    int lineCount() const { return static_cast<int>(m_lines.size()); }

protected:
    bool init(char const* font, float scale, float width, float lineHeight);
    void tick(float dt);
    void reveal(int chars);
    void finish();

    std::string m_font;
    float m_scale = 1.f;
    float m_width = 100.f;
    float m_lineHeight = 12.f;
    cocos2d::ccColor3B m_color { 255, 255, 255 };
    std::vector<std::string> m_lines;
    std::vector<cocos2d::CCLabelBMFont*> m_labels;
    int m_total = 0;
    int m_shown = 0;
    float m_progress = 0.f;
    bool m_typing = false;
    std::function<void()> m_onDone;
};

// Bob standing in a patch of ground. He can burst out of it, idle, and talk.
class BobStage : public cocos2d::CCNode {
public:
    // size = the whole area; Bob is bobHeight tall.
    static BobStage* create(cocos2d::CCSize const& size, float bobHeight);

    // Starts underground and bursts out with dirt, rumble and sound.
    void digOut(std::function<void()> onDone = nullptr);
    // Already above ground (no animation).
    void standUp();
    void startTalking();
    void stopTalking();
    void cheer();

protected:
    bool init(cocos2d::CCSize const& size, float bobHeight);
    void popUp();
    void finishDig();
    void burstDirt();
    void startIdle();

    cocos2d::CCNode* m_shake = nullptr;
    cocos2d::CCSprite* m_bob = nullptr;
    cocos2d::CCPoint m_restPos;
    cocos2d::CCPoint m_hiddenPos;
    float m_groundY = 0.f;
    float m_bobHeight = 0.f;
    float m_baseScale = 1.f;
    std::function<void()> m_onDug;
};

// A small Bob that slides in at the top right with a message, then leaves.
// Queued, so several messages show one after another.
class BobToast : public cocos2d::CCNode {
public:
    static void show(bob::Role role, std::string const& text, float minSeconds = 3.5f, char const* sound = nullptr);

protected:
    static void showNext();
    bool init(bob::Role role, std::string const& text, float minSeconds, char const* sound);
    void onExit() override;
    void typed();
    void leave();

    BobSpeech* m_speech = nullptr;
    float m_minSeconds = 3.5f;
    bool m_finishedNormally = false;
};
