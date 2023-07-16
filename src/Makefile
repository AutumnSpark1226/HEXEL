# use this compiler
COMPILER = g++

# use these global compiler flags
# -w suppresses all warnings
# -g used for debug builds
# use c++20
GLOBAL_COMPILER_FLAGS = -std=c++20 -g

# client

# compile this file (and its dependecies)
CLIENT_FILE = ./client/main.cpp
# use these libraries
CLIENT_LINKER_FLAGS = -lSDL2 -lSDL2_image -lSDL2_ttf
# executable name
CLIENT_BINARY_NAME = HEXEL.out
# use these additional compile flags
CLIENT_COMPILER_FLAGS = # currently not required

#server

# compile this file (and its dependecies)
SERVER_FILE = ./server/main.cpp
# use these libraries
SERVER_LINKER_FLAGS = -lSDL2
# executable name
SERVER_BINARY_NAME = HEXEL_server.out
# use these additional compile flags
SERVER_COMPILER_FLAGS = # currently not required

help:
	@echo \'$(CLIENT_BINARY_NAME)\': compile the client
	@echo \'run\': compile and run the client
	@echo \'run_client\': compile and run the client
	@echo \'$(SERVER_BINARY_NAME)\': compile the server
	@echo \'run_server\': compile and run the server
	@echo \'clean\': delete compiled binaries

# compile the client
$(CLIENT_BINARY_NAME): clean_client
	$(COMPILER) $(CLIENT_FILE) $(GLOBAL_COMPILER_FLAGS) $(CLIENT_COMPILER_FLAGS) $(CLIENT_LINKER_FLAGS) -o $(CLIENT_BINARY_NAME)

# run the client (for easier running using IDE plugins)
run: $(CLIENT_BINARY_NAME)
	./$(CLIENT_BINARY_NAME)

# run the client
run_client: $(CLIENT_BINARY_NAME)
	./$(CLIENT_BINARY_NAME)

# compile the server
$(SERVER_BINARY_NAME): clean_server
	$(COMPILER) $(SERVER_FILE) $(GLOBAL_COMPILER_FLAGS) $(SERVER_COMPILER_FLAGS) $(SERVER_LINKER_FLAGS) -o $(SERVER_BINARY_NAME)

# run the server
run_server: $(SERVER_BINARY_NAME)
	./$(SERVER_BINARY_NAME)

# delete compiled client binary
clean_client:
	-rm ./HEXEL.out

# delete compiled server binary
clean_server:
		-rm ./HEXEL_server.out
