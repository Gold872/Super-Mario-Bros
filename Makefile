SRC_DIR = src

#OBJS specifies which files to compile as part of the project
OBJS = $(wildcard $(SRC_DIR)/*.cpp) $(wildcard $(SRC_DIR)/*/*.cpp)

#CC specifies which compiler we're using
CC = g++

#Specifies the compiler flags/settings to put at the beginning of the command
PRECOMMAND_FLAGS = -std=c++17 -static-libgcc -static-libstdc++ -mwindows

#COMPILER_FLAGS specifies the additional compilation options we're using
# -w suppresses all warnings
COMPILER_FLAGS = -O1 -o

#Location of the SDL2 folder
SDL2_LOCATION = *********ENTER DIRECTORY TO SDL2*********

#Location of the SDL2_image folder
SDL2_IMAGE_LOCATION = *********ENTER DIRECTORY TO SDL2_IMAGE*********

#Location of the SDL2_mixer folder
SDL2_MIXER_LOCATION = *********ENTER DIRECTORY TO SDL2_MIXER*********

#Location of the SDL2_ttf folder
SDL2_TTF_LOCATION = *********ENTER DIRECTORY TO SDL2_TTF*********

#INCLUDE_FLAGS specifies the directories to include in our project
INCLUDE_FLAGS = -Iinclude -I$(SDL2_LOCATION)/include -I$(SDL2_LOCATION)/include/SDL2 -I$(SDL2_IMAGE_LOCATION)/include -I$(SDL2_MIXER_LOCATION)/include -I$(SDL2_TTF_LOCATION)/include

#LIBRARY_SEARCHES specifies the search paths for the libraries
LIBRARY_SEARCHES = -L$(SDL2_LOCATION)/lib -L$(SDL2_IMAGE_LOCATION)/bin -L$(SDL2_MIXER_LOCATION)/bin -L$(SDL2_TTF_LOCATION)/bin

#SDL2_LINKER_FLAGS specifies the SDL2 libraries to link
SDL2_LINKER_FLAGS = -lSDL2main -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf

#LINKER_FLAGS specifies the libraries we're linking against
#**********IF YOU'RE USING MINGW32 ADD "-lmingw32" BEFORE "$(SDL2_LINKER_FLAGS)"**********
LINKER_FLAGS = $(SDL2_LINKER_FLAGS)

#OBJ_NAME specifies the name of our exectuable
OBJ_NAME = "Super Mario Bros"

#Specifies the resource file to use for the executable
RESOURCE_FILES = res/super_mario_bros.res

#This is the target that compiles our executable
all : $(OBJS)
	$(CC) $(PRECOMMAND_FLAGS) $(INCLUDE_FLAGS) $(OBJS) $(COMPILER_FLAGS) $(OBJ_NAME) $(RESOURCE_FILES) $(LIBRARY_SEARCHES) $(LINKER_FLAGS)