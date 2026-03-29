#include "Aircraft.h"

Aircraft::Aircraft(const std::string_view name,
                   const std::string_view modelPath) : name(name),
                                                       model(LoadModel(modelPath.data())) {
}
