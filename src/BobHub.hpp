#pragma once

#include <Geode/Geode.hpp>
#include "Shared.hpp"

// Problem Bob's popup from the main menu button. He digs out of the ground,
// thanks you for freeing him, then offers his help.
class BobHubPopup : public geode::Popup {
public:
    static BobHubPopup* create();
    static bool isOpen() { return s_open; }
    ~BobHubPopup() override;

protected:
    bool init();
    void onClose(cocos2d::CCObject* sender) override;

    void startIntro();
    void sayThanks();
    void thanksDone();
    void sayHello(char const* line);
    void say(std::string const& text, bool menuAfter);

    void showMenu();
    void hideMenu();
    void setRole(bob::Role role);
    void tickSkip(float);

    void onFixes(cocos2d::CCObject*);
    void onLevelHelp(cocos2d::CCObject*);
    void onGuide(cocos2d::CCObject*);
    void onSettings(cocos2d::CCObject*);
    void onBack(cocos2d::CCObject*);
    void onSkip(cocos2d::CCObject*);

private:
    static bool s_open;
    static bool s_freed;

    BobStage* m_stage = nullptr;
    BobSpeech* m_speech = nullptr;
    cocos2d::CCLabelBMFont* m_name = nullptr;
    cocos2d::CCMenu* m_menu = nullptr;
    CCMenuItemSpriteExtra* m_backBtn = nullptr;
    CCMenuItemSpriteExtra* m_skipBtn = nullptr;
    int m_tip = 0;
};
