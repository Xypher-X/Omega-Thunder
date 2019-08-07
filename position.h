/*____________________________________________________________________
|
| File: position.h
|
| (C) Copyright 2016 Timothy E. Roden.
| Edited by: David Sta Cruz
|___________________________________________________________________*/

// move commands
#define POSITION_MOVE_FORWARD 0x1
#define POSITION_MOVE_BACK    0x2
#define POSITION_MOVE_RIGHT   0x4
#define POSITION_MOVE_LEFT    0x8

#define POSITION_ROTATE_UP    0x1
#define POSITION_ROTATE_DOWN  0x2
#define POSITION_ROTATE_RIGHT 0x4
#define POSITION_ROTATE_LEFT  0x8

#define RUN_SPEED (7.3f*3)  // feet per second (based on a 12-minute mile run)

#define ROTATE_UP_MAX		((float)-50)
#define ROTATE_DOWN_MAX		((float)50)           
#define ROTATE_LEFT_MAX		((float)-80)
#define ROTATE_RIGHT_MAX	((float)80)

#define BOUNDARY_X			98

// Init starting position, other parameters
void Position_Init (
  gx3dVector *position, 
  gx3dVector *heading,      // 0,0,1 for cubic environment mapping to work correctly (why?)
  float       move_speed ); // move speed in feet per second

// Free any resources
void Position_Free ();

// Sets new move speed (in fps)
void Position_Set_Speed (float move_speed);

// Sets tether distance (in feet)
void Position_Set_Camera_Tether_Distance (float distance);

// Sets distance of camera from player position (going up along y axis)
void Position_Set_Camera_Eye_Level (float distance);

// Lerps the camera back to its starting position
void Position_Lerp_Camera_Start(float timer, float duration, gx3dVector *new_heading);

// Update position
void Position_Update (
  unsigned    elapsed_time,
  unsigned    move,
  int         xrotate,
  int         yrotate,
  bool        update_all,       // boolean           
  bool       *position_changed, // returns true if position has changed, else false
  bool       *camera_changed,   // return true if heading has changed
  gx3dVector *new_position,
  gx3dVector *new_heading,
  int		 *x_rotate,
  int		 *y_rotate );
