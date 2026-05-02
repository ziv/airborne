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
#include "../../lib/ray.hpp"
#include "httplib.h"

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

std::string night_url(const int zoom, const int x, const int z, const std::string& token) {
  return std::format("https://api.tomtom.com/map/1/tile/basic/night/{}/{}/{}.png?tileSize=256&view=Unified&language=NGT&key={}", zoom, x, z, token);
}

class pool {
  // struct Job {
  //   std::string path;
  //   std::string url;
  //   std::promise<void> promise;
  // };

  struct ImageJob {
    std::string path;
    std::string url;
    std::promise<Image> promise;
  };

  std::vector<std::thread> workers;
  // std::queue<Job> queue;
  std::queue<ImageJob> image_queue;
  // std::map<std::string, std::shared_future<void>> in_flight;
  std::map<std::string, std::shared_future<Image>> in_flight_images;
  std::mutex mtx;
  std::condition_variable cv;
  bool stop = false;

  void worker_loop() {
    while (true) {
      ImageJob img_job;
      bool has_img_job = false;
      {
        std::unique_lock lock(mtx);
        cv.wait(lock, [this] { return stop || !image_queue.empty(); });
        if (stop && image_queue.empty()) return;
        if (!image_queue.empty()) {
          img_job = std::move(image_queue.front());
          image_queue.pop();
          has_img_job = true;
        }
      }
      if (has_img_job) {
        download(img_job.path, img_job.url);
        // first load, then release
        img_job.promise.set_value(LoadImage(img_job.path.c_str()));
        {
          std::lock_guard lock(mtx);
          in_flight_images.erase(img_job.path);
        }
      }
    }
  }

  static void download(const std::string& path, const std::string& url) {
    if (std::filesystem::exists(path)) return;

    // Parse host and target from url
    // url format: https://host/path?query
    const std::string https = "https://";
    const auto host_start = https.size();
    const auto slash = url.find('/', host_start);
    const std::string host = url.substr(host_start, slash - host_start);
    const std::string target = url.substr(slash);

    httplib::SSLClient cli(host);
    cli.set_follow_location(true);
    cli.set_connection_timeout(10);
    cli.set_read_timeout(5);
    cli.enable_server_certificate_verification(false);

    const auto res = cli.Get(target);
    if (!res || res->status != 200) {
      TraceLog(LOG_WARNING, "tile download failed: %s status=%d err=%s", path.c_str(), res ? res->status : -1, httplib::to_string(res.error()).c_str());
      return;
    }

    std::filesystem::create_directories(std::filesystem::path(path).parent_path());
    std::FILE* f = std::fopen(path.c_str(), "wb");
    if (!f) {
      TraceLog(LOG_WARNING, "tile fopen failed: %s", path.c_str());
      return;
    }
    std::fwrite(res->body.data(), 1, res->body.size(), f);
    std::fclose(f);
    TraceLog(LOG_DEBUG, "tile downloaded: %s", path.c_str());
  }

 public:
  explicit pool(const int thread_count = 4) {
    workers.reserve(thread_count);
    for (int i = 0; i < thread_count; ++i) workers.emplace_back([this] { worker_loop(); });
  }

  ~pool() {
    {
      std::lock_guard lock(mtx);
      stop = true;
    }
    cv.notify_all();
    for (auto& t : workers) t.join();
  }

  // returns a shared_future that resolves with the decoded Image after download.
  // if the same path is already in flight, returns the existing future.
  std::shared_future<Image> enqueue_and_load(const std::string& path, const std::string& url) {
    std::lock_guard lock(mtx);
    if (in_flight_images.contains(path)) return in_flight_images[path];

    // if (const auto it = in_flight_images.find(path); it != in_flight_images.end()) return it->second;
    std::promise<Image> promise;
    std::shared_future<Image> future = promise.get_future().share();
    in_flight_images[path] = future;
    image_queue.push({path, url, std::move(promise)});
    cv.notify_one();
    return future;
  }
};

// pool& get_tile_downloader() {
//   static pool instance(4);
//   return instance;
// }

std::shared_future<Image> enqueue_and_load(const std::string& path, const std::string& url) {
  static pool instance(4);
  return instance.enqueue_and_load(path, url);
}

}  // namespace tile_downloader