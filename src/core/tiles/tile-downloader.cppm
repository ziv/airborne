module;
#include <condition_variable>
#include <exception>
#include <filesystem>
#include <format>
#include <future>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <stop_token>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include "../../lib/ray.hpp"
#include "httplib.h"

export module TileDownloader;

export namespace tile_downloader {

// todo URLs should come from options/configuration

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
  struct ImageJob {
    std::string path;
    std::string url;
    std::promise<Image> promise;
  };

  std::vector<std::jthread> workers;
  std::queue<ImageJob> image_queue;
  std::map<std::string, std::shared_future<Image>> in_flight_images;
  std::mutex mtx;
  std::condition_variable_any cv;

  // tracks every live SSLClient across all workers so the destructor can abort
  // in-flight HTTP requests. raw pointers are valid until the owning worker's
  // local map destructs — which happens only after the jthread joins.
  std::mutex clients_mtx;
  std::vector<httplib::SSLClient*> all_clients;

  void register_client(httplib::SSLClient* cli) {
    std::lock_guard l(clients_mtx);
    all_clients.push_back(cli);
  }

  void worker_loop(const std::stop_token& st) {
    // per-worker keep-alive HTTP clients, one per host.
    std::unordered_map<std::string, std::unique_ptr<httplib::SSLClient>> clients;

    while (true) {
      ImageJob img_job;
      {
        std::unique_lock lock(mtx);
        if (!cv.wait(lock, st, [this] { return !image_queue.empty(); })) return;
        img_job = std::move(image_queue.front());
        image_queue.pop();
      }

      try {
        download(*this, clients, img_job.path, img_job.url);
        Image img = LoadImage(img_job.path.c_str());
        if (img.data == nullptr) throw std::runtime_error("LoadImage returned empty image: " + img_job.path);
        img_job.promise.set_value(img);
      } catch (...) {
        try {
          img_job.promise.set_exception(std::current_exception());
        } catch (...) {
          // promise already satisfied or broken; nothing else we can do
        }
      }

      {
        std::lock_guard lock(mtx);
        in_flight_images.erase(img_job.path);
      }
    }
  }

  static httplib::SSLClient& get_client(pool& self, std::unordered_map<std::string, std::unique_ptr<httplib::SSLClient>>& clients, const std::string& host) {
    if (const auto it = clients.find(host); it != clients.end()) return *it->second;

    auto cli = std::make_unique<httplib::SSLClient>(host);
    cli->set_follow_location(true);
    cli->set_connection_timeout(10);
    cli->set_read_timeout(5);
    cli->set_keep_alive(true);
#ifdef __APPLE__
    cli->enable_server_certificate_verification(false);
#endif
    const auto [it, _] = clients.emplace(host, std::move(cli));
    httplib::SSLClient& ref = *it->second;
    self.register_client(&ref);
    return ref;
  }

  static void download(pool& self, std::unordered_map<std::string, std::unique_ptr<httplib::SSLClient>>& clients, const std::string& path, const std::string& url) {
    if (std::filesystem::exists(path)) return;

    // Parse host and target from url
    //  format: https://host/path?query
    const std::string https = "https://";
    const auto host_start = https.size();
    const auto slash = url.find('/', host_start);
    const std::string host = url.substr(host_start, slash - host_start);
    const std::string target = url.substr(slash);

    auto& cli = get_client(self, clients, host);
    const auto res = cli.Get(target);
    if (!res || res->status != 200) {
      const int status = res ? res->status : -1;
      const std::string err = res ? std::string{} : httplib::to_string(res.error());
      TraceLog(LOG_WARNING, "tile download failed: %s status=%d err=%s", path.c_str(), status, err.c_str());
      throw std::runtime_error(std::format("download failed: {} status={} err={}", path, status, err));
    }

    std::filesystem::create_directories(std::filesystem::path(path).parent_path());
    const std::string tmp_path = path + ".tmp";
    std::FILE* f = std::fopen(tmp_path.c_str(), "wb");
    if (!f) {
      TraceLog(LOG_WARNING, "tile fopen failed: %s", tmp_path.c_str());
      throw std::runtime_error("fopen failed: " + tmp_path);
    }
    std::fwrite(res->body.data(), 1, res->body.size(), f);
    std::fclose(f);
    std::error_code ec;
    std::filesystem::rename(tmp_path, path, ec);
    if (ec) {
      TraceLog(LOG_WARNING, "tile rename failed: %s -> %s (%s)", tmp_path.c_str(), path.c_str(), ec.message().c_str());
      throw std::runtime_error("rename failed: " + path);
    }
    TraceLog(LOG_DEBUG, "tile downloaded: %s", path.c_str());
  }

 public:
  explicit pool(const int thread_count = 4) {
    workers.reserve(thread_count);
    for (int i = 0; i < thread_count; ++i) workers.emplace_back([this](const std::stop_token& st) { worker_loop(st); });
  }

  // jthread auto-requests stop and joins on destruction; condition_variable_any
  // wakes from wait() when stop is requested. We additionally call stop() on
  // every live HTTP client so any worker currently blocked inside Get() returns
  // immediately instead of waiting on its socket timeout.
  ~pool() {
    for (auto& w : workers) w.request_stop();
    {
      std::lock_guard l(clients_mtx);
      for (auto* c : all_clients) c->stop();
    }
    // Explicitly join workers here, while the rest of the members (mtx, cv,
    // clients_mtx) are still alive — they would otherwise be destroyed first
    // (members destruct in reverse declaration order) and the workers would
    // touch destroyed primitives on their way out.
    workers.clear();
  }

  // returns a shared_future that resolves with the decoded Image after download.
  // if the same path is already in flight, returns the existing future.
  std::shared_future<Image> enqueue_and_load(const std::string& path, const std::string& url) {
    std::lock_guard lock(mtx);
    if (const auto it = in_flight_images.find(path); it != in_flight_images.end()) return it->second;

    std::promise<Image> promise;
    std::shared_future<Image> future = promise.get_future().share();
    in_flight_images.emplace(path, future);
    image_queue.push({path, url, std::move(promise)});
    cv.notify_one();
    return future;
  }
};

std::shared_future<Image> enqueue_and_load(const std::string& path, const std::string& url) {
  static pool instance(4);
  return instance.enqueue_and_load(path, url);
}

}  // namespace tile_downloader