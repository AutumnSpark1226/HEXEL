# HEXEL
a cool strategy game

# This game is currently work in progress!

## Controls
### Move
- W/A/S/D: move your view
- Mouse wheel up/down: zoom in/out
- Escape: leave a menu or got to quit menu


## Compile
### Requirements

For debian-based operating systems
- SDL2 (libsdl2-dev)
- SDL2_image (libsdl2_image-dev)
- SDL2_ttf (libsd2_ttf-dev)
- make (make)
- g++ (g++)

Any other linux distro: Look for similar packages.  

Windows: currently not tested  

### Compile
Server
- Compile the server: 'make HEXEL_server.out'
- Run the server: './HEXEL_server.out'
OR  
- Compile and run: 'make run_server'
Client
- Compile the client: 'make HEXEL.out'
- Run the client: './HEXEL.out'
OR  
- Compile and run: 'make run'
