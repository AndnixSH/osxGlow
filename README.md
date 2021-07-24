# osxGlow



Description:

A small internal glowhack for CSGO on macOS.



To build:

you may need to install Command Line Tools for Xcode with `xcode-select --install`

download zip or `git clone` the repository

navigate to the source directory with `cd`

on intel-based macs, build with `gcc -std=c++17 -lstdc++ -dynamiclib -Os main.cpp -o libosxGlow.dylib`

on arm-based macs, you can use rosetta2 to build by prepending `arch -x86_64` to the above command



To load:

run `load.command` (requires lldb)



To unload:

run `unload.command` (requires lldb)



Notes:

comments and credits in the code
special thanks to Jin & TheArmKing

todo:
	add debug script



Enjoy!
