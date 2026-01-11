#include "objmain.h"
#include <structs.h>

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "renderer.h"
#include "instances.h"
#include "math.h"

extern SDL_Window *window;
extern ClientData client;
extern gameWorld game;

extern double deltaTime;
extern float timer;
//extern Camera currentCamera;
extern ButtonMap keyList[KEYBINDCOUNT];
extern SDL_MouseButtonFlags mouseState;
extern SDL_FPoint mousePos;
extern SDL_FPoint storedMousePos;
extern ButtonMap mouseButtons[3];

extern Mesh *teapotMesh;
extern Mesh *playerMesh;
extern Mesh *cubeMesh;
extern Mesh *cubePrim;
extern Mesh *spherePrim;

extern SDL_Texture *playerTex;
extern SDL_Texture *homerTex;

void playerInit(DataObj* object){
	object->pos.y = 0;
}
void playerUpdate(DataObj* object){
	if(object != game.currPlayer) return;
	
	Vector3 *playerVel = &object->objVel;
	
	SDL_FPoint playerMove = {0, 0};
	
	bool plrMoving = abs(keyList[KEYBIND_D].down - keyList[KEYBIND_A].down) + abs(keyList[KEYBIND_S].down - keyList[KEYBIND_W].down);
	
	if(plrMoving){
		playerMove = normalize2((SDL_FPoint){
			(SDL_cos(game.currCamera->rot.y) * (keyList[KEYBIND_D].down - keyList[KEYBIND_A].down)) + (SDL_sin(game.currCamera->rot.y) * (keyList[KEYBIND_S].down - keyList[KEYBIND_W].down)),
			(-SDL_sin(game.currCamera->rot.y) * (keyList[KEYBIND_D].down - keyList[KEYBIND_A].down)) + (SDL_cos(game.currCamera->rot.y) * (keyList[KEYBIND_S].down - keyList[KEYBIND_W].down)),
		});
		object->rot.y = atan2(playerMove.x, playerMove.y) + 3.14159;
	}
	
	if(game.currCamera->focusDist == 0){
		object->rot.y = game.currCamera->rot.y;
	}
	
	playerVel->x = (playerVel->x + playerMove.x) * 0.92;
	playerVel->z = (playerVel->z + playerMove.y) * 0.92;
	playerVel->y += -1 + 0.4 * (keyList[KEYBIND_SPACE].down && playerVel->y > 0);
	
	//object->pos.x += playerMove.x * 4 * deltaTime;
	//object->pos.y += (keyList[KEYBIND_SPACE].down - keyList[KEYBIND_SHIFT].down) * 4 * deltaTime;
	//object->pos.z += playerMove.y * 4 * deltaTime;
	object->pos = (Vector3){object->pos.x + playerVel->x * deltaTime, object->pos.y + playerVel->y * deltaTime, object->pos.z + playerVel->z * deltaTime};
	
	if(object->pos.y < 0){
		object->pos.y = 0;
		playerVel->y = 18 * keyList[KEYBIND_SPACE].pressed;
	}

	//object->pos.y = SDL_cos(timer) / 2 + 2;
	game.currCamera->pos = (Vector3){object->pos.x + (SDL_cos(game.currCamera->rot.x) * SDL_sin(game.currCamera->rot.y)) * game.currCamera->focusDist, object->pos.y + 2 - SDL_sin(game.currCamera->rot.x) * game.currCamera->focusDist, object->pos.z + (SDL_cos(game.currCamera->rot.x) * SDL_cos(game.currCamera->rot.y)) * game.currCamera->focusDist};
}
void playerDraw(DataObj* object){
	drawMesh(playerMesh, object->transform, (SDL_FColor){1, 1, 1, 1});
	//drawCube((Vector3){object->pos.x - 1, object->pos.y + 4, object->pos.z - 1}, (Vector3){2, 4, 2}, (SDL_FColor){1, 1, 1, 1});
	//drawBillboard(playerTex, (SDL_FRect){0, 0, 128, 128}, object->pos, (SDL_FPoint){8, 16}, (SDL_FPoint){4, 4});
}

DataType meshClass = (DataType){"Mesh\0", 4, 0, NULL, NULL, NULL};

void blockDraw(DataObj* object){
	//drawCube(object->pos, object->scale, ConvertSDLColour(object->colour));
	Mesh *itemMesh = cubePrim;
	DataObj *meshItem = firstChildOfType(object, meshClass);
	if(meshItem)
		itemMesh = meshItem->asVoidptr[OBJVAL_MESH];
	drawMesh(itemMesh, object->transform, ConvertSDLColour(object->colour));

	if (!strcmp(object->name, "Red Teapot"))
		object->rot = (Vector3){object->rot.x + 0.02, object->rot.y + 0.02, object->rot.z};
}

void homerDraw(DataObj* object){
	//drawCube(object->pos, object->scale, ConvertSDLColour(object->colour));
	drawBillboard(homerTex, (SDL_FRect){0, 0, 300, 500}, object->pos, (SDL_FPoint){1.5, 2.5}, (SDL_FPoint){3, 5});
}

DataType playerClass = {"Player\0", 2, 0, playerInit, playerUpdate, playerDraw};
DataType fuckingBeerdrinkerClass = {"beer drinker\0", 666, 0, NULL, NULL, homerDraw};
DataType blockClass = {"Block\0", 3, 0, NULL, NULL, blockDraw};
addNewClass(&playerClass); addNewClass(&fuckingBeerdrinkerClass); addNewClass(&blockClass); 