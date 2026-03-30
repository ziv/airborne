#include "AppConfig.h"
#include "Utils.h"

AppConfig::AppConfig() {
    config = LoadJson("app.json");
}
