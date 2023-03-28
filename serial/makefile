# C Makefile
# - Author:		André Nascimento
# - Github:		Arckenimuz
# - Email:		andreffnascimento@outlook.com
# - Version:	1.2

# Executable properties
APP_NAME	:= CPD - Traveling Salesperson Problem
EXE_NAME	:= tsp
MACROS 		?= #-D__DEBUG__



# Directory Paths
PATH_MAKE		:= $(shell realpath $(firstword $(MAKEFILE_LIST)))
ROOT_DIR_TEMP	:= $(shell echo ${PATH_MAKE} | sed -n "s:^\(.*\)makefile$$:\1:p")
ROOT_DIR		:= $(shell echo ${ROOT_DIR_TEMP} | sed -e "s:[ ]:\\\\ :g")
DIR_SRC 		:= src/
DIR_BIN 		:= bin/

# Compiler flags
CC   	  	:= gcc
LD   	 	:= gcc
CCFLAGS		:= -g -Wall -Wextra -pedantic -std=gnu99 -MD -O3 -fopenmp
LDFLAGS		:= -lm
INCLUDES	:= -I$(ROOT_DIR)$(DIR_SRC)

# Source Objects
FILES_SRC	:= $(shell find $(DIR_SRC) -type f -name "*.c")
FILES_OBJ	:= $(patsubst $(DIR_SRC)%.c, $(DIR_BIN)%.o, $(FILES_SRC))



# Make Actions
.PHONY: clean compile build rebuild
.DEFAULT_GOAL := build

clean:
	@ rm -rf $(DIR_BIN)
	@ rm  -f $(EXE_NAME)

compile: $(FILES_OBJ)

build: $(EXE_NAME)

rebuild: clean build




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