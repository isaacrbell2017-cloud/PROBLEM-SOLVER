#include "LevelBeater.hpp"
#include "Shared.hpp"

#include <Geode/modify/PlayLayer.hpp>

#include <algorithm>
#include <iterator>
#include <map>

using namespace geode::prelude;

namespace bob {

namespace {
// {} is replaced with the spot where you keep dying, like "47%".
constexpr char const* kTips[] = {
    "Go into Practice Mode, put a checkpoint just before {}, and play that part until you get it 5 times in a row.",
    "Listen instead of looking: click on the music. Most parts are built to the beat.",
    "Look a little ahead of your icon, not at it. You'll see the next part coming sooner.",
    "Take a 2 minute break. Tired hands make more mistakes than you think.",
    "Say the rhythm out loud: tap, tap, hold. Rhythm sticks better than memory.",
    "Small taps beat big holds in ship and wave parts. Try shorter clicks around {}.",
    "Make sure Click Between Frames is on in the settings, so every click lands right when you press.",
};
constexpr int kPracticeTip = 0;
}

int tipCount() {
    return static_cast<int>(std::size(kTips));
}

std::string levelTip(int index, int pct, bool inPractice) {
    int i = ((index % tipCount()) + tipCount()) % tipCount();
    // No point telling you to go into practice when you're already there.
    if (inPractice && i == kPracticeTip) i = 1;
    std::string tip = kTips[i];
    std::string spot = pct > 0 ? fmt::format("{}%", pct) : std::string("the hard part");
    std::string out;
    size_t pos = 0;
    while (true) {
        size_t found = tip.find("{}", pos);
        if (found == std::string::npos) {
            out += tip.substr(pos);
            break;
        }
        out += tip.substr(pos, found - pos);
        out += spot;
        pos = found + 2;
    }
    return out;
}

} // namespace bob

// Level Beater Bob watches where you die. He never touches your inputs.
class $modify(BeaterPlayLayer, PlayLayer) {
    struct Fields {
        std::map<int, int> deaths; // 5% section -> deaths there
        int best = 0;
        int attempts = 0;
        int tip = 0;
    };

    void resetLevel() {
        int pct = static_cast<int>(this->getCurrentPercent());
        PlayLayer::resetLevel();
        this->noteAttempt(pct);
    }

    void noteAttempt(int pct) {
        auto mod = Mod::get();
        if (!mod->getSettingValue<bool>("level-beater")) return;
        if (m_isPlatformer) return;
        if (pct <= 0 || pct >= 100) return;

        bool practice = m_isPracticeMode;
        if (practice && !mod->getSettingValue<bool>("in-practice")) return;

        auto fields = m_fields.operator->();
        fields->attempts++;

        if (pct > fields->best) {
            int previous = fields->best;
            fields->best = pct;
            if (!practice && previous > 0 && pct >= 10 && fields->attempts >= 3 &&
                mod->getSettingValue<bool>("new-best-cheer")) {
                BobToast::show(
                    bob::Role::LevelBeater,
                    fmt::format("NEW BEST: {}%! You're getting it, keep going!", pct),
                    2.5f,
                    "highscoreGet02.ogg"
                );
                return;
            }
        }

        int section = pct / 5;
        int count = ++fields->deaths[section];
        int every = static_cast<int>(std::max<int64_t>(3, mod->getSettingValue<int64_t>("deaths-before-tip")));
        if (count % every != 0) return;

        BobToast::show(
            bob::Role::LevelBeater,
            fmt::format("{} deaths around {}%. {}", count, pct, bob::levelTip(fields->tip++, pct, practice)),
            4.5f
        );
    }
};
