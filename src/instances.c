#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#include <structs.h>
#include "instances.h"
#include "renderer.h"
#include "math.h"
#include "loader.h"

extern SDL_Renderer *renderer;
extern SDL_Texture *fontTex;

extern GameWorld game;
extern float timer;

void updateObjects(DataObj* item, int nodeDepth, int *idCount, bool uord){ //uord = update or draw
	//int i = (*idCount)++;
	if (uord){
		item->transform = newMatrix();
		rotateMatrix2(item->transform, item->rot);
		translateMatrix2(item->transform, (Vector3){item->pos.x, item->pos.y, item->pos.z});
		scaleMatrix2(item->transform, (Vector3){item->scale.x, item->scale.y, item->scale.z});
		
		if(item->classData->draw)item->classData->draw(item);
		free(item->transform);
		
		//drawText(renderer, fontTex, item->name, 32, nodeDepth * 24, 24 + i * 16, 16, 16, 12);
	}else{ 
		item->rot = (Vector3){fmod(item->rot.x, 6.28318), fmod(item->rot.y, 6.28318), fmod(item->rot.z, 6.28318)};
		if(item->classData->update)item->classData->update(item);
	}
	DataObj* child = item->child;
	while (child) {
		DataObj *next = child->next;
		updateObjects(child, nodeDepth + 1, idCount, uord);
		child = next;
	}
}

void cleanupObjects(DataObj* item){
	DataObj* child = item->child;
	while (child) {
		DataObj *next = child->next;
		cleanupObjects(child);
		child = next;
	}
	free(item);
}

DataObj* newObject(DataObj* parent, DataType* classData){
	DataObj *newObj = calloc(1, sizeof(DataObj)); 
	if(newObj == NULL){
		printf("Failed to create object of type '%s'.\n", classData->name);
		return NULL;
	}
	newObj->parent = parent;
	if (parent == NULL) parent = game.headObj;
	
	/* first added is first rendered/updated, last added is last rendered/updated
	newObj->next = NULL;
	//newObj->next = parent->child;
	if(!parent->child){
		parent->child = newObj;
	}else{
		DataObj *loopItem = parent->child;
		while(loopItem->next){
			loopItem = loopItem->next;
		}
		loopItem->next = newObj;
		newObj->prev = loopItem;
	}
	*/
	
	// first added is last rendered/updated, last added is first rendered/updated
	newObj->prev = NULL;
	newObj->next = parent->child;
	if (parent->child)
		parent->child->prev = newObj;
	parent->child = newObj;

	newObj->pos = (Vector3){0,0,0};
	newObj->transform = NULL;
	newObj->scale = (Vector3){1,1,1};
	newObj->rot = (Vector3){0,0,0};
	newObj->colour = (CharColour){255, 255, 255, 255, 0, COLOURMODE_RGB};
	newObj->name = classData->name;
	newObj->classData = classData;
	
	newObj->studioOpen = false;
	//newObj->values = NULL;
	if (classData->init)
		classData->init(newObj);

	printf("Created new object of type '%s'.\n", classData->name);
	
	return newObj;
}

void removeObject(DataObj* object){
	//object->onRemove(object);
	
	if(object == game.headObj){
		printf("are you out of your mind?\n");
		return;
	}
	
	DataObj *prevItem = object->prev; DataObj *nextItem = object->next; DataObj *parentItem = object->parent; DataObj *childItem = object->child;
	
	if(prevItem)
		prevItem->next = nextItem;
	
	if(childItem){
		childItem->parent = parentItem;
		
		DataObj *loopItem = object->child;
		while(loopItem->next){
			loopItem->parent = parentItem;
			loopItem = loopItem->next;
		}
		
		loopItem = parentItem->child;
		while(loopItem->next){
			loopItem = loopItem->next;
		}
		loopItem->next = childItem;
	}
	
	//segmentation fault... wtf? getting data from gameHeader headObj causes error????
	
	if(parentItem->child == object)
		parentItem->child = nextItem;
	
	printf("Object '%s' removed.\n", object->name);
	free(object);
}

bool parentObject(DataObj* child, DataObj* parent){
	if(parent->child == NULL){
		parent->child = child;
		//printf("%s -> %s\n", parent->name, parent->child->name);
		return 0;
	}

	DataObj* loopItem = parent->child;
	Uint32 loopCount = 1;
	for(Uint32 i = 0; i < loopCount; i++){
		if(loopItem->next == NULL) continue;
		loopItem = loopItem->next;
		loopCount++;
	}

	child->parent = parent;
	loopItem->next = child;
	child->prev = child;

	//printf("%s -> %s\n", parent->name, loopItem->next->name);

	return 0;

}

DataObj* firstChildOfType(DataObj* item, DataType classData){
	if(!item->child) return NULL;
	DataObj *loopItem = item->child;
	while(loopItem){
		if(loopItem->classData->id == classData.id){
			return loopItem;
		}
		loopItem = loopItem->next;
	}
	return NULL;
}

CollsionReturn* getCollision(CollisionHull* itemA, CollisionHull* itemB){
	CollsionReturn *output = NULL;
	
	if(itemA->shape == COLLHULL_CUBE && itemB->shape == COLLHULL_CUBE){
		if(!(between(itemA->pos.x, itemB->pos.x, itemB->pos.x + itemB->scale.x) && between(itemA->pos.y, itemB->pos.y, itemB->pos.y + itemB->scale.y) && between(itemA->pos.z, itemB->pos.z, itemB->pos.z + itemB->scale.z))) return NULL;
		output = malloc(sizeof(CollsionReturn));
		output->outNorm = (Vector3){0, itemA->pos.y - itemB->pos.y, 0};
	}
	
	/*if(collide is yes) then do'eth
		tell me the collision outputs then please
	  else*/
	return output;
	//end
}

typedef struct ClassListItem{
	DataType *pointer;
	struct ClassListItem *next;
} ClassListItem;

ClassListItem classListHead;
ClassListItem *classListLast = &classListHead;

bool addNewClass(DataType *newClass){
	ClassListItem *newItem = malloc(sizeof(ClassListItem));
	newItem->pointer = newClass; newItem->next = NULL;
	classListLast->next = newItem; classListLast = newItem;
}

DataType* getClassByName(char* string){
	ClassListItem *loopItem = &classListHead;
	while(loopItem){
		if(!strcmp(loopItem->pointer->name, string)){
			return loopItem->pointer;
		}
		loopItem = loopItem->next;
	}
}