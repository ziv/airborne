#include "MinihudView.h"
#include "raymath.h"


void MinihudView::draw(const Camera &camera, const Directions &directions, const Quaternion &quaternion) {
    // Quaternion navballRotation = QuaternionInvert(quaternion);
    horizonModel.transform = MatrixIdentity(); // QuaternionToMatrix(navballRotation);
    horizonModel.transform = MatrixRotateX(90.0f * DEG2RAD);
    Vector3 spherePosition = Vector3Add(camera.position, Vector3Scale(directions.forward, 5.0f));
    DrawModel(horizonModel, spherePosition, 1.0f, WHITE);
}
