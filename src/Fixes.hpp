#pragma once

#include <string>
#include <vector>

namespace bob {

// One bug Problem Bob has fixed.
//
// To add a fix:
//   1. Add an entry to the list in Fixes.cpp.
//   2. Add a bool setting called "fix-<id>" (default true) to mod.json.
//   3. Put the hook that fixes it in its own file in src/ and only change
//      behaviour when bob::fixOn("<id>") is true.
// Bob then announces it once on the main menu and lists it in his popup.
struct BugFix {
    char const* id;       // short id, also used for the "fix-<id>" setting
    char const* name;     // shown as: You know the "<name>"? Well I fixed it!
    char const* details;  // one line about what was going wrong
    char const* versions; // which GD versions had the bug, e.g. "2.2081"
};

std::vector<BugFix> const& fixes();
bool fixOn(std::string const& id);
// Shows a toast for every fix Bob hasn't told you about yet.
void announceFixes();

} // namespace bob
