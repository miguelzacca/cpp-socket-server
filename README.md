# C++ Socket Server

Simple socket server with C++, Windows/Linux code.

## Compilation

```
g++ -o main main.cpp
```

## Linux to Windows Compilation

```
x86_64-w64-mingw32-g++ -o main main.cpp -static-libstdc++ -static-libgcc -mwindows -lws2_32
```
