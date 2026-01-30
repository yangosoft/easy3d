#pragma once

#include "manifold/meshIO.h"
#include <GL/glew.h>

namespace easy3d::render
{

    void update_view(const manifold::MeshGL &mesh);
    void draw_plane_lines();
    void draw_planes_grid(bool showXY, bool showYZ, bool showXZ);
    void draw_scene(const manifold::MeshGL &mesh);
    void destroy();

} // namespace render