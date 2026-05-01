module;
#include <raylib.h>

#include <entt/entt.hpp>
#include <nlohmann/json.hpp>
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
  RenderTextureResourceLoader(const RenderTextureResourceLoader &) = delete;
  RenderTextureResourceLoader &operator=(const RenderTextureResourceLoader &) = delete;
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

export void unload_resource_manager(entt::registry &registry) {
  auto &[textures, models, images, shaders, music_streams, sounds, render_textures] = get_resource_manager(registry);
  models.clear();
  images.clear();
  textures.clear();
  shaders.clear();
  music_streams.clear();
  sounds.clear();
  render_textures.clear();
  registry.ctx().erase<ResourceManager>();
}

export namespace resources {

void load_resource(entt::registry &registry, const nlohmann::json &res) {
  auto &rm = get_resource_manager(registry);
  const std::string type = res["type"].get<std::string>();
  const std::string name = res["name"].get<std::string>();
  const std::string path = res["path"].get<std::string>();
  const auto res_id = entt::hashed_string(name.data());
  TraceLog(LOG_DEBUG, TextFormat("preloading resource '%s' of type '%s' from path '%s'", name.c_str(), type.c_str(), path.c_str()));

  // todo add support for shaders
  if (type == "texture") {
    rm.textures.load(res_id, path);
  } else if (type == "model") {
    rm.models.load(res_id, path);
  } else if (type == "image") {
    rm.images.load(res_id, path);
  } else if (type == "fragment") {
    rm.shaders.load(res_id, path);
  } else if (type == "music") {
    rm.music_streams.load(res_id, path);
  } else if (type == "sound") {
    rm.sounds.load(res_id, path);
  } else {
    TraceLog(LOG_WARNING, TextFormat("Unknown resource type '%s' for resource '%s'", type.c_str(), name.c_str()));
  }
}

}  // namespace resources