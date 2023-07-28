#include <stdio.h>
#include "peak_listener.hpp"
#include "SDL.h"
#undef main

const int WINDOW_WIDTH = 640;
const int WINDOW_HEIGHT = 640;

struct EnemyPosition { float posX; float posY; };

void spawnEnemy(EnemyPosition** pEnemyPositions, int* pEnemyPositionsLength) {
	EnemyPosition* newEnemyPositions;
	newEnemyPositions = new EnemyPosition[*pEnemyPositionsLength + 1];
	for (int i = 0; i < *pEnemyPositionsLength; i++) {
		newEnemyPositions[i] = *(*pEnemyPositions + i);
	}
	float newEnemyPosX = rand() % WINDOW_WIDTH;
	float newEnemyPosY = rand() % WINDOW_HEIGHT;
	newEnemyPositions[*pEnemyPositionsLength] = { newEnemyPosX, newEnemyPosY };

	delete[] *pEnemyPositions;
	*pEnemyPositions = newEnemyPositions;
	*pEnemyPositionsLength += 1;
}

struct SpawnEnemyCallbackParams {
	EnemyPosition** pEnemyPositions;
	int* pEnemyPositionsLength;
};

Uint32 spawnEnemyCallback(Uint32 interval, void* param) {
	spawnEnemy(((SpawnEnemyCallbackParams*)param)->pEnemyPositions, ((SpawnEnemyCallbackParams*)param)->pEnemyPositionsLength);
	return interval;
}

void main() {
	// SETUP
	// window setup
	PeakListener peakListener = PeakListener();
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
	SDL_Window* pWindow = SDL_CreateWindow("music slasher", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
	SDL_Surface* pSurface = SDL_GetWindowSurface(pWindow);
	SDL_Surface* pBufferSurface = SDL_CreateRGBSurface(0, 640, 640, 32, 0, 0, 0, 0);
	int sdlUpdateDelay = 20;

	// drawing setup
	SDL_Rect drawing_rect;
	Uint32 black_pixel = SDL_MapRGB(pSurface->format, 0, 0, 0);
	Uint32 white_pixel = SDL_MapRGB(pSurface->format, 255, 255, 255);
	Uint32 red_pixel = SDL_MapRGB(pSurface->format, 255, 0, 0);

	// input setup
	const Uint8* keyboardState = SDL_GetKeyboardState(NULL);

	// entities setup
	float playerPosX = WINDOW_WIDTH / 2;
	float playerPosY = WINDOW_HEIGHT / 2;
	float playerWidth = 20;
	float playerSpeed = 5;
	SDL_Color player_color = { 255, 255, 255, 255 };

	EnemyPosition* enemyPositions;
	int enemyPositionsLength = 1;
	enemyPositions = new EnemyPosition[enemyPositionsLength];
	enemyPositions[0] = { -20, -20 };	// first enemy position in the array is just for keeping the array non-empty!! don`t interact with it
	EnemyPosition* newEnemyPositions;
	float enemyWidth = 20;
	SDL_Color enemy_color = { 255, 0, 0, 255 };
	// enemy spawning
	SpawnEnemyCallbackParams spawnEnemyCallbackParams = { &enemyPositions, &enemyPositionsLength };
	SDL_TimerID enemySpawnTimerID = SDL_AddTimer(1000, spawnEnemyCallback, &spawnEnemyCallbackParams);

	// MAIN LOOP
	bool isGameRunning = true;
	SDL_Event event;
	while (isGameRunning) {

		// INPUT
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_QUIT:
				isGameRunning = false;
			}
		}
		// player movement
		SDL_PumpEvents();
		if (keyboardState[SDL_SCANCODE_UP]) playerPosY -= playerSpeed;
		if (keyboardState[SDL_SCANCODE_DOWN]) playerPosY += playerSpeed;
		if (keyboardState[SDL_SCANCODE_LEFT]) playerPosX -= playerSpeed;
		if (keyboardState[SDL_SCANCODE_RIGHT]) playerPosX += playerSpeed;
		
		// PROCESSING
		// collisions
		for (int i = 0; i < enemyPositionsLength; i++) {
			float distance = sqrt(pow((playerPosX - enemyPositions[i].posX), 2) + pow((playerPosY - enemyPositions[i].posY), 2));
			float minDistance = playerWidth / 2 + enemyWidth / 2;
			if (distance < minDistance) {
				printf("YOU ARE DEAD\n");
				sdlUpdateDelay = 3000;
				isGameRunning = false;
			}
		}

		// RENDERING
		SDL_FillRect(pBufferSurface, NULL, black_pixel);
		// draw player
		drawing_rect.w = playerWidth;
		drawing_rect.h = playerWidth;
		drawing_rect.x = playerPosX - (playerWidth / 2);
		drawing_rect.y = playerPosY - (playerWidth / 2);
		SDL_FillRect(pBufferSurface, &drawing_rect, white_pixel);
		// draw enemies
		for (int i = 1; i < enemyPositionsLength; i++) {
			drawing_rect.w = enemyWidth;
			drawing_rect.h = enemyWidth;
			drawing_rect.x = enemyPositions[i].posX - (enemyWidth / 2);
			drawing_rect.y = enemyPositions[i].posY - (enemyWidth / 2);
			SDL_FillRect(pBufferSurface, &drawing_rect, red_pixel);
		}

		SDL_BlitSurface(pBufferSurface, NULL, pSurface, NULL);
		SDL_UpdateWindowSurface(pWindow);

		SDL_Delay(sdlUpdateDelay);
	}

	// cleanup
	SDL_RemoveTimer(enemySpawnTimerID);
	SDL_FreeSurface(pBufferSurface);
	SDL_DestroyWindow(pWindow);
	SDL_Quit();
}