#ifndef MAGNETIC_DEBUG_DRAW_H
#define MAGNETIC_DEBUG_DRAW_H

#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/immediate_mesh.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/standard_material3d.hpp>
#include "magneticbody3d.h"

class MagneticDebugDraw : public Node3D {
    GDCLASS(MagneticDebugDraw, Node3D)

public:
    MagneticDebugDraw();
    ~MagneticDebugDraw() = default;

    void _ready() override;
    void _process(double delta) override;

protected:
    static void _bind_methods();

private:
    // Drawing helpers
    ImmediateMesh* forceMesh;
    MeshInstance3D* forceMeshInstance;
    StandardMaterial3D* forceMaterial;

    ImmediateMesh* influenceMesh;
    MeshInstance3D* influenceMeshInstance;
    StandardMaterial3D* influenceMaterial;

    void draw_force_vector(const Vector3& start, const Vector3& force, const Color& color);
    void draw_influence_sphere(const Vector3& center, float radius, const Color& color);
    void update_debug_visuals();
};

#endif // MAGNETIC_DEBUG_DRAW_H