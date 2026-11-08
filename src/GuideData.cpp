#include "GuideData.hpp"

#include <algorithm>
#include <iterator>

using namespace geode::prelude;

namespace guide {

namespace {

// BG, Ground, Main, Accent
std::array<Style, 8> const kStyles = { {
    {
        "Modern",
        "Electronic music with hard drops, like dubstep, DnB or hardstyle, around 140-175 BPM.",
        "Plain square blocks with thin outlines. Keep every shape sharp and lined up.",
        "Glow edges, thin neon lines and floating squares. Glow is the star of this style.",
        "Pulse the glow on the beat and flash the background at every drop.",
        { { { "BG", { 10, 8, 28 } }, { "Ground", { 16, 12, 40 } }, { "Main", { 255, 0, 200 } }, { "Accent", { 0, 230, 255 } } } },
    },
    {
        "Null",
        "Dark, minimal or glitchy electronic music, around 120-160 BPM.",
        "Black blocks with only a white outline. No fill detail at all.",
        "White outlines, small glitch squares and lots of empty space. Less is more.",
        "Hard cuts: instant color flips and short shakes on the big hits.",
        { { { "BG", { 0, 0, 0 } }, { "Ground", { 0, 0, 0 } }, { "Main", { 255, 255, 255 } }, { "Accent", { 130, 130, 150 } } } },
    },
    {
        "Space",
        "Trance, synthwave or spacey electronic music, around 120-140 BPM.",
        "Dark blocks with a soft purple glow on the edges, like floating platforms.",
        "Tiny star dots, planets made from circles, and soft glow clouds far behind.",
        "Slow background color fades and stars drifting in the back.",
        { { { "BG", { 14, 6, 40 } }, { "Ground", { 24, 10, 62 } }, { "Main", { 150, 90, 255 } }, { "Accent", { 255, 255, 255 } } } },
    },
    {
        "Heaven",
        "Uplifting trance, orchestral or happy hardcore, around 128-150 BPM.",
        "White or cream blocks with gold trim. Round the corners where you can.",
        "Clouds, pillars, light rays and gold glow. Keep it bright and airy.",
        "Warm white flashes at the drops and slow floating clouds.",
        { { { "BG", { 196, 228, 255 } }, { "Ground", { 255, 248, 232 } }, { "Main", { 255, 210, 110 } }, { "Accent", { 255, 255, 255 } } } },
    },
    {
        "Hell",
        "Heavy dubstep, metal or hard electronic music, around 150-180 BPM.",
        "Dark red blocks with cracks and orange glowing edges.",
        "Spikes, chains, ember dots and lava glow along the bottom.",
        "Red flashes, screen shakes and lava that pulses on the beat.",
        { { { "BG", { 40, 0, 0 } }, { "Ground", { 72, 6, 0 } }, { "Main", { 255, 80, 0 } }, { "Accent", { 255, 200, 40 } } } },
    },
    {
        "Castle",
        "Orchestral, medieval or dark melodic music, around 100-140 BPM.",
        "Brick and stone style blocks, with darker edges for depth.",
        "Chains, torches with orange glow, arches and windows in the back.",
        "Flickering torch pulses and slow camera moves through the halls.",
        { { { "BG", { 24, 20, 30 } }, { "Ground", { 46, 40, 52 } }, { "Main", { 112, 102, 124 } }, { "Accent", { 255, 140, 30 } } } },
    },
    {
        "Nature",
        "Chill, folk-style or happy electronic music, around 100-135 BPM.",
        "Dirt blocks with grass on top. Make the ground bumpy, not flat.",
        "Grass, bushes, trees, flowers and hills in the background.",
        "Gentle color shifts from day to sunset, and plants that sway.",
        { { { "BG", { 120, 200, 255 } }, { "Ground", { 92, 60, 30 } }, { "Main", { 60, 180, 60 } }, { "Accent", { 255, 230, 90 } } } },
    },
    {
        "Retro",
        "Classic Newgrounds-style electro or dance music, around 128-150 BPM.",
        "The classic grid blocks from the first build pages. Clean and simple.",
        "Simple outlines, a few pulsing circles and classic saws. Go easy on glow.",
        "A color change at each new section, like the official levels.",
        { { { "BG", { 40, 125, 255 } }, { "Ground", { 0, 80, 220 } }, { "Main", { 255, 255, 255 } }, { "Accent", { 255, 240, 0 } } } },
    },
} };

constexpr char const* kLevels[] = { "High", "Medium", "Low" };

constexpr char const* kSteps[] = {
    "Song", "Layout", "Colors", "Blocks", "Decoration", "Effects", "Optimize", "Verify & Upload",
};

char const* pick(int level, char const* high, char const* medium, char const* low) {
    if (level <= 0) return high;
    if (level == 1) return medium;
    return low;
}

} // namespace

int styleCount() {
    return static_cast<int>(kStyles.size());
}

Style const& style(int index) {
    return kStyles[std::clamp(index, 0, styleCount() - 1)];
}

char const* levelName(int level) {
    return kLevels[std::clamp(level, 0, 2)];
}

int stepCount() {
    return static_cast<int>(std::size(kSteps));
}

char const* stepTitle(int step) {
    return kSteps[std::clamp(step, 0, stepCount() - 1)];
}

std::string hex(ccColor3B c) {
    return fmt::format("#{:02X}{:02X}{:02X}", static_cast<int>(c.r), static_cast<int>(c.g), static_cast<int>(c.b));
}

std::string stepText(int step, int styleIndex, int q, int d) {
    auto const& s = style(styleIndex);
    switch (step) {
        case 0:
            return fmt::format(
                "Let's build a {} quality, {} detail {} level!\n{} {} Music Library songs work for every player.",
                levelName(q), levelName(d), s.name, s.song,
                pick(q,
                    "Play the song in the editor and record guidelines on the beats before you build anything.",
                    "Mark the drops and big beats with guidelines so the main parts line up.",
                    "Pick a song you love. Syncing can come later.")
            );
        case 1:
            return fmt::format(
                "Build the gameplay first with plain blocks. No deco yet!\n{}",
                pick(q,
                    "Sync every click to the guidelines, mix gamemodes and make smooth transitions. "
                    "Test each part 3 times from a Start Pos.",
                    "Sync the main hits and switch gamemode at the big parts. Test each part from a Start Pos.",
                    "Keep it simple, fair and easy to read. Always show the player where to go.")
            );
        case 2:
            return fmt::format(
                "Here are your {} colors. Set BG and Ground in Level Settings, and put Main and Accent on their own "
                "color channels. Type the codes into the color picker's hex box.\n{}",
                s.name,
                pick(q,
                    "Change colors at every drop with Color Triggers.",
                    "Change colors 2-3 times at the main sections.",
                    "One palette for the whole level is fine.")
            );
        case 3:
            return fmt::format(
                "{}\n{}",
                s.blocks,
                pick(d,
                    "Use 2-3 block styles per part, and add edge and corner pieces so nothing looks cut off.",
                    "Stick to 1-2 block styles with clean edges.",
                    "One block style, kept neat and lined up.")
            );
        case 4:
            return fmt::format(
                "{}\n{}",
                s.deco,
                pick(d,
                    "Build 3 layers: far back, middle and front. Use darker shades of Main further back, "
                    "and add the small details last.",
                    "Use 2 layers: one in the background and one around the blocks.",
                    "One clean layer around the blocks is enough.")
            );
        case 5:
            return fmt::format(
                "{}\n{}",
                s.effects,
                pick(q,
                    "Use Pulse on the beats, Move triggers for motion and a Shader trigger at the biggest drop. "
                    "Keep camera moves smooth.",
                    "Add Pulse at the drops and a few Move triggers.",
                    "Color changes are all you need. Skip the rest.")
            );
        case 6:
            return pick(d,
                "Lots of detail means lots of objects. Mark extra deco as High Detail so Low Detail Mode players "
                "can still play, and don't stack loads of glow on top of each other.",
                "Mark the extra bits as High Detail and keep an eye on the object count.",
                "Your level is light already. Just delete stray objects you aren't using.");
        default:
            return fmt::format(
                "Play the whole level from 0% in normal mode. That's verifying, and you need it to upload.\n{}\n"
                "Buster Bob out!",
                pick(q,
                    "Get a friend to test it, fix anything unfair, then upload with a great name and description.",
                    "Fix anything that felt unfair on your run, then upload.",
                    "Beat it once, then hit Upload. You did it!")
            );
    }
}

} // namespace guide
