#include "raylib.h"
#include <cfloat>
#include <cstdlib>
#include <vector>
#include <cstdint>
#include <map>

enum color { W, R, O, Y, G, B };

Color edgeColors[12][2] = {
    { W, G }, { W, O }, { W, B }, { W, R },
    { G, O }, { O, B }, { B, R }, { R, G },
    { G, Y }, { O, Y }, { B, Y }, { R, Y }
};

Color cornerColors[8][3] = {
    { W, G, O }, { W, O, B }, { W, B, R }, { W, R, G },
    { G, O, Y }, { O, B, Y }, { B, R, Y }, { R, G, Y }
};

enum side { FRONT, BACK, LEFT, RIGHT, TOP, BOTTOM };

std::map<color, Color> colorMap = {
    { W, WHITE }, { R, RED }, { O, ORANGE }, { Y, YELLOW }, { G, GREEN }, { B, BLUE }
};


struct CubeState {
    // FRONT FACE IS GREEN
    // FIRST BIT REPRESENTS ORIENTATION

    // LAYER                |------------D-----------|-----------M-----------|-----------U-----------|
    // POSITIONS            |---11-|--10-|--9--|--8--|--7--|--6--|--5--|--4--|--3--|--2--|--1--|--0--|
    uint64_t edges_solved = 0b01011'01010'01001'01000'00111'00110'00101'00100'00011'00010'00001'00000;

    // FIRST TWO BITS REPRESENT ORIENTATION
    // LAYER                |------------D-------------|-----------U-----------|
    // POSITIONS            |-----7--|--6--|--5--|--4--|--3--|--2--|--1--|--0--|
    uint64_t corners_solved = 0b00111'00110'00101'00100'00011'00010'00001'00000;

    bool isEdgeSolved(int edge) {
        return (edges_solved >> edge) & 1;
    }

    bool isCornerSolved(int corner) {
        return (corners_solved >> corner) & 1;
    }

    CubeState() {}
};

class Face {
public:
    Vector3 position;
    Vector3 volume;
    Color color;

    Face(Vector3 position, Vector3 volume, Color color) : position(position), volume(volume), color(color) {}

    void draw(Camera camera) {
        DrawCube(position, volume.x, volume.y, volume.z, color);
    }
};

class Cubelet {
public:
    Vector3 position;
    Vector3 volume;
    Color color = DARKGRAY;
    bool selected = false;

    Cubelet(Vector3 position, Vector3 volume) : position(position), volume(volume) {}

    void draw(Camera camera) {
        if (selected) {

            DrawCube(position, volume.x, volume.y, volume.z, color);
            DrawCubeWires(position, volume.x + 0.2f, volume.y + 0.2f, volume.z + 0.2f, GREEN);
        } else {
            DrawCube(position, volume.x, volume.y, volume.z, color);
            DrawCubeWires(position, volume.x, volume.y, volume.z, DARKGRAY);
        }
    }

    void draw(Camera camera, Color color) {
        this->color = color;
        draw(camera);
    }

    BoundingBox boundingBox() {
        return {
            position.x - volume.x / 2,
            position.y - volume.y / 2,
            position.z - volume.z / 2,
            position.x + volume.x / 2,
            position.y + volume.y / 2,
            position.z + volume.z / 2
        };
    }

    bool isPointInside(Vector3 point) {
        BoundingBox box = boundingBox();
        return point.x >= box.min.x && point.x <= box.max.x &&
               point.y >= box.min.y && point.y <= box.max.y &&
               point.z >= box.min.z && point.z <= box.max.z;
    }
};

class Corner : public Cubelet {

public:
    std::vector<Face> faces;

    Corner(Vector3 position, Vector3 volume) : Cubelet(position, volume){
        faces.reserve(3);
        Color color1 = position.y < 0 ? colorMap[W] : colorMap[Y];
        Color color2 = position.z < 0 ? colorMap[G] : colorMap[B];
        Color color3 = position.x < 0 ? colorMap[R] : colorMap[O];

        int signX = position.x < 0 ? -1 : 1;
        int signY = position.y < 0 ? -1 : 1;
        int signZ = position.z < 0 ? -1 : 1;

        Vector3 position1 = { position.x, position.y + signY * volume.y / 2, position.z };
        Vector3 position2 = { position.x, position.y, position.z + signZ * volume.z / 2 };
        Vector3 position3 = { position.x + signX * volume.x / 2, position.y, position.z };

        faces.push_back(Face(position1, { 1, .1, 1 }, color1));
        faces.push_back(Face(position2, { 1, 1, .1 }, color2));
        faces.push_back(Face(position3, { .1, 1, 1 }, color3));
    }

    void draw(Camera camera) {
        Cubelet::draw(camera);
        for (Face f: faces) {
            f.draw(camera);
        }
    }
};

class Edge : public Cubelet {
public:
    std::vector<Face> faces;

    Edge(Vector3 position, Vector3 volume) : Cubelet(position, volume) {
        faces.reserve(2);

        int signX = position.x == 0 ? 0 : (position.x < 0 ? -1 : 1);
        int signY = position.y == 0 ? 0 : (position.y < 0 ? -1 : 1);
        int signZ = position.z == 0 ? 0 : (position.z < 0 ? -1 : 1);

        // Y face — top or bottom edges only
        if (signY != 0) {
            Color colorY = position.y < 0 ? colorMap[W] : colorMap[Y];
            Vector3 posY = { position.x, position.y + signY * volume.y / 2, position.z };
            faces.push_back(Face(posY, { 1.0f, 0.1f, 1.0f }, colorY));
        }

        // Z face — front or back
        if (signZ != 0) {
            Color colorZ = position.z < 0 ? colorMap[G] : colorMap[B];
            Vector3 posZ = { position.x, position.y, position.z + signZ * volume.z / 2 };
            faces.push_back(Face(posZ, { 1.0f, 1.0f, 0.1f }, colorZ));
        }

        // X face — left or right
        if (signX != 0) {
            Color colorX = position.x < 0 ? colorMap[O] : colorMap[R];
            Vector3 posX = { position.x + signX * volume.x / 2, position.y, position.z };
            faces.push_back(Face(posX, { 0.1f, 1.0f, 1.0f }, colorX));
        }
    }

    void draw(Camera camera) {
        Cubelet::draw(camera);
        for (Face f : faces)
            f.draw(camera);
    }
};

class Core : public Cubelet {
public:
    std::vector<Face> faces;

    Core(Vector3 position, Vector3 volume) : Cubelet(position, volume) {
        faces.reserve(1);

        int signX = position.x == 0 ? 0 : (position.x < 0 ? -1 : 1);
        int signY = position.y == 0 ? 0 : (position.y < 0 ? -1 : 1);
        int signZ = position.z == 0 ? 0 : (position.z < 0 ? -1 : 1);

        if (signY != 0) {
            Color colorY = position.y < 0 ? colorMap[W] : colorMap[Y];
            Vector3 posY = { position.x, position.y + signY * volume.y / 2, position.z };
            faces.push_back(Face(posY, { 1.0f, 0.1f, 1.0f }, colorY));
        }

        if (signZ != 0) {
            Color colorZ = position.z < 0 ? colorMap[G] : colorMap[B];
            Vector3 posZ = { position.x, position.y, position.z + signZ * volume.z / 2 };
            faces.push_back(Face(posZ, { 1.0f, 1.0f, 0.1f }, colorZ));
        }

        if (signX != 0) {
            Color colorX = position.x < 0 ? colorMap[O] : colorMap[R];
            Vector3 posX = { position.x + signX * volume.x / 2, position.y, position.z };
            faces.push_back(Face(posX, { 0.1f, 1.0f, 1.0f }, colorX));
        }
    }

    void draw(Camera camera) {
        Cubelet::draw(camera);
        for (Face f : faces)
            f.draw(camera);
    }
};

class Cube {
    public:
    CubeState state;
    std::vector<Cubelet*> cubelets;

    Cube() {
        cubelets.reserve(27);
        initializeCubes();
    };

    void selectCube(Ray ray, RayCollision& collision) {

        RayCollision closest = { 0 };
        closest.hit = false;
        closest.distance = FLT_MAX;
        int closestIndex = -1;

        for (int i = 0; i < cubelets.size(); i++) {
            RayCollision hit = GetRayCollisionBox(ray, cubelets[i]->boundingBox());
            if (hit.hit && hit.distance < closest.distance) {
                closest = hit;
                closestIndex = i;
            }
        }
        collision = closest;
        if (closestIndex != -1) {
            deselectCubes();
            cubelets[closestIndex]->selected = true;
        }
    }
    void deselectCubes() {
        for (Cubelet* cube: cubelets) {
               cube->selected = false;
        }
    }

    void draw(Camera camera) {
        for (Cubelet* cub: cubelets) {
            cub->draw(camera);
        }
    }

    void initializeCubes() {
        float padding = 0.2f;
        float stride = 1.0 + padding;

        for (float x = -1; x < 2; x++) {
            for (float y = -1; y < 2; y++) {
                for (float z = -1; z < 2; z++) {
                    Vector3 pos = { x * stride, y * stride, z * stride };
                    Vector3 vol = { 1, 1, 1 };
                    int type = (x != 0 ? 1 : 0) + (y != 0 ? 1 : 0) + (z != 0 ? 1 : 0);
                    if (type == 3) {
                        cubelets.push_back(new Corner{ pos, vol });
                    }
                    if (type == 2) {
                        cubelets.push_back(new Edge{ pos, vol });
                    }
                    if (type == 1) {
                        cubelets.push_back(new Core{ pos, vol });
                    }
                }
            }
        }
    }
};

// class CubeCollection {
// public:
//     std::vector<Cube> cubes;

//     void draw(Camera camera) {
//         for (Cube& cube : cubes) {
//             cube.draw(camera);
//         }
//     }

//     void initializeCubes() {
//         float padding = 0.2f;
//         float stride = 1.0 + padding;

//         for (float x = -1; x < 2; x++) {
//             for (float y = -1; y < 2; y++) {
//                 for (float z = -1; z < 2; z++) {
//                     cubes.push_back(Cube({ x * stride, y * stride, z * stride }, { 1, 1, 1 }, GRAY));
//                 }
//             }
//         }
//     }

//     void selectCube(Ray ray, RayCollision& collision) {

//         RayCollision closest = { 0 };
//         closest.hit = false;
//         closest.distance = FLT_MAX;
//         int closestIndex = -1;

//         for (int i = 0; i < cubes.size(); i++) {
//             RayCollision hit = GetRayCollisionBox(ray, cubes[i].boundingBox());
//             if (hit.hit && hit.distance < closest.distance) {
//                 closest = hit;
//                 closestIndex = i;
//             }
//         }
//         collision = closest;
//         if (closestIndex != -1) {
//             deselectCubes();
//             cubes[closestIndex].selected = true;
//         }

//     }

//     void deselectCubes() {
//         for (Cube& cube : cubes) {
//             cube.selected = false;
//         }
//     }
// };

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [core] example - 3d picking");

    // Define the camera to look into our 3d world
    Camera camera = { 0 };
    camera.position = (Vector3){ 10.0f, 10.0f, 10.0f }; // Camera position
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };      // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;             // Camera projection type

    Ray ray = { 0 };                    // Picking line ray
    RayCollision collision = { 0 };     // Ray collision hit info

    Cube* cubes;
    cubes->initializeCubes();

    SetTargetFPS(60);                   // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())        // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        if (IsCursorHidden()) UpdateCamera(&camera, CAMERA_FIRST_PERSON);

        // Toggle camera controls
        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
        {
            if (IsCursorHidden()) EnableCursor();
            else DisableCursor();
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            ray = GetScreenToWorldRay(GetMousePosition(), camera);
            collision = { 0 };

            cubes->selectCube(ray, collision);
        }
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            BeginMode3D(camera);

                cubes->draw(camera);

                DrawRay(ray, MAROON);
                DrawGrid(10, 1.0f);

            EndMode3D();

            DrawText("Try clicking on the box with your mouse!", 240, 10, 20, DARKGRAY);

            if (collision.hit) DrawText("BOX SELECTED", (screenWidth - MeasureText("BOX SELECTED", 30))/2, (int)(screenHeight*0.1f), 30, GREEN);

            DrawText("Right click mouse to toggle camera controls", 10, 430, 10, GRAY);

            DrawFPS(10, 10);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
