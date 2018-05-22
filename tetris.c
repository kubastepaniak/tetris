#include "primlib.h"
#include "tiles.h"
#include <stdlib.h>
#include <time.h>

#define NUMBER_OF_TILES 7
#define NUMBER_OF_ROTATIONS 4
#define TILE_DIMENSIONS 4
#define DEFAULT_STATE 0
#define scoreIncrease 100

// flags
#define TRUE 1
#define FALSE 0

// pixel spacing constants
#define TEXT_SPACING 20
#define TOP_MARGIN 50
#define LEFT_MARGIN 50
#define RIGHT_MARGIN 200
#define RIGH_AREA 300
#define VERTICAL_TILES 20
#define HORIZONTAL_TILES 10
#define BLOCK_SIZE 30

// function codes
#define HEIGHT 1
#define WIDTH 2
#define DOWN 1
#define LEFT 2
#define RIGHT 3
#define MOVING 1
#define MOVING_ROT 2
#define STILL 3
#define EMPTY 0

// time constants
#define TIME_STEP 5
#define DESCENT_TIME 1000
#define END_DELAY 2000

// game states constants
#define GAMESTATE_FALL 1
#define GAMESTATE_MERGE 2
#define GAMESTATE_NEW_TILE 3

struct point
{
	int x;
	int y;
} position;

int gameMatrix[VERTICAL_TILES][HORIZONTAL_TILES];
int gamestate = GAMESTATE_FALL;
int gameTime = DEFAULT_STATE;
int rotationState = DEFAULT_STATE;
int score = DEFAULT_STATE;
int endCondition = DEFAULT_STATE;
int currentPiece;

void drawArena();
void keyInput();
void startup();
void addPiece();
void drawNextPiece(int pieceId);
void resetPosition();
void updateFallingPiece();
void clearPosition();
void descent();
void merge();
void end();
void rotate();
int mergePossible(int* mergeWhere);
int rotationPossible();
int movePossible(int direction);
int rollPiece();
int arenaSize(int dimension);
int arenaMidIndex();

int main(int argc, char const *argv[])
{
	if(initGraph(LEFT_MARGIN + RIGH_AREA + arenaSize(WIDTH),
		         2 * TOP_MARGIN + arenaSize(HEIGHT)))
		exit(3);
	srand(time(NULL));

	int nextPiece = rollPiece();
	currentPiece = rollPiece();

	startup();
	resetPosition();
	addPiece();

	while(1)
	{
		if(endCondition) end();
		
		drawArena();
		drawNextPiece(nextPiece);
		updateScreen();
		
		switch(gamestate)
		{
			case GAMESTATE_FALL:
			{
				keyInput();
				if(gameTime >= DESCENT_TIME)
				{
					if(movePossible(DOWN)) descent();
					else gamestate = GAMESTATE_MERGE;
					gameTime = 0;
				}
				break;
			}
			case GAMESTATE_MERGE:
			{
				merge();
				gamestate = GAMESTATE_NEW_TILE;
				break;
			}
			case GAMESTATE_NEW_TILE:
			{
				resetPosition();
				currentPiece = nextPiece;
				addPiece();
				nextPiece = rollPiece();
				gamestate = GAMESTATE_FALL;
				break;
			}
		}
		gameTime += TIME_STEP;
	}
	return 0;
}

void drawArena()
{
	filledRect(0, 0, screenWidth() - 1, screenHeight() - 1, BLACK);
	rect(LEFT_MARGIN, TOP_MARGIN,
		 LEFT_MARGIN + arenaSize(WIDTH), TOP_MARGIN + arenaSize(HEIGHT), RED);
	
	char buff[40];
	sprintf(buff, "%d", score);
	textout(screenWidth() - RIGHT_MARGIN, screenHeight() / 3 , "Next piece", RED);
	textout(screenWidth() - RIGHT_MARGIN - TEXT_SPACING, 2 * screenHeight() / 3 , "score: ", RED);
	textout(screenWidth() - RIGHT_MARGIN + 2 * TEXT_SPACING, 2 * screenHeight() / 3 , buff, RED);
	
	for(int vert = 0; vert < VERTICAL_TILES; vert++)
	{
		for(int hor = 0; hor < HORIZONTAL_TILES; hor++)
		{
			switch(gameMatrix[vert][hor])
			{
				case MOVING:
				{
					filledRect(LEFT_MARGIN + hor * BLOCK_SIZE, TOP_MARGIN + vert * BLOCK_SIZE,
							   LEFT_MARGIN + (hor + 1) * BLOCK_SIZE, TOP_MARGIN + (vert + 1) * BLOCK_SIZE, MAGENTA);
					break;
				}
				case MOVING_ROT:
				{
					filledRect(LEFT_MARGIN + hor * BLOCK_SIZE, TOP_MARGIN + vert * BLOCK_SIZE,
							   LEFT_MARGIN + (hor + 1) * BLOCK_SIZE, TOP_MARGIN + (vert + 1) * BLOCK_SIZE, RED);
					break;
				}
				case STILL:
				{
					filledRect(LEFT_MARGIN + hor * BLOCK_SIZE, TOP_MARGIN + vert * BLOCK_SIZE,
							   LEFT_MARGIN + (hor + 1) * BLOCK_SIZE, TOP_MARGIN + (vert + 1) * BLOCK_SIZE, YELLOW);
					break;
				}
			}
		}
	}
}

void keyInput()
{
	int key;
	switch(key = pollkey())
	{
		case SDLK_ESCAPE:
		{
			exit(3);
			break;
		}
		case SDLK_SPACE:
		{
			if(rotationPossible())
			{
				clearPosition();
				rotate();
				updateFallingPiece();
			}
			break;
		}
		case SDLK_DOWN:
		{
			while(movePossible(DOWN)) descent();
			gamestate = GAMESTATE_MERGE;
			break;
		}
		case SDLK_LEFT:
		{
			if(movePossible(LEFT))
			{
				clearPosition();
				position.x--;
				updateFallingPiece();
				drawArena();
			}
			break;
		}
		case SDLK_RIGHT:
		{
			if(movePossible(RIGHT))
			{
				clearPosition();
				position.x++;
				updateFallingPiece();
				drawArena();
			}
			break;
		}
	}
}

void startup()
{
	for(int vert = 0; vert < VERTICAL_TILES; vert++)
	{
		for(int hor = 0; hor < HORIZONTAL_TILES; hor++)
		{
			gameMatrix[vert][hor] = EMPTY;
		}
	}
}

void addPiece()
{
	for(int vert = 0; vert < TILE_DIMENSIONS; vert++)
	{
		for(int hor = 0; hor < TILE_DIMENSIONS; hor++)
		{
			if(pieces[currentPiece][rotationState][vert][hor] != EMPTY && 
			   gameMatrix[position.y + vert][position.x + hor] != EMPTY)
			{
				endCondition = TRUE;
			}

			if(pieces[currentPiece][rotationState][vert][hor] != EMPTY)
				gameMatrix[position.y + vert][position.x + hor] = pieces[currentPiece][rotationState][vert][hor];
		}
	}
}

void drawNextPiece(int pieceId)
{
	for(int vert = 0; vert < TILE_DIMENSIONS; vert++)
	{
		for(int hor = 0; hor < TILE_DIMENSIONS; hor++)
		{
			if(pieces[pieceId][0][vert][hor] != 0)
			{
				filledRect((screenWidth() - RIGHT_MARGIN) + hor * BLOCK_SIZE, 2 * TOP_MARGIN + vert * BLOCK_SIZE,
						   (screenWidth() - RIGHT_MARGIN) + (hor + 1) * BLOCK_SIZE, 2 * TOP_MARGIN + (vert + 1) * BLOCK_SIZE, MAGENTA);
			}
		}
	}
}

void resetPosition()
{
	position.x = arenaMidIndex();
	position.y = 0;
}

void updateFallingPiece()
{
	for(int vert = 0; vert < TILE_DIMENSIONS; vert++)
	{
		for(int hor = 0; hor < TILE_DIMENSIONS; hor++)
		{
			if(pieces[currentPiece][rotationState][vert][hor] != EMPTY)
				gameMatrix[position.y + vert][position.x + hor] = pieces[currentPiece][rotationState][vert][hor];
		}
	}
}

void clearPosition()
{
	for(int vert = 0; vert < TILE_DIMENSIONS; vert++)
	{
		for(int hor = 0; hor < TILE_DIMENSIONS; hor++)
		{
			if(pieces[currentPiece][rotationState][vert][hor] != EMPTY)
				gameMatrix[position.y + vert][position.x + hor] = EMPTY;
		}
	}
}

void descent()
{
	if(movePossible(DOWN))
	{
		clearPosition();
		position.y++;
		updateFallingPiece();
	}
}

void merge()
{
	for(int vert = 0; vert < TILE_DIMENSIONS; vert++)
	{
		for(int hor = 0; hor < TILE_DIMENSIONS; hor++)
		{
			if(pieces[currentPiece][rotationState][vert][hor] != EMPTY)
				gameMatrix[position.y + vert][position.x + hor] = STILL;
		}
	}

	int mergeWhere;
	int* pMergeWhere = &mergeWhere;
	while(mergePossible(pMergeWhere))
	{
		score += scoreIncrease;
		for(int vert = *pMergeWhere; vert > 0; vert--)
		{
			for(int hor = 0; hor < HORIZONTAL_TILES; hor++)
			{
				gameMatrix[vert][hor] = gameMatrix[vert - 1][hor];
				gameMatrix[vert - 1][hor] = EMPTY;
			}
		}
	}
}

void end()
{
	textout(screenWidth() - RIGHT_MARGIN, screenHeight() / 2 , "Game over!", RED);
	updateScreen();
	SDL_Delay(END_DELAY);
	exit(3);
}

void rotate()
{
	int oldX, oldY, newX, newY;
	int nextRotation = rotationState + 1;
	if(nextRotation == NUMBER_OF_ROTATIONS) nextRotation = 0;

	for(int vert = 0; vert < TILE_DIMENSIONS; vert++)
	{
		for(int hor = 0; hor < TILE_DIMENSIONS; hor++)
		{
			if(pieces[currentPiece][rotationState][vert][hor] == MOVING_ROT)
			{
				oldX = hor;
				oldY = vert;
			}
			if(pieces[currentPiece][nextRotation][vert][hor] == MOVING_ROT)
			{
				newX = hor;
				newY = vert;
			}
		}
	}

	rotationState = nextRotation;
	position.x -= (newX - oldX);
	position.y -= (newY - oldY);
}

int rotationPossible()
{	
	int oldX, oldY, newX, newY;
	int nextRotation = rotationState + 1;
	if(nextRotation == NUMBER_OF_ROTATIONS) nextRotation = 0;

	for(int vert = 0; vert < TILE_DIMENSIONS; vert++)
	{
		for(int hor = 0; hor < TILE_DIMENSIONS; hor++)
		{
			if(pieces[currentPiece][rotationState][vert][hor] == MOVING_ROT)
			{
				oldX = hor;
				oldY = vert;
			}
			if(pieces[currentPiece][nextRotation][vert][hor] == MOVING_ROT)
			{
				newX = hor;
				newY = vert;
			}
		}
	}

	int testPositionX = position.x - (newX - oldX);
	int testPositionY = position.y - (newY - oldY);

	if(testPositionY < 0 || testPositionY > VERTICAL_TILES - 1 ||
	   testPositionX < 0 || testPositionX > HORIZONTAL_TILES - 1) return FALSE;

	clearPosition();
	for(int vert = 0; vert < TILE_DIMENSIONS; vert++)
	{
		for(int hor = 0; hor < TILE_DIMENSIONS; hor++)
		{
			if(pieces[currentPiece][nextRotation][vert][hor] != EMPTY)
			{
				if(testPositionX + hor > HORIZONTAL_TILES - 1 || testPositionY + vert > VERTICAL_TILES -1)
				{
					updateFallingPiece();
					return FALSE;
				}
				if(gameMatrix[testPositionY + vert][testPositionX + hor] != EMPTY)
				{
					updateFallingPiece();
					return FALSE;
				}
			}
		}
	}
	updateFallingPiece();
	return TRUE;
}

int mergePossible(int* mergePointer)
{
	int mergeFlag;
	for(int vert = VERTICAL_TILES - 1; vert > 0; vert--)
	{
		mergeFlag = TRUE;
		
		for(int hor = 0; hor < HORIZONTAL_TILES; hor++)
		{
			if(gameMatrix[vert][hor] == EMPTY)
			{
				mergeFlag = FALSE;
				break;
			}
		}
		
		if(mergeFlag)
		{
			*mergePointer = vert;
			return mergeFlag;
		}
	}
	return mergeFlag;
}

int movePossible(int direction)
{
	int flag = TRUE;
	switch(direction)
	{
		case DOWN:
		{
			for(int hor = TILE_DIMENSIONS - 1; hor >= 0; hor--)
			{
				int vert = TILE_DIMENSIONS - 1;
				while(pieces[currentPiece][rotationState][vert][hor] == EMPTY && vert > 0) vert--;
				if(position.x + hor < HORIZONTAL_TILES)
				{
					if(position.y + vert < VERTICAL_TILES - 1)
					{
						if(position.y + vert + 1 < VERTICAL_TILES)
						{
							if(gameMatrix[position.y + vert + 1][position.x + hor] != EMPTY && 
							   pieces[currentPiece][rotationState][vert][hor] != EMPTY) flag = FALSE;
						}
					}
					else flag = FALSE;
				}
			}
			break;
		}
		case RIGHT:
		{
			for(int vert = TILE_DIMENSIONS - 1; vert >= 0; vert--)
			{
				int hor = TILE_DIMENSIONS - 1;
				while(pieces[currentPiece][rotationState][vert][hor] == EMPTY && hor > 0) hor--;
				if(position.x + hor < HORIZONTAL_TILES - 1)
				{
					if(position.x + hor + 1 < HORIZONTAL_TILES - 1 && position.y + vert < VERTICAL_TILES - 1)
					{
						if(gameMatrix[position.y + vert][position.x + hor + 1] != EMPTY && 
						   pieces[currentPiece][rotationState][vert][hor] != EMPTY) flag = FALSE;
					}
				}
				else flag = FALSE;
			}
			break;
		}
		case LEFT:
		{
			for(int vert = 0; vert < TILE_DIMENSIONS; vert++)
			{
				int hor = 0;
				while(pieces[currentPiece][rotationState][vert][hor] == EMPTY && hor < 3) hor++;
				if(hor == 3) continue;
				if(position.x + hor > 0 && position.y + vert < VERTICAL_TILES - 1)
				{
					if(gameMatrix[position.y + vert][position.x + hor - 1] != EMPTY &&
					   pieces[currentPiece][rotationState][vert][hor] != EMPTY) flag = FALSE;
				}
				else flag = FALSE;
			}
			break;
		}
	}
	return flag;
}

int rollPiece()
{
	return rand() % NUMBER_OF_TILES;
}

int arenaSize(int dimension)
{
	if(dimension == WIDTH) // horizontal length
	{
		return HORIZONTAL_TILES * BLOCK_SIZE;
	}
	if(dimension == HEIGHT) // vertical length
	{
		return VERTICAL_TILES * BLOCK_SIZE;
	}
	return 0;
}

int arenaMidIndex()
{
	return HORIZONTAL_TILES / 2 - 1;
}
