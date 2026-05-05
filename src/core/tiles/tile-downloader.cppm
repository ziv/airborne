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
#include <optional>
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

std::string openstreet_url(const int zoom, const int x, const int z, const std::string& token) {
  return std::format("https://tile.openstreetmap.org/{}/{}/{}.png", zoom, x, z);
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

  // The pool owns every SSLClient — workers only hold non-owning pointers in
  // their per-thread maps. This guarantees clients outlive workers regardless
  // of which side teardown happens on first.
  std::mutex clients_mtx;
  std::vector<std::unique_ptr<httplib::SSLClient>> all_clients;

  httplib::SSLClient* create_client(const std::string& host) {
    auto cli = std::make_unique<httplib::SSLClient>(host);
    cli->set_follow_location(true);
    cli->set_connection_timeout(10);
    cli->set_read_timeout(5);
    cli->set_keep_alive(true);
#ifdef __APPLE__
    cli->enable_server_certificate_verification(false);
#endif
    auto* ref = cli.get();
    std::lock_guard l(clients_mtx);
    all_clients.push_back(std::move(cli));
    return ref;
  }

  void worker_loop(const std::stop_token& st) {
    // per-worker host -> client map. non-owning; the pool owns the clients.
    std::unordered_map<std::string, httplib::SSLClient*> clients;

    while (true) {
      ImageJob img_job;
      {
        std::unique_lock lock(mtx);
        if (!cv.wait(lock, st, [this] { return !image_queue.empty(); })) return;
        img_job = std::move(image_queue.front());
        image_queue.pop();
      }

      try {
        Image img;
        if (auto body = fetch(*this, clients, img_job.path, img_job.url); body) {
          // decode the freshly downloaded bytes directly, skipping a disk round-trip.
          img = LoadImageFromMemory(".png", reinterpret_cast<const unsigned char*>(body->data()), static_cast<int>(body->size()));
          write_atomic(img_job.path, *body);
        } else {
          // tile already cached on disk.
          img = LoadImage(img_job.path.c_str());
        }
        if (img.data == nullptr) throw std::runtime_error("image decode returned empty image: " + img_job.path);
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

  static httplib::SSLClient& get_client(pool& self, std::unordered_map<std::string, httplib::SSLClient*>& clients, const std::string& host) {
    if (const auto it = clients.find(host); it != clients.end()) return *it->second;

    httplib::SSLClient* cli = self.create_client(host);
    clients.emplace(host, cli);
    return *cli;
  }

  // returns the response body if a network fetch happened, or nullopt if the file
  // was already cached on disk. throws on HTTP failure.
  static std::optional<std::string> fetch(pool& self, std::unordered_map<std::string, httplib::SSLClient*>& clients, const std::string& path, const std::string& url) {
    if (std::filesystem::exists(path)) return std::nullopt;

    // Parse host and target from url
    //  format: https://host/path?query
    const std::string https = "https://";
    const auto host_start = https.size();
    const auto slash = url.find('/', host_start);
    const std::string host = url.substr(host_start, slash - host_start);
    const std::string target = url.substr(slash);

    auto& cli = get_client(self, clients, host);
    auto res = cli.Get(target);
    if (!res || res->status != 200) {
      const int status = res ? res->status : -1;
      const std::string err = res ? std::string{} : httplib::to_string(res.error());
      TraceLog(LOG_WARNING, "tile download failed: %s status=%d err=%s", path.c_str(), status, err.c_str());
      throw std::runtime_error(std::format("download failed: {} status={} err={}", path, status, err));
    }
    TraceLog(LOG_DEBUG, "tile downloaded: %s", path.c_str());
    return std::move(res->body);
  }

  static void write_atomic(const std::string& path, const std::string& bytes) {
    std::filesystem::create_directories(std::filesystem::path(path).parent_path());
    const std::string tmp_path = path + ".tmp";
    std::FILE* f = std::fopen(tmp_path.c_str(), "wb");
    if (!f) {
      TraceLog(LOG_WARNING, "tile fopen failed: %s", tmp_path.c_str());
      throw std::runtime_error("fopen failed: " + tmp_path);
    }
    std::fwrite(bytes.data(), 1, bytes.size(), f);
    std::fclose(f);
    std::error_code ec;
    std::filesystem::rename(tmp_path, path, ec);
    if (ec) {
      TraceLog(LOG_WARNING, "tile rename failed: %s -> %s (%s)", tmp_path.c_str(), path.c_str(), ec.message().c_str());
      throw std::runtime_error("rename failed: " + path);
    }
  }

 public:
  explicit pool(const int thread_count = 4) {
    workers.reserve(thread_count);
    for (int i = 0; i < thread_count; ++i) workers.emplace_back([this](const std::stop_token& st) { worker_loop(st); });
  }

  // Tear down in this exact order:
  //   1. Abort in-flight HTTP so any worker blocked inside Get() returns now.
  //   2. Request stop on the workers (so threads idle in cv.wait exit) and join.
  //   3. Destroy the SSLClients — only safe once we know no worker can touch them.
  // Doing (2) before (1) would race: a worker waking from cv.wait would unblock
  // with no work and exit; if its client were worker-owned it would be freed
  // while we're about to call stop() on it. With the pool owning clients we
  // dodge that, but we still order (1) before (2) so blocked Gets are aborted
  // before we wait on join.
  ~pool() {
    {
      std::lock_guard l(clients_mtx);
      for (auto& c : all_clients) c->stop();
    }
    for (auto& w : workers) w.request_stop();
    workers.clear();
    {
      std::lock_guard l(clients_mtx);
      all_clients.clear();
    }
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

// Global singleton lifetime is managed explicitly: callers must invoke
// shutdown() before process exit (e.g. just before raylib's CloseWindow), so
// that OpenSSL is still alive when the pool aborts in-flight requests. Relying
// on a magic-static destructor races with libssl's own static teardown and
// will crash inside ssl3_shutdown.
namespace detail {
inline std::mutex& instance_mutex() {
  static std::mutex m;
  return m;
}
inline std::unique_ptr<pool>& instance_ptr() {
  static std::unique_ptr<pool> p;
  return p;
}
inline pool& get_instance() {
  std::lock_guard l(instance_mutex());
  auto& p = instance_ptr();
  if (!p) p = std::make_unique<pool>(4);
  return *p;
}
}  // namespace detail

std::shared_future<Image> enqueue_and_load(const std::string& path, const std::string& url) {
  return detail::get_instance().enqueue_and_load(path, url);
}

// Tears down the worker pool. Safe to call multiple times. Call this before
// CloseWindow / process exit while libssl is still loaded.
void shutdown() {
  std::lock_guard l(detail::instance_mutex());
  detail::instance_ptr().reset();
}

}  // namespace tile_downloader