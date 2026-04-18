module;
#include <raylib.h>

export module RaylibResource;

/**
 * @brief Move-only RAII wrapper for a raylib resource.
 *
 * @tparam T        The raylib resource type (e.g. Texture2D, Model, Shader).
 * @tparam Unloader Pointer to the raylib unload function for @p T.
 *
 * On destruction the unloader is called automatically. Copy is deleted;
 * move transfers ownership and leaves the source in a non-owning state.
 */
export template<typename T, void (*Unloader)(T)>
class RaylibResource {
    bool has_ownership;

public:
    T res;

    explicit RaylibResource(T loadedResource) : res(loadedResource), has_ownership(true) {
    }

    ~RaylibResource() {
        if (has_ownership) {
            Unloader(res);
        }
    }

    /// @brief Move constructor — transfers ownership from @p other.
    RaylibResource(RaylibResource &&other) noexcept : res(other.res), has_ownership(other.has_ownership) {
        other.has_ownership = false;
    }

    /// @brief Move assignment — releases current resource, then takes ownership.
    RaylibResource &operator=(RaylibResource &&other) noexcept {
        if (this != &other) {
            if (has_ownership) Unloader(res);
            res = other.res;
            has_ownership = other.has_ownership;
            other.has_ownership = false;
        }
        return *this;
    }

    RaylibResource(const RaylibResource &) = delete; ///< Non-copyable.
    RaylibResource &operator=(const RaylibResource &) = delete; ///< Non-copyable.

    /// @brief Implicit conversion to the underlying raylib type.
    operator T() const { return res; }

    /// @brief Member access to the underlying struct (e.g. model->materials).
    T *operator->() { return &res; }
    const T *operator->() const { return &res; }

    // pointer access (will be uncommented if necessary, too dangerous)
    // T *operator&() { return &res; }
};

export
{
    /// Convenience aliases for common raylib resource types.
    using TextureHandle = RaylibResource<Texture2D, UnloadTexture>;
    using RenderTextureHandle = RaylibResource<RenderTexture2D, UnloadRenderTexture>;
    using ImageHandle = RaylibResource<Image, UnloadImage>;
    using ShaderHandle = RaylibResource<Shader, UnloadShader>;
    using ModelHandle = RaylibResource<Model, UnloadModel>;
    using MusicHandle = RaylibResource<Music, UnloadMusicStream>;
    using SoundHandle = RaylibResource<Sound, UnloadSound>;
    using FontHandle = RaylibResource<Font, UnloadFont>;
};
