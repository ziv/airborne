#include "Session.h"
#include <fstream>
#include "raylib.h"

void Session::read() {
    std::ifstream file(path);
    if (!file.is_open()) {
        TraceLog(LOG_WARNING, TextFormat("[Session::read] No previous session found at: %s. Starting fresh.", path.c_str()));
        data = json::object();
        return;
    }
    try {
        data = json::parse(file, nullptr, true, true);
    } catch (const json::parse_error &e) {
        error(TextFormat("[Session::read] Corrupted session file! JSON parse error: %s", e.what()));
    }
}

void Session::write() {
    std::ofstream file(path);
    if (!file.is_open()) {
        error(TextFormat("[Session::write] Unable to write session to: %s", path.c_str()));
    }
    file << data.dump(2);
}

void Session::error(const std::string &msg) {
    TraceLog(LOG_ERROR, msg.c_str());
    throw std::runtime_error(msg);
}
