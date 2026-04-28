CFLAGS		+= -I./sources/app/game/archery_game
CPPFLAGS	+= -I./sources/app/game/archery_game

VPATH += sources/app/game/archery_game

# CPP source files 
# Dino game
SOURCES_CPP += sources/app/game/archery_game/ar_game_dino.cpp
SOURCES_CPP += sources/app/game/archery_game/ar_game_objects.cpp
SOURCES_CPP += sources/app/game/archery_game/ar_game_rf.cpp
SOURCES_CPP += sources/app/game/archery_game/ar_game_world.cpp
SOURCES_CPP += sources/app/game/archery_game/ar_game_background.cpp
