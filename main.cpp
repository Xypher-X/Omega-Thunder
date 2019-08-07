/*____________________________________________________________________
|
| File: main.c
|
| Description: Main module in Demo1 program.
|
| Functions:  Program_Get_User_Preferences
|             Program_Init
|							 Init_Graphics
|								Set_Mouse_Cursor
|             Program_Run
|             Program_Free
|             Program_Immediate_Key_Handler               
|
| (C) Copyright 2017 Abonvita Software LLC.
| Licensed under the GX Toolkit License, Version 1.0.
|___________________________________________________________________*/

#define _MAIN_

/*___________________
|
| Include Files
|__________________*/

#include <first_header.h>
#include "dp.h"
#include "..\Framework\win_support.h"
#include <rom8x8.h>

#include "main.h"
#include "position.h"
#include "render.h"

/*___________________
|
| Type definitions
|__________________*/

typedef struct {
  unsigned resolution;
  unsigned bitdepth;
} UserPreferences;

/*___________________
|
| Function Prototypes
|__________________*/

static int Init_Graphics (unsigned resolution, unsigned bitdepth, unsigned stencildepth, int *generate_keypress_events);
static void Set_Mouse_Cursor (void);

/*___________________
|
| Constants
|__________________*/

#define MAX_VRAM_PAGES  2
#define GRAPHICS_RESOLUTION  \
  (                          \
    gxRESOLUTION_640x480   | \
    gxRESOLUTION_800x600   | \
    gxRESOLUTION_1024x768  | \
    gxRESOLUTION_1152x864  | \
    gxRESOLUTION_1280x960  | \
    gxRESOLUTION_1400x1050 | \
    gxRESOLUTION_1440x1080 | \
    gxRESOLUTION_1600x1200 | \
    gxRESOLUTION_1152x720  | \
    gxRESOLUTION_1280x800  | \
    gxRESOLUTION_1440x900  | \
    gxRESOLUTION_1680x1050 | \
    gxRESOLUTION_1920x1200 | \
    gxRESOLUTION_2048x1280 | \
    gxRESOLUTION_1280x720  | \
    gxRESOLUTION_1600x900  | \
    gxRESOLUTION_1920x1080 | \
    gxRESOLUTION_2560x1600   \
  )
#define GRAPHICS_STENCILDEPTH 0
#define GRAPHICS_BITDEPTH (gxBITDEPTH_24 | gxBITDEPTH_32)

#define AUTO_TRACKING    1
#define NO_AUTO_TRACKING 0

/*____________________________________________________________________
|
| Function: Program_Get_User_Preferences
|
| Input: Called from TheWin::Init()
| Output: Allows program to popup dialog boxes, etc. to get any user
|   preferences such as screen resolution.  Returns preferences via a
|   pointer.  Returns true on success, else false to quit the program.
|___________________________________________________________________*/

int Program_Get_User_Preferences (void **preferences)
{
  static UserPreferences user_preferences;

  if (gxGetUserFormat (GRAPHICS_DRIVER, GRAPHICS_RESOLUTION, GRAPHICS_BITDEPTH, &user_preferences.resolution, &user_preferences.bitdepth)) {
    *preferences = (void *)&user_preferences;
    return (1);
  }
  else
    return (0);
}

/*____________________________________________________________________
|
| Function: Program_Init
|
| Input: Called from TheWin::Start_Program_Thread()
| Output: Starts graphics mode.  Returns # of user pages available if 
|       successful, else 0.
|___________________________________________________________________*/
 
int Program_Init (void *preferences, int *generate_keypress_events)
{
  UserPreferences *user_preferences = (UserPreferences *) preferences;
  int initialized = FALSE;

  if (user_preferences) 
    initialized = Init_Graphics (user_preferences->resolution, user_preferences->bitdepth, GRAPHICS_STENCILDEPTH, generate_keypress_events);
    
  return (initialized);
}

/*____________________________________________________________________
|
| Function: Init_Graphics
|
| Input: Called from Program_Init()
| Output: Starts graphics mode.  Returns # of user pages available if 
|       successful, else 0.
|___________________________________________________________________*/

static int Init_Graphics (unsigned resolution, unsigned bitdepth, unsigned stencildepth, int *generate_keypress_events)
{
  int num_pages;
  byte *font_data;
  unsigned font_size;

/*____________________________________________________________________
|
| Init globals
|___________________________________________________________________*/

  Pgm_num_pages   = 0;
  Pgm_system_font = NULL;

/*____________________________________________________________________
|
| Start graphics mode and event processing
|___________________________________________________________________*/

  font_data = font_data_rom8x8;
  font_size = sizeof(font_data_rom8x8);
                                                                      
  // Start graphics mode                                      
  num_pages = gxStartGraphics (resolution, bitdepth, stencildepth, MAX_VRAM_PAGES, GRAPHICS_DRIVER);
  if (num_pages == MAX_VRAM_PAGES) {
	// Init system, drawing fonts 
	Pgm_system_font  = gxLoadFontData (gxFONT_TYPE_GX, font_data, font_size);
	// Make system font the default drawing font 
	gxSetFont (Pgm_system_font);

	// Start event processing
	evStartEvents (evTYPE_MOUSE_LEFT_PRESS     | evTYPE_MOUSE_RIGHT_PRESS   |
					evTYPE_MOUSE_LEFT_RELEASE   | evTYPE_MOUSE_RIGHT_RELEASE |
					evTYPE_MOUSE_WHEEL_BACKWARD | evTYPE_MOUSE_WHEEL_FORWARD |
//                   evTYPE_KEY_PRESS | 
					evTYPE_RAW_KEY_PRESS | evTYPE_RAW_KEY_RELEASE,       
					AUTO_TRACKING, EVENT_DRIVER);
	*generate_keypress_events = FALSE;  // true if using evTYPE_KEY_PRESS in the above mask

	// Set a custom mouse cursor
	Set_Mouse_Cursor ();  
	// Hide the mouse cursor
	msHideMouse();

    // Set globals
    Pgm_num_pages = num_pages;
  }

  return (Pgm_num_pages);
}

/*____________________________________________________________________
|
| Function: Set_Mouse_Cursor
|
| Input: Called from Init_Graphics()
| Output: Sets custom mouse cursor.
|___________________________________________________________________*/
     
static void Set_Mouse_Cursor (void)
{
  gxColor fc, bc;

  // Set cursor to a medium sized red arrow
  fc.r = 255;
  fc.g = 0;
  fc.b = 0;
  fc.a = 0;
  bc.r = 1;
  bc.g = 1;
  bc.b = 1;
  bc.a = 0;
  msSetCursor (msCURSOR_MEDIUM_ARROW, fc, bc);
}

/*____________________________________________________________________
|
| Function: Program_Run
|
| Input: Called from Program_Thread()
| Output: Runs program in the current video mode.  Begins with mouse
|   hidden.
|___________________________________________________________________*/
                    
#define QUIT (NUM_MENU_OPTIONS-1)

void Program_Run (void)
{
  bool quit;
  gx3dDriverInfo dinfo;
  char str[256];
  int render, force_update;
  unsigned elapsed_time, last_time, new_time;
  

  // Game State Declaration
  int state = STATE_TITLE_SCREEN;

/*____________________________________________________________________
|
| Print info about graphics driver to debug file.
|___________________________________________________________________*/

  gx3d_GetDriverInfo (&dinfo);
  debug_WriteFile ("_______________ Device Info ______________");
  sprintf (str, "max texture size: %dx%d", dinfo.max_texture_dx, dinfo.max_texture_dy);
  debug_WriteFile (str);
  sprintf (str, "max active lights: %d", dinfo.max_active_lights);
  debug_WriteFile (str);
  sprintf (str, "max user clip planes: %d", dinfo.max_user_clip_planes);
  debug_WriteFile (str);
  sprintf (str, "max simultaneous texture stages: %d", dinfo.max_simultaneous_texture_stages);
  debug_WriteFile (str);
  sprintf (str, "max texture stages: %d", dinfo.max_texture_stages);
  debug_WriteFile (str);
  sprintf (str, "max texture repeat: %d", dinfo.max_texture_repeat);
  debug_WriteFile (str);
  debug_WriteFile ("__________________________________________");

/*____________________________________________________________________
|
| Initialize the graphics state
|___________________________________________________________________*/

  // Set 2d graphics state
  Pgm_screen.xleft   = 0;
  Pgm_screen.ytop    = 0;
  Pgm_screen.xright  = gxGetScreenWidth() - 1;
  Pgm_screen.ybottom = gxGetScreenHeight() - 1;
  gxSetWindow (&Pgm_screen);
  gxSetClip   (&Pgm_screen);
  gxSetClipping (FALSE);

  // Set the 3D viewport
  gx3d_SetViewport (&Pgm_screen);
  // Init other 3D stuff
  Init_Render_State ();

  // DEBUGGING - Comment out to play the game how it should be
  //state = STATE_STARTING;

  /*____________________________________________________________________
  |
  | Program loop
  |___________________________________________________________________*/
  for (quit = false; !quit; ) {

	  /*____________________________________________________________________
	  |
	  | Init support routines
	  |___________________________________________________________________*/

	  // Init 3D graphics
	  Render_Init(&state);

	  /*____________________________________________________________________
	  |
	  | Render the appropriate screen
	  |___________________________________________________________________*/

	  quit = Render_Game_Loop(&state); 
  }
}

/*____________________________________________________________________
|
| Function: Program_Free
|
| Input: Called from TheWin::OnClose()
| Output: Exits graphics mode.
|___________________________________________________________________*/

void Program_Free ()
{
  // Stop event processing 
  evStopEvents ();
  // Return to text mode 
  if (Pgm_system_font)
    gxFreeFont (Pgm_system_font);
  // Exit graphics mode (back to desktop mode)
  gxStopGraphics ();
}

/*____________________________________________________________________
|
| Function: Init_Render_State
|
| Input: Called from Program_Run()
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