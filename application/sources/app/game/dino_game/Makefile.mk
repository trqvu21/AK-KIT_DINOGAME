CFLAGS		+= -I./sources/app/game/dino_game
CPPFLAGS	+= -I./sources/app/game/dino_game

VPATH += sources/app/game/dino_game

# CPP source files 
# Dino game
SOURCES_CPP += sources/app/game/dino_game/ar_game_dino.cpp
SOURCES_CPP += sources/app/game/dino_game/ar_game_objects.cpp
SOURCES_CPP += sources/app/game/dino_game/ar_game_rf.cpp
SOURCES_CPP += sources/app/game/dino_game/ar_game_world.cpp
SOURCES_CPP += sources/app/game/dino_game/ar_game_background.cpp
