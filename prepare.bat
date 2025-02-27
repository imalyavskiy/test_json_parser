@echo off
cd ./submodules/googletest
cmake -G "Visual Studio 15 2017 Win64"
cmake --build .