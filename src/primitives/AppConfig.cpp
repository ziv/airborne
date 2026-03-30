#include "AppConfig.h"
#include "../utils/loaders.h"

AppConfig::AppConfig() {
    config = UtilsLoaders::LoadJson("app.json");
}
