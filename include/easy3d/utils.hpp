#pragma once

#include <GL/glew.h>
#include <GL/glu.h>
#include <cmath>

namespace easy3d
{

    // Project a 3D point to 2D screen coordinates
    bool projectToScreen(const float obj[3], int &x, int &y, int width,
                         int height)
    {
        GLdouble model[16], proj[16];
        GLint view[4];
        glGetDoublev(GL_MODELVIEW_MATRIX, model);
        glGetDoublev(GL_PROJECTION_MATRIX, proj);
        glGetIntegerv(GL_VIEWPORT, view);
        GLdouble winX, winY, winZ;
        if (!gluProject(obj[0], obj[1], obj[2], model, proj, view, &winX, &winY,
                        &winZ))
            return false;
        x = static_cast<int>(winX);
        y = height - static_cast<int>(winY); // OpenGL origin is bottom-left
        return true;
    }

    // Simple camera struct for navigation
    struct Camera
    {
        float distance = 100.0f;
        float azimuth = 0.0f;   // horizontal angle (radians)
        float elevation = 0.0f; // vertical angle (radians)
        float panX = 0.0f, panY = 0.0f;
    };

    // Helper: set up a basic modelview matrix (no shaders, fixed pipeline)
    void setCamera(const Camera &cam)
    {
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        // Camera position in spherical coordinates
        float x = cam.distance * cosf(cam.elevation) * sinf(cam.azimuth);
        float y = cam.distance * sinf(cam.elevation);
        float z = cam.distance * cosf(cam.elevation) * cosf(cam.azimuth);
        gluLookAt(x + cam.panX, y + cam.panY, z, cam.panX, cam.panY, 0.0f, 0.0f, 1.0f,
                  0.0f);
    }

    void resetCamera(Camera &cam)
    {
        cam.distance = 100.0f;
        cam.azimuth = 0.0f;
        cam.elevation = 0.0f;
        cam.panX = 0.0f;
        cam.panY = 0.0f;
    }

    void setOrthoProjection(int width, int height)
    {
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        float aspect = static_cast<float>(width) / static_cast<float>(height);
        if (aspect >= 1.0f)
        {
            glOrtho(-50.0f * aspect, 50.0f * aspect, -50.0f, 50.0f, 0.1f, 1000.0f);
        }
        else
        {
            glOrtho(-50.0f, 50.0f, -50.0f / aspect, 50.0f / aspect, 0.1f, 1000.0f);
        }
        glMatrixMode(GL_MODELVIEW);
    }

    void resetProjection(int width, int height)
    {
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        float aspect = static_cast<float>(width) / static_cast<float>(height);
        gluPerspective(45.0f, aspect, 0.1f, 1000.0f);
        glMatrixMode(GL_MODELVIEW);
    }

} // namespace easy3d