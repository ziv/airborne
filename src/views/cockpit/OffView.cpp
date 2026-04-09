#include "OffView.h"

void OffView::draw(const AircraftState &state, const Vector2 &position) {
    DrawText("OFF", static_cast<int>(position.x) + 60, static_cast<int>(position.y) + 70, 15, DARKGREEN);
}
