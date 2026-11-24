CARD GAME - FINAL RADIO/AUDIO PATCH

Replace in your project:
  CMakeLists.txt
  GLProject/CardGameMain.cpp
  GLProject/Room.cpp

Copy/replace:
  GLProject/audio/card_slide_better.wav

Copy only if missing:
  GLProject/textures/card_back.png

KEEP your existing file:
  GLProject/audio/radio.wav

Radio playback resolves the WAV path from the executable folder (rather than
VS Code's working directory), opens it through MCI, and uses PlaySound as a WAV
fallback if MCI cannot open it. Press R to toggle radio playback. The card sound
is a new short paper/felt swish. MCI radio playback is independent of card sound.

Controls:
  1 = top-down camera; cards lie flat
  2 = player camera; cards stand upright
  G = deal one random remaining card; after five cards, further presses do nothing
  R = toggle radio.wav playback
  ESC = exit

Standing cards are required: camera 1 is top-down with flat cards; camera 2 is
the player's view with upright cards. Card.cpp is included unchanged for
reference and does not need replacement.

CMake links winmm. The post-build copy command copies GLProject assets beside
the executable. Keep radio.wav inside GLProject/audio before building.

Build was not run here because the complete project dependencies and the user's
local radio.wav are not available in this environment.
