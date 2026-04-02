#pragma once
#include "raylib.h"

// RAII template for raylib resources
template<typename T, void (*Unloader)(T)>
class RaylibResource {
    T res;
    bool has_ownership;

public:
    explicit RaylibResource(T loadedResource) : res(loadedResource), has_ownership(true) {
    }

    ~RaylibResource() {
        if (has_ownership) {
            Unloader(res);
        }
    }

    /// @brief move constructor mark remove the ownership from the source
    RaylibResource(RaylibResource &&other) noexcept : res(other.res), has_ownership(other.has_ownership) {
        other.has_ownership = false;
    }

    // move assignment make sure to unload current
    RaylibResource &operator=(RaylibResource &&other) noexcept {
        if (this != &other) {
            if (has_ownership) Unloader(res);
            res = other.res;
            has_ownership = other.has_ownership;
            other.has_ownership = false;
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
using MusicHandle = RaylibResource<Music, UnloadMusicStream>;
using SoundHandle = RaylibResource<Sound, UnloadSound>;
using FontHandle = RaylibResource<Font, UnloadFont>;
