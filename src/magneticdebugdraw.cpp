#include "magneticdebugdraw.h"
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/variant/color.hpp>

MagneticDebugDraw::MagneticDebugDraw() {
    // Initialize meshes and materials
    forceMesh = memnew(ImmediateMesh);
    forceMeshInstance = memnew(MeshInstance3D);
    forceMaterial = memnew(StandardMaterial3D);

    influenceMesh = memnew(ImmediateMesh);
    influenceMeshInstance = memnew(MeshInstance3D);
    influenceMaterial = memnew(StandardMaterial3D);
}

void MagneticDebugDraw::_bind_methods() {
    // No properties to bind for now
}

void MagneticDebugDraw::_ready() {
    // Set up force vector visualization
    forceMaterial->set_shading_mode(StandardMaterial3D::SHADING_MODE_UNSHADED);
    forceMaterial->set_transparency(StandardMaterial3D::TRANSPARENCY_ALPHA);
    forceMeshInstance->set_mesh(forceMesh);
    forceMeshInstance->set_material_override(forceMaterial);
    add_child(forceMeshInstance);

    // Set up influence sphere visualization
    influenceMaterial->set_shading_mode(StandardMaterial3D::SHADING_MODE_UNSHADED);
    influenceMaterial->set_transparency(StandardMaterial3D::TRANSPARENCY_ALPHA);
    influenceMeshInstance->set_mesh(influenceMesh);
    influenceMeshInstance->set_material_override(influenceMaterial);
    add_child(influenceMeshInstance);
}

void MagneticDebugDraw::draw_force_vector(const Vector3& start, const Vector3& force, const Color& color) {
    // Scale force for visualization
    const float scale = 0.5f; // Adjust this to make forces more visible
    Vector3 end = start + force * scale;
    
    // Draw line for force vector
    forceMesh->surface_begin(Mesh::PRIMITIVE_LINES);
    forceMesh->surface_set_color(color);
    forceMesh->surface_add_vertex(start);
    forceMesh->surface_add_vertex(end);
    
    // Draw arrow head
    const float arrow_size = 0.1f;
    Vector3 direction = (end - start).normalized();
    Vector3 side1, side2;
    if (Math::abs(direction.y) < 0.99f) {
        side1 = direction.cross(Vector3(0, 1, 0)).normalized();
    } else {
        side1 = direction.cross(Vector3(1, 0, 0)).normalized();
    }
    side2 = direction.cross(side1);
    
    Vector3 arrow_point1 = end - direction * arrow_size + side1 * arrow_size * 0.5f;
    Vector3 arrow_point2 = end - direction * arrow_size - side1 * arrow_size * 0.5f;
    Vector3 arrow_point3 = end - direction * arrow_size + side2 * arrow_size * 0.5f;
    Vector3 arrow_point4 = end - direction * arrow_size - side2 * arrow_size * 0.5f;
    
    forceMesh->surface_add_vertex(end);
    forceMesh->surface_add_vertex(arrow_point1);
    forceMesh->surface_add_vertex(end);
    forceMesh->surface_add_vertex(arrow_point2);
    forceMesh->surface_add_vertex(end);
    forceMesh->surface_add_vertex(arrow_point3);
    forceMesh->surface_add_vertex(end);
    forceMesh->surface_add_vertex(arrow_point4);
    
    forceMesh->surface_end();
}

void MagneticDebugDraw::draw_influence_sphere(const Vector3& center, float radius, const Color& color) {
    const int segments = 32;
    const float step = Math_PI * 2.0f / segments;
    
    // Draw three circles for XY, XZ, and YZ planes
    influenceMesh->surface_begin(Mesh::PRIMITIVE_LINE_STRIP);
    influenceMesh->surface_set_color(color);
    
    // XY plane circle
    for (int i = 0; i <= segments; i++) {
        float angle = i * step;
        Vector3 point(
            radius * Math::cos(angle) + center.x,
            radius * Math::sin(angle) + center.y,
            center.z
        );
        influenceMesh->surface_add_vertex(point);
    }
    
    // XZ plane circle
    for (int i = 0; i <= segments; i++) {
        float angle = i * step;
        Vector3 point(
            radius * Math::cos(angle) + center.x,
            center.y,
            radius * Math::sin(angle) + center.z
        );
        influenceMesh->surface_add_vertex(point);
    }
    
    // YZ plane circle
    for (int i = 0; i <= segments; i++) {
        float angle = i * step;
        Vector3 point(
            center.x,
            radius * Math::cos(angle) + center.y,
            radius * Math::sin(angle) + center.z
        );
        influenceMesh->surface_add_vertex(point);
    }
    
    influenceMesh->surface_end();
}

void MagneticDebugDraw::update_debug_visuals() {
    // Clear previous frame's debug drawings
    forceMesh->clear_surfaces();
    influenceMesh->clear_surfaces();
    
    // Get all magnetic bodies in the scene
    const std::vector<MagneticBody3D*>& magnets = MagneticBody3D::get_magnets_registry();
    
    for (const auto& magnet : magnets) {
        if (!magnet->get_on()) continue;
        
        Vector3 magnet_pos = magnet->get_global_position();
        float influence_radius = Math::sqrt(magnet->get_max_influence_radius_sqr());
        
        // Draw influence sphere
        Color sphere_color;
        switch (magnet->get_magnet_type()) {
            case MagneticBody3D::Permanent:
                sphere_color = Color(0, 0, 1, 0.2f); // Blue
                break;
            case MagneticBody3D::Temporary:
                sphere_color = Color(0, 1, 0, 0.2f); // Green
                break;
            case MagneticBody3D::Electromagnet:
                sphere_color = Color(1, 0, 0, 0.2f); // Red
                break;
        }
        draw_influence_sphere(magnet_pos, influence_radius, sphere_color);
        
        // Draw force vectors between this magnet and others
        for (const auto& other : magnets) {
            if (other == magnet) continue;
            if (!other->get_on()) continue;
            
            if (magnet->will_be_influenced_by(*other)) {
                Vector3 force = magnet->calculate_force_from_magnet(*other);
                draw_force_vector(magnet_pos, force, Color(1, 1, 0, 0.8f)); // Yellow
            }
        }
    }
}

void MagneticDebugDraw::_process(double delta) {
    update_debug_visuals();
}