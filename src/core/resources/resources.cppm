module;
#include <entt/entt.hpp>
export module Resources;

export namespace resources {
constexpr auto engine_sound = entt::hashed_string("engine-sound");
constexpr auto gear_sound = entt::hashed_string("gear-sound");

constexpr auto world_model = entt::hashed_string("world_model");
constexpr auto cloud_model = entt::hashed_string("cloud_model");
}  // namespace resources
