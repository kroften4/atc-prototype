# air traffic controller multiplayer
inspired by bsd-games atc

# the plan
the plan is to allow multiple flight controllers to play, possibly with a head
controller, and extend the base game/add game mods featuring new mechanics (e.
g. if a plane runs out of fuel, make it descend and still allow it to land) or
random events (like weather idk)

and port it to esp32

# TODO
!sort -r

general
- 100 planes are spawning too often (i think in the original implementation the spawn would fail a lot, and in mine it doesnt; it rerolls every fail instead of forcing)
- 100 handle input (plane controls, advance time)
- 100 display comms, stats
- 090 tests
- 070 platform agnostic server logic
- 070 pc server port
- 060 generate_test_runner.rb does not transfer any symbols other then test_* so i have to put them in a global header
- 060 esp client & server port
- 050 setup .editorconfig .clang and stuff
- 020 ci/cd (docs on gh pages, tests)
- 010 better docs generation (readthedocs, man page output)

bugs
- 100 game is failing with event 9 randomly (maybe when prop planes spawn on odd ticks something makes left_endpoint flag not work)

# ideas
## esp32

## multiplayer

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
