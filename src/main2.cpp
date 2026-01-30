#include "main.hpp"
#include "sengi/sengi.hpp"

#include <GL/glew.h>

#include <array>
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_sdl2.h>
#include <functional>
#include <imgui.h>

#include "easy3d.hpp"
#include "manifold/meshIO.h"
#include <cmath>
#include <iostream>
#include <map>
#include <memory>
#include <vector>

using namespace manifold;

std::map<std::shared_ptr<sengi::Node>, std::shared_ptr<Manifold>> node_map;

sengi::Scene scene;

std::vector<std::shared_ptr<Manifold>> models;
MeshGL sceneMesh;
// Scene dirty flag: set to true when models vector changes
bool sceneDirty = false;
void update_view(MeshGL &mesh);

// Combine all models and upload to OpenGL
void update_scene_mesh()
{
    if (models.empty())
        return;
    Manifold combined = *models[0];
    for (size_t i = 1; i < models.size(); ++i)
    {
        combined += *models[i];
    }
    sceneMesh = combined.GetMeshGL();
    update_view(sceneMesh);
}

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

// Callback type for mesh click: receives model index
using MeshClickCallback = std::function<void(int modelIndex)>;

// Check if any mesh in models was clicked, given mouse position and camera
void check_sphere_click(int mouseX, int mouseY, int winW, int winH,
                        const MeshClickCallback &cb)
{
    constexpr float radius = 8.0f; // pixels, match glPointSize
    for (size_t mi = 0; mi < models.size(); ++mi)
    {
        const auto &m = models[mi];
        auto mesh = m->GetMeshGL();
        // Use vertPos if available, else vertProperties
        const float *verts = nullptr;
        size_t nVerts = 0;

        if (!mesh.vertProperties.empty())
        {
            verts = reinterpret_cast<const float *>(mesh.vertProperties.data());
            nVerts = mesh.vertProperties.size() / 3;
        }
        else if (!mesh.vertProperties.empty())
        {
            verts = mesh.vertProperties.data();
            nVerts = mesh.vertProperties.size() / 3;
        }
        for (size_t vi = 0; vi < nVerts; ++vi)
        {
            int sx, sy;
            if (projectToScreen(&verts[vi * 3], sx, sy, winW, winH))
            {
                float dx = mouseX - sx;
                float dy = mouseY - sy;
                if (dx * dx + dy * dy < radius * radius)
                {
                    cb(static_cast<int>(mi));
                    return;
                }
            }
        }
    }
    // Still allow axis spheres for navigation
    std::array<std::array<float, 3>, 3> sphere_pos = {
        {{30.0f, 0.0f, 0.0f}, {0.0f, 30.0f, 0.0f}, {0.0f, 0.0f, 30.0f}}};
    for (int i = 0; i < 3; ++i)
    {
        int sx, sy;
        if (projectToScreen(sphere_pos[i].data(), sx, sy, winW, winH))
        {
            float dx = mouseX - sx;
            float dy = mouseY - sy;
            if (dx * dx + dy * dy < radius * radius)
            {
                cb(-1 - i); // negative values for axis
                return;
            }
        }
    }
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

GLuint vao, vbo, ebo;

Manifold RoundedFrame(double edgeLength, double radius, int circularSegments)
{
    Manifold edge = Manifold::Cylinder(edgeLength, radius, -1, circularSegments);
    Manifold corner = Manifold::Sphere(radius, circularSegments);

    Manifold edge1 = corner + edge;
    edge1 = edge1.Rotate(-90).Translate({-edgeLength / 2, -edgeLength / 2, 0});

    Manifold edge2 = edge1.Rotate(0, 0, 180);
    edge2 += edge1;
    edge2 += edge.Translate({-edgeLength / 2, -edgeLength / 2, 0});

    Manifold edge4 = edge2.Rotate(0, 0, 90);
    edge4 += edge2;

    Manifold frame = edge4.Translate({0, 0, -edgeLength / 2});
    frame += frame.Rotate(180);

    return frame;
}

void update_view(MeshGL &mesh)
{
    // Upload mesh data to OpenGL (positions only)

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * mesh.vertProperties.size(),
                 mesh.vertProperties.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void *)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.triVerts.size() * sizeof(GLuint),
                 mesh.triVerts.data(), GL_STATIC_DRAW);
}

void handle_events(SDL_Event &event, bool &leftDown, bool &rightDown,
                   int &lastX, int &lastY, Camera &cam, bool &wireframe,
                   int &retFlag);

void draw_help(Camera &cam);

int main(int /*argc*/, char ** /*argv*/)
{
    std::cout << "HELLO\n";

    SDL_Init(SDL_INIT_VIDEO);
    int winW = 800, winH = 600;
    SDL_Window *window = SDL_CreateWindow(
        "Manifold + SDL2 + OpenGL", SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED, winW, winH, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

    SDL_GLContext glContext = SDL_GL_CreateContext(window);
    glewInit();

    init_ui(window, glContext);

    // Example scene: add a frame, a cube, and some spheres
    models.clear();
    models.push_back(std::make_shared<Manifold>(RoundedFrame(20, 5, 120)));
    auto cube = std::make_shared<Manifold>(Manifold::Cube({30, 30, 30}));
    auto rm_cube = Manifold::Cube({25, 25, 50}).Translate({2.5, 2.5, 2.5});
    *cube -= rm_cube;
    models.push_back(cube);
    for (int i = 0; i < 10; i++)
    {
        auto sphere = Manifold::Sphere(2.0, 32);
        sphere = sphere.Translate({(double)(rand() % 200 - 10),
                                   (double)(rand() % 200 - 10),
                                   (double)(rand() % 20 - 10)});
        models.push_back(std::make_shared<Manifold>(sphere));
    }
    // Initial mesh upload
    update_scene_mesh();

    // Example callback for sphere click
    auto on_sphere_click = [](int axis)
    {
        if (axis == 0)
            std::cout << "X axis sphere clicked!\n";
        if (axis == 1)
            std::cout << "Y axis sphere clicked!\n";
        if (axis == 2)
            std::cout << "Z axis sphere clicked!\n";
    };

    // Camera and mouse state
    Camera cam;
    int lastX = 0, lastY = 0;
    bool leftDown = false, rightDown = false;

    bool wireframe = false;
    bool running = true;
    // For deferred sphere click check
    bool pendingSphereClick = false;
    int clickX = 0, clickY = 0;

    while (running)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                running = false;

            int retFlag;
            // Store mouse click for later check
            if (event.type == SDL_MOUSEBUTTONDOWN &&
                event.button.button == SDL_BUTTON_LEFT)
            {
                clickX = event.button.x;
                clickY = event.button.y;
                pendingSphereClick = true;
                std::cout << "Stored click at " << clickX << ", " << clickY << "\n";
            }
            handle_events(event, leftDown, rightDown, lastX, lastY, cam, wireframe,
                          retFlag);
            if (retFlag == 3)
                continue;
        }

        // If scene is dirty, update mesh
        if (sceneDirty)
        {
            update_scene_mesh();
            sceneDirty = false;
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        setCamera(cam);

        // Now matrices are correct, check for sphere click
        if (pendingSphereClick)
        {
            check_sphere_click(clickX, clickY, winW, winH, on_sphere_click);
            pendingSphereClick = false;
        }

        draw_plane_lines();

        glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, sceneMesh.triVerts.size() * 3, GL_UNSIGNED_INT,
                       0);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // reset for UI overlays if any

        // Start the ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        draw_help(cam);
        // Example: Show the ImGui demo window

        // Render ImGui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        SDL_GL_SwapWindow(window);
    }

    /*manifold::ExportMesh("rounded_frame.3mf", rf.GetMeshGL(),
                         manifold::ExportOptions());*/
    // manifold::ExportMesh("box.3mf", cube.GetMeshGL(),
    // manifold::ExportOptions());
    //  Export the model

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

void draw_help(Camera &cam)
{
    // Show navigation help window
    ImGui::Begin("Navigation Help");
    ImGui::Text("3D Navigation Controls:");
    ImGui::Separator();
    ImGui::BulletText("Left Mouse Drag: Orbit");
    ImGui::BulletText("Right Mouse Drag: Pan");
    ImGui::BulletText("Mouse Wheel: Zoom");
    ImGui::BulletText("Arrow Keys: Pan");
    ImGui::BulletText("Shift + Arrow Keys: Orbit");
    ImGui::BulletText("+ / - : Zoom");
    ImGui::BulletText("V: Toggle Wireframe");
    ImGui::BulletText("Click axis spheres: Select axis");
    ImGui::Separator();
    ImGui::Text("Quick Camera Views:");
    if (ImGui::Button("Top"))
    {
        cam.azimuth = 0.0f;
        cam.elevation = 1.5708f;
        cam.distance = 100.0f;
    }
    ImGui::SameLine();
    if (ImGui::Button("Bottom"))
    {
        cam.azimuth = 0.0f;
        cam.elevation = -1.5708f;
        cam.distance = 100.0f;
    }
    ImGui::SameLine();
    if (ImGui::Button("Front"))
    {
        cam.azimuth = 0.0f;
        cam.elevation = 0.0f;
        cam.distance = 100.0f;
    }
    ImGui::SameLine();
    if (ImGui::Button("Back"))
    {
        cam.azimuth = 3.1416f;
        cam.elevation = 0.0f;
        cam.distance = 100.0f;
    }
    if (ImGui::Button("Left"))
    {
        cam.azimuth = -1.5708f;
        cam.elevation = 0.0f;
        cam.distance = 100.0f;
    }
    ImGui::SameLine();
    if (ImGui::Button("Right"))
    {
        cam.azimuth = 1.5708f;
        cam.elevation = 0.0f;
        cam.distance = 100.0f;
    }
    ImGui::SameLine();
    if (ImGui::Button("Iso"))
    {
        cam.azimuth = 0.7854f;
        cam.elevation = 0.6155f;
        cam.distance = 120.0f;
    }

    ImGui::Separator();
    static float cube_size[3] = {10.0f, 10.0f, 10.0f};
    static float cube_pos[3] = {0.0f, 0.0f, 0.0f};
    static float cyl_height = 10.0f, cyl_radius = 5.0f;
    static int cyl_segments = 32;
    static float cyl_pos[3] = {0.0f, 0.0f, 0.0f};

    ImGui::Text("Add Model:");
    ImGui::InputFloat3("Cube Size", cube_size);
    ImGui::InputFloat3("Cube Pos", cube_pos);
    if (ImGui::Button("Add Cube"))
    {
        auto cube = std::make_shared<Manifold>(
            Manifold::Cube({cube_size[0], cube_size[1], cube_size[2]}));
        *cube = cube->Translate({cube_pos[0], cube_pos[1], cube_pos[2]});
        models.push_back(cube);
        sceneDirty = true;
    }
    ImGui::InputFloat("Cyl Height", &cyl_height);
    ImGui::InputFloat("Cyl Radius", &cyl_radius);
    ImGui::InputInt("Cyl Segments", &cyl_segments);
    ImGui::InputFloat3("Cyl Pos", cyl_pos);
    if (ImGui::Button("Add Cylinder"))
    {
        auto cyl = std::make_shared<Manifold>(
            Manifold::Cylinder(cyl_height, cyl_radius, -1, cyl_segments));
        *cyl = cyl->Translate({cyl_pos[0], cyl_pos[1], cyl_pos[2]});
        models.push_back(cyl);
        sceneDirty = true;
    }
    ImGui::End();
}

void handle_events(SDL_Event &event, bool &leftDown, bool &rightDown,
                   int &lastX, int &lastY, Camera &cam, bool &wireframe,
                   int &retFlag)
{
    retFlag = 1;
    // Only handle navigation if ImGui is NOT capturing mouse/keyboard
    ImGuiIO &io = ImGui::GetIO();
    if (io.WantCaptureMouse &&
        (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP ||
         event.type == SDL_MOUSEMOTION || event.type == SDL_MOUSEWHEEL))
    {
        retFlag = 3;
        return;
    };
    if (io.WantCaptureKeyboard && event.type == SDL_KEYDOWN)
    {
        retFlag = 3;
        return;
    };

    if (event.type == SDL_MOUSEBUTTONDOWN)
    {
        if (event.button.button == SDL_BUTTON_LEFT)
            leftDown = true;
        if (event.button.button == SDL_BUTTON_RIGHT)
            rightDown = true;
        lastX = event.button.x;
        lastY = event.button.y;
    }
    else if (event.type == SDL_MOUSEBUTTONUP)
    {
        if (event.button.button == SDL_BUTTON_LEFT)
            leftDown = false;
        if (event.button.button == SDL_BUTTON_RIGHT)
            rightDown = false;
    }
    else if (event.type == SDL_MOUSEMOTION)
    {
        int dx = event.motion.x - lastX;
        int dy = event.motion.y - lastY;
        lastX = event.motion.x;
        lastY = event.motion.y;
        if (leftDown)
        {
            cam.azimuth += dx * 0.01f;
            cam.elevation += dy * 0.01f;
            if (cam.elevation > 1.5f)
                cam.elevation = 1.5f;
            if (cam.elevation < -1.5f)
                cam.elevation = -1.5f;
        }
        else if (rightDown)
        {
            cam.panX += dx * 0.1f;
            cam.panY -= dy * 0.1f;
        }
    }
    else if (event.type == SDL_MOUSEWHEEL)
    {
        cam.distance -= event.wheel.y * 2.0f;
        if (cam.distance < 10.0f)
            cam.distance = 10.0f;
        if (cam.distance > 500.0f)
            cam.distance = 500.0f;
    }
    else if (event.type == SDL_KEYDOWN)
    {
        // Arrow keys: pan, Shift+arrow: rotate, +/-: zoom
        bool shift = (event.key.keysym.mod & KMOD_SHIFT);
        switch (event.key.keysym.sym)
        {
        case SDLK_LEFT:
            if (shift)
                cam.azimuth -= 0.1f;
            else
                cam.panX -= 2.0f;
            break;
        case SDLK_RIGHT:
            if (shift)
                cam.azimuth += 0.1f;
            else
                cam.panX += 2.0f;
            break;
        case SDLK_UP:
            if (shift)
                cam.elevation += 0.1f;
            else
                cam.panY += 2.0f;
            break;
        case SDLK_DOWN:
            if (shift)
                cam.elevation -= 0.1f;
            else
                cam.panY -= 2.0f;
            break;
        case SDLK_EQUALS: // '+' key (usually shift + '=')
        case SDLK_KP_PLUS:
            cam.distance -= 5.0f;
            if (cam.distance < 10.0f)
                cam.distance = 10.0f;
            break;
        case SDLK_MINUS:
        case SDLK_KP_MINUS:
            cam.distance += 5.0f;
            if (cam.distance > 500.0f)
                cam.distance = 500.0f;
            break;
        case SDLK_v:
            wireframe = !wireframe;
            break;
        default:
            break;
        }
    }
}

void init_ui(SDL_Window *window, SDL_GLContext glContext)
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForOpenGL(window, glContext);
    ImGui_ImplOpenGL3_Init("#version 130");

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0); // Main directional light
    glEnable(GL_LIGHT1); // Fill light
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    // Main directional light (white, from above/front/right)
    GLfloat light0_pos[] = {0.7f, 1.0f, 1.0f, 0.0f};
    GLfloat light0_ambient[] = {0.25f, 0.25f, 0.25f, 1.0f};
    GLfloat light0_diffuse[] = {0.9f, 0.9f, 0.9f, 1.0f};
    GLfloat light0_specular[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, light0_pos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light0_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light0_specular);

    // Fill light (soft, from below/left/back)
    GLfloat light1_pos[] = {-0.7f, -0.5f, -1.0f, 0.0f};
    GLfloat light1_ambient[] = {0.10f, 0.10f, 0.10f, 1.0f};
    GLfloat light1_diffuse[] = {0.4f, 0.4f, 0.4f, 1.0f};
    GLfloat light1_specular[] = {0.2f, 0.2f, 0.2f, 1.0f};
    glLightfv(GL_LIGHT1, GL_POSITION, light1_pos);
    glLightfv(GL_LIGHT1, GL_AMBIENT, light1_ambient);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, light1_diffuse);
    glLightfv(GL_LIGHT1, GL_SPECULAR, light1_specular);

    // Set a material color (e.g., blue)
    glColor3f(0.2f, 0.4f, 0.8f);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, 800.0 / 600.0, 0.1, 1000.0);
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
