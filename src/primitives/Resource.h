#pragma once
#include "raylib.h"

// RAII template for raylib resources
template<typename T, void (*Unloader)(T)>
class RaylibResource {
    T res;

public:
    explicit RaylibResource(T loadedResource) : res(loadedResource) {
    }

    ~RaylibResource() {
        if (res.id != 0) {
            Unloader(res);
        }
    }

    // move constructor mark resource for deletion
    RaylibResource(RaylibResource &&other) noexcept : res(other.res) {
        other.res.id = 0;
    }

    // move assignment make sure to unload current
    RaylibResource &operator=(RaylibResource &&other) noexcept {
        if (this != &other) {
            if (res.id != 0) Unloader(res);
            res = other.res;
            other.res.id = 0;
        }
        return *this;
    }

    // protect against copy
    RaylibResource(const RaylibResource &) = delete;

    // protect against assignment
    RaylibResource &operator=(const RaylibResource &) = delete;

    // assignment operator
    operator T() const { return res; }

    // pointer access (will be uncommented if necessary, too dangerous)
    // T *operator&() { return &res; }
};

using TextureHandle = RaylibResource<Texture2D, UnloadTexture>;
using ShaderHandle = RaylibResource<Shader, UnloadShader>;
using ModelHandle = RaylibResource<Model, UnloadModel>;
