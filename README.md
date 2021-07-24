# osxGlow



Description:

A small internal glowhack for CSGO on macOS.



To build:

you may need to install Command Line Tools for Xcode with `xcode-select --install`

download zip or `git clone` the repository

navigate to the source directory with `cd`

build with `gcc -std=c++17 -lstdc++ -dynamiclib -Os main.cpp -o libosxGlow.dylib`



To load:

run `load.command` (requires lldb)

Alternatively, you could use [osxinj](https://github.com/scen/osxinj)



To unload:

run `unload.command` (requires lldb)



Notes:

comments and credits in the code
special thanks to Jin

todo:
	add debug script



Enjoy!
