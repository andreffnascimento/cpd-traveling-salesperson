# C Makefile
# - Author:		André Nascimento
# - Github:		Arckenimuz
# - Email:		andreffnascimento@outlook.com
# - Version:	1.1

# Executable properties
APP_NAME	:= CPD - Traveling Salesperson Problem
EXE_NAME	:= tsp



# Directory Paths
ROOT_DIR	:= $(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))/
DIR_SRC 	:= src/
DIR_BIN 	:= bin/

# Compilation properties
INCLUDES			:= -I$(ROOT_DIR)$(DIR_SRC)
MACROS 		    ?= -D__DEBUG__

# Compiler flags
CC   	  := gcc
LD   	 	:= gcc
CCFLAGS	:= -Wall -std=gnu99 -MD -O3
LDFLAGS	:= -lm -fopenmp

# Source Objects
FILES_SRC	:= $(shell find $(DIR_SRC) -type f -name "*.c")
FILES_OBJ	:= $(patsubst $(DIR_SRC)%.c, $(DIR_BIN)%.o, $(FILES_SRC))



# A phony target is one that is not really the name of a file
# https://www.gnu.org/software/make/manual/html_node/Phony-Targets.html

.PHONY: clean compile build run
.DEFAULT_GOAL := build

clean:
	@ rm -rf $(DIR_BIN)
	@ rm  -f $(EXE_NAME)

compile: $(FILES_OBJ)

build: $(EXE_NAME)



# Build targets
$(DIR_BIN)%.o: $(DIR_SRC)%.c
	@ mkdir -p $(dir $@)
	@ $(CC) $(CCFLAGS) -o $@ -c $< $(MACROS) $(INCLUDES)
	@ echo "\e[32m[Compiled]:\e[0m" $@

$(EXE_NAME): $(FILES_OBJ)
	@ $(LD) $(CCFLAGS) $(LDFLAGS) -o $@ $(FILES_OBJ)
	@ echo "\n\t\e[32m[Build Finished]: \e[0;4;96m"$(APP_NAME)"\e[0m"
	@ echo "\e[2m\t - cc-flags:" $(CCFLAGS) "\e[0m"
	@ echo "\e[2m\t - ld-flags:" $(LDFLAGS) "\e[0m"
	@ echo "\e[2m\t - includes:" $(INCLUDES) "\e[0m"
	@ echo "\e[2m\t - macros:" $(MACROS) "\e[0m\n"

-include $(FILES_OBJ:.o=.d)