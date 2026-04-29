module;
#include <condition_variable>
#include <filesystem>
#include <format>
#include <future>
#include <map>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

// #define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"
#include "../lib/ray.hpp"

export module TileDownloader;

// ---------------------------------------------------------------------------
// URL builders — one per tile type, matching download_tile.mjs logic
// ---------------------------------------------------------------------------

export namespace tile_downloader {

std::string texture_url(const int zoom, const int x, const int z, const std::string& token) {
  return std::format("https://api.mapbox.com/v4/mapbox.satellite/{}/{}/{}.png?access_token={}", zoom, x, z, token);
}

std::string heightmap_url(const int zoom, const int x, const int z, const std::string& token) {
  return std::format("https://api.mapbox.com/v4/mapbox.terrain-rgb/{}/{}/{}.pngraw?access_token={}", zoom, x, z, token);
}

std::string map_url(const int zoom, const int x, const int z, const std::string& token) {
  return std::format("https://api.tomtom.com/map/1/tile/basic/main/{}/{}/{}.png?tileSize=256&view=Unified&language=NGT&key={}", zoom, x, z, token);
}

// ---------------------------------------------------------------------------
// Thread pool with deduplication
// ---------------------------------------------------------------------------

class pool {
  struct Job {
    std::string               path;
    std::string               url;
    std::promise<void>        promise;
  };

  std::vector<std::thread>          workers;
  std::queue<Job>                   queue;
  std::map<std::string, std::shared_future<void>> in_flight; // path → future
  std::mutex                        mtx;
  std::condition_variable           cv;
  bool                              stop = false;

  void worker_loop() {
    while (true) {
      Job job;
      {
        std::unique_lock lock(mtx);
        cv.wait(lock, [this] { return stop || !queue.empty(); });
        if (stop && queue.empty()) return;
        job = std::move(queue.front());
        queue.pop();
      }
      download(job.path, job.url);
      {
        std::lock_guard lock(mtx);
        in_flight.erase(job.path);
      }
      job.promise.set_value();
    }
  }

  static void download(const std::string& path, const std::string& url) {
    if (std::filesystem::exists(path)) return;

    // Parse host and target from url
    // url format: https://host/path?query
    const std::string https = "https://";
    const auto host_start = https.size();
    const auto slash = url.find('/', host_start);
    const std::string host   = url.substr(host_start, slash - host_start);
    const std::string target = url.substr(slash);

    httplib::SSLClient cli(host);
    cli.set_follow_location(true);
    cli.set_connection_timeout(10);
    cli.set_read_timeout(30);
    cli.enable_server_certificate_verification(false);

    const auto res = cli.Get(target);
    if (!res || res->status != 200) {
      TraceLog(LOG_WARNING, "tile download failed: %s status=%d err=%s",
               path.c_str(), res ? res->status : -1,
               httplib::to_string(res.error()).c_str());
      return;
    }

    std::filesystem::create_directories(std::filesystem::path(path).parent_path());
    std::FILE* f = std::fopen(path.c_str(), "wb");
    if (!f) { TraceLog(LOG_WARNING, "tile fopen failed: %s", path.c_str()); return; }
    std::fwrite(res->body.data(), 1, res->body.size(), f);
    std::fclose(f);
    TraceLog(LOG_DEBUG, "tile downloaded: %s", path.c_str());
  }

 public:
  explicit pool(const int thread_count = 4) {
    workers.reserve(thread_count);
    for (int i = 0; i < thread_count; ++i)
      workers.emplace_back([this] { worker_loop(); });
  }

  ~pool() {
    { std::lock_guard lock(mtx); stop = true; }
    cv.notify_all();
    for (auto& t : workers) t.join();
  }

  // Returns a shared_future that resolves when the file is on disk.
  // If the same path is already queued or downloading, returns the existing future.
  std::shared_future<void> enqueue(const std::string& path, const std::string& url) {
    std::lock_guard lock(mtx);
    if (const auto it = in_flight.find(path); it != in_flight.end())
      return it->second;

    std::promise<void> promise;
    std::shared_future<void> future = promise.get_future().share();
    in_flight[path] = future;
    queue.push({path, url, std::move(promise)});
    cv.notify_one();
    return future;
  }
};

// ---------------------------------------------------------------------------
// Global singleton — shared by all streamers
// ---------------------------------------------------------------------------

pool& get() {
  static pool instance(4);
  return instance;
}

std::shared_future<void> enqueue(const std::string& path, const std::string& url) {
  return get().enqueue(path, url);
}

}  // namespace tile_downloader