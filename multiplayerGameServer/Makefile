# OBJS specifies which files to compile as part of the project
OBJS = main.c

# CC specifies which compiler we're using
CC = gcc

# Headers files
HEADERS = server.c network.c utils.c

# COMPILER_FLAGS specifies the additional compilation options we're using
COMPILER_FLAGS = -std=c11 -Wall

# OBJ_NAME specifies the name of our exectuable
OBJ_NAME = game

# LIB_FLAGS specifies the additional library to link
LIB_FLAGS = -L"C:\Program Files (x86)\Windows Kits\10\Lib\10.0.19041.0\um\x64\WS2_32.lib" -lwsock32 -lpthread

all : $(OBJS)
	$(CC) $(HEADERS) $(OBJS) $(COMPILER_FLAGS) $(LIB_FLAGS) -o $(OBJ_NAME)