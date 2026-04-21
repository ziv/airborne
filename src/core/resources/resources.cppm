module;
#include <entt/entt.hpp>
export module Resources;

export namespace resources {
constexpr auto engine_sound = entt::hashed_string("assets/sound/engine.mp3");
constexpr auto gear_sound = entt::hashed_string("assets/sound/gear-short.wav");

constexpr auto world_model = entt::hashed_string("world_model");
constexpr auto cloud_model = entt::hashed_string("cloud_model");
}  // namespace resources
