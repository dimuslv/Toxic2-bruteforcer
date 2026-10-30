#include "raylib.h"
#define DO_VISUALIZATION

us savedVisits[2][level_height*level_width];
us borderVisits[2][level_height*level_width];
us winVisits[2][level_height*level_width];

us savedMax[2]{1, 1};
us borderMax[2]{1, 1};
us winMax[2]{1, 1};

uc levelPixels[level_height*level_width][3];

bool curBuffer = false;

atomic_flag threadRead{};

void savedVisit(const playerState &p) {
	setIfGreater(savedMax[curBuffer], ++savedVisits[curBuffer][p._y*level_width + p._x.truncate()]);
}

void borderVisit(const playerState &p) {
	setIfGreater(borderMax[curBuffer], ++borderVisits[curBuffer][p._y*level_width + p._x.truncate()]);
}

void winVisit(const playerState &p) {
	setIfGreater(winMax[curBuffer], ++winVisits[curBuffer][p._y*level_width + p._x.truncate()]);
}

void updateVisualization() {
	threadRead.wait(true);
	curBuffer = !curBuffer;
	threadRead.test_and_set();
	
	savedMax[curBuffer] = savedMax[!curBuffer];
	borderMax[curBuffer] = 1;
	winMax[curBuffer] = winMax[!curBuffer];
	
	memcpy(savedVisits[curBuffer], savedVisits[!curBuffer], sizeof savedVisits[curBuffer]);
	memset(borderVisits[curBuffer], 0, sizeof borderVisits[curBuffer]);
	memcpy(winVisits[curBuffer], winVisits[!curBuffer], sizeof winVisits[curBuffer]);
}

void DrawTextureToFit(const Texture2D &texture) {
	DrawTextureEx(texture, {0, 0}, 0, min((float)GetScreenWidth() / texture.width, (float)GetScreenHeight() / texture.height), WHITE);
}

void windowUpdate(stop_token st) {
    SetTraceLogLevel(LOG_NONE);
    InitWindow(800, 450, "Bruteforcer");
	SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(60);
    
    Image levelImage{levelPixels, level_width, level_height, 1, PIXELFORMAT_UNCOMPRESSED_R8G8B8};
    
    Texture2D levelTexture = LoadTextureFromImage(levelImage);
    
    uint imgNum = 0;
    
    if (!DirectoryExists("img")) {
		MakeDirectory("img");
	}
	
	while (!WindowShouldClose()) {
        // Update
		if (threadRead.test()) {
			for (int i = 0; i < level_height*level_width; i++) {
				levelPixels[i][0] = divUp(borderVisits[!curBuffer][i] * UCHAR_MAX, borderMax[!curBuffer]);
				levelPixels[i][1] = divUp(savedVisits[!curBuffer][i] * UCHAR_MAX, savedMax[!curBuffer]);
				levelPixels[i][2] = divUp(winVisits[!curBuffer][i] * CHAR_MAX, winMax[!curBuffer])
					+ (!getCollision(i % level_width, i / level_width)) * 128;
			}
			
			threadRead.clear();
			threadRead.notify_one();
			
			ExportImage(levelImage, TextFormat("img/%u.png", imgNum++));
			
			UpdateTexture(levelTexture, levelImage.data);
		}
		
        BeginDrawing();
        ClearBackground(WHITE);
		
		DrawTextureToFit(levelTexture);
		
        EndDrawing();
    }
    
    //UnloadImage(levelImage);
    UnloadTexture(levelTexture);

    CloseWindow();
}
