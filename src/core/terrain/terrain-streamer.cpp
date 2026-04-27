module;
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <nlohmann/json.hpp>
#include <string>

#include "../../lib/httplib.h"

module TerrainStreaming;

namespace terrain_streamer {
std::string download_file(const std::string& mapbox_url, const std::string& request_path) {
  httplib::Client cli(mapbox_url);
  auto res = cli.Get(request_path);
  if (res && res->status != 200) return "";
  return res->body;
}
}  // namespace terrain_streamer