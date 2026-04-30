module;
#include <raylib.h>

#include <entt/entt.hpp>
#include <memory>
#include <string>

export module ResourceManager;

export template <typename T, T (*Loader)(const char *), void (*Unloader)(T)>
struct ResourceLoader {
  T res;

  explicit ResourceLoader(const std::string &path) : res(Loader(path.c_str())) {}

  explicit ResourceLoader(T ready) : res(ready) {}

  ~ResourceLoader() { Unloader(res); }

  ResourceLoader(const ResourceLoader &) = delete;

  ResourceLoader &operator=(const ResourceLoader &) = delete;
};

// its loader is different from the template ones
export struct ShaderLoader {
  Shader res;

  // fragment only shader constructor
  explicit ShaderLoader(const std::string &path) : res(LoadShader(nullptr, path.c_str())) {}

  // vertex + fragment shader constructor
  explicit ShaderLoader(const std::string &v, const std::string &f) : res(LoadShader(v.c_str(), f.c_str())) {}

  explicit ShaderLoader(const Shader ready) : res(ready) {}

  ~ShaderLoader() { UnloadShader(res); }

  ShaderLoader(const ShaderLoader &) = delete;

  ShaderLoader &operator=(const ShaderLoader &) = delete;
};

export {
  using TextureResourceLoader = ResourceLoader<Texture2D, LoadTexture, UnloadTexture>;
  using ModelResourceLoader = ResourceLoader<Model, LoadModel, UnloadModel>;
  using ImageResourceLoader = ResourceLoader<Image, LoadImage, UnloadImage>;
  using MusicStreamResourceLoader = ResourceLoader<Music, LoadMusicStream, UnloadMusicStream>;
  using SoundResourceLoader = ResourceLoader<Sound, LoadSound, UnloadSound>;
}

export struct RenderTextureResourceLoader {
  RenderTexture2D res;
  explicit RenderTextureResourceLoader(RenderTexture2D rt) : res(rt) {}
  ~RenderTextureResourceLoader() { UnloadRenderTexture(res); }
  RenderTextureResourceLoader(const RenderTextureResourceLoader&) = delete;
  RenderTextureResourceLoader& operator=(const RenderTextureResourceLoader&) = delete;
};

export struct ResourceManager {
  entt::resource_cache<TextureResourceLoader> textures;
  entt::resource_cache<ModelResourceLoader> models;
  entt::resource_cache<ImageResourceLoader> images;
  entt::resource_cache<ShaderLoader> shaders;
  entt::resource_cache<MusicStreamResourceLoader> music_streams;
  entt::resource_cache<SoundResourceLoader> sounds;
  entt::resource_cache<RenderTextureResourceLoader> render_textures;
};

constexpr auto MANAGER_ID = entt::hashed_string("ResourceManager");

export void create_resource_manager(entt::registry &registry) { registry.ctx().emplace_as<ResourceManager>(MANAGER_ID); }

export ResourceManager &get_resource_manager(entt::registry &registry) { return registry.ctx().get<ResourceManager>(MANAGER_ID); }
