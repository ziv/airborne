#pragma once
#include <map>
#include <string>


struct Payload {
    std::map<std::string, std::map<std::string, std::string>> hardpoints;
};
