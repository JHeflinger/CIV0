# Civilization 0

Civilization 0 is a multiplayer game based on Conway's Game of Life! Users can either host or join
a hosted game server. Each player will get an arsenal of cells to place down, and can place cellular automata
to capture and expand their territory!

## How to Use
Flora itself is split into 2 main components! This makes it easier for developers to branch out and use flora as not just 
an editor, but as an API in their own standalone solutions! 

### Bazel
If you have bazel installed, this will be really easy to compile for yourself! To get started, run the following to clone the repository:
```
git clone --recursive https://github.com/JHeflinger/CIV0.git
```
Once everything is properly cloned, all you have to do is run the build and run script! If you're on windows, it'll look like this:
```
./build.bat
./run.bat
```
If you're on linux, it'll look like this: (Don't forget to chmod it!)
```
./build.sh
./run.sh
```

### Pre-built Binaries
If you don't have bazel or are too lazy to install it, no worries! Check out the respective windows and linux executables 
from the releases!
