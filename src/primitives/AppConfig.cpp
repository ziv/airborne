#include "AppConfig.h"
#include "../primitives/Utils.h"

AppConfig::AppConfig() {
    config = UtilsLoaders::LoadJson("app.json");
}
