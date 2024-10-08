# JoltWrapper
Little C wrapper to make Jolt Physics C library compatible with Love 2D

## Overview

[Jolt Physics](https://github.com/jrouwe/JoltPhysics) is a physics engine and [JoltC](https://github.com/amerkoleci/joltc) a C version of the API which allows to be used in [Love](https://github.com/love2d/love) via the [FFI library](https://luajit.org/ext_ffi.html). 

Because of **LuaJit**, callbacks cannot be triggered from other threads. Since Jolt does it, it creates panic errors. This little wrapper library has been made to workaround this problem.

## How it works

The wrapper focus on handling `OnContactAdded` and `OnContactRemoved` callbacks by storing the arguments in arrays to be accessed later. The library use a mutex to lock and unlock the array (one per event) to prevent several threads from editing it at the same time. The mutex is handled via the Unix `pthread` library.

The library use a fixed array size to store the events. A counter has to be reset once the events have been consumed.

## How to build

Refer to the file `Wrapper/build.py`.

## How to use

Refer to the file `Lua/demo.lua`.

## Notes

- The code provided (mainly the lua and python files) are not plug and play and more here as guidelines/context.
- The `pthread` library doesn't exist on Windows. Either compile with `mingw` [on Windows](https://winlibs.com/) (and include `libwinpthread-1.dll`), or use [something like this Cross platform thread wrapper](https://nachtimwald.com/2019/04/05/cross-platform-thread-wrapper/).
- This only handle the generic ContactListener, something similar for Character listener would need to be done as well.
- Jolt ensure that all callbacks are trigerred once the Update() call is done, hence why the event arrays are accessed after on Lua side.
