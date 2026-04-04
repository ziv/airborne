#pragma once
#include "../primitives/AppConfig.h"
#include "AircraftStructs.h"

class AircraftControls {
    // configuration
    float pitchRatio;
    float rollRatio;
    float yawRatio;

public:
    explicit AircraftControls(const AppConfig &config);

    /// @brief set controllers state
    void update(AircraftState &state, float dt) const;
};
