# air traffic controller multiplayer
inspired by bsd-games atc

# the plan
- rewrite basic atc
- maybe port to web with emscripten for itch.io
- multiplayer
- perhaps extend the base game/add game mods featuring new mechanics
- and maybe port it to esp32

# TODO
current stage: rewriting the basic atc

!sort -r
working

general
- 100 display comms, stats
- 090 implement turn towards & checking if delay beacon is on flight path
- 090 diagonal circle command in atc from bsdgames2 package is implemented HELLA weirdly
- 080 logs, leaderboard
- 080 proper game finishing and quitting/interrupting
- 070 implement TOKEN_HELP
- 070 tests
- 070 platform agnostic server logic
- 070 pc server port
- 060 generate_test_runner.rb does not transfer any symbols other then test_* so i have to put them in a global header
- 060 esp client & server port
- 050 setup .editorconfig .clang and stuff
- 030 handle ^L redraw
- 020 ci/cd (docs on gh pages, tests)
- 010 better docs generation (readthedocs, man page output)
- 010 whats the TOKEN_SHELL ('!') in original atc for

bugs
- 090 the screen clears on start and sometimes flashes on game advance
- 090 set fuel based on box w+h, low fuel threshold to 15
- 070 left box boundary is offset left by 1 (and maybe upper analogously)
- 070 landing at airport says "exited via the wrong exit"

# ideas
## esp32
- hard to make a whole keyboard (button matrix, but might be too uncomfortable
  to play)
- would be sick to have mic comms (but prob requires some good hardware for mic
  processing, STT model, etc., and a good enough mic; could offload to a remote
  server maybe)
## multiplayer
- how to advance game ticks? maybe dont allow it and just make the game a bit
  faster
- mic comms
- shifts mode
  - players switch every minute
  - when taking over they have a short preview
- ownership mode
  - to start to send comms to a plane, one must take ownership
  - a player can request to transfer his or take the other player's ownership.
    The other player has to confirm (or, one can resign from ownership, and
    others can take the free plane)
  - maybe a head controller role to assign ownerships
  - e. g.: Map: default. Planes A(E3->E6), B(E7->A0), C(E2->E4) spawn in. Head
    controller assigns A&B to player 1, C to player 2. Player 1 sets the initial
    course of plane B to Beacon 1, and resigns from its ownership, then
    proceeds to handle plane A. Player 2 turns plane C to E4. Head controller
    assigns plane B to player 2 for him to handle landing to A0. They all can
    communicate through game chat/voice
- black box (flight recorder) with voice recordings

## QoL
- space instead of enter for game advance
- ^W to delete 1 command rule
- black box
- unicode for diagonal airports (↘0 ↗1 ↙2 ↖3)
- ascii real plane crashing animation

gameplay changing:
- allow altitude/marking to be a comm

## extended game mode
- if low on fuel, disallow to exit the arena, must refuel at any airport
- dont stop when a plane runs out of fuel, make it descend, decrease max turn angle to 45
- allow to land at a different airport then a destination e. g. to refuel
- implement (day to day) scoring and penalties for running out of fuel etc. and
  maybe a whole ass story mode. penalties:
  - no penalty if caused by random event
  - ran out of fuel (emergency landing)
  - delaying flights too much
- geographical features e. g. mountains, water, etc, that maybe affect flight
  path and maybe allow for different emergency landing scenarios
- weather: idk
- type of plane with max turn = 1
- choose not a random endpoint from all airports and exits but make some
  endpoints/routes more popular
- detailed plane view (fuel, direction, etc.)
- multiple stacking comms
