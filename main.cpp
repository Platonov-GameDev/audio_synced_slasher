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
	SDL_Renderer* pRenderer = SDL_CreateRenderer(pWindow, -1, 0);
	int mainLoopUpdateDelay = 20;

	// drawing setup
	SDL_Rect drawingRect;

	// input setup
	const Uint8* keyboardState = SDL_GetKeyboardState(NULL);

	// entities setup
	float playerPosX = WINDOW_WIDTH / 2;
	float playerPosY = WINDOW_HEIGHT / 2;
	float playerWidth = 20;
	float playerSpeed = 5;

	EnemyPosition* enemyPositions;
	int enemyPositionsLength = 1;
	enemyPositions = new EnemyPosition[enemyPositionsLength];
	enemyPositions[0] = { -20, -20 };	// first enemy position in the array is just for keeping the array non-empty!! don`t interact with it
	EnemyPosition* newEnemyPositions;
	float enemyWidth = 20;
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
				mainLoopUpdateDelay = 3000;
				isGameRunning = false;
			}
		}

		// RENDERING
		// clear screen
		SDL_SetRenderDrawColor(pRenderer, 0, 0, 0, 255);
		SDL_RenderClear(pRenderer);
		// draw player
		drawingRect.w = playerWidth;
		drawingRect.h = playerWidth;
		drawingRect.x = playerPosX - (playerWidth / 2);
		drawingRect.y = playerPosY - (playerWidth / 2);
		SDL_SetRenderDrawColor(pRenderer, 255, 255, 255, 255);
		SDL_RenderDrawRect(pRenderer, &drawingRect);
		SDL_RenderFillRect(pRenderer, &drawingRect);
		// draw enemies
		for (int i = 1; i < enemyPositionsLength; i++) {
			drawingRect.w = enemyWidth;
			drawingRect.h = enemyWidth;
			drawingRect.x = enemyPositions[i].posX - (enemyWidth / 2);
			drawingRect.y = enemyPositions[i].posY - (enemyWidth / 2);
			SDL_SetRenderDrawColor(pRenderer, 255, 0, 0, 255);
			SDL_RenderDrawRect(pRenderer, &drawingRect);
			SDL_RenderFillRect(pRenderer, &drawingRect);
		}

		SDL_RenderPresent(pRenderer);

		SDL_Delay(mainLoopUpdateDelay);
	}

	// cleanup
	SDL_RemoveTimer(enemySpawnTimerID);
	SDL_DestroyRenderer(pRenderer);
	SDL_DestroyWindow(pWindow);
	SDL_Quit();
}