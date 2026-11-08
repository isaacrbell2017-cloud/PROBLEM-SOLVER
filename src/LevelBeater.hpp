#pragma once

#include <string>

namespace bob {
    int tipCount();
    // A level-beating tip. pct is where you keep dying (0 = no specific spot).
    std::string levelTip(int index, int pct, bool inPractice);
}
