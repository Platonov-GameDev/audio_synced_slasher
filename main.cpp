#include <stdio.h>
#include "peak_listener.hpp"
#include <iostream>
#include "math.h"
#include "SDL.h"
#undef main

const int WINDOW_WIDTH = 900;
const int WINDOW_HEIGHT = 900;

struct EnemyPosition { float posX; float posY; };

void spawnEnemy(EnemyPosition** pEnemyPositions, int* pEnemyPositionsLength, float* pPlayerPosX, float* pPlayerPosY) {
	EnemyPosition* newEnemyPositions;
	newEnemyPositions = new EnemyPosition[*pEnemyPositionsLength + 1];
	for (int i = 0; i < *pEnemyPositionsLength; i++) {
		newEnemyPositions[i] = *(*pEnemyPositions + i);
	}
	bool spawnPointIsValid = false;
	float newEnemyPosX;
	float newEnemyPosY;
	while (!spawnPointIsValid) {
		newEnemyPosX = rand() % WINDOW_WIDTH;
		newEnemyPosY = rand() % WINDOW_HEIGHT;
		spawnPointIsValid = true;
		if (
			(newEnemyPosX >= *pPlayerPosX - 240 && newEnemyPosX <= *pPlayerPosX + 240) &&
			(newEnemyPosY >= *pPlayerPosY - 240 && newEnemyPosY <= *pPlayerPosY + 240)
			) spawnPointIsValid = false;
	}
	newEnemyPositions[*pEnemyPositionsLength] = { newEnemyPosX, newEnemyPosY };
	
	delete[] *pEnemyPositions;
	*pEnemyPositions = newEnemyPositions;
	*pEnemyPositionsLength += 1;
}

struct SpawnEnemyCallbackParams {
	EnemyPosition** pEnemyPositions;
	int* pEnemyPositionsLength;
	float* pCurrentPeak;
	float* pPlayerPosX;
	float* pPlayerPosY;
};

Uint32 spawnEnemyCallback(Uint32 interval, void* param) {
	float currentPeak = *((SpawnEnemyCallbackParams*)param)->pCurrentPeak;
	if (currentPeak != 0)
	{
		spawnEnemy(
			((SpawnEnemyCallbackParams*)param)->pEnemyPositions,
			((SpawnEnemyCallbackParams*)param)->pEnemyPositionsLength,
			((SpawnEnemyCallbackParams*)param)->pPlayerPosX,
			((SpawnEnemyCallbackParams*)param)->pPlayerPosY
		);
	}
	return 1 + 2000 * (1 - sqrt(sqrt(currentPeak)));
}

void despawnEnemy(EnemyPosition** pEnemyPositions, int* pEnemyPositionsLength, int index) {
	EnemyPosition* newEnemyPositions;
	newEnemyPositions = new EnemyPosition[*pEnemyPositionsLength + 1];
	for (int i = 0; i < *pEnemyPositionsLength - 1; i++) {
		if (i >= index) newEnemyPositions[i] = *(*pEnemyPositions + i + 1);
		else newEnemyPositions[i] = *(*pEnemyPositions + i);
	}
	delete[] * pEnemyPositions;
	*pEnemyPositions = newEnemyPositions;
	*pEnemyPositionsLength -= 1;
}

struct Point {
	float x;
	float y;
};

bool intersection(Point p1, Point p2, Point p3, Point p4) {
	// Store the values for fast access and easy

	// equations-to-code conversion

	float x1 = p1.x, x2 = p2.x, x3 = p3.x, x4 = p4.x;
	float y1 = p1.y, y2 = p2.y, y3 = p3.y, y4 = p4.y;

	float d = (x1 - x2) * (y3 - y4) -(y1 - y2) * (x3 - x4);
	// If d is zero, there is no intersection

	if (d == 0) return false;

	// Get the x and y

	float pre = (x1 * y2 - y1 * x2), post = (x3 * y4 - y3 * x4);

	float x = (pre * (x3 - x4) -(x1 - x2) * post) / d;
	float y = (pre * (y3 - y4) -(y1 - y2) * post) / d;

	// Check if the x and y coordinates are within both lines
	float epsilon = 0.001;
	if (x < (min(x1, x2) - epsilon) ||
		x > (max(x1, x2) + epsilon) ||
		x < (min(x3, x4) - epsilon) ||
		x > (max(x3, x4) + epsilon))
		return false;
	if (y < (min(y1, y2) - epsilon) ||
		y > (max(y1, y2) + epsilon) ||
		y < (min(y3, y4) - epsilon) ||
		y > (max(y3, y4) + epsilon))
		return false;

	return true;
}

bool isEnemySideIntersectingWithDash(
	float playerPosX, float playerPosY, float playerWidth, float aimingPosX, float aimingPosY, Point sidePoint1, Point sidePoint2
) {
	Point dashStartPoint1 = { playerPosX + (playerWidth / 2), playerPosY + (playerWidth / 2) };
	Point dashStartPoint2 = { playerPosX + (playerWidth / 2), playerPosY - (playerWidth / 2) };
	Point dashStartPoint3 = { playerPosX - (playerWidth / 2), playerPosY + (playerWidth / 2) };
	Point dashStartPoint4 = { playerPosX - (playerWidth / 2), playerPosY - (playerWidth / 2) };
	Point dashEndPoint1 = { aimingPosX + (playerWidth / 2), aimingPosY + (playerWidth / 2) };
	Point dashEndPoint2 = { aimingPosX + (playerWidth / 2), aimingPosY - (playerWidth / 2) };
	Point dashEndPoint3 = { aimingPosX - (playerWidth / 2), aimingPosY + (playerWidth / 2) };
	Point dashEndPoint4 = { aimingPosX - (playerWidth / 2), aimingPosY - (playerWidth / 2) };
	bool intersects = false;
	if (intersection(dashStartPoint1, dashEndPoint1, sidePoint1, sidePoint2)) intersects = true;
	if (intersection(dashStartPoint2, dashEndPoint2, sidePoint1, sidePoint2)) intersects = true;
	if (intersection(dashStartPoint3, dashEndPoint3, sidePoint1, sidePoint2)) intersects = true;
	if (intersection(dashStartPoint4, dashEndPoint4, sidePoint1, sidePoint2)) intersects = true;
	return intersects;
}

void main() {

	// SETUP
	// window setup
	PeakListener peakListener = PeakListener();
	float currentPeak = 0;
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
	SDL_Window* pWindow = SDL_CreateWindow("music slasher", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
	SDL_Renderer* pRenderer = SDL_CreateRenderer(pWindow, -1, 0);

	// drawing setup
	int mainLoopUpdateDelay = 20;
	SDL_Rect drawingRect;
	float aimingPosX = 0;
	float aimingPosY = 0;
	SDL_SetRenderDrawBlendMode(pRenderer, SDL_BLENDMODE_BLEND);

	// input setup
	const Uint8* keyboardState = SDL_GetKeyboardState(NULL);

	// entities setup
	// player
	float playerPosX = WINDOW_WIDTH / 2;
	float playerPosY = WINDOW_HEIGHT / 2;
	float playerWidth = 20;
	float playerSpeed = 5;
	int playerMovX = 0;
	int playerMovY = 0;
	float playerDashPower = 120;
	bool isPlayerDashing = false;
	// enemies
	EnemyPosition* enemyPositions;
	int enemyPositionsLength = 1;
	enemyPositions = new EnemyPosition[enemyPositionsLength];
	enemyPositions[0] = { -20, -20 };	// first enemy position in the array is just for keeping the array non-empty!! don`t interact with it
	float enemyWidth = 20;
	float enemyMovX = 0;
	float enemyMovY = 0;
	float enemySpeed = 30;
	// enemy spawning
	SpawnEnemyCallbackParams spawnEnemyCallbackParams = { &enemyPositions, &enemyPositionsLength, &currentPeak, &playerPosX, &playerPosY };
	SDL_TimerID enemySpawnTimerID = SDL_AddTimer(100, spawnEnemyCallback, &spawnEnemyCallbackParams);

	// MAIN LOOP
	bool isGameRunning = true;
	bool gameReset = false;
	SDL_Event event;
	while (isGameRunning) {
		if (gameReset == true) {
			gameReset = false;
			mainLoopUpdateDelay = 20;
			enemyPositionsLength = 1;
			delete[] enemyPositions;
			enemyPositions = new EnemyPosition[enemyPositionsLength];
			enemyPositions[0] = { -20, -20 };
			playerPosX = WINDOW_WIDTH / 2;
			playerPosY = WINDOW_HEIGHT / 2;
		}
		currentPeak = peakListener.getPeak();
		// INPUT
		{
			while (SDL_PollEvent(&event)) {
				switch (event.type) {
				// exit input
				case SDL_QUIT:
					isGameRunning = false;
				case SDL_KEYDOWN:
					if(event.key.keysym.scancode == SDL_SCANCODE_SPACE) isPlayerDashing = true;
				}
			}
		}
		{
			// player input
			SDL_PumpEvents();
			playerMovX = 0;
			playerMovY = 0;
			if (keyboardState[SDL_SCANCODE_W] | keyboardState[SDL_SCANCODE_UP]) playerMovY--;
			if (keyboardState[SDL_SCANCODE_S] | keyboardState[SDL_SCANCODE_DOWN]) playerMovY++;
			if (keyboardState[SDL_SCANCODE_A] | keyboardState[SDL_SCANCODE_LEFT]) playerMovX--;
			if (keyboardState[SDL_SCANCODE_D] | keyboardState[SDL_SCANCODE_RIGHT]) playerMovX++;
		}
		
		// PROCESSING
		{
			// process dash collisions
			if (isPlayerDashing) {
				for (int i = 1; i < enemyPositionsLength; i++) {
					EnemyPosition enemyPos = enemyPositions[i];
					bool isEnemyHit = false;
					// top side
					Point enemyTopPoint1 = { enemyPos.posX - (enemyWidth / 2), enemyPos.posY + (enemyWidth / 2) };
					Point enemyTopPoint2 = { enemyPos.posX + (enemyWidth / 2), enemyPos.posY + (enemyWidth / 2) };
					if (isEnemySideIntersectingWithDash(
						playerPosX,playerPosY,playerWidth, aimingPosX, aimingPosY, enemyTopPoint1, enemyTopPoint2
					)) isEnemyHit = true;
					// bot side
					Point enemyBotPoint1 = { enemyPos.posX - (enemyWidth / 2), enemyPos.posY - (enemyWidth / 2) };
					Point enemyBotPoint2 = { enemyPos.posX + (enemyWidth / 2), enemyPos.posY - (enemyWidth / 2) };
					if (isEnemySideIntersectingWithDash(
						playerPosX, playerPosY, playerWidth, aimingPosX, aimingPosY, enemyBotPoint1, enemyBotPoint2
					)) isEnemyHit = true;
					// left side
					Point enemyLeftPoint1 = { enemyPos.posX - (enemyWidth / 2), enemyPos.posY + (enemyWidth / 2) };
					Point enemyLeftPoint2 = { enemyPos.posX - (enemyWidth / 2), enemyPos.posY - (enemyWidth / 2) };
					if (isEnemySideIntersectingWithDash(
						playerPosX, playerPosY, playerWidth, aimingPosX, aimingPosY, enemyLeftPoint1, enemyLeftPoint2
					)) isEnemyHit = true;
					// right side
					Point enemyRightPoint1 = { enemyPos.posX + (enemyWidth / 2), enemyPos.posY + (enemyWidth / 2) };
					Point enemyRightPoint2 = { enemyPos.posX + (enemyWidth / 2), enemyPos.posY - (enemyWidth / 2) };
					if (isEnemySideIntersectingWithDash(
						playerPosX, playerPosY, playerWidth, aimingPosX, aimingPosY, enemyRightPoint1, enemyRightPoint2
					)) isEnemyHit = true;
					// result
					if (isEnemyHit) {
						despawnEnemy(&enemyPositions, &enemyPositionsLength, i);
						i--;
					}
				}
			}
		}
		{
			// process player movement
			playerPosX += playerMovX * playerSpeed;
			playerPosY += playerMovY * playerSpeed;
		}
		{
			// process player aiming
			aimingPosX = playerPosX + playerDashPower * playerMovX;
			aimingPosY = playerPosY + playerDashPower * playerMovY;
		}
		{
			// process player dashing
			if (isPlayerDashing) {
				isPlayerDashing = false;
				playerPosX += playerMovX * playerDashPower;
				playerPosY += playerMovY * playerDashPower;
			}
		}
		{
			// process enemies movement
			for (int i = 1; i < enemyPositionsLength; i++) {
				EnemyPosition enemyPos = enemyPositions[i];
				enemyMovX = playerPosX - enemyPos.posX;
				enemyMovY = playerPosY - enemyPos.posY;
				float length = sqrt(enemyMovX * enemyMovX + enemyMovY * enemyMovY);
				std::cout << enemyMovX << enemyMovY << '\n';
				switch (abs(enemyMovX) >= abs(enemyMovY)) {
					case true: {
						enemyMovX /= length;
						enemyPositions[i].posX += enemyMovX * enemySpeed * pow(currentPeak, 3);
					}
					case false: {
				enemyMovY /= length;
						enemyPositions[i].posY += enemyMovY * enemySpeed * pow(currentPeak, 3);
					}
				}
			}
		}
		{
			// process collisions
			for (int i = 1; i < enemyPositionsLength; i++) {
				float distance = sqrt(pow((playerPosX - enemyPositions[i].posX), 2) + pow((playerPosY - enemyPositions[i].posY), 2));
				float minDistance = playerWidth / 2 + enemyWidth / 2;
				if (distance < minDistance) {
					printf("YOU ARE DEAD\n");
					gameReset = true;
					mainLoopUpdateDelay = 3000;
				}
			}
		}

		// RENDERING
		{
			// clear screen
			SDL_SetRenderDrawColor(pRenderer, 0, 0, 0, 255);
			SDL_RenderClear(pRenderer);
		}
		{
			// draw enemies
			for (int i = 1; i < enemyPositionsLength; i++) {
				drawingRect.w = enemyWidth;
				drawingRect.h = enemyWidth;
				drawingRect.x = enemyPositions[i].posX - (enemyWidth / 2);
				drawingRect.y = enemyPositions[i].posY - (enemyWidth / 2);
				SDL_SetRenderDrawColor(pRenderer, 255, 0, 0, 255 * pow(currentPeak, 2));
				SDL_RenderDrawRect(pRenderer, &drawingRect);
				SDL_RenderFillRect(pRenderer, &drawingRect);
			}
		}
		{
			// draw player
			drawingRect.w = playerWidth;
			drawingRect.h = playerWidth;
			drawingRect.x = playerPosX - (playerWidth / 2);
			drawingRect.y = playerPosY - (playerWidth / 2);
			SDL_SetRenderDrawColor(pRenderer, 255, 255, 255, 255);
			SDL_RenderDrawRect(pRenderer, &drawingRect);
			SDL_RenderFillRect(pRenderer, &drawingRect);
		}
		{
			// draw dash aiming
			SDL_SetRenderDrawColor(pRenderer, 255, 255, 255, 50);
			SDL_RenderDrawLineF(pRenderer, playerPosX, playerPosY, aimingPosX, aimingPosY);
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