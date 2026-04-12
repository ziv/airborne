+++
date = '2026-03-22T00:00:00+00:00'
draft = false
title = 'Tech Stack'
cover = "cover/i1.png"
+++

When I started this project, I didn't have a clear idea of the tech stack I would be using. I just knew I wanted to dive into low-level graphics programming and leverage my past experience with C++.

During my research, I discovered [raylib](https://github.com/raysan5/raylib), a simple and easy-to-use graphics library. It simplifies the starting point tremendously and saves me from having to build a rendering engine from scratch.

I explored options like Rust, Go, and Zig, but since raylib is written in C, I ultimately chose C++ to take advantage of its native C compatibility.

Coming from languages with robust package managers, the lack of a standard one in C++ was a bit jarring at first. However, I've gotten used to it, and I honestly don't miss it that much.

I selected CMake as my build system. Aside from raylib, I have only one other dependency: nlohmann/json, which is the de facto standard for parsing JSON in the C++ world.

C++ has evolved significantly, and I wanted to utilize its modern capabilities, so I chose C++20 (Concepts!!!) as the standard for this project. I'm still learning the ropes, but I'm excited to use it. I frequently stumble upon features I didn't know about; it's a great way to learn the language, and having LLMs around to help explain things makes the journey much smoother.