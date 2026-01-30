#include "easy3d/render.hpp"

#include <GL/glew.h>
#include <GL/glu.h>
#include <vector>

using namespace manifold;

namespace
{
    GLuint vao = 0, vbo = 0, ebo = 0;
} // anonymous namespace

namespace easy3d::render
{

    void update_view(const MeshGL &mesh)
    {
        // Generate buffers if not already
        if (vao == 0)
            glGenVertexArrays(1, &vao);
        if (vbo == 0)
            glGenBuffers(1, &vbo);
        if (ebo == 0)
            glGenBuffers(1, &ebo);

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * mesh.vertProperties.size(),
                     mesh.vertProperties.data(), GL_STATIC_DRAW);

        // Use mesh.numProp as the stride in case extra per-vertex properties exist
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * mesh.numProp,
                              (void *)0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.triVerts.size() * sizeof(GLuint),
                     mesh.triVerts.data(), GL_STATIC_DRAW);

        // Unbind VAO for safety
        glBindVertexArray(0);
    }

    void draw_scene(const MeshGL &mesh)
    {
        if (vao == 0)
            return; // nothing uploaded yet
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mesh.triVerts.size()), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    void destroy()
    {
        if (vao != 0)
        {
            glDeleteVertexArrays(1, &vao);
            vao = 0;
        }
        if (vbo != 0)
        {
            glDeleteBuffers(1, &vbo);
            vbo = 0;
        }
        if (ebo != 0)
        {
            glDeleteBuffers(1, &ebo);
            ebo = 0;
        }
    }

    void draw_plane_lines()
    {
        // Draw 3D axes (arrows) at the origin
        glDisable(GL_LIGHTING);
        glLineWidth(3.0f);
        glBegin(GL_LINES);
        // X axis (red)
        glColor3f(1.0f, 0.0f, 0.0f);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(30.0f, 0.0f, 0.0f);
        // Y axis (green)
        glColor3f(0.0f, 1.0f, 0.0f);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(0.0f, 30.0f, 0.0f);
        // Z axis (blue)
        glColor3f(0.0f, 0.0f, 1.0f);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(0.0f, 0.0f, 30.0f);
        glEnd();
        // Draw arrow heads
        glPointSize(8.0f);
        glBegin(GL_POINTS);
        glColor3f(1.0f, 0.0f, 0.0f);
        glVertex3f(30.0f, 0.0f, 0.0f);
        glColor3f(0.0f, 1.0f, 0.0f);
        glVertex3f(0.0f, 30.0f, 0.0f);
        glColor3f(0.0f, 0.0f, 1.0f);
        glVertex3f(0.0f, 0.0f, 30.0f);
        glEnd();
        glEnable(GL_LIGHTING);
    }

    void draw_planes_grid(bool showXY, bool showYZ, bool showXZ)
    {
        if (!showXY && !showYZ && !showXZ)
            return;

        const float extent = 200.0f;
        const float step = 5.0f;
        glDisable(GL_LIGHTING);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glLineWidth(1.0f);

        // XY plane (z = 0)
        if (showXY)
        {
            glColor4f(0.6f, 0.6f, 0.6f, 0.4f);
            glBegin(GL_LINES);
            for (float x = -extent; x <= extent; x += step)
            {
                glVertex3f(x, -extent, 0.0f);
                glVertex3f(x, extent, 0.0f);
            }
            for (float y = -extent; y <= extent; y += step)
            {
                glVertex3f(-extent, y, 0.0f);
                glVertex3f(extent, y, 0.0f);
            }
            glEnd();
        }

        // YZ plane (x = 0)
        if (showYZ)
        {
            glColor4f(0.6f, 0.6f, 0.6f, 0.35f);
            glBegin(GL_LINES);
            for (float y = -extent; y <= extent; y += step)
            {
                glVertex3f(0.0f, y, -extent);
                glVertex3f(0.0f, y, extent);
            }
            for (float z = -extent; z <= extent; z += step)
            {
                glVertex3f(0.0f, -extent, z);
                glVertex3f(0.0f, extent, z);
            }
            glEnd();
        }

        // XZ plane (y = 0)
        if (showXZ)
        {
            glColor4f(0.6f, 0.6f, 0.6f, 0.35f);
            glBegin(GL_LINES);
            for (float x = -extent; x <= extent; x += step)
            {
                glVertex3f(x, 0.0f, -extent);
                glVertex3f(x, 0.0f, extent);
            }
            for (float z = -extent; z <= extent; z += step)
            {
                glVertex3f(-extent, 0.0f, z);
                glVertex3f(extent, 0.0f, z);
            }
            glEnd();
        }

        glDisable(GL_BLEND);
        glEnable(GL_LIGHTING);
    }

} // namespace render
