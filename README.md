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
- 100 platform agnostic game logic
- 090 ncurses display
- 070 platform agnostic server logic
- 070 pc server port
- 060 esp client & server port
- 020 ci/cd (docs on gh pages, maybe something else for esp32 later)
- 010 better docs generation (readthedocs, man page output)

# ideas
## esp32

## multiplayer

## extended game mode
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
