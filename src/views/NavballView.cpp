#include "NavballView.h"
#include <string_view>
#include "raymath.h"
#include "rlgl.h"
#include "../primitives/Resource.h"

NavballView::NavballView(const AppConfig &config) : tex(LoadTexture(
                                                        config.config["views"]["navballTexture"].get<std::string_view>().data()
                                                    )),
                                                    sphere(GenMeshSphere(
                                                        config.config["views"]["navballRadius"].get<float>(),
                                                        config.config["views"]["navballRings"].get<int>(),
                                                        config.config["views"]["navballRings"].get<int>()
                                                    )),
                                                    navball(LoadModelFromMesh(sphere)),
                                                    vtex(LoadRenderTexture(200, 200)) {
    SetMaterialTexture(&navball.materials[0], MATERIAL_MAP_DIFFUSE, tex);
    // navball.transform = MatrixIdentity();
    // navball.transform = MatrixMultiply(MatrixRotateX(90.0f * DEG2RAD), MatrixRotateX(40 * DEG2RAD));
    // navball.transform = MatrixRotateX(90.0f * DEG2RAD);

    // virtual studio
    vcam.position = {0.0f, 0.0f, 10.0f};
    vcam.target = {0.0f, 0.0f, 0.0f};
    vcam.up = GamePhysics::WorldUp;
    vcam.fovy = 60.0f;
    vcam.projection = CAMERA_PERSPECTIVE;
}

void NavballView::draw(const AircraftState &state) {
    navball.transform = MatrixMultiply(
        MatrixRotateX(90.0f * DEG2RAD),
        QuaternionToMatrix(state.orientation.rotation)
    );
    BeginTextureMode(vtex);
    ClearBackground(BLANK);
    BeginMode3D(vcam);
    DrawModel(navball, {0.0f, 0.0f, 0.0f}, 10.0f, WHITE);
    EndMode3D();
    EndTextureMode();
    Rectangle src = {0.0f, 0.0f, static_cast<float>(vtex.texture.width), -static_cast<float>(vtex.texture.height)};
    Vector2 positionOnScreen = {800.0f, 600.0f};
    DrawTextureRec(vtex.texture, src, positionOnScreen, WHITE);


    // rlDisableDepthTest();
    // rlPushMatrix();
    // auto pos = Vector3Add(state.position, Vector3Scale(state.orientation.forward, 2.0f));
    // pos = Vector3Add(pos, Vector3Scale(state.orientation.up, -2.2f));
    // pos = Vector3Add(pos, Vector3Scale(state.orientation.right, 2.0f));
    // pos = Vector3Add(pos, Vector3Scale(directions.right, 5.0f));

    // rlEnableDepthTest();
}
