#ifndef GAME_H 
#define GAME_H

typedef enum {
    TITLE,
    CORE,
} GameState;

void RunGame();
void ManageScenes();
void ChangeScene(GameState state);

#endif
