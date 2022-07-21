// Raylib Minesweeper
// Andrew Hamel Codes
// https://github.com/AndrewHamel111/raylib-minesweeper
// YOUTUBE LINK

#include <stdlib.h>
#include <time.h>

#if defined(PLATFORM_WEB)
	#include <emscripten/emscripten.h>
#endif

#include "raylib.h"
#include "raymath.h"

#define COLS 15
#define ROWS 15

const int screenWidth = 600;
const int screenHeight = 600;
const Vector2 screenSize = {screenWidth, screenHeight};

const int cellWidth = screenWidth / COLS;
const int cellHeight = screenHeight / ROWS;

const char* youLose = "YOU LOSE!";
const char* youWin = "YOU WIN!";
const char* pressRToRestart = "Press 'r' to play again!";

typedef struct Cell
{
	int i;
	int j;
	bool containsMine;
	bool revealed;
	bool flagged;
	int nearbyMines;
} Cell;

Cell grid[COLS][ROWS];

Texture2D flagSprite;
Texture2D tileAtlas;
int tilesRevealed;
int minesPresent;

Rectangle tileSource = {0, 0, 16, 16};

typedef enum GameState
{
	PLAYING,
	LOSE,
	WIN
} GameState;

GameState state;

float timeGameStarted;
float timeGameEnded;

Camera2D camera;

void CellDraw(Cell);
bool IndexIsValid(int, int);
void CellReveal(int, int);
void CellFlag(int, int);
int CellCountMines(int, int);
void GridInit(void);
void GridFloodClearFrom(int, int);
void GameInit(void);
void ResetCamera(void);
bool _dragging;
Vector2 _dragStartScreenPosition;

#if defined(PLATFORM_WEB)
	float mouseWheelDirection = -1.0f;
#else
	float mouseWheelDirection = 1.0f;
#endif

void UpdateDrawFrame(void);

int main()
{
	srand(time(0));

	InitWindow(screenWidth, screenHeight, "Raylib Minesweeper by Andrew Hamel");

	flagSprite = LoadTexture("resources/flag.png");
	tileAtlas = LoadTexture("resources/tiles.png");

	GameInit();
	
#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 60, 1); // Attach main loop
#else
	while(!WindowShouldClose())
	{
		UpdateDrawFrame();
	}
#endif
	
	CloseWindow();
	
	return 0;
}

void CellDraw(Cell cell)
{
	Rectangle dest = {cell.i * cellWidth, cell.j * cellHeight, cellWidth, cellHeight};
	if (cell.revealed)
	{
		if (cell.containsMine)
		{
			DrawRectangle(cell.i * cellWidth, cell.j * cellHeight, cellWidth, cellHeight, RED);
		}
		else
		{
			DrawRectangle(cell.i * cellWidth, cell.j * cellHeight, cellWidth, cellHeight, LIGHTGRAY);

			if (cell.nearbyMines > 0)
			{
				Rectangle source = tileSource;
				source.x = (cell.nearbyMines - 1) * 16;
				Vector2 origin = {0, 0};
				DrawTexturePro(tileAtlas, source, dest, origin, 0.0f, WHITE);
			}
		}
	}
	else if (cell.flagged)
	{
		Rectangle source = {0, 0, flagSprite.width, flagSprite.height};
		Vector2 origin = {0, 0};

		DrawTexturePro(flagSprite, source, dest, origin, 0.0f, Fade(WHITE, 0.5f));
	}

	DrawRectangleLines(cell.i * cellWidth, cell.j * cellHeight, cellWidth, cellHeight, BLACK);
}

bool IndexIsValid(int i, int j)
{
	return i >= 0 && i < COLS && j >= 0 && j < ROWS;
}

void CellReveal(int i, int j)
{
	if (grid[i][j].flagged || grid[i][j].revealed)
	{
		return;
	}

	grid[i][j].revealed = true;

	if (grid[i][j].containsMine)
	{
		state = LOSE;
		timeGameEnded = GetTime();
	}
	else
	{
		if (grid[i][j].nearbyMines == 0)
		{
			GridFloodClearFrom(i, j);
		}

		tilesRevealed++;

		if (tilesRevealed >= ROWS * COLS - minesPresent)
		{
			state = WIN;
			timeGameEnded = GetTime();
		}
	}
}

void CellFlag(int i, int j)
{
	if (grid[i][j].revealed)
	{
		return;
	}

	grid[i][j].flagged = !grid[i][j].flagged;
}

int CellCountMines(int i, int j)
{
	int count = 0;
	for (int iOff = -1; iOff <= 1; iOff++)
	{
		for (int jOff = -1; jOff <= 1; jOff++)
		{
			if (iOff == 0 && jOff == 0)
			{
				continue;
			}

			if (!IndexIsValid(i + iOff, j + jOff))
			{
				continue;
			}

			if (grid[i + iOff][j + jOff].containsMine)
			{
				count++;
			}
		}
	}

	return count;
}

void GridInit(void)
{
	for (int i = 0; i < COLS; i++)
	{
		for (int j = 0; j < ROWS; j++)
		{
			grid[i][j] = (Cell)
			{
				.i = i,
				.j = j,
				.containsMine = false,
				.revealed = false,
				.flagged = false,
				.nearbyMines = -1
			};
		}
	}

	minesPresent = (int)(ROWS * COLS * 0.1f);
	int minesToPlace = minesPresent;
	while (minesToPlace > 0)
	{
		int i = rand() % COLS;
		int j = rand() % ROWS;

		if (!grid[i][j].containsMine)
		{
			grid[i][j].containsMine = true;
			minesToPlace--;
		}
	}

	for (int i = 0; i < COLS; i++)
	{
		for (int j = 0; j < ROWS; j++)
		{
			if (!grid[i][j].containsMine)
			{
				grid[i][j].nearbyMines = CellCountMines(i, j);
			}
		}
	}
}

void GridFloodClearFrom(int i, int j)
{
	for (int iOff = -1; iOff <= 1; iOff++)
	{
		for (int jOff = -1; jOff <= 1; jOff++)
		{
			if (iOff == 0 && jOff == 0)
			{
				continue;
			}

			if (!IndexIsValid(i + iOff, j + jOff))
			{
				continue;
			}

			CellReveal(i + iOff, j + jOff);
		}
	}
}

void GameInit(void)
{
	GridInit();
	state = PLAYING;
	tilesRevealed = 0;
	timeGameStarted = GetTime();
	_dragging = false;

	ResetCamera();
}

void ResetCamera(void)
{
	camera = (Camera2D)
	{
		.offset = Vector2Scale(screenSize, 0.5f),
		.rotation = 0.0f,
		.target = Vector2Scale(screenSize, 0.5f),
		.zoom = 1.0f
	};
}

void UpdateDrawFrame(void)
{
	if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
	{
		_dragStartScreenPosition = GetMousePosition();
	}
	else if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) || IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
	{
		if (Vector2Distance(_dragStartScreenPosition, GetMousePosition()) > 10)
		{
			_dragging = true;
		}
	}
	else if (IsMouseButtonReleased(MOUSE_BUTTON_RIGHT))
	{
		if (_dragging)
		{
			_dragging = false;
		}
		else
		{
			Vector2 mPos = GetScreenToWorld2D(GetMousePosition(), camera);
			int indexI = mPos.x / cellWidth;
			int indexJ = mPos.y / cellHeight;

			if (state == PLAYING && IndexIsValid(indexI, indexJ))
			{
				CellFlag(indexI, indexJ);
			}
		}
	}
	else if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
	{
		if (_dragging)
		{
			_dragging = false;
		}
		else
		{
			Vector2 mPos = GetScreenToWorld2D(GetMousePosition(), camera);
			int indexI = mPos.x / cellWidth;
			int indexJ = mPos.y / cellHeight;

			if (state == PLAYING && IndexIsValid(indexI, indexJ))
			{
				CellReveal(indexI, indexJ);
			}
		}
	}

	if (_dragging)
	{
		Vector2 screenScroll = Vector2Subtract(GetMousePosition(), _dragStartScreenPosition);
		screenScroll = Vector2Scale(screenScroll, 1.0f / camera.zoom);
		_dragStartScreenPosition = GetMousePosition();

		camera.target = Vector2Subtract(camera.target, screenScroll);
	}

	if (IsKeyPressed(KEY_R) && state != PLAYING)
	{
		GameInit();
	}
	else if (IsKeyPressed(KEY_ZERO))
	{
		ResetCamera();
	}

	float move = GetMouseWheelMove();
	if (move > 0.5f)
	{
		camera.zoom += 0.1f * mouseWheelDirection;
	}
	else if (move < -0.5f)
	{
		camera.zoom -= 0.1f * mouseWheelDirection;
	}

	BeginDrawing();

		ClearBackground(RAYWHITE);

		BeginMode2D(camera);
		
			for (int i = 0; i < COLS; i++)
			{
				for (int j = 0; j < ROWS; j++)
				{
					CellDraw(grid[i][j]);
				}
			}

		EndMode2D();

		if (state == LOSE)
		{
			DrawRectangle(0, 0, screenWidth,screenHeight, Fade(WHITE, 0.8f));
			DrawText(youLose, screenWidth / 2 - MeasureText(youLose, 40) / 2, screenHeight / 2 - 10, 40, DARKGRAY);
			DrawText(pressRToRestart, screenWidth / 2 - MeasureText(pressRToRestart, 20) / 2, screenHeight * 0.75f - 10, 20, DARKGRAY);

			int minutes = (int)(timeGameEnded - timeGameStarted) / 60;
			int seconds = (int)(timeGameEnded - timeGameStarted) % 60;
			DrawText(TextFormat("Time played: %d minutes, %d seconds.", minutes, seconds), 20, screenHeight - 40, 20, DARKGRAY);
		}

		if (state == WIN)
		{
			DrawRectangle(0, 0, screenWidth,screenHeight, Fade(WHITE, 0.8f));
			DrawText(youWin, screenWidth / 2 - MeasureText(youWin, 40) / 2, screenHeight / 2 - 10, 40, DARKGRAY);
			DrawText(pressRToRestart, screenWidth / 2 - MeasureText(pressRToRestart, 20) / 2, screenHeight * 0.75f - 10, 20, DARKGRAY);

			int minutes = (int)(timeGameEnded - timeGameStarted) / 60;
			int seconds = (int)(timeGameEnded - timeGameStarted) % 60;
			DrawText(TextFormat("Time played: %d minutes, %d seconds.", minutes, seconds), 20, screenHeight - 40, 20, DARKGRAY);
		}

	EndDrawing();
}