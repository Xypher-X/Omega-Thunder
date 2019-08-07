/*____________________________________________________________________
|
| File: render.h
|
| (C) Copyright 2013 Abonvita Software LLC.
| Licensed under the GX Toolkit License, Version 1.0.
|___________________________________________________________________*/

// Game States
#define STATE_TITLE_SCREEN	0
#define STATE_HELP_SCREEN	1
#define STATE_STARTING		2
#define STATE_RUNNING		3
#define STATE_GAME_ENDING	4
#define STATE_GAME_OVER		5

#define ROTATE_UP_MAX   ((float)-50)
#define ROTATE_DOWN_MAX ((float)50)           
#define ROTATE_LEFT_MAX ((float)-80)
#define ROTATE_RIGHT_MAX ((float)80)

static void Init_Render_State(void);
void Render_Init(int *state);
void Render_Free();
//void Render_Restore(int *state);
bool Render_Game_Loop(int *state);
float Inverse_Lerp(float start, float end, float t);