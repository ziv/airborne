module;
#include <entt/entt.hpp>
export module Resources;

export namespace resources {
constexpr auto config_path = "assets/config.jsonc";
constexpr auto scenario_path = "assets/scenario.json";
constexpr auto options_path = "assets/options.json";

constexpr auto sky_vertex_shader_path = "assets/shaders/sky.vs";
constexpr auto sky_fragment_shader_path = "assets/shaders/sky.fs";

constexpr auto engine_sound = entt::hashed_string("engine-sound");
constexpr auto gear_sound = entt::hashed_string("gear-sound");

int fog_shader_pos_loc = -99;
constexpr auto fog_shader = entt::hashed_string("fog_shader");
constexpr auto sky_shader = entt::hashed_string("sky_shader");

constexpr auto sky_model = entt::hashed_string("sky_model");

constexpr auto world_model = entt::hashed_string("world_model");
constexpr auto cloud_model = entt::hashed_string("cloud_model");
}  // namespace resources
