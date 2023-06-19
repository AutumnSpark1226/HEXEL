# HEXEL

# compile these files
FILES = main.cpp

# use this compiler
COMPILER = g++

# use these additional compiler flags
# -w suppresses all warnings (not used for debug builds)
COMPILER_FLAGS = #-w

# link against these libraries
LINKER_FLAGS = -lSDL2 -lSDL2_image

# executable name
BINARY_NAME = test.out

# compile
all : $(FILES)
	$(COMPILER) $(FILES) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(BINARY_NAME)

run:
	$(BINARY_NAME) $(ARGS)
