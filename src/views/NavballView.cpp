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
                                                    navball(LoadModelFromMesh(sphere)) {
    SetMaterialTexture(&navball.materials[0], MATERIAL_MAP_DIFFUSE, tex);
    // navball.transform = MatrixIdentity();
    // navball.transform = MatrixMultiply(MatrixRotateX(90.0f * DEG2RAD), MatrixRotateX(40 * DEG2RAD));
    navball.transform = MatrixRotateX(90.0f * DEG2RAD);

}

void NavballView::draw(const Vector3 &position, const Directions &directions) {
    rlDisableDepthTest();
    auto pos = Vector3Add(position, Vector3Scale(directions.forward, 2.0f));
    pos = Vector3Add(pos, Vector3Scale(directions.up, -1.2f));
    // pos = Vector3Add(pos, Vector3Scale(directions.right, 5.0f));
    DrawModel(navball, pos, 1.0f, WHITE);
    rlEnableDepthTest();
}
