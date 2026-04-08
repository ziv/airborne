+++
date = '2026-04-01T12:00:40+03:00'
draft = false
title = 'Tech Stack'
+++

When I started this project, I didn't have a clear idea of the tech stack I would be using.
I knew I wanted to use low level graphics programming, and I had some experience with C++.

During the research I got to know about the library "[_raylib_](https://github.com/raysan5/raylib)", which is a simple
and easy-to-use library for graphics. Simplify the starting point and save me from reinventing the wheel.

I checked out _Rust_, _Go_ and _Zig_ options, but since I intended to use "raylib" that is written in _C_ I choose _C++_
as the
programming language where C code is natively supported.

Since I come from languages with package managers, it was confusing at first to not have one, but I got used to it and I
don't miss it that much.

Cmake chosen as the build system, and except for the "raylib" library, I have just another library for parsing JSON
files, which is "[nlohmann/json](https://github.com/nlohmann/json)" (the de-factor standard for reading JSON in C++
world).

C++ changes a lot, and I wanted to use the latest features, so I chose C++17 as the standard for this project (there are futures in the standard library !!! 😍)
I'm still learning it, but I'm excited to use it. From time to time, I stumble upon some features that I didn't know about, and it's a great way to learn the language and LLMs are a great help in that.

