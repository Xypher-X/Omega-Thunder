/***************************************************************
*
* File: render.cpp
*
* Author: David Sta Cruz
*
* Description: Functions to draw the world
*
***************************************************************/
/*___________________
|
| Include Files
|__________________*/

#include <first_header.h>
#include "dp.h"

#include "render.h"
#include "position.h"

/*___________________
|
| Constants
|__________________*/

// Full Screen
#define FULL_SCREEN_NEAR_PLANE  ((float)0.1)
#define FULL_SCREEN_FAR_PLANE   ((float)100.0)
#define FULL_SCREEN_FOV			((float)50.0)

// Game Screen
#define GAME_NEAR_PLANE         ((float)0.1)
#define GAME_FAR_PLANE          ((float)5000.0)
#define GAME_FOV				((float)80.0)
#define MAX_STRUCTURE_COUNT		10
#define MAX_STRUCTURE_LIGHTS	5
#define MAX_ENEMY_COUNT			100
#define MAX_GAME_TIME			359999000 // 99 hrs 59 mins 59 secs all in milliseconds
#define MAX_SCORE_FONTS			9
#define MAX_HP_FONTS			4
#define MAX_LV_FONTS			2
#define MAX_TIME_FONTS			2
#define MAX_DEFEATED_FONTS		6
#define NORMAL_SPEED			((float) 100.0) // ((float)60.0) // feet per second
#define RAIU_MAX_HP				1000
#define HOSHU_MAX_HP			100
#define HOSHU_MAX_LV			10
#define HOSHU_SCORE				200
#define HOSHU_EXP				100
#define MAX_SCORE				999999999
#define MAX_LV					10
#define MAX_GROUND_LENGTH		((float)5000.0)
#define MAX_PROJECTILE_DISTANCE 4000 // 4000 ft max projectile distance
#define RAIU_MAX_LASER_COUNT	20
#define HOSHU_MAX_LASER_COUNT	1
#define FX_NORMAL_DURATION		1000 // 1 second
#define STRUCTURE_SIDE_LEFT		-1
#define STRUCTURE_SIDE_RIGHT	1
#define STRUCTURE_SIDE_BOTH		0

// Game Over Screen
#define WINNING_SCORE			100000 // score needed to reach in order to win the game

#define COMBINE					1
#define SECONDS					1000.0f // 1000 milliseconds in 1 second
#define SCREENSHOT_FILENAME		"screenshots\\Omega Thunder-screenshot "
#define SCREENSHOT_GAMEOVER		"screenshots\\Omega Thunder-game over "

/*___________________
|
| Function prototypes
|__________________*/

static void Init_Render_State(void);

// Loading Screen
static void Init_LoadingScreen();
static void Display_LoadingScreen();

// Title Screen
static void Init_TitleScreen();
bool Render_TitleScreen(int *state);
static void Free_TitleScreen();

// Game Screen
static void Init_GameScreen();
bool Render_GameScreen(int *state);
static void Free_GameScreen();
static gx3dMotion *Load_Motion(gx3dMotionSkeleton *mskeleton, char *filename, int fps, gx3dMotionMetadataRequest *metadata_requested, int num_metadata_requested, bool load_all_metadata);
static void Display_Fonts(gx3dObject *billboards[], char buf[], int buf_size, int max_index, gx3dMatrix m, gx3dTexture tex, bool show_zeros);
static void Display_Font(gx3dObject *billboard, char ch, gx3dMatrix m, gx3dTexture tex);
static void Play_FX(gx3dTexture effect, gx3dVector normal, gx3dVector position, gx3dVector scale, float duration, unsigned time, int alpha_test, bool repeat);
float Inverse_Lerp(float start, float end, float t);
static void Update_Light(gx3dLight *light, gx3dColor color, gx3dVector *position, float range, unsigned time_elapsed, bool flicker);
static void Update_Light(gx3dLight *light, gx3dColor color, gx3dVector *position, float range, unsigned time_elapsed, bool flicker, float constant, float linear, float quadratic);

// Game Over Screen
static void Init_GameOverScreen();
bool Render_GameOverScreen(int *state);
static void Free_GameOverScreen();

/*___________________
|
| Macros
|__________________*/

#define RESTORE_PROGRAM_WITH_MOUSE  				  \
  {													  \
	msHideMouse ();	      							  \
	bool quit = false;								  \
	for (;;)                                          \
		if (evGetEvent (&event)) {					  \
		if (event.type == evTYPE_WINDOW_ACTIVE)		  \
			break;                        			  \
		else if (event.type == evTYPE_WINDOW_CLOSE)	{ \
					return true;					  \
			break;                                    \
		}                                             \
			}										  \
	if (NOT quit) {                                   \
		gxRestoreDirectX ();						  \
		evFlushEvents ();							  \
		msHideMouse ();                               \
	}                                                 \
  }

#define RESTORE_PROGRAM                               \
  {													  \
	bool quit = false;								  \
	for (;;)                                          \
		if (evGetEvent (&event)) {					  \
		if (event.type == evTYPE_WINDOW_ACTIVE)		  \
			break;                        			  \
		else if (event.type == evTYPE_WINDOW_CLOSE)	{ \
					return true;					  \
			break;                                    \
		}										      \
		}											  \
	if (NOT quit) {                                   \
		gxRestoreDirectX ();						  \
		Init_Render_State();                          \
		evFlushEvents ();							  \
		force_update = true;                          \
	}                                                 \
  }


/*___________________
|
| Global variables
|__________________*/

//========== Matrices ==========//
gx3dMatrix m, m1, m2, m3, m4, m5;

//========== Vectors ==========//
gx3dVector v, v1, v2;

//========== Colors ==========//
gxColor color;
gx3dColor color3d_white =		{ 1, 1, 1, 0 };
gx3dColor color3d_dim =			{ 0.2f, 0.2f, 0.2f };
gx3dColor color3d_black =		{ 0, 0, 0, 0 };
gx3dColor color3d_darkgray =	{ 0.3f, 0.3f, 0.3f, 0 };
gx3dColor color3d_gray =		{ 0.5f, 0.5f, 0.5f, 0 };
gx3dColor laser_red =			{ 1.0f, 0.5f, 0.5f, 0.0f };
gx3dColor lightning_purple =	{ 0.50f, 0.0f, 1.0f, 0.0f };
gx3dColor lightning_blue =		{ 0.0f, 0.9f, 1.0f, 0.0f };
gx3dColor lightning_green =		{ 0.01f, 1.0f, 0.5f, 0.0f };
gx3dColor blue_cyan =			{ 0.0f, 0.5f, 1.0f, 0.0f };
gx3dColor explosion_orange =	{ 1.0f, 0.6f, 0.2f, 0.0f };


//========== Materials ==========//
static gx3dMaterialData material_default = {
	{ 1, 1, 1, 1 }, // ambient color
	{ 1, 1, 1, 1 }, // diffuse color
	{ 0.1, 0.1, 0.1, 0.1 }, // specular color
	{ 0, 0, 0, 0 }, // emissive color
	15              // specular sharpness (0=disabled, 0.01=sharp, 10=diffused)
};

static gx3dMaterialData material_structures = {
	{ 1, 1, 1, 1 }, // ambient color
	{ 1, 1, 1, 1 }, // diffuse color
	{ 0, 0, 0, 0 }, // specular color
	{ 0, 0, 0, 0 }, // emissive color
	0               // specular sharpness (0=disabled, 0.01=sharp, 10=diffused)
};

static gx3dMaterialData material_raiu = {
	{ 1, 1, 1, 1 }, // ambient color
	{ 1, 1, 1, 1 }, // diffuse color
	{ 0.01, 0.01, 0.01, 0.01 }, // specular color
	{ 0, 0, 0, 0 }, // emissive color
	20               // specular sharpness (0=disabled, 0.01=sharp, 10=diffused)
};

static gx3dMaterialData material_hoshu = {
	{ 1, 1, 1, 1 }, // ambient color
	{ 1, 1, 1, 1 }, // diffuse color
	{ 0.1, 0.1, 0.1, 0.1 }, // specular color
	{ 0, 0, 0, 0 }, // emissive color
	15               // specular sharpness (0=disabled, 0.01=sharp, 10=diffused)
};

static gx3dMaterialData material_blue_laser = {
	{ 0, 1, 1, 1 }, // ambient color
	{ 0, 1, 1, 1 }, // diffuse color
	{ 0, 0, 0, 0 }, // specular color
	{ 0, 1, 1, 1 }, // emissive color
	0               // specular sharpness (0=disabled, 0.01=sharp, 10=diffused)
};

static gx3dMaterialData material_red_laser = {
	{ 1, 0.5, 0.5, 1 }, // ambient color
	{ 1, 0.5, 0.5, 1 }, // diffuse color
	{ 0, 0, 0, 0 }, // specular color
	{ 1, 0.5, 0.5, 1 }, // emissive color
	0               // specular sharpness (0=disabled, 0.01=sharp, 10=diffused)
};

//========== Lights ==========//
gx3dLightData light_data, lightdata;
gx3dLight dir_light, explosion_light;

//========== Object Structures ==========//
// Structure for world structures
struct World_Structures {
	int type;
	gx3dVector pos;
	int side;				// (-1) left | (0) both left and right | (1) right
	bool spawned;			// is the structure currently spawned?
	bool rotated = false;	// has the structure already been rotated when needed?
};

// Structure for lights used by the world structures
struct Structure_Lights {
	gx3dLight light;				// light
	int world_structure_index = -1;	// index of the structure currently using the light (-1 : light is not being used by a structure)
	bool enabled = false;			// is the light enabled?
};

// Structure for the floor lights world structure
struct Floor_Lights {
	gx3dVector pos;
};

// Structure for the electric fence (or health pad)
struct Health_Pad {
	gx3dSphere sphere;
	gx3dVector pos;
	gx3dLight light;
	gx3dParticleSystem psys;
	int heal_amt;
	bool ps_enable = false;		// is the particle system activated on the pad?
	bool draw = false;			// is it currently drawn in the world?
};

// Structure for a laser projectile
struct Laser {
	gx3dSphere sphere;								// bounding sphere
	gx3dVector pos;									// position
	gx3dVector distance = { 0,0,0 };				// distance that the projectile have traveled so far in each axes
	gx3dTrajectory trajectory;						// direction and speed of the projectile
	bool hit = false;								// will the laser hit its target?
	int hit_index = -1;								// used by the character which saves the index of the enemy that is going to be hit first
	float world_shift = 0.0;						// total amount of world transformation applied since the projectile was fired
	bool destroyed = false;							// used on enemies when the character has the blade active on the moment of impact
	bool draw = false;								// should it be drawn?
	float hit_timer = -1;							// timer that starts when the laser had just hit an object, enemies, or the character
};

// Structure for the Hoshu
struct Hoshu {
	gx3dSphere sphere;								// bounding sphere
	gx3dVector pos;									// position
	gx3dVector view;								// Hoshu's normal view vector
	int lv;											// level
	int hp;											// health
	int score_amt;									// amount of score given when defeated
	int exp_amt;									// amount of experience points given when defeated
	int gun_damage;									// gun damage
	unsigned gun_delay;								// delay for shooting a laser
	unsigned gun_timer;								// timer for shooting delay
	Laser laser[HOSHU_MAX_LASER_COUNT];				// Hoshu's ammunition
	float fire_rate;								// how fast the Hoshu can shoot a projectile with respect to the delay
	int laser_index;								// index of the next shootable laser
	bool onScreen = false;							// is it on the screen?
	bool draw = false;								// should it be drawn? (spawned?)
	float explosion_timer = -1;						// timer that starts for when the Hoshu was just destroyed (-1 disables the timer)
	int explosion_type = -1;						// type of explosion initialized (used for updating explosion effects) (-1 disables the explosion from being generated)
	bool explode_snd_initialized = false;			// indicates if the explosion sound had been initialized
	bool blade_mark_1 = false;						// marker for when the Hoshu has already taken damage from blade swing 1
	bool blade_mark_2 = false;						// marker for when the Hoshu has already taken damage from blade swing 2
};

// Structure for enemies
struct Enemy {
	Hoshu hoshu[MAX_ENEMY_COUNT];
	int hoshu_index;
};

// Structure for Raiu
struct Raiu {
	gx3dSphere sphere;					// bounding sphere
	gx3dVector pos;						// position
	gx3dVector view;					// Raiu's normal view vector
	gx3dLight light;					// character's light
	int hp;								// health
	int blade_lv;						// blade level
	int blade_damage;					// blade damage
	float blade_delay;					// delay for using the blade
	float blade_timer;					// timer for using the blade
	int gun_lv;							// gun level
	int gun_damage;						// gun damage
	unsigned gun_delay;					// delay for shooting the ray gun
	unsigned gun_timer;					// timer for shooting delay
	int exp;							// experience points
	Laser laser[RAIU_MAX_LASER_COUNT];	// Raiu's ammunition
	int laser_index;					// index of the next shootable laser
};

//========== Sounds ==========//
// Title Screen
Sound s_title_screen_bgm, s_select;

// Game Screen
Sound s_game_bgm, s_starting, s_ending, s_lv_up, s_electric_fence,/* s_hoshu_walk,*/ s_enemy_lv_up;
Sound s_blade_1, s_blade_2, s_laser_1, s_laser_2, s_footstep; /*, s_scrap_get;*/
Sound s_raiu_nice, s_raiu_grunt_1, s_raiu_hurt_1, s_raiu_hurt_2;
Sound s_explosion_1, s_explosion_2, s_explosion_3;

// Game Over Screen
Sound s_game_over_bgm;

//========== Objects & Textures ==========//
// Loading Screen
gx3dObject *obj_loading;
gx3dTexture tex_loading_text;

// Title Screen
gx3dObject *obj_billboards, *obj_quit_button;
gx3dTexture tex_title_screen_l, tex_title_screen_r, tex_button_start_game, tex_button_quit_game, tex_help_screen;

// Game Screen
gx3dObject *obj_hud, *obj_hp_fonts[MAX_HP_FONTS*2], *obj_score_fonts[MAX_SCORE_FONTS], *obj_weapon_lv_fonts[MAX_LV_FONTS*2], *obj_raiu, *obj_hoshu;
gx3dObject *obj_skydome, *obj_ground, *obj_structures, *obj_laser, *obj_fence;
gx3dTexture tex_hp, tex_hp_bar, tex_score_bar, tex_fonts, tex_weapons_lv, tex_raiu, tex_hoshu, tex_blue_laser, tex_red_laser;
gx3dTexture tex_skydome, tex_earth, tex_ground, tex_ground_inner, tex_ground_under, tex_structures;
gx3dTexture fx_run_charge, fx_fence, fx_explosion_1, fx_explosion_2, fx_explosion_3, fx_laser_blue, fx_laser_red, fx_level_up;
gx3dTexture fx_destruct_shock, fx_destruct_charge, fx_destruct_charge_loop, fx_destruct_flash;
Raiu raiu;
Enemy enemies;

// Game Over Screen
gx3dObject *obj_hr_fonts[MAX_TIME_FONTS], *obj_min_fonts[MAX_TIME_FONTS], *obj_sec_fonts[MAX_TIME_FONTS], *obj_defeated_fonts[MAX_DEFEATED_FONTS];
gx3dTexture tex_game_over_l, tex_game_over_r;
gx3dTexture fx_fade_white;

//========== Animation Variables ==========//
gx3dMotionSkeleton *raiu_skeleton = 0, *hoshu_skeleton = 0;
gx3dMotion *ani_raiu_entrance, *ani_raiu_run, *ani_raiu_swing_1, *ani_raiu_swing_2, *ani_raiu_trip, *ani_raiu_self_destruct;
gx3dMotion *ani_raiu_neutral, *ani_raiu_aim_up, *ani_raiu_aim_down, *ani_raiu_aim_left, *ani_raiu_aim_right;
gx3dBlendNode *bnode_entrance, *bnode_trip, *bnode_self_destruct, *bnode_run, *bnode_swing, *bnode_adder_aim_swing;
gx3dBlendNode *bnode_aim_ud, *bnode_aim_lr, *bnode_aim_udlr, *bnode_adder_run_aim;
gx3dBlendTree *btree_entrance, *btree_movement, *btree_trip, *btree_self_destruct;

//========== Other Variables ==========//
evEvent event;
gxRelation relation;
gx3dObjectLayer *layer;
gx3dVector position, heading;
int move_x, move_y;
static int first_run = TRUE;
static int initialized = FALSE;
bool next_screen;
int take_screenshot;
bool isLoading = false, loaded = false;
float sfx_volume = 85;
float bgm_volume = 90;
float world_shift_amt; // variable representing the amount of translation to be performed on the world
World_Structures structure[MAX_STRUCTURE_COUNT];
Structure_Lights structure_light[MAX_STRUCTURE_LIGHTS]; // maximum of 8 lights can be initialized at a time (2 already used: dir_light and character light)
Health_Pad heal_pad;
int score;
unsigned game_timer;
int enemies_defeated, hoshus_defeated;

/*____________________________________________________________________
|
| Function: Render_Init
|
| Input: Called from main.cpp
| Output: Initialize all data structures used for the current game state.
|___________________________________________________________________*/

void Render_Init(int *state)
{
	if (NOT initialized) {

		// Loads the graphics for the loading screen first after launching the game
		if (first_run) {
			Init_LoadingScreen();
			Display_LoadingScreen();
			first_run = FALSE;
		}

		// Init 3D parameters specific to the current game state
		if (*state == STATE_TITLE_SCREEN) {
			Init_TitleScreen();
		}
		
		else if (*state == STATE_STARTING) {
			Init_GameScreen();
		}

		else if (*state == STATE_GAME_OVER) {
			Init_GameOverScreen();
		}

	}

	switch (*state) {
		// 2D full screen images only
	case STATE_TITLE_SCREEN:
	case STATE_GAME_OVER:
		// Set starting camera position
		position.x = 0;
		position.y = 5;
		position.z = -100;
		// Set starting camera view direction (heading)
		heading.x = 0;  // {0,0,1} for cubic environment mapping to work correctly
		heading.y = 0;
		heading.z = 1;
		Position_Init(&position, &heading, 0);
		break;
		
		// Game view involving 2D and 3D objects
	case STATE_STARTING:
		// Set starting camera position
		position.x = 0;
		position.y = 5;
		position.z = -100;
		// Set starting camera view direction (heading)
		heading.x = 0;  // {0,0,1} for cubic environment mapping to work correctly
		heading.y = 0;
		heading.z = 1;
		Position_Init(&position, &heading, (NORMAL_SPEED / 2));
		break;
	}

	/*____________________________________________________________________
	|
	| Flush input queue
	|___________________________________________________________________*/

	// Flush input queue
	evFlushEvents();
	// Zero mouse movement counters
	msGetMouseMovement(&move_x, &move_y);  // call this here so the next call will get movement that has occurred since it was called here                                    
	// Hide mouse cursor
	msHideMouse();
}

/*____________________________________________________________________
|
| Function: Render_Game_Loop
|
| Input: Called from main.cpp
| Output: Executes the appropriate game loop depending on the game state.
|         Returns true when quitting game.
|___________________________________________________________________*/

bool Render_Game_Loop(int *state)
{
	bool quit;
	switch (*state) {
	case STATE_TITLE_SCREEN:
		quit = Render_TitleScreen(*&state);
		break;
	case STATE_STARTING:
		quit = Render_GameScreen(*&state);
		break;
	case STATE_GAME_OVER:
		quit = Render_GameOverScreen(*&state);
		break;
	}
	return quit;
}

/*____________________________________________________________________
|
| Function: Init_LoadingScreen
|
| Input: Called from Render_TitleScreen
| Output:
|___________________________________________________________________*/

static void Init_LoadingScreen() {
	/*____________________________________________________________________
	|
	| Init 3D graphics
	|___________________________________________________________________*/

	// Set projection matrix
	gx3d_SetProjectionMatrix(FULL_SCREEN_FOV, FULL_SCREEN_NEAR_PLANE, FULL_SCREEN_FAR_PLANE);
	gx3d_SetFillMode(gx3d_FILL_MODE_GOURAUD_SHADED);

	// Sets the 3D viewport clear color to black
	color.r = 0;
	color.g = 0;
	color.b = 0;
	color.a = 0;

	/*____________________________________________________________________
	|
	| Load loading screen objects
	|___________________________________________________________________*/
	// Load models
	gx3d_ReadLWO2File("Objects\\billboards.lwo", &obj_loading, gx3d_VERTEXFORMAT_DEFAULT, gx3d_DONT_LOAD_TEXTURES);

	// Load textures
	tex_loading_text = gx3d_InitTexture_File("Objects\\Images\\omega_thunder_loading.bmp", "Objects\\Images\\omega_thunder_loading_fa.bmp", 0);
}

/*____________________________________________________________________
|
| Function: Display_LoadingScreen
|
| Input: Called from Render_TitleScreen
| Output:
|___________________________________________________________________*/

static void Display_LoadingScreen() {

	// Parameters
	const float loading_scale = 0.15;
	const float loading_x = 0.70;
	const float loading_y = -0.40;

	// Clear viewport
	gx3d_ClearViewport(gx3d_CLEAR_SURFACE | gx3d_CLEAR_ZBUFFER, color, gx3d_MAX_ZBUFFER_VALUE, 0);
	// Start rendering in 3D
	if (gx3d_BeginRender()) {
		// Set the default light
		gx3d_SetAmbientLight(color3d_white);
		// Set the default material
		gx3d_SetMaterial(&material_default);

		/*____________________________________________________________________
		|
		| Draw 2D graphics on top of 3D
		|___________________________________________________________________*/

		// Save Current view matrix
		gx3dMatrix view_save;
		gx3d_GetViewMatrix(&view_save);

		// Set new view matrix
		gx3dVector tfrom = { 0,0,-2 }, tto = { 0,0,0 }, twup = { 0,1,0 };
		gx3d_CameraSetPosition(&tfrom, &tto, &twup, gx3d_CAMERA_ORIENTATION_LOOKTO_FIXED);
		gx3d_CameraSetViewMatrix();

		// Do necessary transformations to fit the title screen objects on screen
		layer = gx3d_GetObjectLayer(obj_loading, "loading_text");
		gx3d_GetScaleMatrix(&m1, loading_scale, loading_scale, loading_scale);
		gx3d_GetTranslateMatrix(&m2, loading_x, loading_y, 0);
		gx3d_MultiplyMatrix(&m1, &m2, &m1);
		gx3d_SetObjectLayerMatrix(obj_loading, layer, &m1);

		// Update transforms
		gx3d_Object_UpdateTransforms(obj_loading);

		// Draw the appropriate objects on screen
		gx3d_DisableZBuffer();
		gx3d_EnableAlphaBlending();

		layer = gx3d_GetObjectLayer(obj_loading, "loading_text");
		gx3d_SetTexture(0, tex_loading_text);
		gx3d_DrawObjectLayer(layer, 0);

		gx3d_DisableAlphaBlending();
		gx3d_EnableZBuffer();

		// Restore view matrix
		gx3d_SetViewMatrix(&view_save);

		// Stop rendering
		gx3d_EndRender();

		// Page flip (so user can see it)
		gxFlipVisualActivePages(FALSE);
	}
}

/*____________________________________________________________________
|
| Function: Init_TitleScreen
|
| Input: Called from Render_TitleScreen
| Output:
|___________________________________________________________________*/

static void Init_TitleScreen() {

	/*____________________________________________________________________
	|
	| Init 3D graphics
	|___________________________________________________________________*/

	// Set projection matrix
	gx3d_SetProjectionMatrix(FULL_SCREEN_FOV, FULL_SCREEN_NEAR_PLANE, FULL_SCREEN_FAR_PLANE);
	gx3d_SetFillMode(gx3d_FILL_MODE_GOURAUD_SHADED);

	// Sets the 3D viewport clear color to black
	color.r = 0;
	color.g = 0;
	color.b = 0;
	color.a = 0;

	/*____________________________________________________________________
	|
	| Initialize title screen sound library
	|___________________________________________________________________*/

	snd_Init(22, 16, 2, 1, 1);

	s_title_screen_bgm = snd_LoadSound("wav\\velcer_space_bgm.wav", snd_CONTROL_VOLUME, 0);
	s_select = snd_LoadSound("wav\\menu_select.wav", snd_CONTROL_VOLUME, 0);

	/*____________________________________________________________________
	|
	| Load title screen objects
	|___________________________________________________________________*/

	// Load models
	gx3d_ReadLWO2File("Objects\\billboards.lwo", &obj_billboards, gx3d_VERTEXFORMAT_DEFAULT, gx3d_DONT_LOAD_TEXTURES);
	gx3d_ReadLWO2File("Objects\\billboards.lwo", &obj_quit_button, gx3d_VERTEXFORMAT_DEFAULT, gx3d_DONT_LOAD_TEXTURES);

	// Load textures
	tex_title_screen_l = gx3d_InitTexture_File("Objects\\Images\\omega_thunder_title_screen_1.bmp", 0, 0);
	tex_title_screen_r = gx3d_InitTexture_File("Objects\\Images\\omega_thunder_title_screen_2.bmp", 0, 0);
	tex_button_start_game = gx3d_InitTexture_File("Objects\\Images\\start_game_button.bmp", "Objects\\Images\\button_fa.bmp", 0);
	tex_button_quit_game = gx3d_InitTexture_File("Objects\\Images\\quit_game_button.bmp", "Objects\\Images\\button_fa.bmp", 0);
	tex_help_screen = gx3d_InitTexture_File("Objects\\Images\\help_screen.bmp", "Objects\\Images\\help_screen_fa.bmp", 0);
}

/*____________________________________________________________________
|
| Function: Init_GameScreen
|
| Input: Called from Render_Init
| Output:
|___________________________________________________________________*/

void Init_GameScreen() 
{

	/*____________________________________________________________________
	|
	| Init 3D graphics
	|___________________________________________________________________*/

	// Set projection matrix
	gx3d_SetProjectionMatrix(GAME_FOV, GAME_NEAR_PLANE, GAME_FAR_PLANE);
	gx3d_SetFillMode(gx3d_FILL_MODE_GOURAUD_SHADED);

	// Sets the 3D viewport clear color to black
	color.r = 0;
	color.g = 0;
	color.b = 0;
	color.a = 0;


	/*____________________________________________________________________
	|
	| Initialize game screen sound library
	|___________________________________________________________________*/

	snd_Init(22, 16, 2, 1, 1);
	snd_SetListenerDistanceFactorToFeet(snd_3D_APPLY_NOW);

	s_game_bgm = snd_LoadSound("wav\\cool_adventure_bgm.wav", snd_CONTROL_VOLUME, 0);
	s_starting = snd_LoadSound("wav\\raiu_game_start.wav", snd_CONTROL_VOLUME, 0);
	s_ending = snd_LoadSound("wav\\raiu_self_destruct.wav", snd_CONTROL_VOLUME, 0);
	s_lv_up = snd_LoadSound("wav\\level_up.wav", snd_CONTROL_VOLUME, 0);
	s_enemy_lv_up = snd_LoadSound("wav\\enemy_alarm.wav", snd_CONTROL_VOLUME, 0);
	s_electric_fence = snd_LoadSound("wav\\electric_fence.wav",snd_CONTROL_3D | snd_CONTROL_VOLUME, 0);
	//s_hoshu_walk = snd_LoadSound("wav\\robot_footstep.wav", snd_CONTROL_3D | snd_CONTROL_VOLUME, 0);
	s_blade_1 = snd_LoadSound("wav\\blade_slash1.wav", snd_CONTROL_VOLUME, 0);
	s_blade_2 = snd_LoadSound("wav\\blade_slash2.wav", snd_CONTROL_VOLUME, 0);
	s_laser_1 = snd_LoadSound("wav\\laser_beam1.wav", snd_CONTROL_VOLUME, 0);
	s_laser_2 = snd_LoadSound("wav\\laser_beam2.wav", snd_CONTROL_3D | snd_CONTROL_VOLUME, 0);
	s_footstep = snd_LoadSound("wav\\footstep_metal.wav", snd_CONTROL_VOLUME | snd_CONTROL_FREQUENCY, 0);
	//s_scrap_get = snd_LoadSound("wav\\scrap_get.wav", snd_CONTROL_VOLUME, 0);
	s_raiu_nice = snd_LoadSound("wav\\raiu_electric_nice.wav", snd_CONTROL_VOLUME, 0);
	s_raiu_grunt_1 = snd_LoadSound("wav\\raiu_grunt1.wav", snd_CONTROL_VOLUME, 0);
	s_raiu_hurt_1 = snd_LoadSound("wav\\raiu_hurt1.wav", snd_CONTROL_VOLUME, 0);
	s_raiu_hurt_2 = snd_LoadSound("wav\\raiu_hurt2.wav", snd_CONTROL_VOLUME, 0);
	s_explosion_1 = snd_LoadSound("wav\\explosion1.wav", snd_CONTROL_3D | snd_CONTROL_VOLUME, 0);
	s_explosion_2 = snd_LoadSound("wav\\explosion2.wav", snd_CONTROL_3D | snd_CONTROL_VOLUME, 0);
	s_explosion_3 = snd_LoadSound("wav\\explosion3.wav", snd_CONTROL_3D | snd_CONTROL_VOLUME, 0);

	/*____________________________________________________________________
	|
	| Load game screen objects
	|___________________________________________________________________*/

	// Load particle system for health pad (electric fence)
	heal_pad.psys = Script_ParticleSystem_Create("electric_current.gxps");

	// Load models
	gx3d_ReadLWO2File("Objects\\billboards.lwo", &obj_hud, gx3d_VERTEXFORMAT_DEFAULT, gx3d_DONT_LOAD_TEXTURES);
	for (int i = 0; i < MAX_HP_FONTS * 2; i++) {
		gx3d_ReadLWO2File("Objects\\billboards.lwo", &obj_hp_fonts[i], gx3d_VERTEXFORMAT_DEFAULT, gx3d_DONT_LOAD_TEXTURES);
	}
	for (int i = 0; i < MAX_LV_FONTS * 2; i++) {
		gx3d_ReadLWO2File("Objects\\billboards.lwo", &obj_weapon_lv_fonts[i], gx3d_VERTEXFORMAT_DEFAULT, gx3d_DONT_LOAD_TEXTURES);
	}
	for (int i = 0; i < MAX_SCORE_FONTS; i++) {
		gx3d_ReadLWO2File("Objects\\billboards.lwo", &obj_score_fonts[i], gx3d_VERTEXFORMAT_DEFAULT, gx3d_DONT_LOAD_TEXTURES);
	}
	gx3d_ReadLWO2File("Objects\\raiu.lwo", &obj_raiu, gx3d_VERTEXFORMAT_TEXCOORDS | gx3d_VERTEXFORMAT_WEIGHTS, gx3d_MERGE_DUPLICATE_VERTICES | gx3d_DONT_LOAD_TEXTURES);
	raiu_skeleton = gx3d_MotionSkeleton_Read_GX3DSKEL_File("ani\\raiu_run.gx3dskel"); // read in the skeleton from a gx3dskel file (faster than reading from an LWS file)
	gx3d_Skeleton_Attach(obj_raiu);
	gx3d_ReadLWO2File("Objects\\hoshu.lwo", &obj_hoshu, gx3d_VERTEXFORMAT_TEXCOORDS | gx3d_VERTEXFORMAT_WEIGHTS, gx3d_MERGE_DUPLICATE_VERTICES | gx3d_DONT_LOAD_TEXTURES);
	gx3d_ReadLWO2File("Objects\\space_skydome.lwo", &obj_skydome, gx3d_VERTEXFORMAT_DEFAULT, gx3d_MERGE_DUPLICATE_VERTICES | gx3d_SMOOTH_DISCONTINUOUS_VERTICES | gx3d_DONT_LOAD_TEXTURES);
	gx3d_ReadLWO2File("Objects\\spacecraft_ground.lwo", &obj_ground, gx3d_VERTEXFORMAT_DEFAULT, gx3d_DONT_LOAD_TEXTURES);
	gx3d_ReadLWO2File("Objects\\spacecraft_structures.lwo", &obj_structures, gx3d_VERTEXFORMAT_DEFAULT, gx3d_DONT_LOAD_TEXTURES);
	gx3d_ReadLWO2File("Objects\\electric_fence.lwo", &obj_fence, gx3d_VERTEXFORMAT_DEFAULT, gx3d_DONT_LOAD_TEXTURES);
	gx3d_ReadLWO2File("Objects\\projectile_laser.lwo", &obj_laser, gx3d_VERTEXFORMAT_DEFAULT, gx3d_MERGE_DUPLICATE_VERTICES | gx3d_DONT_LOAD_TEXTURES);

	//========== Load textures ==========//
	tex_hp = gx3d_InitTexture_File("Objects\\Images\\hp.bmp", "Objects\\Images\\hp_fa.bmp", 0);
	tex_hp_bar = gx3d_InitTexture_File("Objects\\Images\\hp_bar.bmp", 0, 0);
	tex_score_bar = gx3d_InitTexture_File("Objects\\Images\\score_bar.bmp", "Objects\\Images\\score_bar_fa.bmp", 0);
	tex_fonts = gx3d_InitTexture_File("Objects\\Images\\score_fonts.bmp", "Objects\\Images\\score_fonts_fa.bmp", 0);
	tex_weapons_lv = gx3d_InitTexture_File("Objects\\Images\\weapons_lv.bmp", "Objects\\Images\\weapons_lv_fa.bmp", 0);
	tex_raiu = gx3d_InitTexture_File("Objects\\Images\\raiu_texture.bmp", "Objects\\Images\\raiu_texture_fa.bmp", 0);
	tex_hoshu = gx3d_InitTexture_File("Objects\\Images\\hoshu_texture.bmp", "Objects\\Images\\hoshu_texture_fa.bmp", 0);
	tex_skydome = gx3d_InitTexture_File("Objects\\Images\\space_texture.bmp", 0, 0);
	tex_earth = gx3d_InitTexture_File("Objects\\Images\\earth.bmp", "Objects\\Images\\earth_fa.bmp", 0);
	tex_ground = gx3d_InitTexture_File("Objects\\Images\\spacecraft_ground_texture.bmp", 0, 0);
	tex_ground_inner = gx3d_InitTexture_File("Objects\\Images\\spacecraft_inner_texture.bmp", 0, 0);
	tex_ground_under = gx3d_InitTexture_File("Objects\\Images\\spacecraft_underground_texture.bmp", 0, 0);
	tex_structures = gx3d_InitTexture_File("Objects\\Images\\spacecraft_structure_texture.bmp", 0, 0);
	tex_blue_laser = gx3d_InitTexture_File("Objects\\Images\\blue_laser.bmp", "Objects\\Images\\laser_fa.bmp", 0);
	tex_red_laser = gx3d_InitTexture_File("Objects\\Images\\red_laser.bmp", "Objects\\Images\\laser_fa.bmp", 0);

	//========== Load effects textures ==========//
	fx_run_charge = gx3d_InitTexture_File("Objects\\FX\\electricity_1.bmp", "Objects\\FX\\electricity_1_fa.bmp", 0);
	fx_fence = gx3d_InitTexture_File("Objects\\FX\\electricity_3.bmp", "Objects\\FX\\electricity_3_fa.bmp", 0);
	fx_explosion_1 = gx3d_InitTexture_File("Objects\\FX\\explosion_1.bmp", "Objects\\FX\\explosion_1_fa.bmp", 0);
	fx_explosion_2 = gx3d_InitTexture_File("Objects\\FX\\explosion_2.bmp", "Objects\\FX\\explosion_2_fa.bmp", 0);
	fx_explosion_3 = gx3d_InitTexture_File("Objects\\FX\\explosion_3.bmp", "Objects\\FX\\explosion_3_fa.bmp", 0);
	fx_laser_blue = gx3d_InitTexture_File("Objects\\FX\\laser_hit_blue.bmp", "Objects\\FX\\laser_hit_blue_fa.bmp", 0);
	fx_laser_red = gx3d_InitTexture_File("Objects\\FX\\laser_hit_red.bmp", "Objects\\FX\\laser_hit_red_fa.bmp", 0);
	fx_level_up = gx3d_InitTexture_File("Objects\\FX\\level_up.bmp", "Objects\\FX\\level_up_fa.bmp", 0);
	fx_destruct_shock = gx3d_InitTexture_File("Objects\\FX\\electricity_2.bmp", "Objects\\FX\\electricity_2_fa.bmp", 0);
	fx_destruct_charge = gx3d_InitTexture_File("Objects\\FX\\self_destruct_charge.bmp", "Objects\\FX\\self_destruct_charge_fa.bmp", 0);
	fx_destruct_charge_loop = gx3d_InitTexture_File("Objects\\FX\\self_destruct_charge_loop.bmp", "Objects\\FX\\self_destruct_charge_loop_fa.bmp", 0);
	fx_destruct_flash = gx3d_InitTexture_File("Objects\\FX\\self_destruct_flash.bmp", "Objects\\FX\\self_destruct_flash_fa.bmp", 0);
	fx_fade_white = gx3d_InitTexture_File("Objects\\FX\\fade_white.bmp", "Objects\\FX\\fade_white_fa.bmp", 0);

	//========== Load animations ==========//
	// read in the motion from a gx3dani file(faster than reading from an LWS file)
	ani_raiu_neutral = gx3d_Motion_Read_GX3DANI_File(raiu_skeleton, "ani\\raiu_neutral.gx3dani");
	ani_raiu_run = gx3d_Motion_Read_GX3DANI_File(raiu_skeleton, "ani\\raiu_run.gx3dani");
	ani_raiu_swing_1 = gx3d_Motion_Read_GX3DANI_File(raiu_skeleton, "ani\\raiu_swing_blade_1.gx3dani");
	ani_raiu_swing_1 = gx3d_Motion_Compute_Difference(ani_raiu_run, ani_raiu_swing_1);
	ani_raiu_swing_2 = gx3d_Motion_Read_GX3DANI_File(raiu_skeleton, "ani\\raiu_swing_blade_2.gx3dani");
	ani_raiu_swing_2 = gx3d_Motion_Compute_Difference(ani_raiu_run, ani_raiu_swing_2);
	ani_raiu_entrance = gx3d_Motion_Read_GX3DANI_File(raiu_skeleton, "ani\\raiu_game_start.gx3dani");
	ani_raiu_aim_up = gx3d_Motion_Read_GX3DANI_File(raiu_skeleton, "ani\\raiu_aim_up.gx3dani");
	ani_raiu_aim_up = gx3d_Motion_Compute_Difference(ani_raiu_neutral, ani_raiu_aim_up);
	ani_raiu_aim_down = gx3d_Motion_Read_GX3DANI_File(raiu_skeleton, "ani\\raiu_aim_down.gx3dani");
	ani_raiu_aim_down = gx3d_Motion_Compute_Difference(ani_raiu_neutral, ani_raiu_aim_down);
	ani_raiu_aim_left = gx3d_Motion_Read_GX3DANI_File(raiu_skeleton, "ani\\raiu_aim_left.gx3dani");
	ani_raiu_aim_left = gx3d_Motion_Compute_Difference(ani_raiu_neutral, ani_raiu_aim_left);
	ani_raiu_aim_right = gx3d_Motion_Read_GX3DANI_File(raiu_skeleton, "ani\\raiu_aim_right.gx3dani");
	ani_raiu_aim_right = gx3d_Motion_Compute_Difference(ani_raiu_neutral, ani_raiu_aim_right);
	ani_raiu_trip = gx3d_Motion_Read_GX3DANI_File(raiu_skeleton, "ani\\raiu_fall.gx3dani");
	ani_raiu_self_destruct = gx3d_Motion_Read_GX3DANI_File(raiu_skeleton, "ani\\raiu_self_destruct.gx3dani");

	//========== Setup animation blend tree ==========//
	// blend node for entrance animation
	bnode_entrance = gx3d_BlendNode_Init(raiu_skeleton, gx3d_BLENDNODE_TYPE_SINGLE); // create single input blending node (doesn't blend, just reads in)
	gx3d_Motion_Set_Output(ani_raiu_entrance, bnode_entrance, gx3d_BLENDNODE_TRACK_0);  // set output of animation to blending node (track 0)

	// blend node for running animation
	bnode_run = gx3d_BlendNode_Init(raiu_skeleton, gx3d_BLENDNODE_TYPE_SINGLE);
	gx3d_Motion_Set_Output(ani_raiu_run, bnode_run, gx3d_BLENDNODE_TRACK_0);

	// blend node for left-right aim - (0.0): Full left aim | (1.0) Full right aim
	bnode_aim_lr = gx3d_BlendNode_Init(raiu_skeleton, gx3d_BLENDNODE_TYPE_LERP2);
	gx3d_Motion_Set_Output(ani_raiu_aim_left, bnode_aim_lr, gx3d_BLENDNODE_TRACK_0);
	gx3d_Motion_Set_Output(ani_raiu_aim_right, bnode_aim_lr, gx3d_BLENDNODE_TRACK_1);

	// blend node for up-down aim - (0.0): Full up aim | (1.0) Full down aim
	bnode_aim_ud = gx3d_BlendNode_Init(raiu_skeleton, gx3d_BLENDNODE_TYPE_LERP2);
	gx3d_Motion_Set_Output(ani_raiu_aim_up, bnode_aim_ud, gx3d_BLENDNODE_TRACK_0);
	gx3d_Motion_Set_Output(ani_raiu_aim_down, bnode_aim_ud, gx3d_BLENDNODE_TRACK_1);

	// blend node for 2D aim [blends both up-down & left-right aim] - (0.0): Full up/down aim | (0.5): Half up/down & half left/right | (1.0): Full left/right aim
	bnode_aim_udlr = gx3d_BlendNode_Init(raiu_skeleton, gx3d_BLENDNODE_TYPE_LERP2);
	gx3d_BlendNode_Set_Output(bnode_aim_ud, bnode_aim_udlr, gx3d_BLENDNODE_TRACK_0);
	gx3d_BlendNode_Set_Output(bnode_aim_lr, bnode_aim_udlr, gx3d_BLENDNODE_TRACK_1);

	// blend node for blade swing - (0.0): Blade swing 1 | (1.0): Blade swing 2
	bnode_swing = gx3d_BlendNode_Init(raiu_skeleton, gx3d_BLENDNODE_TYPE_LERP2);
	gx3d_Motion_Set_Output(ani_raiu_swing_1, bnode_swing, gx3d_BLENDNODE_TRACK_0);
	gx3d_Motion_Set_Output(ani_raiu_swing_2, bnode_swing, gx3d_BLENDNODE_TRACK_1);

	// blend node for the character tripping at the beggining of game state game ending
	bnode_trip = gx3d_BlendNode_Init(raiu_skeleton, gx3d_BLENDNODE_TYPE_SINGLE);
	gx3d_Motion_Set_Output(ani_raiu_trip, bnode_trip, gx3d_BLENDNODE_TRACK_0);

	// blend node for the character self destruct animation
	bnode_self_destruct = gx3d_BlendNode_Init(raiu_skeleton, gx3d_BLENDNODE_TYPE_SINGLE);
	gx3d_Motion_Set_Output(ani_raiu_self_destruct, bnode_self_destruct, gx3d_BLENDNODE_TRACK_0);

	// Entrance animation blend tree
	btree_entrance = gx3d_BlendTree_Init(raiu_skeleton);			// create blendtree
	gx3d_BlendTree_Add_Node(btree_entrance, bnode_entrance);		// add blending node to the tree
	gx3d_BlendTree_Set_Output(btree_entrance, obj_raiu->layer);		// set output of tree to a model object layer (containing vertices to be animated)

	// Movement animation blend tree
	// Aim-Slash adder node
	bnode_adder_aim_swing = gx3d_BlendNode_Init(raiu_skeleton, gx3d_BLENDNODE_TYPE_ADD);
	gx3d_BlendNode_Set_Output(bnode_aim_udlr, bnode_adder_aim_swing, gx3d_BLENDNODE_TRACK_0);
	gx3d_BlendNode_Set_Output(bnode_swing, bnode_adder_aim_swing, gx3d_BLENDNODE_TRACK_1);

	// Run-Aim adder node
	bnode_adder_run_aim = gx3d_BlendNode_Init(raiu_skeleton, gx3d_BLENDNODE_TYPE_ADD);
	gx3d_BlendNode_Set_Output(bnode_run, bnode_adder_run_aim, gx3d_BLENDNODE_TRACK_0);
	gx3d_BlendNode_Set_Output(bnode_adder_aim_swing, bnode_adder_run_aim, gx3d_BLENDNODE_TRACK_1);

	// Add nodes to movement blend tree
	btree_movement = gx3d_BlendTree_Init(raiu_skeleton);
	gx3d_BlendTree_Add_Node(btree_movement, bnode_run);
	gx3d_BlendTree_Add_Node(btree_movement, bnode_aim_ud);
	gx3d_BlendTree_Add_Node(btree_movement, bnode_aim_lr);
	gx3d_BlendTree_Add_Node(btree_movement, bnode_aim_udlr);
	gx3d_BlendTree_Add_Node(btree_movement, bnode_swing);
	gx3d_BlendTree_Add_Node(btree_movement, bnode_adder_aim_swing);
	gx3d_BlendTree_Add_Node(btree_movement, bnode_adder_run_aim);
	gx3d_BlendTree_Set_Output(btree_movement, obj_raiu->layer);

	// Ending animation blend trees
	btree_trip = gx3d_BlendTree_Init(raiu_skeleton);
	gx3d_BlendTree_Add_Node(btree_trip, bnode_trip);
	gx3d_BlendTree_Set_Output(btree_trip, obj_raiu->layer);

	btree_self_destruct = gx3d_BlendTree_Init(raiu_skeleton);
	gx3d_BlendTree_Add_Node(btree_self_destruct, bnode_self_destruct);
	gx3d_BlendTree_Set_Output(btree_self_destruct, obj_raiu->layer);

	/*____________________________________________________________________
	|
	| create lights
	|___________________________________________________________________*/

	light_data.light_type = gx3d_LIGHT_TYPE_DIRECTION;
	light_data.direction.diffuse_color.r = 1;
	light_data.direction.diffuse_color.g = 1;
	light_data.direction.diffuse_color.b = 1;
	light_data.direction.diffuse_color.a = 0;
	light_data.direction.specular_color.r = 1;
	light_data.direction.specular_color.g = 1;
	light_data.direction.specular_color.b = 1;
	light_data.direction.specular_color.a = 0;
	light_data.direction.ambient_color.r = 0;
	light_data.direction.ambient_color.g = 0;
	light_data.direction.ambient_color.b = 0;
	light_data.direction.ambient_color.a = 0;
	light_data.direction.dst.x = -100;
	light_data.direction.dst.y = -100;
	light_data.direction.dst.z = -500;

	dir_light = gx3d_InitLight(&light_data);

	/*light_data.light_type = gx3d_LIGHT_TYPE_POINT;
	light_data.point.diffuse_color.r = 1;  // red light
	light_data.point.diffuse_color.g = 0.10;
	light_data.point.diffuse_color.b = 0.10;
	light_data.point.diffuse_color.a = 0;
	light_data.point.specular_color.r = 1;
	light_data.point.specular_color.g = 1;
	light_data.point.specular_color.b = 1;
	light_data.point.specular_color.a = 0;
	light_data.point.ambient_color.r = 0;  // ambient turned off
	light_data.point.ambient_color.g = 0;
	light_data.point.ambient_color.b = 0;
	light_data.point.ambient_color.a = 0;
	light_data.point.src.x = 0; // change position dynamically in the code
	light_data.point.src.y = 0; 
	light_data.point.src.z = 0;
	light_data.point.range = 100;
	light_data.point.constant_attenuation = 0.0; // change attenuation dynamically
	light_data.point.linear_attenuation = 0.0;
	light_data.point.quadratic_attenuation = 0.0;

	for (int i = 0; i < MAX_STRUCTURE_LIGHTS; i++)
		structure_light[i].light = gx3d_InitLight(&light_data);*/

	light_data.light_type = gx3d_LIGHT_TYPE_POINT;
	light_data.point.diffuse_color.r = 1;  // red light
	light_data.point.diffuse_color.g = 0.10;
	light_data.point.diffuse_color.b = 0.10;
	light_data.point.diffuse_color.a = 0;
	light_data.point.specular_color.r = 1;
	light_data.point.specular_color.g = 1;
	light_data.point.specular_color.b = 1;
	light_data.point.specular_color.a = 0;
	light_data.point.ambient_color.r = 0;  // ambient turned off
	light_data.point.ambient_color.g = 0;
	light_data.point.ambient_color.b = 0;
	light_data.point.ambient_color.a = 0;
	light_data.point.src.x = 0; // change position dynamically in the code
	light_data.point.src.y = 0;
	light_data.point.src.z = 0;
	light_data.point.range = 100;
	light_data.point.constant_attenuation = 0.0; // change attenuation dynamically
	light_data.point.linear_attenuation = 0.0;
	light_data.point.quadratic_attenuation = 0.0;

	explosion_light = gx3d_InitLight(&light_data);
}

/*____________________________________________________________________
|
| Function: Init_GameOverScreen
|
| Input: Called from Render_GameOverScreen
| Output:
|___________________________________________________________________*/

static void Init_GameOverScreen() {

	/*____________________________________________________________________
	|
	| Init 3D graphics
	|___________________________________________________________________*/

	// Set projection matrix
	gx3d_SetProjectionMatrix(FULL_SCREEN_FOV, FULL_SCREEN_NEAR_PLANE, FULL_SCREEN_FAR_PLANE);
	gx3d_SetFillMode(gx3d_FILL_MODE_GOURAUD_SHADED);

	// Sets the 3D viewport clear color to white
	color.r = 0;
	color.g = 0;
	color.b = 0;
	color.a = 0;

	/*____________________________________________________________________
	|
	| Initialize title screen sound library and load game over textures
	|___________________________________________________________________*/

	snd_Init(22, 16, 2, 1, 1);

	s_select = snd_LoadSound("wav\\menu_select.wav", snd_CONTROL_VOLUME, 0);

	if (score >= WINNING_SCORE) {
		s_game_over_bgm = snd_LoadSound("wav\\game_over_win.wav", snd_CONTROL_VOLUME, 0);
		tex_game_over_l = gx3d_InitTexture_File("Objects\\Images\\omega_thunder_game_over_win_1.bmp", 0, 0);
		tex_game_over_r = gx3d_InitTexture_File("Objects\\Images\\omega_thunder_game_over_win_2.bmp", 0, 0);
	}
	else {
		s_game_over_bgm = snd_LoadSound("wav\\game_over_lose.wav", snd_CONTROL_VOLUME, 0);
		tex_game_over_l = gx3d_InitTexture_File("Objects\\Images\\omega_thunder_game_over_lose_1.bmp", 0, 0);
		tex_game_over_r = gx3d_InitTexture_File("Objects\\Images\\omega_thunder_game_over_lose_2.bmp", 0, 0);
	}

	for (int i = 0; i < MAX_TIME_FONTS; i++) {
		gx3d_ReadLWO2File("Objects\\billboards.lwo", &obj_hr_fonts[i], gx3d_VERTEXFORMAT_DEFAULT, gx3d_DONT_LOAD_TEXTURES);
		gx3d_ReadLWO2File("Objects\\billboards.lwo", &obj_min_fonts[i], gx3d_VERTEXFORMAT_DEFAULT, gx3d_DONT_LOAD_TEXTURES);
		gx3d_ReadLWO2File("Objects\\billboards.lwo", &obj_sec_fonts[i], gx3d_VERTEXFORMAT_DEFAULT, gx3d_DONT_LOAD_TEXTURES);
	}

	for (int i = 0; i < MAX_DEFEATED_FONTS; i++)
		gx3d_ReadLWO2File("Objects\\billboards.lwo", &obj_defeated_fonts[i], gx3d_VERTEXFORMAT_DEFAULT, gx3d_DONT_LOAD_TEXTURES);

	fx_fade_white = gx3d_InitTexture_File("Objects\\FX\\fade_white.bmp", "Objects\\FX\\fade_white_fa.bmp", 0);
	
}

/*____________________________________________________________________
|
| Function: Render_TitleScreen
|
| Input: Called from Render_Game_Loop
| Output: 
|___________________________________________________________________*/

bool Render_TitleScreen(int *state) {

	/*____________________________________________________________________
	|
	| Main game loop
	|___________________________________________________________________*/

	// Variables
	int selection;
	bool selected, help_screen_enter_pressed;
	unsigned elapsed_time, last_time, new_time;

	// Init loop variables
	const int selection_count = 2; // change when adding more selections to title screen
	selection = 0;
	selected = false;
	help_screen_enter_pressed = false;
	
	// Setup title screen sounds
	snd_SetSoundVolume(s_title_screen_bgm, bgm_volume);
	snd_SetSoundVolume(s_select, sfx_volume);

	// Plays the background music repeatedly
	snd_PlaySound(s_title_screen_bgm, 1);

	// Game loop
	for (next_screen = FALSE; NOT next_screen || snd_IsPlaying(s_select); ) {

		/*____________________________________________________________________
		|
		| Process user input
		|___________________________________________________________________*/

		// Any event ready?
		if (evGetEvent(&event)) {
			// key press?
			if (event.type == evTYPE_RAW_KEY_PRESS) {
				// If ESC pressed, exit the program
				if (event.keycode == evKY_ESC)
					return true;
				if (!snd_IsPlaying(s_select)) {
					switch (*state) {
					case STATE_TITLE_SCREEN: // Title Screen Controls
						if (event.keycode == evKY_UP_ARROW)
							selection = (selection + 1) % selection_count;
						else if (event.keycode == evKY_DOWN_ARROW)
							selection = abs((selection - 1) % selection_count);
						else if (event.keycode == evKY_ENTER) {
							selected = true;
							snd_PlaySound(s_select, 0);
							// open selected menu
							switch (selection) {
							case 0: // Start Game - open help screen before starting game
								snd_StopSound(s_title_screen_bgm);
								*state = STATE_HELP_SCREEN;
								break;
							case 1: // Quit Game - exits the game
								return true;
								break;
							case 2: // (0) start game, (1) options, (2) quit game (Future Updates)
								break;
							}
						}
						break;

					case STATE_HELP_SCREEN: // Help Screen Controls
						if (event.keycode == evKY_ENTER) {
							snd_PlaySound(s_select, 0);
							help_screen_enter_pressed = true;
						}
						break;
					}
				}
			}
		}

		// start game after the sound effect when enter is pressed at the help screen
		if (help_screen_enter_pressed && !snd_IsPlaying(s_select)) {
			*state = STATE_STARTING;
			next_screen = true;
		}

		/*____________________________________________________________________
		|
		| Draw graphics
		|___________________________________________________________________*/

		// Displays loading screen when getting ready to switch screens
		if (next_screen)
			Display_LoadingScreen();

		else {
			// Clear viewport
			gx3d_ClearViewport(gx3d_CLEAR_SURFACE | gx3d_CLEAR_ZBUFFER, color, gx3d_MAX_ZBUFFER_VALUE, 0);
			// Start rendering in 3D
			if (gx3d_BeginRender()) {
				// Set the default light
				gx3d_SetAmbientLight(color3d_white);
				// Set the default material
				gx3d_SetMaterial(&material_default);
				/*____________________________________________________________________
				|
				| Draw 2D graphics on top of 3D
				|___________________________________________________________________*/

				// Save Current view matrix
				gx3dMatrix view_save;
				gx3d_GetViewMatrix(&view_save);

				// Set new view matrix
				gx3dVector tfrom = { 0,0,-2 }, tto = { 0,0,0 }, twup = { 0,1,0 };
				gx3d_CameraSetPosition(&tfrom, &tto, &twup, gx3d_CAMERA_ORIENTATION_LOOKTO_FIXED);
				gx3d_CameraSetViewMatrix();

				// Do necessary transformations to fit the title screen objects on screen
				if (*state == STATE_TITLE_SCREEN) {
					const float button_scale = 0.17;

					layer = gx3d_GetObjectLayer(obj_billboards, "menu_button");
					gx3d_GetScaleMatrix(&m1, button_scale, button_scale, button_scale);
					gx3d_GetTranslateMatrix(&m2, -0.705, -0.078, 0);
					gx3d_MultiplyMatrix(&m1, &m2, &m1);
					gx3d_SetObjectLayerMatrix(obj_billboards, layer, &m1);

					layer = gx3d_GetObjectLayer(obj_quit_button, "menu_button");
					gx3d_GetScaleMatrix(&m1, button_scale, button_scale, button_scale);
					gx3d_GetTranslateMatrix(&m2, -0.705, -0.23, 0);
					gx3d_MultiplyMatrix(&m1, &m2, &m1);
					gx3d_SetObjectLayerMatrix(obj_quit_button, layer, &m1);

					// Update transforms
					gx3d_Object_UpdateTransforms(obj_billboards);
					gx3d_Object_UpdateTransforms(obj_quit_button);
				}

				// Draw the appropriate objects on screen
				gx3d_DisableZBuffer();
				gx3d_EnableAlphaBlending();

				// Title Screen Objects
				if (*state == STATE_TITLE_SCREEN) {
					layer = gx3d_GetObjectLayer(obj_billboards, "full_screen_l");
					gx3d_SetTexture(0, tex_title_screen_l);
					gx3d_DrawObjectLayer(layer, 0);

					layer = gx3d_GetObjectLayer(obj_billboards, "full_screen_r");
					gx3d_SetTexture(0, tex_title_screen_r);
					gx3d_DrawObjectLayer(layer, 0);

					// Draw elements that requires the use of texture matrix (changing of UV texture coords)
					gx3d_EnableTextureMatrix(0);

					//=============== Start Game Button ===============//
					layer = gx3d_GetObjectLayer(obj_billboards, "menu_button");
					if (selection == 0) {
						if (selected) {
							gx3d_GetTranslateTextureMatrix(&m, 0, 1); // upper half of texture coords
							selected = false; // blinks selection
						}
						else if (!snd_IsPlaying(s_select)) {
							gx3d_GetTranslateTextureMatrix(&m, 0, 0.5); // lower half of texture coords
						}
					}
					else
						gx3d_GetTranslateTextureMatrix(&m, 0, 1); // upper half of texture coords

					gx3d_SetTextureMatrix(0, &m);
					gx3d_SetTexture(0, tex_button_start_game);
					gx3d_DrawObjectLayer(layer, 0);

					//=============== Quit Game Button ===============//
					layer = gx3d_GetObjectLayer(obj_quit_button, "menu_button");
					if (selection == 1) {
						if (selected) {
							gx3d_GetTranslateTextureMatrix(&m, 0, 1); // upper half of texture coords
							selected = false; // blinks selection
						}
						else if (!snd_IsPlaying(s_select)) {
							gx3d_GetTranslateTextureMatrix(&m, 0, 0.5); // lower half of texture coords
						}
					}
					else
						gx3d_GetTranslateTextureMatrix(&m, 0, 1); // upper half of texture coords

					gx3d_SetTextureMatrix(0, &m);
					gx3d_SetTexture(0, tex_button_quit_game);
					gx3d_DrawObjectLayer(layer, 0);

					gx3d_DisableTextureMatrix(0);
				}

				// Help Screen Objects
				else {
					layer = gx3d_GetObjectLayer(obj_billboards, "help_screen");
					gx3d_SetTexture(0, tex_help_screen);
					gx3d_DrawObjectLayer(layer, 0);
				}
				gx3d_DisableAlphaBlending();
				gx3d_EnableZBuffer();

				// Restore view matrix
				gx3d_SetViewMatrix(&view_save);

				// Stop rendering
				gx3d_EndRender();

				// Page flip (so user can see it)
				gxFlipVisualActivePages(FALSE);
			}
		}
	}
	Free_TitleScreen();

	return false; // continue to next screen
}

/*____________________________________________________________________
|
| Function: Render_GameScreen
|
| Input: Called from Render_Game_Loop
| Output:
|___________________________________________________________________*/

bool Render_GameScreen(int *state) {

	/*____________________________________________________________________
	|
	| Main game loop
	|___________________________________________________________________*/

	// Variables
	unsigned elapsed_time, last_time, new_time;
	bool force_update, key_changed;
	unsigned cmd_move;
	float hp_bar_x_factor;
	float ani_raiu_run_time, ani_raiu_entrance_time, ani_raiu_ending_time, entrance_delay_timer, entrance_delay_limit;
	unsigned raiu_laser_delay_timer, raiu_laser_delay_limit, hoshu_laser_delay_timer, hoshu_laser_delay_limit;
	unsigned game_ending_speed_timer, enemy_spawn_timer, spawn_timer_limit, heal_spawn_timer, heal_spawn_timer_limit;
	bool speed_initialized, sfx_initialized, structure_created;
	int hoshu_lv, current_max_enemy_count, enemy_count;
	float speed, spd_multiplier, distance, ground_init_z, ground_1_z, ground_2_z;
	float structure_interval, spawn_structure_distance, structure_spawn_chance, floor_light_distance, floor_light_interval;
	int structure_index;
	int current_aim_x, current_aim_y;
	float aim_x, aim_y;
	float current_bgm_volume;
	float explode_snd_min_distance, explode_snd_max_distance, laser_snd_min_distance, laser_snd_max_distance, fence_snd_min_distance, fence_snd_max_distance;
	float raiu_laser_speed, hoshu_laser_speed, hoshu_laser_scale;
	float enemy_spawn_chance, heal_spawn_chance;
	int raiu_levels[MAX_LV], hoshu_levels[MAX_LV];
	bool play_swing_1, play_swing_2, blade_active;
	float swing_type, swing_active, level_up_fx_timer, heal_fx_timer;
	float lerp_speed_duration, camera_lerp_duration;
	gx3dVector billboard_normal, hoshu_normal, hoshu_view;
	bool pause, snd_paused, restored, update_once;

	// USED FOR DEBUGGING
	bool stop = false; 


	//========== Initial loop parameters ==========//
	// Game variables
	cmd_move =					0;
	last_time =					0;
	force_update =				false;
	sfx_volume =				90.0f;
	current_bgm_volume =		60.0f;
	distance =					0; // the amount of distance traveled based on the elapsed time (amount of shift performed in the world)
	speed =						NORMAL_SPEED;
	spd_multiplier =			1;
	swing_type =				0.0;
	score =						0;
	current_aim_x =				0;
	current_aim_y =				0;
	enemies_defeated =			0; // number of enemies defeated so far
	hoshus_defeated =			0; // number of Hoshus defeated so far
	current_max_enemy_count =	MAX_ENEMY_COUNT / MAX_LV; // Increases depending on the max number of levels
	enemy_count =				0; // number of enemies currently spawned
	enemy_spawn_chance =		0.05f; // 5% chance of spawning an enemy for every loop after the spawn cooldown timer expires
	heal_spawn_chance =			0.0001f; // hp > 75%: 0.01% chance of spawning for every loop || hp < 75%: 1% chance of spawning an electric fence for every loop				

	// Vectors
	billboard_normal =			{ 0,0,-1 };
	hoshu_normal =				{ 0,0,-1 }; // normal view vector of the Hoshu model

	// Timers
	game_timer =				0.0f; // amount of time since the game has started
	ani_raiu_run_time =			-1;
	ani_raiu_entrance_time =	-1;
	ani_raiu_ending_time =		-1;
	game_ending_speed_timer =	0; // timer for the game ending state (used for "lerp-ing" the speed at the beginning of the state)
	lerp_speed_duration =		2000; // duration of the game_ending_speed_timer
	camera_lerp_duration =		500; // 0.5 seconds
	entrance_delay_limit =		1000.0f; // 1 second entrance animation delay
	entrance_delay_timer =		0;
	raiu_laser_delay_limit =	250.0f; // 0.25 seconds
	hoshu_laser_delay_limit =	1000.0f; // 1 second enemy laser delay	
	spawn_timer_limit =			1000.0; // 1 second until able to spawn an object
	enemy_spawn_timer =			spawn_timer_limit; // able to spawn an enemy after the game starts
	heal_spawn_timer_limit =	60000; // 1 minute cooldown timer for an electric fence to spawn
	heal_spawn_timer =			0;
	level_up_fx_timer =			-1; // timer for the level up effect when the character levels up
	heal_fx_timer = 			-1; // tiemr for the heal effect when the character regains health

	// Booleans
	sfx_initialized =			false;
	play_swing_1 =				false;
	play_swing_2 =				false;
	blade_active =				false; // is the character swinging his blade?
	speed_initialized =			false;
	pause =						false; // is the game paused?
	key_changed =				false; // is a key pressed while paused?
	snd_paused =				false; // was a sound paused?
	restored =					false; // was the game restored after context switching?
	update_once =				false; // helper variable when restoring after context switching that ensures that the world updates once then pausing
	
	// World Parameters
	ground_init_z =				-200;
	ground_1_z =				ground_init_z;
	ground_2_z =				ground_1_z + MAX_GROUND_LENGTH;
	spawn_structure_distance =	0; // total distance traveled since the last spawned structure
	structure_interval =		1000; // distance in feet that needs to be traveled before spawning a new structure
	structure_spawn_chance =	0.10; // 10% spawn chance for a structure to spawn on every loop where spawn structure distance >= structure interval
	structure_index =			0; // current index of the next structure element in the world structure array
	//floor_light_distance =		0;
	//floor_light_interval =		200;
	explode_snd_min_distance =	1000;
	explode_snd_max_distance =	3000;
	laser_snd_min_distance =	1500;
	laser_snd_max_distance =	3000;
	fence_snd_min_distance =	500;
	fence_snd_max_distance =	2000;

	for (int i = 0; i < MAX_STRUCTURE_COUNT; i++)
		structure[i].spawned = false; // world structures are updated and set to default values when being spawned dynamically

	// Character Parameters
	raiu.sphere =				obj_raiu->bound_sphere;
	raiu.hp =					RAIU_MAX_HP;
	raiu.blade_lv =				1;
	raiu.blade_damage =			40 * raiu.blade_lv; // 10 blade levels : {40, 80, 120, 160, 200, 240, 280, 320, 360, 400}
	raiu.blade_delay =			750; // each swings last for 0.75 second
	raiu.blade_timer =			-1; // timer for blade swing (deactivated when value is at -1)
	raiu.gun_lv =				1;
	raiu.gun_damage =			25 * raiu.gun_lv; // 10 gun levels : {25, 50, 75, 100, 125, 150, 175, 200, 225, 250}
	raiu.gun_delay =			raiu_laser_delay_limit; // 0.5 sec shooting delay
	raiu.gun_timer =			raiu.gun_delay; // start off being able to shoot
	raiu.exp =					0;
	raiu.laser_index =			0;
	raiu.light =				gx3d_InitLight(&light_data);
	for (int i = 0; i < RAIU_MAX_LASER_COUNT; i++)
		raiu.laser[i].draw =	false;
	raiu_laser_speed =			1000.0; // 1000 ft per second
	
	// Enemy Parameters
	hoshu_lv =					1;
	for (int i = 0; i < MAX_ENEMY_COUNT; i++) {
		enemies.hoshu[i].sphere =						obj_hoshu->bound_sphere;
		enemies.hoshu[i].lv =							hoshu_lv;
		enemies.hoshu[i].hp =							100 * enemies.hoshu[i].lv; // 10 hp levels : {100, 200, 300, 400, 500, 600, 700, 800, 900, 1000}
		enemies.hoshu[i].score_amt =					200 * enemies.hoshu[i].lv; // 10 score levels : {200, 400, 600, 800, 1000, 1200, 1400, 1600, 1800, 2000}
		enemies.hoshu[i].exp_amt =						100 * enemies.hoshu[i].lv; // 10 exp levels : {100, 200, 300, 400, 500, 600, 700, 800, 900, 1000}
		enemies.hoshu[i].gun_delay =					hoshu_laser_delay_limit; // 1 sec shooting delay
		enemies.hoshu[i].gun_timer =					enemies.hoshu[i].gun_delay; // Hoshu starts off being able to shoot
		enemies.hoshu[i].gun_damage =					10 * enemies.hoshu[i].lv; // 10 gun levels : {10, 20, 30, 40, 50, 60, 70, 80, 90, 100}
		enemies.hoshu[i].laser_index =					0;
		enemies.hoshu[i].explosion_timer =				-1; // timer is off when set to -1
		enemies.hoshu[i].blade_mark_1 =					false;
		enemies.hoshu[i].blade_mark_2 =					false;
		enemies.hoshu[i].explode_snd_initialized =		false;
		enemies.hoshu[i].draw =							false; // Hoshu positions are updated and set to default values dynamically 
		for (int j = 0; j < HOSHU_MAX_LASER_COUNT; j++)
			enemies.hoshu[i].laser[j].draw =			false; // Hoshu lasers are updated and set to default values dynamically
	}
	enemies.hoshu_index =		0;
	hoshu_laser_speed =			750.0; // 750 ft per second (slower than the character's laser speed to give the players time to react)
	hoshu_laser_scale =			3;

	// Level Up Parameters
	for (int i = 0; i < MAX_LV; i++) {
		if (i == 0) {
			raiu_levels[i] =	1000; // Raiu first levels up after gaining 1000 exp
			hoshu_levels[i] =	20; // Hoshus first levels up after defeating 20 of them
		}
		else {
			raiu_levels[i] =	raiu_levels[i - 1] * 2; // 2x exponential increase in experience points required : {1000, 2000, 4000, 8000, 16000, 32000, 64000, 128000, 256000, 512000}
			hoshu_levels[i] =	hoshu_levels[i - 1] * 2; // 4x exponential increase in number of Hoshus defeted until Hoshus level : {25, 50, 100, 200, 400, 800, 1600, 3200, 6400, 12800}
		}
	}

	// Health Pad (Electric Fence) Parameters
	heal_pad.pos =				{ 0, 0, 4000 }; // position of the electric fence in the world (changes dynamically)
	heal_pad.heal_amt =			RAIU_MAX_HP * 0.25; // electric fence heal 25% of max health
	heal_pad.ps_enable =		heal_pad.draw; // only enable the particle system when the health pad is drawn
	heal_pad.light =			gx3d_InitLight(&light_data);

	// Setup all sound effects
	snd_SetSoundMode(s_explosion_1, snd_3D_MODE_ORIGIN_RELATIVE, snd_3D_APPLY_NOW);
	snd_SetSoundMode(s_explosion_2, snd_3D_MODE_ORIGIN_RELATIVE, snd_3D_APPLY_NOW);
	snd_SetSoundMode(s_explosion_3, snd_3D_MODE_ORIGIN_RELATIVE, snd_3D_APPLY_NOW);
	snd_SetSoundMode(s_laser_2, snd_3D_MODE_ORIGIN_RELATIVE, snd_3D_APPLY_NOW);
	snd_SetSoundMode(s_electric_fence, snd_3D_MODE_ORIGIN_RELATIVE, snd_3D_APPLY_NOW); 
	snd_SetSoundMinDistance(s_explosion_1, explode_snd_min_distance, snd_3D_APPLY_NOW);
	snd_SetSoundMinDistance(s_explosion_2, explode_snd_min_distance, snd_3D_APPLY_NOW);
	snd_SetSoundMinDistance(s_explosion_3, explode_snd_min_distance, snd_3D_APPLY_NOW);
	snd_SetSoundMinDistance(s_laser_2, laser_snd_min_distance, snd_3D_APPLY_NOW);
	snd_SetSoundMinDistance(s_electric_fence, fence_snd_min_distance, snd_3D_APPLY_NOW);
	snd_SetSoundMaxDistance(s_explosion_1, explode_snd_max_distance, snd_3D_APPLY_NOW);
	snd_SetSoundMaxDistance(s_explosion_2, explode_snd_max_distance, snd_3D_APPLY_NOW);
	snd_SetSoundMaxDistance(s_explosion_3, explode_snd_max_distance, snd_3D_APPLY_NOW);
	snd_SetSoundMaxDistance(s_laser_2, laser_snd_max_distance, snd_3D_APPLY_NOW);
	snd_SetSoundMaxDistance(s_electric_fence, fence_snd_max_distance, snd_3D_APPLY_NOW);
	snd_SetSoundVolume(&s_explosion_1, sfx_volume);
	snd_SetSoundVolume(&s_explosion_2, sfx_volume);
	snd_SetSoundVolume(&s_explosion_3, sfx_volume);
	snd_SetSoundVolume(s_blade_1, sfx_volume);
	snd_SetSoundVolume(s_blade_2, sfx_volume);
	snd_SetSoundVolume(s_laser_1, sfx_volume);
	snd_SetSoundVolume(s_laser_2, sfx_volume);
	snd_SetSoundVolume(s_electric_fence, sfx_volume);
	snd_SetSoundVolume(s_footstep, sfx_volume * 0.95f);
	snd_SetSoundVolume(s_raiu_hurt_1, sfx_volume);
	snd_SetSoundVolume(s_raiu_hurt_2, sfx_volume);
	snd_SetSoundVolume(s_raiu_grunt_1, sfx_volume);
	snd_SetSoundVolume(s_raiu_nice, sfx_volume);
	snd_SetSoundVolume(s_starting, sfx_volume);
	snd_SetSoundVolume(s_enemy_lv_up, sfx_volume);
	snd_SetSoundVolume(s_lv_up, sfx_volume);
	snd_SetSoundVolume(s_ending, sfx_volume);
	snd_SetSoundVolume(s_game_bgm, current_bgm_volume);

	// Plays the background music repeatedly
	snd_PlaySound(s_game_bgm, 1);

	// Game loop
	for (next_screen = FALSE; NOT next_screen; ) {

		/*____________________________________________________________________
		|
		| Update clock and timers
		|___________________________________________________________________*/

		// Get the current time (# milliseconds since the game started)
		new_time = timeGetTime();

		if (pause) { // elapsed time should equal to 0 while paused
			last_time = 0;
		}

		// Update the world once after being restored to set the camera back behind the character
		if (restored) {
			if (*state == STATE_STARTING) {
				if (pause)
					pause = false;
			}
			else if (*state == STATE_RUNNING) {
				update_once = true;
				last_time = 0;
			}
		}

		// Compute the elapsed time (in milliseconds) since the last time through this loop
		if (last_time == 0)
			elapsed_time = 0;
		else
			elapsed_time = new_time - last_time;
		last_time = new_time;

		// Update gameplay timer
		if (*state == STATE_RUNNING) {
			game_timer += elapsed_time;
			// makes sure that the gameplay timer does not exceed 99 hours 59 minutes 59 seconds
			if (game_timer > MAX_GAME_TIME)
				game_timer = MAX_GAME_TIME;
		}
		else if (*state == STATE_GAME_ENDING)
			game_ending_speed_timer += elapsed_time;

		// Update spawn timers
		if (enemy_spawn_timer < spawn_timer_limit)
			enemy_spawn_timer += elapsed_time;

		// Update cooldown timers
		if (raiu.gun_timer < raiu.gun_delay)
			raiu.gun_timer += elapsed_time;
		for (int i = 0; i < MAX_ENEMY_COUNT; i++)
			if (enemies.hoshu[i].draw)
				if (enemies.hoshu[i].gun_timer < enemies.hoshu[i].gun_delay)
					enemies.hoshu[i].gun_timer += elapsed_time;

		// Update health pad spawn timer when timer is < its limit
		if (heal_spawn_timer < heal_spawn_timer_limit) {
			heal_spawn_timer += elapsed_time;
		}

		/*____________________________________________________________________
		|
		| Update camera view
		|___________________________________________________________________*/

		// Update camera only when unpaused
		if (!pause) {

			bool position_changed, camera_changed;
			if (*state == STATE_RUNNING)
				Position_Update(elapsed_time, cmd_move, move_y, move_x, force_update,
					&position_changed, &camera_changed, &position, &heading, &current_aim_y, &current_aim_x);
			else if (*state == STATE_GAME_ENDING) {

				// Ensures that the lerp stops after the timer had reached the duration limit
				if ((camera_lerp_duration - game_ending_speed_timer) > 0)
					Position_Lerp_Camera_Start(game_ending_speed_timer, camera_lerp_duration, &heading);
				else
					Position_Lerp_Camera_Start(camera_lerp_duration, camera_lerp_duration, &heading);

				// Lerp the camera back to its starting position
				Position_Update(elapsed_time, cmd_move, 0, 0, force_update,
					&position_changed, &camera_changed, &position, &heading, &current_aim_y, &current_aim_x);
			}
			else
				Position_Update(elapsed_time, cmd_move, 0, 0, force_update,
					&position_changed, &camera_changed, &position, &heading, &current_aim_y, &current_aim_x);

			// Update sound listener position and orientation
			snd_SetListenerPosition(position.x, position.y, position.z, snd_3D_APPLY_NOW);
			snd_SetListenerOrientation(heading.x, heading.y, heading.z, 0, 1, 0, snd_3D_APPLY_NOW);
		}

		/*____________________________________________________________________
		|
		| Update world variables
		|___________________________________________________________________*/

		// update speed
		if (*state == STATE_STARTING && !speed_initialized) {
			speed = 0;
			speed_initialized = true;
		}
		else if (*state == STATE_RUNNING)
			speed = NORMAL_SPEED * spd_multiplier;
		else if (*state == STATE_GAME_ENDING) {

			// Speed gradually slows down at the beginning of game state game ending
			if (spd_multiplier != 1) {
				spd_multiplier = 1;
				speed = NORMAL_SPEED;
			}

			if (lerp_speed_duration >= game_ending_speed_timer)
				speed = Inverse_Lerp(NORMAL_SPEED, 0, (lerp_speed_duration - game_ending_speed_timer) / lerp_speed_duration);
			else
				speed = 0;
		}

		// distance travelled = ([speed -> (distance in feet / time in seconds)] * speed multiplier) * (elapsed time in milliseconds to seconds)
		distance = (speed * spd_multiplier) * (elapsed_time / 1000.0f);

		// update the character's position and heading
		raiu.pos = position;
		raiu.view = heading;

		// update structures spawn distance
		spawn_structure_distance += distance;

		// update levels
		if (raiu.exp >= raiu_levels[raiu.blade_lv] || raiu.exp >= raiu_levels[raiu.gun_lv]) {
			// Update character parameters
			raiu.blade_lv++;
			raiu.blade_damage = 50 * raiu.blade_lv;
			raiu.gun_lv++;
			raiu.gun_damage = 25 * raiu.gun_lv;
			raiu.gun_delay = raiu_laser_delay_limit; // 0.5 sec shooting delay
			raiu.gun_timer = raiu.gun_delay; // start off being able to shoot
			raiu.exp = 0;

			// Activate the level up effects timer
			level_up_fx_timer = 0;

			// Increase max number of enemies that can appear on the screen
			current_max_enemy_count += MAX_ENEMY_COUNT / MAX_LV;
		}
		if (hoshus_defeated >= hoshu_levels[hoshu_lv]) {
			hoshu_lv++;
			hoshus_defeated = 0;

			// Play enemy level up sound effect
			snd_PlaySound(s_enemy_lv_up, 0);
		}

		// update health pad spawn chance depending on the character's health
		if (raiu.hp > RAIU_MAX_HP * 0.75) // 0.01% chance spawn rate for every loop after the cooldown timer expires
			heal_spawn_chance = 0.0001;
		else // 1% chance spawn rate for every loop after the cooldown timer expires
			heal_spawn_chance = 0.01;

		// set movement and the speed multiplier to default if key press changes are made after pausing
		if (!pause && key_changed) {
			cmd_move = 0;
			spd_multiplier = 1;
			snd_ResetSoundFrequency(s_footstep);
			spawn_timer_limit = 1000;
			key_changed = false;
		}

		/*____________________________________________________________________
		|
		| Process user input
		|___________________________________________________________________*/

		// Any event ready?
		if (evGetEvent(&event)) {

			if (event.type == evTYPE_WINDOW_INACTIVE) {
				RESTORE_PROGRAM
				pause = false;
				restored = true;
				// Flush mouse movement counters
				msGetMouseMovement(&move_x, &move_y);
			}

			// key press?
			if (event.type == evTYPE_RAW_KEY_PRESS) {
				// If ESC pressed, exit the program
				if (event.keycode == evKY_ESC)
					return true;
				if (event.keycode == evKY_F1)
					take_screenshot = true;
				switch (*state) {
				case STATE_STARTING: // Disable controls until animation finishes
				case STATE_GAME_ENDING:
					break;

				case STATE_RUNNING: // Game play controls
					if (event.keycode == 'f') {
						if (pause)
							pause = false;
						else
							pause = true;
					}
					/* DEBUGGING : t - key to set the stop variable to true*/
					/*else if (event.keycode == 't')
						stop = true;*/

					// Only update movement inputs when unpaused
					else if (!pause) {

						if (event.keycode == 'w') {
							spd_multiplier = 1.75;
							snd_SetSoundFrequency(s_footstep, (snd_GetSoundFrequency(s_footstep)) * 1.75);
							spawn_timer_limit /= 2;
						}
						else if (event.keycode == 's') {
							spd_multiplier = 0.75;
							snd_SetSoundFrequency(s_footstep, (snd_GetSoundFrequency(s_footstep)) * 0.75);
							spawn_timer_limit *= 2;
						}

						if (event.keycode == 'a')
							cmd_move |= POSITION_MOVE_LEFT;
						else if (event.keycode == 'd')
							cmd_move |= POSITION_MOVE_RIGHT;
					}
					// Indicate that the currently pressed keys are changed while paused
					else {
						if (event.keycode == 'w' || event.keycode == 's' || event.keycode == 'a' || event.keycode == 'd') {
							key_changed = true;
						}
					}
					break;
				}
			}

			// key release?
			else if (event.type == evTYPE_RAW_KEY_RELEASE) {
				if (*state == STATE_RUNNING) {
					
					// Only update movement inputs when unpaused
					if (!pause) {
						if (event.keycode == 'w') {
							spd_multiplier = 1;
							snd_ResetSoundFrequency(s_footstep);
							spawn_timer_limit *= 2;
						}
						else if (event.keycode == 's') {
							spd_multiplier = 1;
							snd_ResetSoundFrequency(s_footstep);
							spawn_timer_limit /= 2;
						}
						else if (event.keycode == 'a')
							cmd_move &= ~(POSITION_MOVE_LEFT);
						else if (event.keycode == 'd')
							cmd_move &= ~(POSITION_MOVE_RIGHT);
					}
					// Indicate that the currently pressed keys are changed while paused
					else {
						if (event.keycode == 'w' || event.keycode == 's' || event.keycode == 'a' || event.keycode == 'd') {
							key_changed = true;
						}
					}
				}
			}

			// Mouse press?
			else if (event.type == evTYPE_MOUSE_LEFT_PRESS) {
				if (*state == STATE_RUNNING) {
					// Only read mouse inputs when unpaused
					if (!pause) {

						// Initialize local variables
						float angle;

						// Shoot a laser beam when cooldown timer for shooting the ray gun has expired
						if (raiu.gun_timer >= raiu.gun_delay) {
							if (!(raiu.laser[raiu.laser_index].draw)) {
								// Reset the cooldown timer
								raiu.gun_timer = 0;

								// Play laser beam sound effect
								snd_PlaySound(s_laser_1, 0);

								// Initialize bounding sphere
								raiu.laser[raiu.laser_index].sphere = obj_laser->bound_sphere;

								// Set laser position and trajectory
								raiu.laser[raiu.laser_index].pos = raiu.sphere.center;
								raiu.laser[raiu.laser_index].pos.x += 1.0; // shift 1.0 ft to the right since the gun is on the right side of the camera view (NOT ALWAYS THE CASE SUCH AS WHEN TURNING LEFT OR RIGHT!.. but close enough)
								raiu.laser[raiu.laser_index].sphere.center = raiu.laser[raiu.laser_index].pos;
								raiu.laser[raiu.laser_index].trajectory.direction = raiu.view;
								raiu.laser[raiu.laser_index].trajectory.velocity = raiu_laser_speed;

								// Indicate that the laser should be drawn in the world
								raiu.laser[raiu.laser_index].draw = true;

								// Update to the next shootable laser
								raiu.laser_index = (raiu.laser_index + 1) % RAIU_MAX_LASER_COUNT;
							}
						}
					}
				}
			}

			else if (event.type == evTYPE_MOUSE_RIGHT_PRESS) {
				if (*state == STATE_RUNNING) {
					// Only read mouse inputs when unpaused
					if (!pause) {

						// Activate swing timer if not yet activated
						if (raiu.blade_timer == -1) {
							raiu.blade_timer = 0;
							swing_type = 0.0;
							snd_PlaySound(s_blade_1, 0);
							play_swing_1 = true;
						}
						if (play_swing_1) {

							// Activate blade swing 2 when the key is pressed again after a certain time window
							if (raiu.blade_timer >= 150 && raiu.blade_timer <= 350) {
								if (!play_swing_2) {
									swing_type = 1.0;
									snd_PlaySound(s_blade_2, 0);
									snd_PlaySound(s_raiu_grunt_1, 0);
									play_swing_2 = true;
								}
							}
						}
					}
				}
			}
		}

		// Check for camera movement (via mouse) - flushes when game state is not "Running"
		msGetMouseMovement(&move_x, &move_y);

		/*____________________________________________________________________
		|
		| Draw graphics
		|___________________________________________________________________*/

		gx3d_SetFogColor(50, 50, 100);
		gx3d_SetLinearPixelFog(2000, 3000);

		// Clear viewport
		gx3d_ClearViewport(gx3d_CLEAR_SURFACE | gx3d_CLEAR_ZBUFFER, color, gx3d_MAX_ZBUFFER_VALUE, 0);

		if (pause && elapsed_time > 0)
			continue;

		// Start rendering in 3D
		if (gx3d_BeginRender()) {

			// Set the default light
			gx3d_SetAmbientLight(color3d_white);
			// Set the default material
			gx3d_SetMaterial(&material_structures);

			// Enable specular lighting for all objects
			gx3d_EnableSpecularLighting();

			// Enable Alpha blending
			gx3d_EnableAlphaBlending();

			// Enable Z Buffer
			gx3d_EnableZBuffer();
			gx3d_DisableFog();

			/*____________________________________________________________________
			|
			| Draw 3D environment
			|___________________________________________________________________*/

			// Set the far plane to the game far plane
			gx3d_SetProjectionMatrix(GAME_FOV, GAME_NEAR_PLANE, GAME_FAR_PLANE);

			// Skydome - space
			layer = gx3d_GetObjectLayer(obj_skydome, "skydome");
			gx3d_GetTranslateMatrix(&m, raiu.pos.x, raiu.pos.y, raiu.pos.z);
			gx3d_SetObjectLayerMatrix(obj_skydome, layer, &m);
			gx3d_Object_UpdateTransforms(obj_skydome);
			gx3d_SetTexture(0, tex_skydome);
			gx3d_DrawObjectLayer(layer, 0);

			// Skydome - earth
			layer = gx3d_GetObjectLayer(obj_skydome, "earth");
			gx3d_SetTexture(0, tex_earth);
			gx3d_DrawObjectLayer(layer, 0);

			// Set lighting for the ground plane and structures
			gx3d_SetAmbientLight(color3d_darkgray);
			gx3d_EnableLight(dir_light);
			
			// Set the far plane to a shorter distance than the game far plane to show the other objects inside the skydome
			gx3d_SetProjectionMatrix(GAME_FOV, GAME_NEAR_PLANE, 3500);

			// Enable fog for the objects inside the skydome
			gx3d_EnableFog();

			// Translates the ground depending on the speed
			// If the end of a ground object reaches 100 ft back from the character, it moves back to the end of the other ground object
			if (ground_1_z <= (ground_init_z - MAX_GROUND_LENGTH)) {
				gx3d_GetTranslateMatrix(&m2, 0, 0, ground_2_z -= distance);
				gx3d_GetTranslateMatrix(&m1, 0, 0, ground_1_z = ground_2_z + MAX_GROUND_LENGTH);
			}
			else if (ground_2_z <= (ground_init_z - MAX_GROUND_LENGTH)) {
				gx3d_GetTranslateMatrix(&m1, 0, 0, ground_1_z -= distance);
				gx3d_GetTranslateMatrix(&m2, 0, 0, ground_2_z = ground_1_z + MAX_GROUND_LENGTH);
			}
			else {
				gx3d_GetTranslateMatrix(&m1, 0, 0, ground_1_z -= distance);
				gx3d_GetTranslateMatrix(&m2, 0, 0, ground_2_z -= distance);
			}

			// Set material for the ground object
			gx3d_SetMaterial(&material_default);

			// Draw two ground objects: one that is close to the camera and the other is connected to the end of the first one
			for (int i = 0; i < 2; i++) {

				// Ground - ground plane
				layer = gx3d_GetObjectLayer(obj_ground, "ground");
				if (i == 0)
					gx3d_SetObjectLayerMatrix(obj_ground, layer, &m1);
				else
					gx3d_SetObjectLayerMatrix(obj_ground, layer, &m2);

				// Update all transforms performed on both ground object
				gx3d_Object_UpdateTransforms(obj_ground);

				// Draw the transformed ground object
				gx3d_SetTexture(0, tex_ground);
				gx3d_DrawObjectLayer(layer, 0);

				// Ground - inner : already a child of ground plane so no transformation needed
				gx3d_SetTexture(0, tex_ground_inner);
				layer = gx3d_GetObjectLayer(obj_ground, "inner");
				gx3d_DrawObjectLayer(layer, 0);

				// Ground - underground : already a child of ground plane so no transformation needed
				gx3d_SetTexture(0, tex_ground_under);
				layer = gx3d_GetObjectLayer(obj_ground, "underground");
				gx3d_DrawObjectLayer(layer, 0);
			}

			// Set material for structures and disable specular lighting
			gx3d_SetMaterial(&material_structures);
			gx3d_DisableSpecularLighting();

			// Generate a randomized structure in the world after a certain distance is reached
			gx3d_SetAmbientLight(color3d_gray);
			if (spawn_structure_distance >= structure_interval && !pause) {

				// Generate a structure based on the spawn chance
				if (structure_spawn_chance >= random_GetFloat()) {

					// Initialize local variables
					int r = random_GetInt(1, 4);
					float spawn_distance_z = 4000; // always spawn new structures 4000 ft away from the character

					// Spawn a random world structure
					structure[structure_index].type = r;

					// Set structure position and side depending on the generated structure type
					if (r == 2) {// structure 2 takes up both left and right sides
						structure[structure_index].side = STRUCTURE_SIDE_BOTH; // (0) both left and right sides
					}
					else if (random_GetFloat() > 0.5f) {
						structure[structure_index].side = STRUCTURE_SIDE_LEFT; // (1) rotate model 180 degrees to move object to the left side
						structure[structure_index].rotated = false; // indicate that the structure needs to be rotated to be on the left side
					}
					else {
						structure[structure_index].side = STRUCTURE_SIDE_RIGHT; // (-1) original model is on the right side so no rotation needed
					}

					structure[structure_index].pos = { 0, 0, spawn_distance_z };

					// Indicate that the structure is currently spawned
					structure[structure_index].spawned = true;

					// Increment structure index by 1
					structure_index = (structure_index + 1) % MAX_STRUCTURE_COUNT;

					// Reset spawn structure distance to 0
					spawn_structure_distance = 0;
				}
			}

			// Update and draw all spawned structures
			for (int i = 0; i < MAX_STRUCTURE_COUNT; i++) {
				if (structure[i].spawned) {

					// Initialize local variables
					char* structure_name;

					switch (structure[i].type) {
					case 1: structure_name = "structure_1"; break;
					case 2: structure_name = "structure_2"; break;
					case 3: structure_name = "structure_3"; break;
					case 4: structure_name = "structure_4"; break;
					}

					// Set the layer to the desired layer in the object file based on its name
					layer = gx3d_GetObjectLayer(obj_structures, structure_name);

					// Set matrix m to the identity matrix
					gx3d_GetIdentityMatrix(&m);

					// Translate the object depending on the side and the distance traveled
					structure[i].pos.z -= distance;
					if (structure[i].side == STRUCTURE_SIDE_LEFT) {						
						if (!structure[i].rotated) {
							gx3d_GetRotateYMatrix(&m, 180);
						}
						gx3d_GetTranslateMatrix(&m1, structure[i].pos.x, structure[i].pos.y, structure[i].pos.z);
						gx3d_MultiplyMatrix(&m, &m1, &m);
						
					}
					else {
						gx3d_GetTranslateMatrix(&m, structure[i].pos.x, structure[i].pos.y, structure[i].pos.z);
					}

					// Set a light depending on the structure type
					if (structure_light[structure_index].world_structure_index == -1) { // light is currently not set to a structure

					}

					// Set object layer matrix to the structures object
					gx3d_SetObjectLayerMatrix(obj_structures, layer, &m);

					// Update the transformations
					gx3d_Object_UpdateTransforms(obj_structures);

					// Draw the spawned structure
					gx3d_SetTexture(0, tex_structures);
					gx3d_DrawObjectLayer(layer, 0);

				}
			}

			// Set ambient light to dim
			gx3d_SetAmbientLight(color3d_dim);

			//========== GAME STATE SPECIFIC CODE ==========//
			// STATE: STARTING
			if (*state == STATE_STARTING) {

				// Set material to reflective and enable specular lighting
				gx3d_SetMaterial(&material_raiu);
				gx3d_EnableSpecularLighting();

				// Add to entrance animation delay timer
				if (entrance_delay_timer < entrance_delay_limit)
					entrance_delay_timer += elapsed_time;

				// Play opening animation
				if (!sfx_initialized && entrance_delay_timer >= entrance_delay_limit) {
					snd_PlaySound(s_starting, 0);
					sfx_initialized = true;
				}

				// Fade in game bgm at starting
				if (current_bgm_volume < bgm_volume)
					snd_SetSoundVolume(s_game_bgm, (current_bgm_volume += 0.20f));
				else {
					// set the volume to bgm volume otherwise
					snd_SetSoundVolume(s_game_bgm, bgm_volume);
				}

				// Start animation timer at first frame
				if (ani_raiu_entrance_time == -1)
					ani_raiu_entrance_time = 0;

				// Add the elapsed frame time to the local timer for the animation
				else if (entrance_delay_timer >= entrance_delay_limit)
					ani_raiu_entrance_time += elapsed_time;

				// Display and play entrance animation after the delay timer expires
				if (entrance_delay_timer >= entrance_delay_limit) {

					if (!snd_IsPlaying(s_starting) && sfx_initialized) {
						*state = STATE_RUNNING;
						sfx_initialized = false;
					}

					// Update speed after a certain amount of time in the animation
					if (ani_raiu_entrance_time >= 2900.0f) {
						if (speed == 0)
							speed = NORMAL_SPEED * 5;
						else if (ani_raiu_entrance_time >= 3200.0f)
							speed = Inverse_Lerp(NORMAL_SPEED * 5, NORMAL_SPEED, (1000.0f - (ani_raiu_entrance_time - 3200.0f)) / 1000.0f);
					}

					// Display and update an effect after a certain amount of time in the animation
					float time_from = 500.0;
					float time_to = 2900.0;
					gx3dVector pos = { 0, 1.0, -1.0 };
					gx3dVector scale = { 7.0f, 3.0f, 1.0f };
					if (ani_raiu_entrance_time >= time_from && ani_raiu_entrance_time <= time_to) {
						Play_FX(fx_run_charge, billboard_normal, pos, scale, time_to - time_from, ani_raiu_entrance_time, 100, true);
						gx3d_SetAmbientLight(color3d_dim);
						// Set character lighting to lightning blue and flicker when within the special effect time window
						Update_Light(&raiu.light, lightning_blue, &pos, 300, elapsed_time, true);
						gx3d_EnableLight(raiu.light);
					}
					else if (ani_raiu_entrance_time > time_to) {
						pos = { raiu.pos.x, raiu.sphere.center.y, raiu.sphere.center.z - 2 };
						gx3d_DisableLight(raiu.light);
						// Set character lighting to blue cyan when time is after the special effect time window
						Update_Light(&raiu.light, blue_cyan, &pos, 100, elapsed_time, false);
						gx3d_EnableLight(raiu.light);
					}

					// Update the animation based on the local timer
					gx3d_Motion_Update(ani_raiu_entrance, ani_raiu_entrance_time / 1000.0f, false);
					gx3d_BlendTree_Update(btree_entrance);

					// Transform character into world
					raiu.sphere = obj_raiu->bound_sphere;
					raiu.sphere.center.x = raiu.pos.x;
					gx3d_GetTranslateMatrix(&m, raiu.pos.x, raiu.pos.y, raiu.pos.z);
					gx3d_SetObjectMatrix(obj_raiu, &m);
					gx3d_SetTexture(0, tex_raiu);
					gx3d_DrawObject(obj_raiu, 0);
				}

				// Disable specular lighting
				gx3d_DisableSpecularLighting();
				
			}

			// STATE: RUNNING
			else if (*state == STATE_RUNNING) {

				// Set ambient light to white
				gx3d_SetAmbientLight(color3d_dim);

				// Update sound effects depending when game is paused or unpaused
				if (!pause) {
					if (snd_paused) {
						snd_SetSoundVolume(s_game_bgm, bgm_volume);
						snd_PlaySound(s_footstep, 1);
						snd_paused = false;
					}
					else if (!snd_IsPlaying(s_footstep))
						snd_PlaySound(s_footstep, 1);
				}
				// Stop sound effect when paused
				else {
					if (snd_IsPlaying(s_footstep)) {
						snd_StopSound(s_footstep);
						snd_SetSoundVolume(s_game_bgm, bgm_volume * 0.75); // bgm volume is decreased while paused
						snd_paused = true;
					}
				}

				// Spawn an electric fence (health pad)?
				if (heal_spawn_timer >= heal_spawn_timer_limit && !pause) {
					if (!heal_pad.draw) {
						if (heal_spawn_chance >= random_GetFloat()) {
							// spawn an electric fence with random x position with respect to the boundary
							heal_pad.pos.x = random_GetFloat() * (BOUNDARY_X - 8) * 2 - (BOUNDARY_X - 8);
							heal_pad.pos.y = 0; // always on top of the ground
							heal_pad.pos.z = 3000.0; // always spawn 3000 ft away from the character
							heal_pad.heal_amt = RAIU_MAX_HP * 0.25; // electric fence heal 25% of max health
							heal_pad.sphere = obj_fence->bound_sphere;
							heal_pad.draw = true;
							heal_pad.ps_enable = true;

							// Set sound position and play the looping sound effect
							snd_SetSoundPosition(s_electric_fence, heal_pad.sphere.center.x, heal_pad.sphere.center.y, heal_pad.sphere.center.z, snd_3D_APPLY_NOW);
							snd_PlaySound(s_electric_fence, 1);

							// Update lighting position
							gx3d_DisableLight(heal_pad.light);
							Update_Light(&heal_pad.light, lightning_green, &heal_pad.sphere.center, 300, elapsed_time, true, 0, 0, 0.001);
							gx3d_EnableLight(heal_pad.light);

							// Reset timer
							heal_spawn_chance = 0;
						}
					}
				}

				// Update the particle system and the electric fence when drawn
				if (heal_pad.draw) {
					// Check first if the current health pad is behind the camera and needs to be recycled
					if (heal_pad.pos.z <= ground_init_z) { // Health pad is at or past the initial ground position
						heal_pad.draw = false;
						heal_pad.ps_enable = false;
						
						// Set timer to half of the limit
						heal_spawn_timer = heal_spawn_timer_limit / 2;

						// Stop the sound effect
						snd_StopSound(s_electric_fence);
					}

					else {
						// Update position
						heal_pad.pos.z -= distance;
						heal_pad.sphere.center.x = heal_pad.pos.x;
						heal_pad.sphere.center.z = heal_pad.pos.z;

						// Update 3D laser sound position (since the world is moving)
						snd_SetSoundPosition(s_electric_fence, heal_pad.sphere.center.x, heal_pad.sphere.center.y, heal_pad.sphere.center.z, snd_3D_APPLY_NOW);

						// Determine if the character has touched the pad
						float x1, x2, y1, y2, z1, z2, d, total_sphere_radius;
						x1 = raiu.sphere.center.x;
						y1 = raiu.pos.y;
						z1 = raiu.sphere.center.z;

						x2 = heal_pad.pos.x;
						y2 = heal_pad.pos.y;
						z2 = heal_pad.pos.z;

						// Calculate the distacne between the character and the health pad
						d = sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1) + (z2 - z1) * (z2 - z1));

						// Calculate the total sphere radius of both spheres
						total_sphere_radius = raiu.sphere.radius + heal_pad.sphere.radius;

						// Determine if the distance between the character and the health pad are close enough for the electric current to "touch" the character
						if (abs(d) <= total_sphere_radius) {

							// Add the heal ouput of the pad to the character's health
							if (heal_fx_timer == -1) {
								raiu.hp += heal_pad.heal_amt;
								if (raiu.hp > RAIU_MAX_HP)
									raiu.hp = RAIU_MAX_HP;
								// Play the "nice" sound effect
								snd_PlaySound(s_raiu_nice, 0);

								// Activate the heal effect timer
								heal_fx_timer = 0;

								// Disable the particle system
								heal_pad.ps_enable = false;

								// Disable the light
								gx3d_DisableLight(heal_pad.light);

								// Stop sound effect
								snd_StopSound(s_electric_fence);

								// Reset the spawn timer
								heal_spawn_timer = 0;
							}
						}

						// Draw the electric fence
						gx3d_GetTranslateMatrix(&m, heal_pad.pos.x, heal_pad.pos.y, heal_pad.pos.z);
						gx3d_SetObjectMatrix(obj_fence, &m);
						gx3d_SetTexture(0, tex_structures);
						gx3d_DrawObject(obj_fence);
					}
				}

				// Only update and draw the particle system when it is enabled
				if (heal_pad.ps_enable) {

					// Update lighting position
					gx3d_DisableLight(heal_pad.light);
					Update_Light(&heal_pad.light, lightning_green, &heal_pad.sphere.center, 300, elapsed_time, true, 0, 0, 0.001);
					gx3d_EnableLight(heal_pad.light);

					// Get the object translate matrix
					gx3d_SetAmbientLight(color3d_white);
					gx3d_EnableAlphaTesting(50);
					gx3d_GetTranslateMatrix(&m, heal_pad.pos.x, heal_pad.sphere.center.y + 2.09, heal_pad.pos.z + 4.6);
					gx3d_SetParticleSystemMatrix(heal_pad.psys, &m);
					gx3d_UpdateParticleSystem(heal_pad.psys, elapsed_time);
					gx3d_DrawParticleSystem(heal_pad.psys, &heading, false);
					gx3d_DisableAlphaTesting();
				}

				// Reset ambient light back to dim
				gx3d_SetAmbientLight(color3d_dim);

				// Spawn an enemy?
				if (enemy_spawn_timer >= spawn_timer_limit && !pause) {
					if (!(enemies.hoshu[enemies.hoshu_index].draw)) {
						if (enemy_spawn_chance >= random_GetFloat()) {
							// spawn a Hoshu with random x position with respect to the boundary
							enemies.hoshu[enemies.hoshu_index].pos.x = random_GetFloat() * BOUNDARY_X * 2 - BOUNDARY_X; // left: -BOUNDARY_X | right: +BOUNDARY_X
							enemies.hoshu[enemies.hoshu_index].pos.y = 0; // always spawn on top of the floor
							enemies.hoshu[enemies.hoshu_index].pos.z = 3000.0; // always spawn at the front 3000 ft away from the character
							enemies.hoshu[enemies.hoshu_index].sphere.radius = obj_hoshu->bound_sphere.radius;
							enemies.hoshu[enemies.hoshu_index].sphere.center.x = enemies.hoshu[enemies.hoshu_index].pos.x;
							enemies.hoshu[enemies.hoshu_index].sphere.center.y = obj_hoshu->bound_sphere.center.y;
							enemies.hoshu[enemies.hoshu_index].sphere.center.z = enemies.hoshu[enemies.hoshu_index].pos.z;
							enemies.hoshu[enemies.hoshu_index].lv = hoshu_lv;
							enemies.hoshu[enemies.hoshu_index].hp = 100 * hoshu_lv;
							enemies.hoshu[enemies.hoshu_index].score_amt = 200 * hoshu_lv;
							enemies.hoshu[enemies.hoshu_index].exp_amt = 100 * hoshu_lv;
							enemies.hoshu[enemies.hoshu_index].gun_delay = 1000.0f; // 1 sec shooting delay
							enemies.hoshu[enemies.hoshu_index].gun_timer = 0.0f;
							enemies.hoshu[enemies.hoshu_index].gun_damage = 10 * hoshu_lv;
							enemies.hoshu[enemies.hoshu_index].fire_rate = random_GetFloat() * 0.1; // Generate a randomized fire rate between 0.0 - 0.5
							enemies.hoshu[enemies.hoshu_index].laser_index = 0;
							enemies.hoshu[enemies.hoshu_index].explosion_timer = -1;
							enemies.hoshu[enemies.hoshu_index].explosion_type = -1;
							enemies.hoshu[enemies.hoshu_index].explode_snd_initialized = false;
							enemies.hoshu[enemies.hoshu_index].blade_mark_1 = false;
							enemies.hoshu[enemies.hoshu_index].blade_mark_2 = false;
							enemies.hoshu[enemies.hoshu_index].draw = true;

							// Increment enemy count by 1
							enemy_count++;

							// Update to next enemy index
							enemies.hoshu_index = (enemies.hoshu_index + 1) % current_max_enemy_count;

							// Reset spawn timer
							enemy_spawn_timer = 0;
						}
					}
				}

				// Update any spawned enemies and draw any newly spawned enemies
				for (int i = 0; i < current_max_enemy_count; i++) {

					// Set material for enemies and turn on specular lighting
					gx3d_SetMaterial(&material_hoshu);
					gx3d_EnableSpecularLighting();

					// Draw the current Hoshu when it is spawned
					if (enemies.hoshu[i].draw) {

						// Initialize local variables
						float angle;

						// Check first if the current Hoshu is behind the camera and needs to be recycled
						if (enemies.hoshu[i].pos.z <= ground_init_z) { // Hoshu is at or past the initial ground position
							enemies.hoshu[i].explosion_timer = -1;
							enemies.hoshu[i].explosion_type = -1;
							enemies.hoshu[i].explode_snd_initialized = false;
							enemies.hoshu[i].draw = false;

							// Decrement enemy count to spawn new enemies
							enemy_count--;
						}

						else {
							// Update z position
							enemies.hoshu[i].pos.z -= distance;
							enemies.hoshu[i].sphere.center.z = enemies.hoshu[i].pos.z;

							// Transform and draw into world
							float pos_x = enemies.hoshu[i].pos.x;
							float pos_y = enemies.hoshu[i].pos.y;
							float pos_z = enemies.hoshu[i].pos.z;

							// Update 3D laser sound position (since the world is moving)
							snd_SetSoundPosition(s_laser_2, enemies.hoshu[i].sphere.center.x, enemies.hoshu[i].sphere.center.y, enemies.hoshu[i].sphere.center.z, snd_3D_APPLY_NOW);

							// Determine if the Hoshu has taken damage from a blade swing
							if (blade_active) {
								float x1, x2, y1, y2, z1, z2, d, total_sphere_radius;
								x1 = raiu.sphere.center.x;
								y1 = raiu.sphere.center.y;
								z1 = raiu.sphere.center.z;

								x2 = enemies.hoshu[i].sphere.center.x;
								y2 = enemies.hoshu[i].sphere.center.y;
								z2 = enemies.hoshu[i].sphere.center.z;

								// Calculate the distacne between the character and the enemy
								d = sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1) + (z2 - z1) * (z2 - z1));

								// Calculate the total sphere radius of both spheres
								total_sphere_radius = raiu.sphere.radius + enemies.hoshu[i].sphere.radius;

								// Determine if the distance between the character and the Hoshu are close enough for the blade swing to be considered as a hit
								if (abs(d) <= total_sphere_radius) {

									// Subract the damage ouput of the blade to the Hoshu's health when its blade markers have not been set to true
									if (play_swing_1) { // blade swing 1 is active (always active even when blade swing 2 is active)
										if (!enemies.hoshu[i].blade_mark_1) {
											enemies.hoshu[i].hp -= raiu.blade_damage;
											enemies.hoshu[i].blade_mark_1 = true;
										}
										if (play_swing_2) { // blade swing 2 is active
											if (!enemies.hoshu[i].blade_mark_2) {
												enemies.hoshu[i].hp -= raiu.blade_damage;
												enemies.hoshu[i].blade_mark_2 = true;
											}
										}
									}

									// Check if the Hoshu's health reaches 0
									// Enemy is destroyed if its hp is at 0 or less
									if (enemies.hoshu[i].explosion_timer == -1) {
										if (enemies.hoshu[i].hp <= 0) {
											// add the enemy score amount to the total score if the max score is not reached (sets it to max score when reached)
											if (score != MAX_SCORE)
												score += enemies.hoshu[i].score_amt;
											else
												score = MAX_SCORE;
											// add the enemy exp amount to the character's total exp
											raiu.exp += enemies.hoshu[i].exp_amt;

											// increment enemies defeated and Hoshus defeated
											enemies_defeated++;
											hoshus_defeated++;
											enemy_count--; // decrement enemy count by 1

											// Initialize explosion timer on the enemy
											enemies.hoshu[i].explosion_timer = 0;
										}
									}
								}
							}

							// Reset blade damage markers for the Hoshu back to false when blade is inactive and both values are set to true
							else {
								if (enemies.hoshu[i].blade_mark_1)
									enemies.hoshu[i].blade_mark_1 = false;
								if (enemies.hoshu[i].blade_mark_2)
									enemies.hoshu[i].blade_mark_2 = false;
							}

							// Play or update the explosion effect while the Hoshu was just destroyed
							if (enemies.hoshu[i].explosion_timer >= 0) {

								// Initialize local variables
								gx3dVector scale = { 10, 10, 10 };
								gx3dTexture fx_explosion;

								// Set explosion light to the current destroyed Hoshu
								Update_Light(&explosion_light, explosion_orange, &enemies.hoshu[i].sphere.center, Inverse_Lerp(100, 0, (FX_NORMAL_DURATION * enemies.hoshu[i].explosion_timer) / FX_NORMAL_DURATION), elapsed_time, true, 0, 0, 0.001);
								gx3d_EnableLight(explosion_light);

								// Generate a randomized explosion effect and sound
								if (enemies.hoshu[i].explosion_type == -1) {
									enemies.hoshu[i].explosion_type = random_GetInt(1, 3);
								}

								// Update 3D explosion sound positions
								snd_SetSoundPosition(s_explosion_1, enemies.hoshu[i].sphere.center.x, enemies.hoshu[i].sphere.center.y, enemies.hoshu[i].sphere.center.z, snd_3D_APPLY_NOW);
								snd_SetSoundPosition(s_explosion_2, enemies.hoshu[i].sphere.center.x, enemies.hoshu[i].sphere.center.y, enemies.hoshu[i].sphere.center.z, snd_3D_APPLY_NOW);
								snd_SetSoundPosition(s_explosion_3, enemies.hoshu[i].sphere.center.x, enemies.hoshu[i].sphere.center.y, enemies.hoshu[i].sphere.center.z, snd_3D_APPLY_NOW);

								if (enemies.hoshu[i].explosion_type == 1) {
									if (!enemies.hoshu[i].explode_snd_initialized) {
										snd_PlaySound(s_explosion_1, 0);
										enemies.hoshu[i].explode_snd_initialized = true;
										gx3d_MultiplyScalarVector(2, &scale, &scale); // doubles the scale
									}
									fx_explosion = fx_explosion_1;
								}
								else if (enemies.hoshu[i].explosion_type == 2) {
									if (!enemies.hoshu[i].explode_snd_initialized) {
										snd_PlaySound(s_explosion_2, 0);
										enemies.hoshu[i].explode_snd_initialized = true;
									}
									fx_explosion = fx_explosion_2;
								}
								else {
									if (!enemies.hoshu[i].explode_snd_initialized) {
										snd_PlaySound(s_explosion_3, 0);
										enemies.hoshu[i].explode_snd_initialized = true;
									}
									fx_explosion = fx_explosion_3;
								}

								// Display and update an effect after a certain amount of time in the animation
								if (enemies.hoshu[i].explosion_timer <= FX_NORMAL_DURATION) {
									Play_FX(fx_explosion, billboard_normal, enemies.hoshu[i].sphere.center, scale, FX_NORMAL_DURATION, enemies.hoshu[i].explosion_timer, 100, false);
									gx3d_SetAmbientLight(color3d_dim);

									// Update timer
									enemies.hoshu[i].explosion_timer += elapsed_time;
								}

								// Resets explosion timer and sets draw to false when the explosion effect has finished
								else {
									enemies.hoshu[i].explosion_timer = -1;
									enemies.hoshu[i].explosion_type == -1;
									enemies.hoshu[i].explode_snd_initialized = false;
									enemies.hoshu[i].draw = false;

									// Disable the explosion light
									gx3d_DisableLight(explosion_light);
								}
							}

							// Continue displaying the Hoshu while not destroyed
							else {
								layer = gx3d_GetObjectLayer(obj_hoshu, "bottom");
								gx3d_GetTranslateMatrix(&m1, pos_x, pos_y, pos_z);
								gx3d_SetObjectLayerMatrix(obj_hoshu, layer, &m1);

								// Set layer to the "top" layer of the Hoshu model
								layer = gx3d_GetObjectLayer(obj_hoshu, "top");

								// Update the Hoshu view vector to point at the character's xz-coordinates
								hoshu_view.x = -(raiu.pos.x - enemies.hoshu[i].pos.x);
								hoshu_view.y = 0;
								hoshu_view.z = (raiu.pos.z - enemies.hoshu[i].pos.z);

								// Compute angle between the two
								angle = gx3d_AngleBetweenVectors(&hoshu_normal, &hoshu_view);
								if ((hoshu_view.x - hoshu_normal.x) < 0)
									angle *= -1;

								// Make sure that angle of rotation does not exceed its max
								if (angle >= 0 && angle > ROTATE_RIGHT_MAX)
									angle = ROTATE_RIGHT_MAX;
								else if (angle < 0 && angle < ROTATE_LEFT_MAX)
									angle = ROTATE_LEFT_MAX;
								gx3d_GetRotateYMatrix(&m1, angle);
								gx3d_SetObjectLayerMatrix(obj_hoshu, layer, &m1);

								// Update transformations
								gx3d_Object_UpdateTransforms(obj_hoshu);

								// Draw layers
								gx3d_SetTexture(0, tex_hoshu);

								layer = gx3d_GetObjectLayer(obj_hoshu, "bottom");
								gx3d_DrawObjectLayer(layer, 0);

								layer = gx3d_GetObjectLayer(obj_hoshu, "top");
								gx3d_DrawObjectLayer(layer, 0);

								// Make the enemy shoot a projectile?
								if (enemies.hoshu[i].gun_timer >= enemies.hoshu[i].gun_delay && !pause) {
									if (!(enemies.hoshu[i].laser[enemies.hoshu[i].laser_index].draw)) {
										if (enemies.hoshu[i].fire_rate >= random_GetFloat()) {

											// Play laser beam sound effect
											snd_SetSoundPosition(s_laser_2, enemies.hoshu[i].sphere.center.x, enemies.hoshu[i].sphere.center.y, enemies.hoshu[i].sphere.center.z, snd_3D_APPLY_NOW);
											snd_PlaySound(s_laser_2, 0);

											// Initialize bounding sphere and position
											enemies.hoshu[i].laser[enemies.hoshu[i].laser_index].sphere = obj_laser->bound_sphere;
											enemies.hoshu[i].laser[enemies.hoshu[i].laser_index].sphere.radius *= hoshu_laser_scale; // scales the Hoshu lasers' size
											enemies.hoshu[i].laser[enemies.hoshu[i].laser_index].pos = enemies.hoshu[i].pos;

											// Set laser position and speed
											enemies.hoshu[i].laser[enemies.hoshu[i].laser_index].pos = enemies.hoshu[i].sphere.center;
											enemies.hoshu[i].laser[enemies.hoshu[i].laser_index].pos.y -= 0.5; // laser is 0.5 ft below the center of the Hoshu
											enemies.hoshu[i].laser[enemies.hoshu[i].laser_index].sphere.center.x = enemies.hoshu[i].laser[enemies.hoshu[i].laser_index].pos.x;
											enemies.hoshu[i].laser[enemies.hoshu[i].laser_index].sphere.center.y = enemies.hoshu[i].laser[enemies.hoshu[i].laser_index].pos.y;
											enemies.hoshu[i].laser[enemies.hoshu[i].laser_index].sphere.center.z = enemies.hoshu[i].laser[enemies.hoshu[i].laser_index].pos.z;
											enemies.hoshu[i].laser[enemies.hoshu[i].laser_index].trajectory.velocity = hoshu_laser_speed;
											enemies.hoshu[i].laser[enemies.hoshu[i].laser_index].hit = false;

											// Rotates the trajectory based on the rotation angle of the Hoshu's "top" model layer
											v = { 0,0,-1 };
											gx3d_GetRotateYMatrix(&m, angle);
											gx3d_MultiplyVectorMatrix(&v, &m, &enemies.hoshu[i].laser[enemies.hoshu[i].laser_index].trajectory.direction);

											gx3d_NormalizeVector(&enemies.hoshu[i].laser[enemies.hoshu[i].laser_index].trajectory.direction, &enemies.hoshu[i].laser[enemies.hoshu[i].laser_index].trajectory.direction);

											// Indicate that the laser should be drawn in the world
											enemies.hoshu[i].laser[enemies.hoshu[i].laser_index].draw = true;

											// Update to the next shootable laser
											enemies.hoshu[i].laser_index = (enemies.hoshu[i].laser_index + 1) % HOSHU_MAX_LASER_COUNT;

											// Reset the cooldown timer
											enemies.hoshu[i].gun_timer = 0;
										}
									}
								}
							}
						}
					}

					// Disable specular lighting for the lasers and set material for red laser
					gx3d_DisableSpecularLighting();
					gx3d_SetMaterial(&material_red_laser);

					// Update any lasers fired by each Hoshu that is drawn in the world
					for (int j = 0; j < HOSHU_MAX_LASER_COUNT; j++) {
						if (enemies.hoshu[i].laser[j].draw) {

							// Initialize local variables
							float x1, y1, z1, x2, y2, z2, d;
							float total_laser_distance;

							// Enable alpha blending and set ambient light to white
							gx3d_DisableAlphaBlending();
							gx3d_SetAmbientLight(color3d_white);

							// Slow down laser movement when destroyed
							if (enemies.hoshu[i].laser[j].destroyed && enemies.hoshu[i].laser[j].trajectory.velocity == hoshu_laser_speed)
								enemies.hoshu[i].laser[j].trajectory.velocity *= 0.10;

							// Calculate the distance traveled by the projectile based on the elapsed time
							total_laser_distance = (enemies.hoshu[i].laser[j].trajectory.velocity) * (elapsed_time / 1000.0f);

							// Don't draw if projectile goes beyond the initial ground position
							if (abs(enemies.hoshu[i].laser[j].pos.x) >= 1000, abs(enemies.hoshu[i].laser[j].pos.y) >= 1000, abs(enemies.hoshu[i].laser[j].pos.z) >= MAX_PROJECTILE_DISTANCE) {
								enemies.hoshu[i].laser[j].hit_timer = -1;
								enemies.hoshu[i].laser[j].draw = false;

								// Reset all laser variables to initial values
								enemies.hoshu[i].laser[j].distance = { 0, 0, 0 };
								enemies.hoshu[i].laser[j].sphere = obj_laser->bound_sphere;
								enemies.hoshu[i].laser[j].pos = enemies.hoshu[i].pos;
								enemies.hoshu[i].laser[j].world_shift = 0;
								enemies.hoshu[i].laser[j].destroyed = false;
								enemies.hoshu[i].laser[j].trajectory = { {0, 0, -1}, 0 };
							}

							// Display the laser or the hit effect otherwise
							else {

								// Calculate distance between the enemy projectile and the character
								x1 = enemies.hoshu[i].laser[j].sphere.center.x;
								y1 = 0; //enemies.hoshu[i].laser[j].sphere.center.y;
								z1 = enemies.hoshu[i].laser[j].sphere.center.z;

								x2 = raiu.sphere.center.x;
								y2 = 0; //raiu.sphere.center.y;
								z2 = raiu.sphere.center.z;

								// distance = sqrt((x2-x2)^2 + (y2-y1)^2 + (z2-z1)^2)
								d = sqrt(((x2 - x1) * (x2 - x1)) + ((y2 - y1) * (y2 - y1)) + ((z2 - z1) * (z2 - z1)));

								// Detect if the laser hits the character
								// Collides if the distance is less than the radius of the character's bounding sphere
								if (d <= raiu.sphere.radius * 2) { // projectile has collided with the character
									if (abs(x2 - x1) <= raiu.sphere.radius) {

										// PLAY LASER HIT SOUND FX???

										// Destroy the laser if blade is active on the time of impact
										if (blade_active)
											enemies.hoshu[i].laser[j].destroyed = true;

										// Character gets damaged otherwise
										if (!enemies.hoshu[i].laser[j].hit && !enemies.hoshu[i].laser[j].destroyed) {

											// Play a random raiu_hurt sfx and a laser hit sfx
											int r = random_GetInt(1, 2);
											if (r == 1) 
												snd_PlaySound(s_raiu_hurt_1, 0);
											else 
												snd_PlaySound(s_raiu_hurt_2, 0);

											// Subract the damage output of the gun to the character's health
											raiu.hp -= enemies.hoshu[i].gun_damage;

											// State switches to "Game Ending" if the character's hp is at 0 or less
											if (raiu.hp <= 0) {
												*state = STATE_GAME_ENDING;
											}
											enemies.hoshu[i].laser[j].hit = true;
											enemies.hoshu[i].laser[j].destroyed = false;
										}

										// Activate the laser hit timer
										enemies.hoshu[i].laser[j].hit_timer = 0;
									}
								}

								// Play or update the laser hit effect while the laser had just hit an object
								if (enemies.hoshu[i].laser[j].hit_timer >= 0) {

									// Initialize local variables
									gx3dVector scale = { 4, 4, 4 };
									gx3dVector pos;

									// Set effect position
									if (enemies.hoshu[i].laser[j].destroyed)
										pos = { enemies.hoshu[i].laser[j].pos.x, enemies.hoshu[i].laser[j].pos.y, enemies.hoshu[i].laser[j].pos.z };
									else
										pos = { raiu.pos.x, raiu.sphere.center.y, raiu.pos.z + 1 };

									// Display and update an effect after a certain amount of time in the animation
									if (enemies.hoshu[i].laser[j].hit_timer <= FX_NORMAL_DURATION) {
										Play_FX(fx_laser_red, billboard_normal, pos, scale, FX_NORMAL_DURATION / 2.0f, enemies.hoshu[i].laser[j].hit_timer, 100, false);
										gx3d_SetAmbientLight(color3d_dim);

										// Update timer
										enemies.hoshu[i].laser[j].hit_timer += elapsed_time;
									}

									// Resets the timer and sets draw to false when the hit effect has finished
									else {
										enemies.hoshu[i].laser[j].hit_timer = -1;
										enemies.hoshu[i].laser[j].draw = false;

										// Reset all laser variables to initial values
										enemies.hoshu[i].laser[j].distance = { 0, 0, 0 };
										enemies.hoshu[i].laser[j].sphere = obj_laser->bound_sphere;
										enemies.hoshu[i].laser[j].pos = enemies.hoshu[i].pos;
										enemies.hoshu[i].laser[j].world_shift = 0;
										enemies.hoshu[i].laser[j].destroyed = false;
										enemies.hoshu[i].laser[j].trajectory = { {0, 0, -1}, 0 };
									}
								}

								else {

									// Translate to world then draw
									gx3d_GetTranslateMatrix(&m1, enemies.hoshu[i].laser[j].sphere.center.x, enemies.hoshu[i].laser[j].pos.y, enemies.hoshu[i].laser[j].pos.z);
									gx3d_GetScaleMatrix(&m2, hoshu_laser_scale, hoshu_laser_scale, hoshu_laser_scale);
									gx3d_MultiplyMatrix(&m2, &m1, &m);
									gx3d_SetObjectMatrix(obj_laser, &m);
									gx3d_SetTexture(0, tex_red_laser);
									gx3d_DrawObject(obj_laser, 0);

								}

								// Update laser position based on the total laser distance traveled
								gx3d_MultiplyScalarVector(total_laser_distance, &enemies.hoshu[i].laser[j].trajectory.direction, &v);
								gx3d_AddVector(&enemies.hoshu[i].laser[j].pos, &v, &enemies.hoshu[i].laser[j].pos);
								enemies.hoshu[i].laser[j].sphere.center.x = enemies.hoshu[i].laser[j].pos.x;
								enemies.hoshu[i].laser[j].sphere.center.y = enemies.hoshu[i].laser[j].pos.y;
								enemies.hoshu[i].laser[j].sphere.center.z = enemies.hoshu[i].laser[j].pos.z;
							}

							// Reset the light to dark gray
							gx3d_SetAmbientLight(color3d_darkgray);
							gx3d_EnableAlphaBlending();
						}
					}
				}

				// Set material to blue laser
				gx3d_SetMaterial(&material_blue_laser);

				// Update any lasers fired by the character and is drawn in the world
				for (int i = 0; i < RAIU_MAX_LASER_COUNT; i++) {

					// Draw lasers that are currently in the world
					if (raiu.laser[i].draw) {

						// Initialize local variables
						gx3dRay ray;

						gx3d_NormalizeVector(&raiu.laser[i].trajectory.direction, &ray.direction);
						ray.origin = raiu.laser[i].sphere.center;

						// Enable alpha blending and set ambient light to white
						gx3d_DisableAlphaBlending();
						gx3d_SetAmbientLight(color3d_white);

						// Don't draw if maximum distance is reached
						if (abs(raiu.laser[i].pos.x) >= MAX_PROJECTILE_DISTANCE || abs(raiu.laser[i].pos.y) >= MAX_PROJECTILE_DISTANCE || raiu.laser[i].pos.z >= MAX_PROJECTILE_DISTANCE) {

							// Reset all laser variables to initial values
							raiu.laser[i].distance = { 0, 0, 0 };
							raiu.laser[i].sphere = obj_laser->bound_sphere;
							raiu.laser[i].pos = raiu.sphere.center;
							raiu.laser[i].world_shift = 0;
							raiu.laser[i].trajectory = { raiu.view, 0 };
							raiu.laser[i].hit = false;
							raiu.laser[i].hit_index = -1;
							raiu.laser[i].hit_timer = -1;
							raiu.laser[i].draw = false;
						}

						// Display the laser otherwise
						else {

							// Initialize local variables
							int j;

							// Calculate the distance traveled by the projectile based on the elapsed time
							// the projectile direction vector multiplied by the distance traveled by the projectile based on the elapsed time in seconds
							if (!pause) { // only update laser position when unpaused
								gx3d_MultiplyScalarVector((raiu.laser[i].trajectory.velocity) * (elapsed_time / 1000.0f), &raiu.laser[i].trajectory.direction, &v);
								gx3d_AddVector(&raiu.laser[i].distance, &v, &raiu.laser[i].distance);
								raiu.laser[i].world_shift += distance;
								raiu.laser[i].pos.x += raiu.laser[i].distance.x;
								raiu.laser[i].pos.y += raiu.laser[i].distance.y;
								raiu.laser[i].pos.z += raiu.laser[i].distance.z - raiu.laser[i].world_shift;
								raiu.laser[i].sphere.center = raiu.laser[i].pos;
							}
							else {

							}

							if (raiu.laser[i].hit)
								j = raiu.laser[i].hit_index;
							else
								j = 0;

							// Detect if the laser hits an enemy
							for (; j < current_max_enemy_count; j++) {
								if (enemies.hoshu[j].draw) {

									// Initialize local variables
									gx3dTrajectory enemy_trajectory, laser_trajectory;
									float collision_time;

									enemy_trajectory.direction = { 0, 0, -1 }; // all enemies move straight from the front to the back of the character
									enemy_trajectory.velocity = speed * spd_multiplier;
									laser_trajectory = raiu.laser[i].trajectory;

									// Detect collision between two moving spheres
									relation = gx3d_Collide_Sphere_Sphere(&raiu.laser[i].sphere, &laser_trajectory, SECONDS, &enemies.hoshu[j].sphere, &enemy_trajectory, &collision_time);
									if (relation != gxRELATION_OUTSIDE || (raiu.laser[i].hit && raiu.laser[i].hit_index == j)) { // projectile will interesect with the enemy
										if (!raiu.laser[i].hit) {
											raiu.laser[i].hit = true;
											raiu.laser[i].hit_index = j;
										}

										// Detected a hit and the laser had already intersected with the target
										if (collision_time <= 0.0 && raiu.laser[i].hit_timer == -1 && enemies.hoshu[j].explosion_timer == -1) {

											// PLAY LASER HIT SOUND FX???

											// Subract the damage output of the laser to the enemy's health
											enemies.hoshu[j].hp -= raiu.gun_damage;

											// Enemy is destroyed if its hp is at 0 or less
											if (enemies.hoshu[j].hp <= 0) {

												// add the enemy score amount to the total score if the max score is not reached (sets it to max score when reached)
												if (score != MAX_SCORE)
													score += enemies.hoshu[j].score_amt;
												else
													score = MAX_SCORE;
												// add the enemy exp amount to the character's total exp
												raiu.exp += enemies.hoshu[j].exp_amt;

												// increment enemies defeated and Hoshus defeated
												enemies_defeated++;
												hoshus_defeated++;
												enemy_count--; // decrement enemy count by 1

												// Initialize explosion timer on the enemy
												enemies.hoshu[j].explosion_timer = 0;
											}

											// Activate laser hit timer otherwise
											else {
												if (raiu.laser[i].hit_timer == -1)
													raiu.laser[i].hit_timer = 0;
											}
										}
									}
								}
								
								// Update laser hit effect while the timer is active
								if (raiu.laser[i].hit_timer >= 0) {

									// Initialize local variables
									gx3dVector scale = { 8, 8, 8 };
									gx3dVector pos;

									pos = { enemies.hoshu[j].sphere.center.x, enemies.hoshu[j].sphere.center.y, enemies.hoshu[j].sphere.center.z - 5 };

									// Display and update an effect after a certain amount of time in the animation
									if (raiu.laser[i].hit_timer <= FX_NORMAL_DURATION) {
										Play_FX(fx_laser_blue, billboard_normal, pos, scale, FX_NORMAL_DURATION / 2.0f, raiu.laser[i].hit_timer, 100, false);
										gx3d_SetAmbientLight(color3d_dim);

										// Update timer
										raiu.laser[i].hit_timer += elapsed_time;
									}
									else {

										// Reset all laser variables to initial values
										raiu.laser[i].distance = { 0, 0, 0 };
										raiu.laser[i].sphere = obj_laser->bound_sphere;
										raiu.laser[i].pos = raiu.sphere.center;
										raiu.laser[i].world_shift = 0;
										raiu.laser[i].trajectory = { raiu.view, 0 };
										raiu.laser[i].hit = false;
										raiu.laser[i].hit_index = -1;
										raiu.laser[i].hit_timer = -1;
										raiu.laser[i].draw = false;
									}
								}

								// Continue displaying laser otherwise
								else {

									// Translate to world then draw
									gx3d_GetTranslateMatrix(&m, raiu.laser[i].sphere.center.x, raiu.laser[i].sphere.center.y, raiu.laser[i].sphere.center.z);
									gx3d_SetObjectMatrix(obj_laser, &m);
									gx3d_SetTexture(0, tex_blue_laser);
									gx3d_DrawObject(obj_laser, 0);
								}

								// Break off from the enemy search loop when the laser indicates a hit
								if (raiu.laser[i].hit)
									break;
							}
						}

						// Reset the light to dark gray
						gx3d_SetAmbientLight(color3d_darkgray);
						gx3d_EnableAlphaBlending();
					}
				}

				// Set material for the character and enable specular lighting
				gx3d_SetMaterial(&material_raiu);
				gx3d_EnableSpecularLighting();

				// Play or update level up effect when its timer is active
				if (level_up_fx_timer >= 0) {
					
					// Play the level up sound effect once when the timer was just activated
					if (level_up_fx_timer == 0)
						snd_PlaySound(s_lv_up, 0);

					Play_FX(fx_level_up, billboard_normal, raiu.sphere.center, { 5, 5, 5 }, FX_NORMAL_DURATION, level_up_fx_timer, 100, false);
					gx3d_SetAmbientLight(color3d_dim);

					// Deactivate timer when its duration is reached
					if (level_up_fx_timer >= FX_NORMAL_DURATION)
						level_up_fx_timer = -1;

					// Update the timer otherwise
					else
						level_up_fx_timer += elapsed_time;
				}

				// Play the running animation, with looping
				// If this is the start of the animation (anim_time == -1) then set the local timer for the animation to 0
				if (ani_raiu_run_time == -1)
					ani_raiu_run_time = 0;
				// Add the elapsed frame time to the local timer for the animation
				else
					ani_raiu_run_time += elapsed_time;

				// Update the animation based on the local timer
				gx3d_Motion_Update(ani_raiu_run, (ani_raiu_run_time / 1000.0f) * spd_multiplier, true); // update run animation speed depending on the speed multiplier
				gx3d_Motion_Update(ani_raiu_aim_up, (30.0f / 1000.0f) * 21, true);
				gx3d_Motion_Update(ani_raiu_aim_down, (30.0f / 1000.0f) * 21, true);
				gx3d_Motion_Update(ani_raiu_aim_left, (30.0f / 1000.0f) * 21, true);
				gx3d_Motion_Update(ani_raiu_aim_right, (30.0f / 1000.0f) * 21, true);

				// Update blade swing timer and animations only if it is active
				if (raiu.blade_timer >= 0) {

					// Set blade to active on a specific time window depending on the type of blade swing animation
					if (play_swing_1 && !play_swing_2) {
						if (raiu.blade_timer <= 300)
							blade_active = true;
						else
							blade_active = false;
					}
					else if (play_swing_2) {
						if (raiu.blade_timer <= 400)
							blade_active = true;
						else
							blade_active = false;
					}

					// Deactivate swing timer when its time limit is reached
					if (raiu.blade_timer >= raiu.blade_delay) {
						raiu.blade_timer = -1;
						play_swing_1 = false;
						play_swing_2 = false;
					}
					
					// Update timer and animation otherwise
					else {
						raiu.blade_timer += elapsed_time;
						gx3d_Motion_Update(ani_raiu_swing_1, (raiu.blade_timer / 1000.0f), false);
						gx3d_Motion_Update(ani_raiu_swing_2, (raiu.blade_timer / 1000.0f), false);
					}
				}

				// Get percent of rotation x and y from its max values for use in blending aim animations below
				// NOTE: 0.5 is the center | x-rotate is a rotation on the x-axis (adjusts up and down aim) and vice versa

				if (current_aim_x < 0) // current x-axis aim is pointing left
					aim_x = 0.5 - (((float)current_aim_x / (float)ROTATE_LEFT_MAX) * 0.5);
				else if (current_aim_x > 0) // current x-axis aim is pointing right
					aim_x = 0.5 + (((float)current_aim_x / (float)ROTATE_RIGHT_MAX) * 0.5);
				else // current x-axis aim is pointing in the center (current_aim_x is 0)
					aim_x = 0.5;

				if (current_aim_y < 0) // current y-axis aim is pointing up
					aim_y = 0.5 - (((float)current_aim_y / (float)ROTATE_UP_MAX) * 0.5);
				else if (current_aim_y > 0) // current y-axis aim is pointing down
					aim_y = 0.5 + (((float)current_aim_y / (float)ROTATE_DOWN_MAX) * 0.5);
				else // current y-axis aim is pointing in the center (current_aim_y is 0)
					aim_y = 0.5;

				// Activate blade swing animation while its timer is active
				if (raiu.blade_timer >= 0) {
					swing_active = 1.0;

					// Determine if blade swing type is currently blade swing 1 or blade swing 2
					if (!play_swing_2)
						swing_type = 0.0; // swing 1 animation
					else
						swing_type = 1.0; // swing 2 animation
				}
				else {
					swing_active = 0.0;
				}

				// Update the movement blend tree
				gx3d_BlendNode_Set_BlendValue(bnode_aim_lr, gx3d_BLENDNODE_TRACK_0, aim_x); // (0.0): Aim full left | (0.5): Aim center | (1.0): Aim full right
				gx3d_BlendNode_Set_BlendValue(bnode_aim_ud, gx3d_BLENDNODE_TRACK_0, aim_y); // (0.0): Aim full up | (0.5): Aim center | (1.0): Aim full down
				gx3d_BlendNode_Set_BlendValue(bnode_aim_udlr, gx3d_BLENDNODE_TRACK_0, 0.5); // (0.0): Full up-down | (0.5): Half up-down & Half left-right | (1.0): Full left-right
				gx3d_BlendNode_Set_BlendValue(bnode_swing, gx3d_BLENDNODE_TRACK_0, swing_type); // (0.0): Blade swing 1 | (1.0): Blade swing 2
				gx3d_BlendNode_Set_BlendValue(bnode_adder_aim_swing, gx3d_BLENDNODE_TRACK_0, swing_active); // (1.0): Add blade swing | (0.0): Deactivate blade swing
				gx3d_BlendNode_Set_BlendValue(bnode_adder_run_aim, gx3d_BLENDNODE_TRACK_0, 1.0); // (1.0): Add full aim | (0.0): Deactivate aim

				gx3d_BlendTree_Update(btree_movement);

				// Play a special effect if the character has regained health
				if (heal_fx_timer >= 0 && heal_fx_timer < FX_NORMAL_DURATION * 2) {
					v = { raiu.pos.x, raiu.sphere.center.y - 2, raiu.pos.z - 1 };
					Play_FX(fx_run_charge, billboard_normal, v, { 7, 3, 1 }, FX_NORMAL_DURATION, heal_fx_timer, 100, true);
					gx3d_SetAmbientLight(color3d_dim);
					// Set character lighting to lightning blue and flicker when within the special effect time window
					Update_Light(&raiu.light, lightning_blue, &v, 300, elapsed_time, true);
					gx3d_EnableLight(raiu.light);

					 // update the timer
					heal_fx_timer += elapsed_time;
				}
				else if (heal_fx_timer >= FX_NORMAL_DURATION) {
					v = { raiu.pos.x, raiu.sphere.center.y, raiu.sphere.center.z - 2 };
					gx3d_DisableLight(raiu.light);
					// Set character lighting to blue cyan when time is after the special effect time window
					Update_Light(&raiu.light, blue_cyan, &v, 100, elapsed_time, false);
					gx3d_EnableLight(raiu.light);
					// Deactivate the heal timer
					heal_fx_timer = -1;
				}
				else {
					// Update character lighting otherwise
					gx3dVector light_pos = { raiu.pos.x, raiu.sphere.center.y, raiu.sphere.center.z - 2 };
					gx3d_DisableLight(raiu.light);
					Update_Light(&raiu.light, blue_cyan, &light_pos, 100, elapsed_time, false);
					gx3d_EnableLight(raiu.light);
				}

				// Transform character into world
				raiu.sphere = obj_raiu->bound_sphere;
				raiu.sphere.center.x = raiu.pos.x;
				gx3d_GetTranslateMatrix(&m, raiu.pos.x, raiu.pos.y, raiu.pos.z);
				gx3d_SetObjectMatrix(obj_raiu, &m);
				gx3d_SetTexture(0, tex_raiu);
				gx3d_DrawObject(obj_raiu, 0);

				// Disable specular lighting and set default material for 2d graphics
				gx3d_DisableSpecularLighting();
				gx3d_SetMaterial(&material_default);

				/*____________________________________________________________________
				|
				| Draw 2D graphics on top of 3D only during 'Running' state
				|___________________________________________________________________*/

				// Save Current view matrix
				gx3dMatrix view_save;
				gx3d_GetViewMatrix(&view_save);

				// Set new view matrix
				gx3dVector tfrom = { 0,0,-2 }, tto = { 0,0,0 }, twup = { 0,1,0 };
				gx3d_CameraSetPosition(&tfrom, &tto, &twup, gx3d_CAMERA_ORIENTATION_LOOKTO_FIXED);
				gx3d_CameraSetViewMatrix();

				// Local Variables
				const float scale_hp = 0.25;
				const float scale_hp_fonts = 0.04;
				const float scale_weapons_lv = 0.40;
				const float scale_weapon_lv_fonts = 0.08;
				const float scale_scorebar = 0.12;
				const float scale_score_fonts = 0.08;

				const float hp_font_spacing = 0.04;
				const float hp_x = -1.40;
				const float hp_y = -0.75;
				float hp_font_x = -1.38;
				const float hp_font_y = -0.77;

				const float weapon_lv_font_spacing = 0.08;
				const float weapon_lv_x = 1.40;
				const float weapon_lv_y = -0.60;
				float weapon_lv_font_x = 1.45;
				float weapon_lv_font_y = -0.73;

				const float score_font_spacing = 0.08;
				const float scorebar_x = 1.25;
				const float scorebar_y = 0.78;
				float score_font_x = 0.97;
				const float score_font_y = 0.78;

				char hp_buf[MAX_HP_FONTS + 1];
				char hp_full_buf[MAX_HP_FONTS * 2 + 1];
				char lv_buf[MAX_LV_FONTS + 1];
				char lv_full_buf[MAX_LV_FONTS * 2 + 1];
				char score_buf[MAX_SCORE_FONTS + 1];
				int dgt_ctr_1, dgt_ctr_2;
				int incr;

				// Update HP
				hp_bar_x_factor = (float)(raiu.hp) / (float)(RAIU_MAX_HP);

				//========== HP setup ==========//
				layer = gx3d_GetObjectLayer(obj_hud, "hp");
				gx3d_GetScaleMatrix(&m1, scale_hp, scale_hp, scale_hp);
				gx3d_GetTranslateMatrix(&m2, hp_x, hp_y, 0);
				gx3d_MultiplyMatrix(&m1, &m2, &m);
				gx3d_SetObjectLayerMatrix(obj_hud, layer, &m);

				// HP Bar
				layer = gx3d_GetObjectLayer(obj_hud, "hp_bar");
				gx3d_GetScaleMatrix(&m1, (scale_hp * hp_bar_x_factor), scale_hp, scale_hp);
				gx3d_GetTranslateMatrix(&m2, hp_x, hp_y, 0);
				gx3d_MultiplyMatrix(&m1, &m2, &m);
				gx3d_SetObjectLayerMatrix(obj_hud, layer, &m);

				// HP Fonts
				gx3d_GetScaleMatrix(&m1, scale_hp_fonts, scale_hp_fonts, scale_hp_fonts);
				for (int i = 0; i < MAX_HP_FONTS * 2; i++) {
					layer = gx3d_GetObjectLayer(obj_hp_fonts[i], "score_fonts");
					// first four digits (Current HP)
					if (i < MAX_HP_FONTS) {
						if (i == 0) // first digit of current hp
							gx3d_GetTranslateMatrix(&m2, hp_font_x, hp_font_y, 0);
						else
							gx3d_GetTranslateMatrix(&m2, hp_font_x += hp_font_spacing, hp_font_y, 0);
					}
					// last four digits (Max HP)
					else {
						if (i == MAX_HP_FONTS) // first digit of max hp
							gx3d_GetTranslateMatrix(&m2, hp_font_x += hp_font_spacing * 1.5, hp_font_y, 0);
						else
							gx3d_GetTranslateMatrix(&m2, hp_font_x += hp_font_spacing, hp_font_y, 0);

					}
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectLayerMatrix(obj_hp_fonts[i], layer, &m);
					gx3d_Object_UpdateTransforms(obj_hp_fonts[i]);
				}

				//========== Weapons Lv setup ==========//
				// Weapons Lv
				layer = gx3d_GetObjectLayer(obj_hud, "weapons_lv");
				gx3d_GetScaleMatrix(&m1, scale_weapons_lv, scale_weapons_lv, scale_weapons_lv);
				gx3d_GetTranslateMatrix(&m2, weapon_lv_x, weapon_lv_y, 0);
				gx3d_MultiplyMatrix(&m1, &m2, &m);
				gx3d_SetObjectLayerMatrix(obj_hud, layer, &m);

				// Weapons Lv Fonts
				gx3d_GetScaleMatrix(&m1, scale_weapon_lv_fonts, scale_weapon_lv_fonts, scale_weapon_lv_fonts);
				for (int i = 0; i < MAX_LV_FONTS * 2; i++) {
					layer = gx3d_GetObjectLayer(obj_weapon_lv_fonts[i], "score_fonts");
					// first two digits (gun lv)
					if (i < MAX_LV_FONTS) {
						if (i == 0) // first digit of gun lv
							gx3d_GetTranslateMatrix(&m2, weapon_lv_font_x, weapon_lv_font_y, 0);
						else
							gx3d_GetTranslateMatrix(&m2, weapon_lv_font_x += weapon_lv_font_spacing, weapon_lv_font_y, 0);
					}
					// last two digits (blade lv)
					else {
						if (i == MAX_LV_FONTS) // first digit of blade lv
							gx3d_GetTranslateMatrix(&m2, weapon_lv_font_x -= (weapon_lv_font_spacing * (MAX_LV_FONTS - 1)), weapon_lv_font_y += 0.19, 0);
						else
							gx3d_GetTranslateMatrix(&m2, weapon_lv_font_x += weapon_lv_font_spacing, weapon_lv_font_y, 0);

					}
					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectLayerMatrix(obj_weapon_lv_fonts[i], layer, &m);
					gx3d_Object_UpdateTransforms(obj_weapon_lv_fonts[i]);
				}

				//========== Score setup ==========//
				// Score Bar
				layer = gx3d_GetObjectLayer(obj_hud, "score_bar");
				gx3d_GetScaleMatrix(&m1, scale_scorebar * 1.5, scale_scorebar, scale_scorebar);
				gx3d_GetTranslateMatrix(&m2, scorebar_x, scorebar_y, 0);
				gx3d_MultiplyMatrix(&m1, &m2, &m);
				gx3d_SetObjectLayerMatrix(obj_hud, layer, &m);

				// Score Fonts
				gx3d_GetScaleMatrix(&m1, scale_score_fonts, scale_score_fonts, scale_score_fonts);
				for (int i = 0; i < MAX_SCORE_FONTS; i++) {
					layer = gx3d_GetObjectLayer(obj_score_fonts[i], "score_fonts");

					if (i == 0) // first digit of score
						gx3d_GetTranslateMatrix(&m2, score_font_x, score_font_y, 0);
					else
						gx3d_GetTranslateMatrix(&m2, score_font_x += score_font_spacing, score_font_y, 0);

					gx3d_MultiplyMatrix(&m1, &m2, &m);
					gx3d_SetObjectLayerMatrix(obj_score_fonts[i], layer, &m);
					gx3d_Object_UpdateTransforms(obj_score_fonts[i]);
				}

				// Update transforms
				gx3d_Object_UpdateTransforms(obj_hud);

				// Draw the appropriate objects on screen
				gx3d_DisableZBuffer();
				gx3d_EnableAlphaBlending();
				// Set the default light
				gx3d_SetAmbientLight(color3d_white);

				// Draw elements that requires the use of texture matrix (changing of UV texture coords)
				gx3d_EnableTextureMatrix(0);

				//========== HP display ==========//
				// HP background
				if (raiu.hp > RAIU_MAX_HP * 0.25) // green when hp is >25%
					gx3d_GetTranslateTextureMatrix(&m, 0, 1); // upper half of texture coords
				else // red when hp is <=25%
					gx3d_GetTranslateTextureMatrix(&m, 0, 0.5); // lower half of texture coords
				layer = gx3d_GetObjectLayer(obj_hud, "hp");
				gx3d_SetTextureMatrix(0, &m);
				gx3d_SetTexture(0, tex_hp);
				gx3d_DrawObjectLayer(layer, 0);

				// HP bar
				layer = gx3d_GetObjectLayer(obj_hud, "hp_bar");
				gx3d_SetTexture(0, tex_hp_bar);
				gx3d_DrawObjectLayer(layer, 0);

				// HP fonts
				incr = 1;
				dgt_ctr_1 = 0;
				dgt_ctr_2 = 0;
				itoa(raiu.hp, hp_buf, 10);		// convert integer to char array (string)
				strcpy(hp_full_buf, hp_buf);	// copy current hp string to a larger buffer
				itoa(RAIU_MAX_HP, hp_buf, 10);	// reuse the smaller buffer to store the max hp string
				strcat(hp_full_buf, hp_buf);	// concatenate both strings using the larger buffer
				if (raiu.hp == 0) // ensures that there is at least 1 digit displayed for the current hp when it reaches 0
					dgt_ctr_1++;
				else {
					while (raiu.hp / incr != 0) { // counts the number of digits in the current hp
						dgt_ctr_1++;
						incr *= 10;
					}
				}
				dgt_ctr_1 += MAX_HP_FONTS;
				Display_Fonts(obj_hp_fonts, hp_full_buf, dgt_ctr_1, MAX_HP_FONTS * 2, m, tex_fonts, false);	// display the fonts on screen
				gx3d_DisableTextureMatrix(0);

				//========== Weapon Lv display ==========//
				// Weapons Lv background
				layer = gx3d_GetObjectLayer(obj_hud, "weapons_lv");
				gx3d_SetTexture(0, tex_weapons_lv);
				gx3d_DrawObjectLayer(layer, 0);

				// Weapon Lv fonts
				incr = 1;
				dgt_ctr_1 = 0;
				dgt_ctr_2 = 0;
				gx3d_EnableTextureMatrix(0);
				itoa(raiu.gun_lv, lv_buf, 10);		// convert integer to char array (string)
				strcpy(lv_full_buf, lv_buf);	// copy gun lv string to a larger buffer
				itoa(raiu.blade_lv, lv_buf, 10);		// reuse the smaller buffer to store the blade lv string

				if (raiu.gun_lv == 0)
					dgt_ctr_1++;
				else
					while (raiu.gun_lv / incr != 0) { // counts the number of digits in gun_lv
						dgt_ctr_1++;
						incr *= 10;
					}
				incr = 1;

				if (raiu.blade_lv == 0)
					dgt_ctr_2++;
				else
					while (raiu.blade_lv / incr != 0) { // counts the number of digits in blade_lv
						dgt_ctr_2++;
						incr *= 10;
					}

				// concatenate '0's to the larger buffer if the number of digits in the second set are less than its max number of digits
				if (dgt_ctr_2 < MAX_LV_FONTS) {
					string str = "";
					int zero_ctr = MAX_LV_FONTS - dgt_ctr_2;
					for (int i = 0; i < zero_ctr; i++) {
						str += "0";
						dgt_ctr_1++;
					}
					strcat(lv_full_buf, str.c_str());
				}
				strcat(lv_full_buf, lv_buf);	// concatenate both strings using the larger buffer
				dgt_ctr_1 += dgt_ctr_2;
				Display_Fonts(obj_weapon_lv_fonts, lv_full_buf, dgt_ctr_1, MAX_LV_FONTS * 2, m, tex_fonts, true);	// display the fonts on screen
				gx3d_DisableTextureMatrix(0);

				//========== Score display ==========//
				// Score Bar
				layer = gx3d_GetObjectLayer(obj_hud, "score_bar");
				gx3d_SetTexture(0, tex_score_bar);
				gx3d_DrawObjectLayer(layer, 0);

				// Score Fonts
				incr = 1;
				dgt_ctr_1 = 0;
				gx3d_EnableTextureMatrix(0);
				itoa(score, score_buf, 10);		// convert integer to char array (string)

				if (score == 0)
					dgt_ctr_1++;
				else
					while (score / incr != 0) { // counts the number of digits in score
						dgt_ctr_1++;
						incr *= 10;
					}

				Display_Fonts(obj_score_fonts, score_buf, dgt_ctr_1, MAX_SCORE_FONTS, m, tex_fonts, true);	// display the fonts on screen
				gx3d_DisableTextureMatrix(0);

				gx3d_DisableAlphaBlending();
				gx3d_EnableZBuffer();

				// Restore view matrix
				gx3d_SetViewMatrix(&view_save);
			}

			// STATE: GAME ENDING
			else if (*state == STATE_GAME_ENDING) {

				// Set lighting to dim
				gx3d_SetAmbientLight(color3d_dim);

				// Initialize local variables
				gx3dVector camera_normal_heading = { 0, 0, 1 };
				float camera_lerp_duration = 1000.0f;

				float fx_shock_time_from = 0.0f, fx_shock_time_to = 1000.0f;
				float fx_destruct_charge_time_from = 2000.0f, fx_destruct_charge_time_to = 3000.0f;
				float fx_destruct_charge_loop_time_from = 3000.0f, fx_destruct_charge_loop_time_to = 5000.0f;
				float fx_destruct_flash_time_from = 6000.0f, fx_destruct_flash_time_to = 10000.0f;

				gx3dVector fx_shock_pos = { raiu.sphere.center.x, 1, 1 };
				gx3dVector fx_destruct_charge_pos = { raiu.sphere.center.x, gx3d_Lerp(2, 0, (FX_NORMAL_DURATION - (ani_raiu_ending_time - fx_destruct_charge_time_from)) / FX_NORMAL_DURATION), raiu.sphere.center.z };
				gx3dVector fx_destruct_charge_loop_pos = { raiu.sphere.center.x,  gx3d_Lerp(4, 2, (FX_NORMAL_DURATION - (ani_raiu_ending_time - fx_destruct_charge_loop_time_from)) / FX_NORMAL_DURATION), 0 };
				gx3dVector fx_destruct_flash_pos = { raiu.sphere.center.x, 5.5, -10 };

				gx3dVector fx_shock_scale = { 4.0f, 4.0f, 4.0f };
				gx3dVector fx_destruct_charge_scale = { 7.0f, 7.0f, 7.0f };
				gx3dVector fx_destruct_flash_scale = { 17.0f, 10.0f, 1.0f };

				// Reset left and right movement to 0
				cmd_move = 0;

				// Smoothly move the camera back to the center
				if (heading.x != 0 && heading.y != 0 && heading.z != 1) {
					gx3d_LerpVector(&heading, &camera_normal_heading, ((camera_lerp_duration - game_ending_speed_timer) / camera_lerp_duration), &heading);
				}

				for (int i = 0; i < current_max_enemy_count; i++) {

					// Set material for enemies and turn on specular lighting
					gx3d_SetMaterial(&material_hoshu);
					gx3d_EnableSpecularLighting();

					// Draw the current Hoshu when it is spawned
					if (enemies.hoshu[i].draw) {

						// Initialize local variables
						float angle;

						// Check first if the current Hoshu is behind the camera and needs to be recycled
						if (enemies.hoshu[i].pos.z <= ground_init_z) { // Hoshu is at or past the initial ground position
							enemies.hoshu[i].explosion_timer = -1;
							enemies.hoshu[i].explosion_type = -1;
							enemies.hoshu[i].explode_snd_initialized = false;
							enemies.hoshu[i].draw = false;

							// Decrement enemy count to spawn new enemies
							enemy_count--;
						}

						else {

							// Update z position
							enemies.hoshu[i].pos.z -= distance;
							enemies.hoshu[i].sphere.center.z = enemies.hoshu[i].pos.z;

							// Transform and draw into world
							float pos_x = enemies.hoshu[i].pos.x;
							float pos_y = enemies.hoshu[i].pos.y;
							float pos_z = enemies.hoshu[i].pos.z;

							// Update 3D laser sound position (since the world is moving)
							snd_SetSoundPosition(s_laser_2, enemies.hoshu[i].sphere.center.x, enemies.hoshu[i].sphere.center.y, enemies.hoshu[i].sphere.center.z, snd_3D_APPLY_NOW);

							// Play or update the explosion effect while the Hoshu was just destroyed
							if (enemies.hoshu[i].explosion_timer >= 0) {

								// Initialize local variables
								gx3dVector scale = { 10, 10, 10 };
								gx3dTexture fx_explosion;

								// Set explosion light to the current destroyed Hoshu
								Update_Light(&explosion_light, explosion_orange, &enemies.hoshu[i].sphere.center, Inverse_Lerp(100, 0, (FX_NORMAL_DURATION * enemies.hoshu[i].explosion_timer) / FX_NORMAL_DURATION), elapsed_time, true, 0, 0, 0.001);
								gx3d_EnableLight(explosion_light);

								// Generate a randomized explosion effect and sound
								if (enemies.hoshu[i].explosion_type == -1) {
									enemies.hoshu[i].explosion_type = random_GetInt(1, 3);
								}

								// Update 3D explosion sound positions
								snd_SetSoundPosition(s_explosion_1, enemies.hoshu[i].sphere.center.x, enemies.hoshu[i].sphere.center.y, enemies.hoshu[i].sphere.center.z, snd_3D_APPLY_NOW);
								snd_SetSoundPosition(s_explosion_2, enemies.hoshu[i].sphere.center.x, enemies.hoshu[i].sphere.center.y, enemies.hoshu[i].sphere.center.z, snd_3D_APPLY_NOW);
								snd_SetSoundPosition(s_explosion_3, enemies.hoshu[i].sphere.center.x, enemies.hoshu[i].sphere.center.y, enemies.hoshu[i].sphere.center.z, snd_3D_APPLY_NOW);

								if (enemies.hoshu[i].explosion_type == 1) {
									if (!enemies.hoshu[i].explode_snd_initialized) {
										snd_PlaySound(s_explosion_1, 0);
										enemies.hoshu[i].explode_snd_initialized = true;
										gx3d_MultiplyScalarVector(2, &scale, &scale); // doubles the scale
									}
									else { // update sound position

									}
									fx_explosion = fx_explosion_1;
								}
								else if (enemies.hoshu[i].explosion_type == 2) {
									if (!enemies.hoshu[i].explode_snd_initialized) {
										snd_PlaySound(s_explosion_2, 0);
										enemies.hoshu[i].explode_snd_initialized = true;
									}
									fx_explosion = fx_explosion_2;
								}
								else {
									if (!enemies.hoshu[i].explode_snd_initialized) {
										snd_PlaySound(s_explosion_3, 0);
										enemies.hoshu[i].explode_snd_initialized = true;
									}
									fx_explosion = fx_explosion_3;
								}

								// Display and update an effect after a certain amount of time in the animation
								if (enemies.hoshu[i].explosion_timer <= FX_NORMAL_DURATION) {
									Play_FX(fx_explosion, billboard_normal, enemies.hoshu[i].sphere.center, scale, FX_NORMAL_DURATION, enemies.hoshu[i].explosion_timer, 100, false);
									gx3d_SetAmbientLight(color3d_dim);

									// Update timer
									enemies.hoshu[i].explosion_timer += elapsed_time;
								}

								// Resets explosion timer and sets draw to false when the explosion effect has finished
								else {
									enemies.hoshu[i].explosion_timer = -1;
									enemies.hoshu[i].explosion_type == -1;
									enemies.hoshu[i].explode_snd_initialized = false;
									enemies.hoshu[i].draw = false;

									// Disable explosion light
									gx3d_DisableLight(explosion_light);
								}
							}

							// Continue displaying the Hoshu while not destroyed
							else {
								layer = gx3d_GetObjectLayer(obj_hoshu, "bottom");
								gx3d_GetTranslateMatrix(&m1, pos_x, pos_y, pos_z);
								gx3d_SetObjectLayerMatrix(obj_hoshu, layer, &m1);

								// Set layer to the "top" layer of the Hoshu model
								layer = gx3d_GetObjectLayer(obj_hoshu, "top");

								// Update the Hoshu view vector to point at the character's xz-coordinates
								hoshu_view.x = -(raiu.pos.x - enemies.hoshu[i].pos.x);
								hoshu_view.y = 0;
								hoshu_view.z = (raiu.pos.z - enemies.hoshu[i].pos.z);

								// Compute angle between the two
								angle = gx3d_AngleBetweenVectors(&hoshu_normal, &hoshu_view);
								if ((hoshu_view.x - hoshu_normal.x) < 0)
									angle *= -1;

								// Make sure that angle of rotation does not exceed its max
								if (angle >= 0 && angle > ROTATE_RIGHT_MAX)
									angle = ROTATE_RIGHT_MAX;
								else if (angle < 0 && angle < ROTATE_LEFT_MAX)
									angle = ROTATE_LEFT_MAX;
								gx3d_GetRotateYMatrix(&m1, angle);
								gx3d_SetObjectLayerMatrix(obj_hoshu, layer, &m1);

								// Update transformations
								gx3d_Object_UpdateTransforms(obj_hoshu);

								// Draw layers
								gx3d_SetTexture(0, tex_hoshu);

								layer = gx3d_GetObjectLayer(obj_hoshu, "bottom");
								gx3d_DrawObjectLayer(layer, 0);

								layer = gx3d_GetObjectLayer(obj_hoshu, "top");
								gx3d_DrawObjectLayer(layer, 0);
							}
						}
					}

					// Disable specular lighting and set materail for red lasers
					gx3d_DisableSpecularLighting();
					gx3d_SetMaterial(&material_red_laser);

					// Update any lasers fired by each Hoshu that is drawn in the world
					for (int j = 0; j < HOSHU_MAX_LASER_COUNT; j++) {
						if (enemies.hoshu[i].laser[j].draw) {

							// Initialize local variables
							float total_laser_distance;

							// Enable alpha blending and set ambient light to white
							gx3d_DisableAlphaBlending();
							gx3d_SetAmbientLight(color3d_white);

							// Slow down laser movement when destroyed
							if (enemies.hoshu[i].laser[j].destroyed && enemies.hoshu[i].laser[j].trajectory.velocity == hoshu_laser_speed)
								enemies.hoshu[i].laser[j].trajectory.velocity *= 0.10;

							// Calculate the distance traveled by the projectile based on the elapsed time
							total_laser_distance = (enemies.hoshu[i].laser[j].trajectory.velocity) * (elapsed_time / 1000.0f);

							// Don't draw if projectile goes beyond the initial ground position
							if (abs(enemies.hoshu[i].laser[j].pos.x) >= MAX_PROJECTILE_DISTANCE, abs(enemies.hoshu[i].laser[j].pos.y) >= MAX_PROJECTILE_DISTANCE, abs(enemies.hoshu[i].laser[j].pos.z) >= MAX_PROJECTILE_DISTANCE) {
								enemies.hoshu[i].laser[j].hit_timer = -1;
								enemies.hoshu[i].laser[j].draw = false;

								// Reset all laser variables to initial values
								enemies.hoshu[i].laser[j].distance = { 0, 0, 0 };
								enemies.hoshu[i].laser[j].sphere = obj_laser->bound_sphere;
								enemies.hoshu[i].laser[j].pos = enemies.hoshu[i].pos;
								enemies.hoshu[i].laser[j].world_shift = 0;
								enemies.hoshu[i].laser[j].destroyed = false;
								enemies.hoshu[i].laser[j].trajectory = { {0, 0, -1}, 0 };
							}

							// Display the laser or the hit effect otherwise
							else {

								// Play or update the laser hit effect while the laser had just hit an object
								if (enemies.hoshu[i].laser[j].hit_timer >= 0) {

									// Initialize local variables
									gx3dVector scale = { 4, 4, 4 };
									gx3dVector pos;

									// Set effect position
									if (enemies.hoshu[i].laser[j].destroyed)
										pos = { enemies.hoshu[i].laser[j].pos.x, enemies.hoshu[i].laser[j].pos.y, enemies.hoshu[i].laser[j].pos.z };
									else
										pos = { raiu.pos.x, raiu.sphere.center.y, raiu.pos.z + 1 };

									// Display and update an effect after a certain amount of time in the animation
									if (enemies.hoshu[i].laser[j].hit_timer <= FX_NORMAL_DURATION) {
										Play_FX(fx_laser_red, billboard_normal, pos, scale, FX_NORMAL_DURATION / 2.0f, enemies.hoshu[i].laser[j].hit_timer, 100, false);
										gx3d_SetAmbientLight(color3d_dim);

										// Update timer
										enemies.hoshu[i].laser[j].hit_timer += elapsed_time;
									}

									// Resets the timer and sets draw to false when the hit effect has finished
									else {
										enemies.hoshu[i].laser[j].hit_timer = -1;
										enemies.hoshu[i].laser[j].draw = false;

										// Reset all laser variables to initial values
										enemies.hoshu[i].laser[j].distance = { 0, 0, 0 };
										enemies.hoshu[i].laser[j].sphere = obj_laser->bound_sphere;
										enemies.hoshu[i].laser[j].pos = enemies.hoshu[i].pos;
										enemies.hoshu[i].laser[j].world_shift = 0;
										enemies.hoshu[i].laser[j].destroyed = false;
										enemies.hoshu[i].laser[j].trajectory = { {0, 0, -1}, 0 };
									}
								}

								else {

									// Translate to world then draw
									gx3d_GetTranslateMatrix(&m1, enemies.hoshu[i].laser[j].sphere.center.x, enemies.hoshu[i].laser[j].pos.y, enemies.hoshu[i].laser[j].pos.z);
									gx3d_GetScaleMatrix(&m2, hoshu_laser_scale, hoshu_laser_scale, hoshu_laser_scale);
									gx3d_MultiplyMatrix(&m2, &m1, &m);
									gx3d_SetObjectMatrix(obj_laser, &m);
									gx3d_SetTexture(0, tex_red_laser);
									gx3d_DrawObject(obj_laser, 0);

								}

								// Update laser position based on the total laser distance traveled
								gx3d_MultiplyScalarVector(total_laser_distance, &enemies.hoshu[i].laser[j].trajectory.direction, &v);
								gx3d_AddVector(&enemies.hoshu[i].laser[j].pos, &v, &enemies.hoshu[i].laser[j].pos);
								enemies.hoshu[i].laser[j].sphere.center.x = enemies.hoshu[i].laser[j].pos.x;
								enemies.hoshu[i].laser[j].sphere.center.y = enemies.hoshu[i].laser[j].pos.y;
								enemies.hoshu[i].laser[j].sphere.center.z = enemies.hoshu[i].laser[j].pos.z;
							}
							gx3d_SetAmbientLight(color3d_dim);
						}
					}
				}

				// Set ambient light to dim
				gx3d_SetAmbientLight(color3d_dim);

				// Update sound effects when unpaused
				snd_SetSoundFrequency(s_footstep, (snd_GetSoundFrequency(s_footstep) * (1.0f - ((NORMAL_SPEED - speed) / NORMAL_SPEED))));
				if (!snd_IsPlaying(s_footstep)) {
					snd_PlaySound(s_footstep, 1);
				}

				// Deactivate swing timer when it is active
				if (raiu.blade_timer != -1) {
					raiu.blade_timer = -1;
					play_swing_1 = false;
					play_swing_2 = false;
				}

				// Set material for character and enable specular lighting
				gx3d_SetMaterial(&material_raiu);
				gx3d_EnableSpecularLighting();

				// Update the animation ending blend trees depending on whichever one is the one playing
				// Play the trip animation right after the speed had decreased to 0
				if (game_ending_speed_timer <= lerp_speed_duration) {
					gx3d_Motion_Update(ani_raiu_trip, (game_ending_speed_timer / 1000.0f), false); // update run animation speed depending on the decreasing speed
					gx3d_BlendTree_Update(btree_trip);
				}

				// Play the self destruct animation and effects after the trip animation
				else {

					// Activate the timer and play the sound effect for self destruct
					if (ani_raiu_ending_time == -1) {
						ani_raiu_ending_time = 0;
						snd_PlaySound(s_ending, 0);
					}

					// State switches to Game Over after the animation and sound effect
					else if (!snd_IsPlaying(s_ending)) {
						// Set the 3D viewport clear color to white
						color.r = 255;
						color.g = 255;
						color.b = 255;
						color.a = 0;
						gx3d_ClearViewport(gx3d_CLEAR_SURFACE | gx3d_CLEAR_ZBUFFER, color, gx3d_MAX_ZBUFFER_VALUE, 0);

						// Update game state to Game Over
						*state = STATE_GAME_OVER;
						next_screen = true; // continue to game over screen
					}

					// Update otherwise
					else {
						ani_raiu_ending_time += elapsed_time;
					}
					gx3d_Motion_Update(ani_raiu_self_destruct, (ani_raiu_ending_time / 1000.0f), false);
					gx3d_BlendTree_Update(btree_self_destruct);

					// Fade out the bgm volume at game ending
					if (current_bgm_volume > 0 && ani_raiu_ending_time >= fx_shock_time_from)
						snd_SetSoundVolume(s_game_bgm, (current_bgm_volume -= 0.12f));
					else if (current_bgm_volume <= 0) {
						// set the volume to bgm volume otherwise
						current_bgm_volume = 0;
						snd_SetSoundVolume(s_game_bgm, current_bgm_volume);
					}
					else if (current_bgm_volume == 0)
						snd_StopSound(s_game_bgm);

					// Display and update an effect after a certain amount of time in the animation
					// Shock FX
					if (ani_raiu_ending_time >= fx_shock_time_from && ani_raiu_ending_time <= fx_shock_time_to) {
						Play_FX(fx_destruct_shock, billboard_normal, fx_shock_pos, fx_shock_scale, FX_NORMAL_DURATION, ani_raiu_ending_time, 100, false);
						gx3d_SetAmbientLight(color3d_dim);
						// Set character lighting to lightning purple and flicker when within the special effect time window
						gx3d_DisableLight(raiu.light);
						Update_Light(&raiu.light, lightning_purple, &fx_shock_pos, 100, elapsed_time, true);
						gx3d_EnableLight(raiu.light);
						
					}
					// Self Destruct Charge Start
					else if (ani_raiu_ending_time >= fx_destruct_charge_time_from && ani_raiu_ending_time <= fx_destruct_charge_time_to) {
						Play_FX(fx_destruct_charge, billboard_normal, fx_destruct_charge_pos, fx_destruct_charge_scale, FX_NORMAL_DURATION, ani_raiu_ending_time - fx_destruct_charge_time_from, 100, false);
						gx3d_SetAmbientLight(color3d_dim);
						// Set character lighting to lightning purple and flicker when within the special effect time window
						gx3d_DisableLight(raiu.light);
						Update_Light(&raiu.light, lightning_purple, &fx_destruct_charge_pos, 300, elapsed_time, true);
						gx3d_EnableLight(raiu.light);
					}
					// Self Destruct Charge Loop
					else if (ani_raiu_ending_time >= fx_destruct_charge_loop_time_from && ani_raiu_ending_time <= fx_destruct_charge_loop_time_to) {
						Play_FX(fx_destruct_charge_loop, billboard_normal, fx_destruct_charge_loop_pos, fx_destruct_charge_scale, FX_NORMAL_DURATION/2, ani_raiu_ending_time - fx_destruct_charge_loop_time_from, 100, true);
						gx3d_SetAmbientLight(color3d_dim);
						gx3d_DisableLight(raiu.light);
						Update_Light(&raiu.light, lightning_purple, &fx_destruct_charge_loop_pos, 300, elapsed_time, true);
						gx3d_EnableLight(raiu.light);
					}
					// Self Destruct Flash
					else if (ani_raiu_ending_time >= fx_destruct_flash_time_from && ani_raiu_ending_time <= fx_destruct_flash_time_to) {
						Play_FX(fx_destruct_flash, billboard_normal, fx_destruct_flash_pos, fx_destruct_flash_scale, FX_NORMAL_DURATION, ani_raiu_ending_time - fx_destruct_flash_time_from, 50, false);
						gx3d_SetAmbientLight(color3d_dim);
					}
				}
				
				// Transform character into world when not moving to the next screen
				if (!next_screen) {
					raiu.sphere = obj_raiu->bound_sphere;
					raiu.sphere.center.x = raiu.pos.x;

					// Update sound listener position
					snd_SetListenerPosition(raiu.pos.x, raiu.sphere.center.y, raiu.pos.z, snd_3D_APPLY_NOW);

					gx3d_GetTranslateMatrix(&m, raiu.pos.x, raiu.pos.y, raiu.pos.z);
					gx3d_SetObjectMatrix(obj_raiu, &m);
					gx3d_SetTexture(0, tex_raiu);
					gx3d_DrawObject(obj_raiu, 0);

					// Reset the light to dark gray
					gx3d_SetAmbientLight(color3d_darkgray);
					gx3d_EnableAlphaBlending();
				}

				// Destroy all enemies on screen and add their score values to the total score and inrement enemeies defeated for each one before continuing to the game over screen
				else {
					for (int i = 0; i < MAX_ENEMY_COUNT; i++) {
						if (enemies.hoshu[i].draw) {
							score += enemies.hoshu[i].score_amt;
							enemies.hoshu[i].draw = false;
							enemies_defeated++;
							hoshus_defeated++;
						}
					}
				}

				// Disable specular lighting
				gx3d_DisableSpecularLighting();
			}

			// Stop rendering
			gx3d_EndRender();

			// Save screenshot
			if (take_screenshot) {
				char str[50];
				char buff[80];
				static int screenshot_count = 0;
				strcpy(str, SCREENSHOT_FILENAME);
				strcat(str, itoa(screenshot_count++, buff, 10));
				strcat(str, ".bmp");
				gxWriteBMPFile(str);
				// Reset the screenshot flag back to false
				take_screenshot = false;
			}

			// Pause the game after updating the world once after the game is restored from context switching
			if (restored && update_once) {
				pause = true;
				restored = false;
				update_once = false;
			}

			// Page flip (so user can see it)
			gxFlipVisualActivePages(FALSE);
		}
	}

	Free_GameScreen();

	// Prevents quitting the game
	return false;
}

/*____________________________________________________________________
|
| Function: Render_GameOverScreen
|
| Input: Called from Render_Game_Loop
| Output:
|___________________________________________________________________*/

bool Render_GameOverScreen(int *state) {

	/*____________________________________________________________________
	|
	| Main game loop
	|___________________________________________________________________*/

	// Variables
	unsigned elapsed_time, new_time, last_time;
	float game_over_timer, current_bgm_volume;
	bool enter_pressed;
	gx3dVector billboard_normal, fade_pos, fade_scale;
	int hours, minutes, seconds;

	// Init loop variables
	last_time = 0;
	enter_pressed = false;
	billboard_normal = { 0,0,-1 };
	fade_pos = { 0, 0, 1 };
	fade_scale = { 17.0f, 10.0f, 1.0f };
	game_over_timer = -1;
	current_bgm_volume = 50.0f;

	const float scale_score_fonts = 0.08;
	const float score_font_spacing = 0.08;
	float score_font_x = -1.2;
	const float score_font_y = 0.3;
	char score_buf[MAX_SCORE_FONTS + 1];

	const float scale_time_fonts = 0.08;
	const float time_font_spacing = 0.08;
	float time_hr_font_x = -1.13;
	float time_min_font_x = -0.94;
	float time_sec_font_x = -0.76;
	const float time_font_y = -0.038;
	char time_hr_buf[MAX_TIME_FONTS + 1], time_min_buf[MAX_TIME_FONTS + 1], time_sec_buf[MAX_TIME_FONTS + 1];

	const float scale_defeated_fonts = 0.08;
	const float defeated_font_spacing = 0.08;
	float defeated_font_x = -0.95;
	const float defeated_font_y = -0.4;
	char defeated_buf[MAX_DEFEATED_FONTS + 1];
	
	int dgt_ctr_1;
	int incr;
	bool fonts_init = false;

	// Setup game over screen sounds
	snd_SetSoundVolume(s_game_over_bgm, current_bgm_volume);
	snd_SetSoundVolume(s_select, sfx_volume);

	// Plays the background music repeatedly
	snd_PlaySound(s_game_over_bgm, 1);

	// Convert total time played in milliseconds to hours : minutes : seconds
	seconds = game_timer / 1000;
	minutes = seconds / 60;

	hours = minutes / 60;
	minutes = minutes % 60;
	seconds = seconds % 60;

	// Game loop
	for (next_screen = FALSE; NOT next_screen || snd_IsPlaying(s_select); ) {

		/*____________________________________________________________________
		|
		| Update elapsed time and other timers
		|___________________________________________________________________*/

		// Get the current time (# milliseconds since the game started)
		new_time = timeGetTime();
		// Compute the elapsed time (in milliseconds) since the last time through this loop
		if (last_time == 0)
			elapsed_time = 0;
		else
			elapsed_time = new_time - last_time;
		last_time = new_time;

		// Update game over timer
		if (game_over_timer <= 3000) {
			game_over_timer += elapsed_time;
		}

		/*____________________________________________________________________
		|
		| Process user input
		|___________________________________________________________________*/

		// Any event ready?
		if (evGetEvent(&event)) {
			// key press?
			if (event.type == evTYPE_RAW_KEY_PRESS) {
				// If ESC pressed, exit the program
				if (event.keycode == evKY_ESC)
					return true;
				if (event.keycode == evKY_F1)
					take_screenshot = true;
				if (!snd_IsPlaying(s_select)) {
					if (event.keycode == evKY_ENTER) {
						snd_PlaySound(s_select, 0);
						enter_pressed = true;
					}
				}
			}
		}

		// start game after the sound effect when enter is pressed at the help screen
		if (enter_pressed && !snd_IsPlaying(s_select)) {
			*state = STATE_TITLE_SCREEN;
			next_screen = true;
		}

		/*____________________________________________________________________
		|
		| Draw graphics
		|___________________________________________________________________*/

		// Displays loading screen when getting ready to switch screens
		if (next_screen)
			Display_LoadingScreen();

		else {
			// Clear viewport
			gx3d_ClearViewport(gx3d_CLEAR_SURFACE | gx3d_CLEAR_ZBUFFER, color, gx3d_MAX_ZBUFFER_VALUE, 0);
			// Start rendering in 3D
			if (gx3d_BeginRender()) {
				// Set the default light
				gx3d_SetAmbientLight(color3d_white);
				// Set the default material
				gx3d_SetMaterial(&material_default);
				// Disable specular lighting
				gx3d_DisableSpecularLighting();

				// Fade in game bgm at starting
				if (current_bgm_volume < bgm_volume)
					snd_SetSoundVolume(s_game_over_bgm, (current_bgm_volume += 1.0f));
				else {
					// set the volume to bgm volume otherwise
					snd_SetSoundVolume(s_game_over_bgm, bgm_volume);
				}

				/*____________________________________________________________________
				|
				| Draw 2D graphics on top of 3D
				|___________________________________________________________________*/

				// Save Current view matrix
				gx3dMatrix view_save;
				gx3d_GetViewMatrix(&view_save);

				// Set new view matrix
				gx3dVector tfrom = { 0,0,-2 }, tto = { 0,0,0 }, twup = { 0,1,0 };
				gx3d_CameraSetPosition(&tfrom, &tto, &twup, gx3d_CAMERA_ORIENTATION_LOOKTO_FIXED);
				gx3d_CameraSetViewMatrix();

				// Draw the appropriate objects on screen
				gx3d_DisableZBuffer();
				gx3d_EnableAlphaBlending();

				// Game Over Screen Objects
				layer = gx3d_GetObjectLayer(obj_hud, "full_screen_l");
				gx3d_SetTexture(0, tex_game_over_l);
				gx3d_DrawObjectLayer(layer, 0);

				layer = gx3d_GetObjectLayer(obj_hud, "full_screen_r");
				gx3d_SetTexture(0, tex_game_over_r);
				gx3d_DrawObjectLayer(layer, 0);

				// Initialize fonts positions when they have not yet been set
				if (!fonts_init) {

					// Final Score Fonts
					gx3d_GetScaleMatrix(&m1, scale_score_fonts, scale_score_fonts, scale_score_fonts);
					for (int i = 0; i < MAX_SCORE_FONTS; i++) {
						layer = gx3d_GetObjectLayer(obj_score_fonts[i], "score_fonts");

						if (i == 0) // first digit of score
							gx3d_GetTranslateMatrix(&m2, score_font_x, score_font_y, 1);
						else
							gx3d_GetTranslateMatrix(&m2, score_font_x += score_font_spacing, score_font_y, 1);

						gx3d_MultiplyMatrix(&m1, &m2, &m);
						gx3d_SetObjectLayerMatrix(obj_score_fonts[i], layer, &m);
						gx3d_Object_UpdateTransforms(obj_score_fonts[i]);
					}

					// Total Gameplay Time Fonts
					gx3d_GetScaleMatrix(&m4, scale_time_fonts, scale_time_fonts, scale_time_fonts);
					for (int i = 0; i < 2; i++) {

						if (i == 0) {// first digit of hours, minutes, and seconds
							gx3d_GetTranslateMatrix(&m1, time_hr_font_x, time_font_y, 1); // hours
							gx3d_GetTranslateMatrix(&m2, time_min_font_x, time_font_y, 1); // minutes
							gx3d_GetTranslateMatrix(&m3, time_sec_font_x, time_font_y, 1); // seconds
						}
						else {
							gx3d_GetTranslateMatrix(&m1, time_hr_font_x += time_font_spacing, time_font_y, 1); // hours
							gx3d_GetTranslateMatrix(&m2, time_min_font_x += time_font_spacing, time_font_y, 1); // minutes
							gx3d_GetTranslateMatrix(&m3, time_sec_font_x += time_font_spacing, time_font_y, 1); // seconds
						}
						gx3d_MultiplyMatrix(&m4, &m1, &m1); // hours
						gx3d_MultiplyMatrix(&m4, &m2, &m2); // minutes
						gx3d_MultiplyMatrix(&m4, &m3, &m3); // seconds

						layer = gx3d_GetObjectLayer(obj_hr_fonts[i], "score_fonts");
						gx3d_SetObjectLayerMatrix(obj_hr_fonts[i], layer, &m1); // hours
						layer = gx3d_GetObjectLayer(obj_min_fonts[i], "score_fonts");
						gx3d_SetObjectLayerMatrix(obj_min_fonts[i], layer, &m2); // minutes
						layer = gx3d_GetObjectLayer(obj_sec_fonts[i], "score_fonts");
						gx3d_SetObjectLayerMatrix(obj_sec_fonts[i], layer, &m3); // seconds

						gx3d_Object_UpdateTransforms(obj_hr_fonts[i]); // hours
						gx3d_Object_UpdateTransforms(obj_min_fonts[i]); // minutes
						gx3d_Object_UpdateTransforms(obj_sec_fonts[i]); // seconds
					}
					
					// Total Enemies Defeated Fonts
					gx3d_GetScaleMatrix(&m1, scale_defeated_fonts, scale_defeated_fonts, scale_defeated_fonts);
					for (int i = 0; i < MAX_DEFEATED_FONTS; i++) {
						layer = gx3d_GetObjectLayer(obj_defeated_fonts[i], "score_fonts");

						if (i == 0) // first digit of score
							gx3d_GetTranslateMatrix(&m2, defeated_font_x, defeated_font_y, 1);
						else
							gx3d_GetTranslateMatrix(&m2, defeated_font_x += defeated_font_spacing, defeated_font_y, 1);

						gx3d_MultiplyMatrix(&m1, &m2, &m);
						gx3d_SetObjectLayerMatrix(obj_defeated_fonts[i], layer, &m);
						gx3d_Object_UpdateTransforms(obj_defeated_fonts[i]);
					}

					fonts_init = true;
				}

				// Draw elements that requires the use of texture matrix (changing of UV texture coords)
				gx3d_EnableTextureMatrix(0);

				// Final Score Display
				incr = 1;
				dgt_ctr_1 = 0;
				itoa(score, score_buf, 10);		// convert integer to char array (string)

				if (score == 0)
					dgt_ctr_1++;
				else
					while (score / incr != 0) { // counts the number of digits in score
						dgt_ctr_1++;
						incr *= 10;
					}

				Display_Fonts(obj_score_fonts, score_buf, dgt_ctr_1, MAX_SCORE_FONTS, m, tex_fonts, false);	// display the fonts on screen

				// Total Time Played Display
				// HOURS
				incr = 1;
				dgt_ctr_1 = 0;
				itoa(hours, time_hr_buf, 10);		// convert integer to char array (string)

				if (hours == 0)
					dgt_ctr_1++;
				else
					while (hours / incr != 0) { // counts the number of digits in hours
						dgt_ctr_1++;
						incr *= 10;
					}

				Display_Fonts(obj_hr_fonts, time_hr_buf, dgt_ctr_1, MAX_TIME_FONTS, m, tex_fonts, true);	// display the fonts on screen

				// MINUTES
				incr = 1;
				dgt_ctr_1 = 0;
				itoa(minutes, time_min_buf, 10); // convert integer to char array (string)

				if (minutes == 0)
					dgt_ctr_1++;
				else
					while (minutes / incr != 0) { // counts the number of digits in minutes
						dgt_ctr_1++;
						incr *= 10;
					}

				Display_Fonts(obj_min_fonts, time_min_buf, dgt_ctr_1, MAX_TIME_FONTS, m, tex_fonts, true);	// display the fonts on screen

				// SECONDS
				incr = 1;
				dgt_ctr_1 = 0;
				itoa(seconds, time_sec_buf, 10); // convert integer to char array (string)

				if (seconds == 0)
					dgt_ctr_1++;
				else
					while (seconds / incr != 0) { // counts the number of digits in seconds
						dgt_ctr_1++;
						incr *= 10;
					}

				Display_Fonts(obj_sec_fonts, time_sec_buf, dgt_ctr_1, MAX_TIME_FONTS, m, tex_fonts, true);	// display the fonts on screen

				// Total Enemies Defeated Display
				incr = 1;
				dgt_ctr_1 = 0;
				itoa(enemies_defeated, defeated_buf, 10);		// convert integer to char array (string)

				if (enemies_defeated == 0)
					dgt_ctr_1++;
				else
					while (enemies_defeated / incr != 0) { // counts the number of digits in enemies defeated
						dgt_ctr_1++;
						incr *= 10;
					}

				Display_Fonts(obj_defeated_fonts, defeated_buf, dgt_ctr_1, MAX_DEFEATED_FONTS, m, tex_fonts, false);	// display the fonts on screen

				// Disable texture matrix and alpha blending
				gx3d_DisableTextureMatrix(0);
				gx3d_DisableAlphaBlending();
				gx3d_EnableZBuffer();

				Play_FX(fx_fade_white, billboard_normal, fade_pos, fade_scale, FX_NORMAL_DURATION, game_over_timer, 0, false);

				// Restore view matrix
				gx3d_SetViewMatrix(&view_save);

				// Stop rendering
				gx3d_EndRender();

				// Save screenshot
				if (take_screenshot) {
					char str[50];
					char buff[80];
					static int gameover_screenshot_count = 0;
					strcpy(str, SCREENSHOT_GAMEOVER);
					strcat(str, itoa(gameover_screenshot_count++, buff, 10));
					strcat(str, ".bmp");
					gxWriteBMPFile(str);
					// Reset the screenshot flag back to false
					take_screenshot = false;
				}

				// Page flip (so user can see it)
				gxFlipVisualActivePages(FALSE);
			}
		}
	}
	Free_GameOverScreen();

	return false; // continue to next screen
}

/*____________________________________________________________________
|
| Function: Render_Free
|
| Input: Called from ____
| Output:
|___________________________________________________________________*/

void Render_Free(void)
{
	if (initialized) {
		gx3d_FreeAllObjects();
		gx3d_FreeAllTextures();
		gx3d_FreeLight(dir_light);
		gx3d_FreeLight(explosion_light);
		gx3d_FreeLight(raiu.light);
		for (Structure_Lights structure : structure_light)
			gx3d_FreeLight(structure.light);

		initialized = FALSE;
	}
}

/*____________________________________________________________________
|
| Function: Free_TitleScreen
|
| Input: Called from Render_TitleScreen
| Output:
|___________________________________________________________________*/

void Free_TitleScreen() {

	/*____________________________________________________________________
	|
	| Free stuff and exit
	|___________________________________________________________________*/
	if (obj_billboards)
		gx3d_FreeObject(obj_billboards);
	if (obj_quit_button)
		gx3d_FreeObject(obj_quit_button);
	if (snd_IsPlaying(s_title_screen_bgm))
		snd_StopSound(s_title_screen_bgm);
	snd_Free();

	initialized = FALSE;
}

/*____________________________________________________________________
|
| Function: Free_GameScreen
|
| Input: Called from Render_GameScreen
| Output:
|___________________________________________________________________*/

void Free_GameScreen() {

	/*____________________________________________________________________
	|
	| Free stuff and exit
	|___________________________________________________________________*/

	// Free Objects
	gx3d_FreeObject(obj_raiu);
	gx3d_FreeObject(obj_hoshu);
	gx3d_FreeObject(obj_skydome);
	gx3d_FreeObject(obj_ground);
	gx3d_FreeObject(obj_structures);
	gx3d_FreeObject(obj_laser);
	gx3d_FreeObject(obj_fence);
	for (int i = 0; i < MAX_HP_FONTS * 2; i++)
		gx3d_FreeObject(obj_hp_fonts[i]);
	for (int i = 0; i < MAX_LV_FONTS; i++)
		gx3d_FreeObject(obj_weapon_lv_fonts[i]);

	// Free Textures
	gx3d_FreeTexture(tex_hp);
	gx3d_FreeTexture(tex_hp_bar);
	gx3d_FreeTexture(tex_score_bar);
	gx3d_FreeTexture(tex_weapons_lv);
	gx3d_FreeTexture(tex_raiu);
	gx3d_FreeTexture(tex_hoshu);
	gx3d_FreeTexture(tex_blue_laser);
	gx3d_FreeTexture(tex_red_laser);
	gx3d_FreeTexture(tex_skydome);
	gx3d_FreeTexture(tex_earth);
	gx3d_FreeTexture(tex_ground);
	gx3d_FreeTexture(tex_ground_inner);
	gx3d_FreeTexture(tex_ground_under);
	gx3d_FreeTexture(tex_structures);
	gx3d_FreeTexture(fx_run_charge);
	gx3d_FreeTexture(fx_fence);
	gx3d_FreeTexture(fx_explosion_1);
	gx3d_FreeTexture(fx_explosion_2);
	gx3d_FreeTexture(fx_explosion_3);
	gx3d_FreeTexture(fx_laser_blue);
	gx3d_FreeTexture(fx_laser_red);
	gx3d_FreeTexture(fx_level_up);
	gx3d_FreeTexture(fx_destruct_shock);
	gx3d_FreeTexture(fx_destruct_charge);
	gx3d_FreeTexture(fx_destruct_charge_loop);
	gx3d_FreeTexture(fx_destruct_flash);

	// Free Sounds
	snd_StopSound(s_game_bgm);
	snd_Free();

	// Free Lights
	gx3d_FreeLight(dir_light);
	gx3d_FreeLight(raiu.light);
	gx3d_FreeLight(explosion_light);
	gx3d_FreeLight(heal_pad.light);

	// Free Particle Systems
	gx3d_FreeParticleSystem(heal_pad.psys);

	initialized = FALSE;
}

/*____________________________________________________________________
|
| Function: Free_GameOverScreen
|
| Input: Called from Render_TitleScreen
| Output:
|___________________________________________________________________*/

void Free_GameOverScreen() {

	/*____________________________________________________________________
	|
	| Free stuff and exit
	|___________________________________________________________________*/
	if (obj_hud)
		gx3d_FreeObject(obj_hud);
	for (int i = 0; i < MAX_SCORE_FONTS; i++)
		if (obj_score_fonts[i])
			gx3d_FreeObject(obj_score_fonts[i]);

	if (snd_IsPlaying(s_game_over_bgm))
		snd_StopSound(s_game_over_bgm);
	snd_Free();

	initialized = FALSE;
}

/*____________________________________________________________________
|
| Function: Load_Motion
|
| Input: Called from ___
| Output: Loads a gx3dMotion and prints info to DEBUG.TXT.
|___________________________________________________________________*/

static gx3dMotion *Load_Motion(gx3dMotionSkeleton *mskeleton, char *filename, int fps, gx3dMotionMetadataRequest *metadata_requested, int num_metadata_requested, bool load_all_metadata)
{
	int i;
	char str[250];
	gx3dMotion *motion = 0;

	motion = gx3d_Motion_Read_LWS_File(mskeleton, filename, fps, metadata_requested, num_metadata_requested, load_all_metadata);
	if (motion) {
		sprintf(str, "ANIMATION LOADED: %s", filename);
		DEBUG_WRITE(str);
		sprintf(str, "  Fps: %d", motion->keys_per_second);
		DEBUG_WRITE(str);
		sprintf(str, "  # keys: %d", motion->max_nkeys);
		DEBUG_WRITE(str);
		//    sprintf (str, "  Duration: %f", 1.0 / motion->keys_per_second * (motion->max_nkeys-1));
		//    DEBUG_WRITE (str);                              
		sprintf(str, "  Duration: %f", motion->duration / 1000.0f);
		DEBUG_WRITE(str);
		for (i = 0; i < motion->num_metadata; i++) {
			sprintf(str, "  Metadata: %s", motion->metadata[i].name);
			DEBUG_WRITE(str);
		}
		DEBUG_WRITE("");
	}
	return (motion);
}

/*____________________________________________________________________
|
| Function: Display_Fonts
|
| Input: Called from Render_GameScreen
| Output: parses a char array of digits (integers) into graphical 
|		  digits for the HUD.
|___________________________________________________________________*/

static void Display_Fonts(gx3dObject *billboards[], char buf[], int buf_size, int max_buf_size, gx3dMatrix m, gx3dTexture tex, bool show_zeros) 
{
	// Local variables
	float u, v;

	// draws each digits in reverse to accomodate the varying size of the buffer
	for (int i = buf_size-1, j = max_buf_size -1; i >= 0; i--, j--) {
		Display_Font(billboards[j], buf[i], m, tex);
	}
	if (show_zeros) {
		u = 0; v = 0;
		gx3d_GetTranslateTextureMatrix(&m, u, v);
		for (int i = max_buf_size - buf_size - 1; i >= 0; i--) {
			layer = gx3d_GetObjectLayer(billboards[i], "score_fonts");
			gx3d_SetTextureMatrix(0, &m);
			gx3d_SetTexture(0, tex);
			gx3d_DrawObjectLayer(layer, 0);
		}
	}
}

static void Display_Font(gx3dObject *billboard, char ch, gx3dMatrix m, gx3dTexture tex) {
	float u, v;

	switch (ch) {

	case '0': u = 0; v = 0;
		goto display;

	case '1': u = 0.25; v = 0;
		goto display;

	case '2': u = 0.50; v = 0;
		goto display;

	case '3': u = 0.75; v = 0;
		goto display;

	case '4': u = 0; v = 0.25;
		goto display;

	case '5': u = 0.25; v = 0.25;
		goto display;

	case '6': u = 0.50; v = 0.25;
		goto display;

	case '7': u = 0.75; v = 0.25;
		goto display;

	case '8': u = 0; v = 0.50;
		goto display;

	case '9': u = 0.25; v = 0.50;
		goto display;

	display:
		gx3d_GetTranslateTextureMatrix(&m, u, v);
		layer = gx3d_GetObjectLayer(billboard, "score_fonts");
		gx3d_SetTextureMatrix(0, &m);
		gx3d_SetTexture(0, tex);
		gx3d_DrawObjectLayer(layer, 0);
		break;
	}
}

/*____________________________________________________________________
|
| Function: Play_FX
|
| Input: Called from Render_GameScreen
| Output: Displays and plays an effect with respect to time.
|		  (Only works on a 4x4 sprite sheet)
|___________________________________________________________________*/

static void Play_FX(gx3dTexture effect, gx3dVector normal, gx3dVector position, gx3dVector scale, float duration, unsigned time, int alpha_test, bool repeat) 
{

	// Check first if the desired effect is looping
	if (repeat)
		time %= (unsigned)duration;
	else if (time > duration)
		time = duration;

	// Initialize local variables;
	gx3dObjectLayer *billboard;
	int frame = (int)gx3d_Lerp(0, 15, ((float)(duration - time) / duration));
	float offset[4] = { 0.75, 0.5, 0.25, 0.0 };

	billboard = gx3d_GetObjectLayer(obj_hud, "fx");
	gx3d_EnableAlphaBlending();
	gx3d_EnableAlphaTesting(alpha_test); // 100
	gx3d_EnableTextureMatrix(0);
	gx3d_SetAmbientLight(color3d_white);

	// Draw object when time is > 0
	if (time > 0) {

		// Translate into world
		gx3d_GetScaleMatrix(&m1, scale.x, scale.y, scale.z);
		gx3d_GetBillboardRotateXYMatrix(&m2, &normal, &heading);
		gx3d_GetTranslateMatrix(&m3, position.x, position.y, position.z);
		gx3d_MultiplyMatrix(&m1, &m2, &m1);
		gx3d_MultiplyMatrix(&m1, &m3, &m);
		gx3d_SetObjectLayerMatrix(obj_hud, billboard, &m);

		gx3d_Object_UpdateTransforms(obj_hud);

		// Define the texture offset to the current frame
		gx3d_GetTranslateTextureMatrix(&m, offset[frame % 4], offset[frame / 4]);
		gx3d_SetTextureMatrix(0, &m);

		// Set texture and draw object
		gx3d_SetTexture(0, effect);
		gx3d_DrawObjectLayer(billboard, 0);

	}

	gx3d_DisableTextureMatrix(0);
	gx3d_DisableAlphaTesting();
	gx3d_DisableAlphaBlending();
}

/*____________________________________________________________________
|
| Function: Inverse_Lerp
|
| Input: Called from Render_GameScreen and position.cpp
| Output: Similar to gx3d_Lerp but calculates with decreasing values
|___________________________________________________________________*/
float Inverse_Lerp(float start, float end, float t) 
{
	return (start - (1.0f - t) * (start - end));
}

/*____________________________________________________________________
|
| Function: Update_Light
|
| Input: Called from Render_GameScreen
| Output: Updates light position and other parameters
|___________________________________________________________________*/

#define RATE  (1000 * 0.30) // # times per 1000 milliseconds
#define TORCH_LIGHT_COLOR_R (1.0f)    // orange 
#define TORCH_LIGHT_COLOR_G (0.6f)
#define TORCH_LIGHT_COLOR_B (0.2f)
#define LIGHT_OUTPUT_VARIANCE (0.95f) // % variance

static void Update_Light(gx3dLight *light, gx3dColor color, gx3dVector *position, float range, unsigned time_elapsed, bool flicker) {
	Update_Light(light, color, position, range, time_elapsed, flicker, 0, 0, 0.05);
}

static void Update_Light(gx3dLight *light, gx3dColor color, gx3dVector *position, float range, unsigned time_elapsed, bool flicker, float constant, float linear, float quadratic)
{
	float f;
	gx3dLightData lightdata;
	static float    variance = LIGHT_OUTPUT_VARIANCE / 2, speed = 0, inc = 1;
	static unsigned total_time = 0;

	if (flicker) {
		// 10 times per second, modulate the light
		total_time += time_elapsed;
		if (total_time >= RATE) {
			// Get a random number betwen 0 and 1
			if (random_GetInt(100, 500) <= 200) {
				// change increment direction
				inc *= -1;
				speed = random_GetFloat() * 0.01f;
			}
			total_time -= RATE;
		}

		variance += speed * inc * time_elapsed;
		if (variance < 0)
			variance = 0;
		else if (variance > LIGHT_OUTPUT_VARIANCE)
			variance = LIGHT_OUTPUT_VARIANCE;

		f = variance;

		// Get a random number between 0 and 1
		//    f = random_GetFloat ();
		f = (1 - LIGHT_OUTPUT_VARIANCE) + (LIGHT_OUTPUT_VARIANCE * f);
		color.r *= f;
		color.g *= f;
		color.b *= f;
		total_time -= RATE;
	}

	lightdata.light_type = gx3d_LIGHT_TYPE_POINT;
	lightdata.point.diffuse_color = color;
	lightdata.point.specular_color = color;
	lightdata.point.ambient_color.r = (float)0; //0.3;
	lightdata.point.ambient_color.g = (float)0; //0.3;
	lightdata.point.ambient_color.b = (float)0; //0.3;
	lightdata.point.ambient_color.a = (float)0;
	lightdata.point.range = range;
	lightdata.point.constant_attenuation = constant;
	lightdata.point.linear_attenuation = linear; //0.1f;
	lightdata.point.quadratic_attenuation = quadratic; //0.0025f;
	lightdata.point.src.x = position->x;
	lightdata.point.src.y = position->y;
	lightdata.point.src.z = position->z;
	gx3d_UpdateLight(*light, &lightdata);
}

/*____________________________________________________________________
|
| Function: Init_Render_State
|
| Input: Called from main.cpp
| Output: Initializes general 3D render state.
|___________________________________________________________________*/

static void Init_Render_State(void)
{
	// Enable zbuffering
	gx3d_EnableZBuffer();

	// Enable lighting
	gx3d_EnableLighting();

	// Set the default alpha blend factor
	gx3d_SetAlphaBlendFactor(gx3d_ALPHABLENDFACTOR_SRCALPHA, gx3d_ALPHABLENDFACTOR_INVSRCALPHA);

	// Init texture addressing mode - wrap in both u and v dimensions
	gx3d_SetTextureAddressingMode(0, gx3d_TEXTURE_DIMENSION_U | gx3d_TEXTURE_DIMENSION_V, gx3d_TEXTURE_ADDRESSMODE_WRAP);
	gx3d_SetTextureAddressingMode(1, gx3d_TEXTURE_DIMENSION_U | gx3d_TEXTURE_DIMENSION_V, gx3d_TEXTURE_ADDRESSMODE_WRAP);
	// Texture stage 0 default blend operator and arguments
	gx3d_SetTextureColorOp(0, gx3d_TEXTURE_COLOROP_MODULATE, gx3d_TEXTURE_ARG_TEXTURE, gx3d_TEXTURE_ARG_CURRENT);
	gx3d_SetTextureAlphaOp(0, gx3d_TEXTURE_ALPHAOP_SELECTARG1, gx3d_TEXTURE_ARG_TEXTURE, 0);
	// Texture stage 1 is off by default
	gx3d_SetTextureColorOp(1, gx3d_TEXTURE_COLOROP_DISABLE, 0, 0);
	gx3d_SetTextureAlphaOp(1, gx3d_TEXTURE_ALPHAOP_DISABLE, 0, 0);

	// Set default texture coordinates
	gx3d_SetTextureCoordinates(0, gx3d_TEXCOORD_SET0);
	gx3d_SetTextureCoordinates(1, gx3d_TEXCOORD_SET1);

	// Enable trilinear texture filtering
	gx3d_SetTextureFiltering(0, gx3d_TEXTURE_FILTERTYPE_TRILINEAR, 0);
	gx3d_SetTextureFiltering(1, gx3d_TEXTURE_FILTERTYPE_TRILINEAR, 0);
}
