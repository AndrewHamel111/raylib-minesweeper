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
int tilesRevealed;
int minesPresent;

typedef enum GameState
{
	PLAYING,
	LOSE,
	WIN
} GameState;

GameState state;

float timeGameStarted;
float timeGameEnded;

void CellDraw(Cell);
bool IndexIsValid(int, int);
void CellReveal(int, int);
void CellFlag(int, int);
int CellCountMines(int, int);
void GridInit(void);
void GridFloodClearFrom(int, int);
void GameInit(void);

void UpdateDrawFrame(void);

int main()
{
	srand(time(0));

	InitWindow(screenWidth, screenHeight, "Raylib Minesweeper by Andrew Hamel");

	flagSprite = LoadTexture("resources/flag.png");

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
				DrawText(TextFormat("%d", cell.nearbyMines), cell.i * cellWidth + 12, cell.j * cellHeight + 4, cellHeight - 8, DARKGRAY);
			}
		}
	}
	else if (cell.flagged)
	{
		// draw flag
		Rectangle source = {0, 0, flagSprite.width, flagSprite.height};
		Rectangle dest = {cell.i * cellWidth, cell.j * cellHeight, cellWidth, cellHeight};
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
}

void UpdateDrawFrame(void)
{
	if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
	{
		Vector2 mPos = GetMousePosition();
		int indexI = mPos.x / cellWidth;
		int indexJ = mPos.y / cellHeight;

		if (state == PLAYING && IndexIsValid(indexI, indexJ))
		{
			CellReveal(indexI, indexJ);
		}
	}
	else if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
	{
		Vector2 mPos = GetMousePosition();
		int indexI = mPos.x / cellWidth;
		int indexJ = mPos.y / cellHeight;

		if (state == PLAYING && IndexIsValid(indexI, indexJ))
		{
			CellFlag(indexI, indexJ);
		}
	}

	if (IsKeyPressed(KEY_R))
	{
		GameInit();
	}

	BeginDrawing();

		ClearBackground(RAYWHITE);
		
		for (int i = 0; i < COLS; i++)
		{
			for (int j = 0; j < ROWS; j++)
			{
				CellDraw(grid[i][j]);
			}
		}

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