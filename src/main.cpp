#include "raylib.h"

int main(){
    // Initialization
    // --------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [core] example window");

    SetTargetFPS(30);
    //---------------------------------------------------------

    //Main Game Loop
    while (!WindowShouldClose()){ // detect window close or esc key
        // Update
        // -------------------------------------
        // TODO: Update variables here
        // -------------------------------------

        BeginDrawing();

        ClearBackground(RAYWHITE);
        DrawText("YIPPEEEE", 190, 200, 20, LIGHTGRAY);

        EndDrawing();
    }

    // De-Initialize
    // --------------------------------------------
    CloseWindow();
    // --------------------------------------------

    return 0;
}
