#include "Fixes.hpp"
#include "Shared.hpp"

using namespace geode::prelude;

namespace bob {

std::vector<BugFix> const& fixes() {
    // Fixes go here once each bug is confirmed and its hook is written.
    // Example:
    // { "editor-zoom", "Editor Zoom Jump Bug", "Zooming made the camera jump.", "2.2081" },
    static std::vector<BugFix> const list = {};
    return list;
}

bool fixOn(std::string const& id) {
    for (auto const& fix : fixes()) {
        if (id == fix.id) {
            return Mod::get()->getSettingValue<bool>(fmt::format("fix-{}", fix.id));
        }
    }
    return false;
}

void announceFixes() {
    if (!Mod::get()->getSettingValue<bool>("fix-announcements")) return;
    for (auto const& fix : fixes()) {
        if (!fixOn(fix.id)) continue;
        auto key = fmt::format("announced-{}", fix.id);
        if (Mod::get()->getSavedValue<bool>(key, false)) continue;
        Mod::get()->setSavedValue<bool>(key, true);
        BobToast::show(
            Role::Problem,
            fmt::format("You know the \"{}\"? Well I fixed it! So when you try it, it's fully patched and fixed.", fix.name),
            4.f,
            "achievement_01.ogg"
        );
    }
}

} // namespace bob
