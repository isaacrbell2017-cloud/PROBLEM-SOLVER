# Problem Solver

A Geode mod for Geometry Dash **2.2081** by Litchin, starring Bob the robot:

- **Problem Bob**: blue robot button next to the Geode button on the main menu. He bursts out of the ground, thanks you for freeing him, and shows his bug fixes.
- **Buster Bob**: in the editor, tap **||** then the robot button (mobile) or press **F** (PC). He walks you through building a level step by step for the style, quality and detail you pick.
- **Level Beater Bob**: tips when one part of a level keeps getting you, and cheers on new bests. No autoplay or cheats.

## Getting the mod
Every push to `main` builds the mod on GitHub. The newest `.geode` file is on the
[`builds` branch](../../tree/builds). Put it in your Geode mods folder and restart the game.

## Changing Bob's look
Bob's picture is `resources/bob.png`. Replace that file with your own PNG (transparent background, about 400 x 500)
and he'll use it everywhere: the buttons, the dig-out, the editor guide and the tips.

## Adding a bug fix
1. Add the bug to the list in `src/Fixes.cpp`.
2. Add a `"fix-<id>"` bool setting to `mod.json`.
3. Write the hook in its own file under `src/`, and only change things when `bob::fixOn("<id>")` is true.

Bob lists it in his popup and announces it once on the main menu.

## Geometry Dash 2.2091
Geode's 2.2091 support is still an alpha (Geode v6). Once it's stable, this mod gets a 2.2091 build too.
