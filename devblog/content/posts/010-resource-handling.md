+++
date = '2026-03-30T00:00:00+00:00'
draft = false
title = "Resource Micro Management"
cover = "cover/i9.png"
+++

In this post, I'm taking a brief break from graphics to talk about under-the-hood architecture-specifically, how I
handle resources in my game. By "resources," I mean all the external assets the game relies on, such as textures,
models, and sounds.

Raylib provides a straightforward way to load and manage resources, but because it's a C library, you have to manually
unload every single resource when you are done with it. This can be tedious and highly error-prone, leading to memory
leaks if you have a lot of assets to juggle.

In C++, there is a powerful idiom called **RAII** (Resource Acquisition Is Initialization) that solves this perfectly.
The core idea is to wrap a resource in a class so that the language automatically unloads the resource when the object
goes out of scope.

I created a custom template to wrap any Raylib resource type and manage its lifetime. This template doesn't handle the
loading phase; it strictly ensures the resource is properly released when it is no longer needed. This way, I never have
to worry about manually calling unload functions again.

Let's walk through the implementation step-by-step (the full code
is [available here](https://github.com/ziv/airborne/blob/main/src/primitives/Resource.h)):

The template takes two parameters: the type of the resource (e.g., `Texture2D`, `Model`) and a pointer to its specific
unloader function (e.g., `UnloadTexture`, `UnloadModel`). We also need a boolean flag to track whether the instance
actually
owns the resource, which is crucial for safe move semantics.

```c++
template<typename T, void (*Unloader)(T)>
class RaylibResource {
    T res;
    bool has_ownership;
public:
    // ...
```

The constructor is straightforward. It takes a newly loaded resource and initializes the member variables, claiming
ownership.

```c++

explicit RaylibResource(T loadedResource) : 
    res(loadedResource), 
    has_ownership(true) {
}
```

The destructor is where the magic happens. When the object goes out of scope, the destructor checks if it holds
ownership. If it does, it calls the provided Raylib unloader function.

```c++
~RaylibResource() {
    if (has_ownership) {
        Unloader(res);
    }
}
```

To pass resources around safely, we need a **move constructor**. This transfers ownership from one instance to another.
It copies the resource data and then revokes ownership from the original instance to prevent double-free errors.

```c++
RaylibResource(RaylibResource &&other) noexcept : 
    res(other.res), 
    has_ownership(other.has_ownership) {
    other.has_ownership = false;
}
```

Similarly, the **move assignment operator** safely transfers ownership. It first checks for self-assignment, releases
its own current resource if it has one, and then steals the resource and ownership from the other instance.

```c++
RaylibResource &operator=(RaylibResource &&other) noexcept {
    if (this != &other) {
        if (has_ownership) Unloader(res);
        res = other.res;
        has_ownership = other.has_ownership;
        other.has_ownership = false;
    }
    return *this;
}
```

To prevent accidental copying (which would inevitably lead to a double-unload crash), we explicitly delete the copy
constructor and copy assignment operator, making the class non-copyable.

```c++

RaylibResource(const RaylibResource &) = delete;        
RaylibResource &operator=(const RaylibResource &) = delete;  

```

Next, we want this wrapper to seamlessly blend in with normal Raylib functions. We provide an **implicit conversion
operator**, allowing us to pass our class directly into Raylib functions that expect the raw type.

```c++
operator T() const { return res; }
```

Finally, we overload the arrow operator `->`. This allows us to access the inner members of complex resources directly
through the wrapper, which is especially useful for types like `Model` that contain meshes and materials.

```c++
T *operator->() { return &res; }
    const T *operator->() const { return &res; }
};
```

With the template complete, we can now create clean, convenient aliases for all the common Raylib resource types:

```c++
using TextureHandle = RaylibResource<Texture2D, UnloadTexture>;
using ImageHandle = RaylibResource<Image, UnloadImage>;
using ShaderHandle = RaylibResource<Shader, UnloadShader>;
using ModelHandle = RaylibResource<Model, UnloadModel>;
/// ...
```

From now on, whenever I deal with assets in my code, I will use these smart handles instead of the raw Raylib structs.
No more memory leaks!