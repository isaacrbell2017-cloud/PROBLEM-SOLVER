#pragma once

#include <Geode/Geode.hpp>
#include <array>
#include <string>

// Everything Buster Bob knows about building a level.
namespace guide {

struct Swatch {
    char const* name;
    cocos2d::ccColor3B color;
};

struct Style {
    char const* name;
    char const* song;
    char const* blocks;
    char const* deco;
    char const* effects;
    std::array<Swatch, 4> colors; // BG, Ground, Main, Accent
};

int styleCount();
Style const& style(int index);

// 0 = High, 1 = Medium, 2 = Low
char const* levelName(int level);

int stepCount();
char const* stepTitle(int step);
// The step that shows colour swatches.
constexpr int kColorStep = 2;
std::string stepText(int step, int style, int quality, int detail);

std::string hex(cocos2d::ccColor3B color);

} // namespace guide
