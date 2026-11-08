#pragma once

#include <Geode/Geode.hpp>
#include <vector>
#include "Shared.hpp"

// Buster Bob: walks you through building a level, from the song and layout
// to colours, objects, effects and verifying.
class BusterBobPopup : public geode::Popup {
public:
    static BusterBobPopup* create();
    static bool isOpen() { return s_open; }
    ~BusterBobPopup() override;

protected:
    bool init();
    void onClose(cocos2d::CCObject* sender) override;

    void buildSetup();
    void buildGuide();
    void arrived();
    void showSetup(bool greet);
    void showStep(int step);
    void buildSwatches();
    void refreshChoices();
    void save();
    void talk(BobSpeech* speech, std::string const& text);
    void closeNow();

    void onStyle(cocos2d::CCObject* sender);
    void onQuality(cocos2d::CCObject* sender);
    void onDetail(cocos2d::CCObject* sender);
    void onStart(cocos2d::CCObject*);
    void onNext(cocos2d::CCObject*);
    void onPrev(cocos2d::CCObject*);
    void onRestyle(cocos2d::CCObject*);
    void onDone(cocos2d::CCObject*);

private:
    static bool s_open;

    BobStage* m_stage = nullptr;
    cocos2d::CCNode* m_setupNode = nullptr;
    cocos2d::CCNode* m_guideNode = nullptr;
    BobSpeech* m_setupSpeech = nullptr;
    BobSpeech* m_guideSpeech = nullptr;
    cocos2d::CCLabelBMFont* m_stepTitle = nullptr;
    cocos2d::CCNode* m_swatches = nullptr;
    std::vector<ButtonSprite*> m_styleBtns;
    std::vector<ButtonSprite*> m_qualityBtns;
    std::vector<ButtonSprite*> m_detailBtns;
    CCMenuItemSpriteExtra* m_prevBtn = nullptr;
    CCMenuItemSpriteExtra* m_nextBtn = nullptr;
    CCMenuItemSpriteExtra* m_doneBtn = nullptr;

    int m_style = 0;
    int m_quality = 1;
    int m_detail = 1;
    int m_step = -1; // -1 = still picking a style
};
