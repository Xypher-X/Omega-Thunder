/*____________________________________________________________________
|
| File: position.cpp
|
| Description: Functions to create and manipulate position and camera.
|
| Functions: Position_Init
|            Position_Free
|            Position_Set_Speed
|            Position_Set_Camera_Tether_Distance
|            Position_Set_Camera_Eye_Level
|            Position_Update
|
| (C) Copyright 2016 Timothy E. Roden.
| Edited by: David Sta Cruz
|___________________________________________________________________*/

/*___________________
|
| Include Files
|__________________*/

#include <first_header.h>
#include <math.h>

#include "dp.h"

#include "position.h"
#include "render.h"

/*___________________
|
| Constants
|__________________*/

#define CAMERA_DISTANCE	10	// distance of 'to' point away from the camera position
#define SCALE_ROTATE_SPEED 0.25 // scales the speed of camera rotation so it does not rotate too fast

/*___________________
|
| Global variables
|__________________*/

static gx3dVector current_position;			  // current position
static gx3dVector start_heading;				  // start heading (normalized)
static gx3dVector current_heading;			  // current heading (normalized)
static float      current_speed;				  // current move speed
static float      current_xrotate;			  // current rotation of camera	
float	            current_yrotate;
static gx3dVector forward_heading = { 0, 0, 1 };

static gx3dVector camera_heading;         // heading of camera (can be different from current heading)
static float      camera_tether_distance;
static float      camera_eye_level;
static float      camera_xrotate;
static float      camera_yrotate;

/*____________________________________________________________________
|
| Function: Position_Init
|
| Input: Called from ____
| Output:
|___________________________________________________________________*/
 
void Position_Init (
  gx3dVector *position, 
  gx3dVector *heading,      // 0,0,1 for cubic environment mapping to work correctly (why?)
  float       move_speed )  // move speed in feet per second
{
  bool b;
  gx3dVector v;

  // Init global variables
  current_position = *position;
  current_heading  = *heading;
  gx3d_NormalizeVector (&current_heading, &current_heading); // just in case its not already normalized
  start_heading    = current_heading;
  current_speed    = move_speed;
  current_xrotate  = 0;
  current_yrotate  = 0;

  camera_heading   = current_heading;
  camera_tether_distance = 20; // default value, can be changed by calling Position_Set_Camera_Tether_Distance()
  camera_eye_level = 6.0;      // default value, can be changed by calling Position_Set_Camera_Eye_Level()
  camera_xrotate = 0;
  camera_yrotate = 0;

  Position_Update (0, 0, 0, 0, true, &b, &b, &v, &v, 0, 0);	// force an update to start the camera off in the correct position
}

/*____________________________________________________________________
|
| Function: Position_Free
|
| Input: Called from ____
| Output:
|___________________________________________________________________*/
 
void Position_Free ()
{

}

/*____________________________________________________________________
|
| Function: Position_Set_Speed
|
| Input: Called from ____
| Output: Sets new move speed.
|___________________________________________________________________*/

void Position_Set_Speed (float move_speed)
{
  current_speed = move_speed;
}

/*____________________________________________________________________
|
| Function: Position_Set_Camera_Tether_Distance
|
| Input: Called from ____
| Output: Sets new camera tether distance.
|___________________________________________________________________*/

void Position_Set_Camera_Tether_Distance (float distance)
{
  camera_tether_distance = distance;
}

/*____________________________________________________________________
|
| Function: Position_Set_Camera_Eye_Level
|
| Input: Called from ____
| Output: Sets new camera eye level (distance from ground up).
|___________________________________________________________________*/

void Position_Set_Camera_Eye_Level (float distance)
{
  camera_eye_level = distance;
}

/*____________________________________________________________________
|
| Function: Position_Lerp_Camera_Start
|
| Input: Called from ____
| Output: Lerps the camera back to its starting position from the 
|		  beginning of the game.
|___________________________________________________________________*/
void Position_Lerp_Camera_Start(float timer, float duration, gx3dVector *new_heading)
{
	gx3dMatrix m, mx, my, mxy;
	gx3dVector from, to, world_up = { 0, 1, 0 }, v1;
	int xrotate, yrotate;
	float t = (duration - timer) / duration;

	if (t != 0) {
		xrotate = Inverse_Lerp(current_xrotate, 0, t);
		yrotate = Inverse_Lerp(current_yrotate, 0, t);

		gx3d_GetRotateXMatrix(&mx, xrotate);
		gx3d_GetRotateYMatrix(&my, yrotate);
		gx3d_MultiplyMatrix(&mx, &my, &mxy);
		gx3d_MultiplyVectorMatrix(&start_heading, &mxy, &current_heading);
		// Make sure heading is normalized
		gx3d_NormalizeVector(&current_heading, &current_heading);
	}
	else {
		// Set camera to same values
		current_heading = start_heading;
		xrotate = current_xrotate;
		yrotate = current_yrotate;
	}

	camera_heading = current_heading;
	camera_xrotate = xrotate;
	camera_yrotate = yrotate;

	// Move camera to current position
	from = current_position;
	// Move camera up to eye level
	from.y += camera_eye_level;

	// Compute a point the camera is looking at
	gx3d_MultiplyScalarVector(CAMERA_DISTANCE, &camera_heading, &v1);
	gx3d_AddVector(&from, &v1, &to);

	// Back camera up
	gx3d_MultiplyScalarVector(-camera_tether_distance, &camera_heading, &v1);
	gx3d_AddVector(&from, &v1, &from);

	// Don't allow camera to go below ground (y=0)
	gx3dPlane plane;
	gx3dVector intersection;
	static gx3dVector p1 = { 0, 0, 0 };
	static gx3dVector p2 = { 1, 0, 0 };
	static gx3dVector p3 = { 0, 0, 1 };
	gx3d_GetPlane(&p1, &p2, &p3, &plane);
	gx3dRay ray;
	ray.origin = from;
	ray.direction = camera_heading;
	if (gx3d_Intersect_Ray_Plane(&ray, camera_tether_distance, &plane, 0, &intersection) == gxRELATION_INTERSECT) {
		from = intersection;
		from.y += 0.05; // slightly lifts up the camera so it does not intersect with the ground plane
	}

	// Set new camera	position
	gx3d_ComputeViewMatrix(&m, &from, &to, &world_up);
	gx3d_SetViewMatrix(&m);

	if (t == 0) {
		current_xrotate = 0;
		current_yrotate = 0;
	}

	*new_heading = current_heading;
}

/*____________________________________________________________________
|
| Function: Position_Update
|
| Input: Called from ____
| Output:
|___________________________________________________________________*/

void Position_Update (
  unsigned    elapsed_time,
  unsigned    move,
  int         xrotate,
  int         yrotate,
  bool        update_all,               
  bool       *position_changed, // returns true if position has changed, else false
  bool       *camera_changed,   // return true if heading has changed
  gx3dVector *new_position,
  gx3dVector *new_heading, 
  int		 *x_rotate,
  int		 *y_rotate)
{
	int n;
	float move_amount;
  gx3dMatrix m, mx, my, mxy;
  gx3dVector to, world_up = { 0, 1, 0 };
	gx3dVector v1, v_right;
	gx3dVector last_position;
  gx3dVector heading;

/*____________________________________________________________________
|
| Init variables
|___________________________________________________________________*/

  *position_changed = false;
  *camera_changed   = false;

	// Compute amount of movement to make, if any
	move_amount = ((float)elapsed_time / 1000) * current_speed;

	last_position = current_position;

/*____________________________________________________________________
|
| Rotate heading?
|___________________________________________________________________*/

  // Smooth out the rotations
	n = xrotate;                                                     
	xrotate = (int) sqrt ((double)(abs(xrotate)));
	if (n < 0)
	  xrotate = -xrotate;                                                  
	n = yrotate;
	yrotate = (int) sqrt ((double)(abs(yrotate)));
	if (n < 0)                                                                   
	  yrotate = -yrotate;                                                                              
                                                                                         
	// Add to the current x axis rotation                                                                
	current_xrotate += (float)xrotate * SCALE_ROTATE_SPEED;	// scale by .5 so doesn't rotate so fast         
  if (current_xrotate < ROTATE_UP_MAX)
    current_xrotate = ROTATE_UP_MAX;
  else if (current_xrotate > ROTATE_DOWN_MAX)
    current_xrotate = ROTATE_DOWN_MAX;
                                                             
	// Add to the current y axis rotation
  current_yrotate += (float)yrotate * SCALE_ROTATE_SPEED;	// scale by .5 so doesn't rotate so fast
  if (current_yrotate < ROTATE_LEFT_MAX)
    current_yrotate = ROTATE_LEFT_MAX;
  else if (current_yrotate > ROTATE_RIGHT_MAX)
    current_yrotate = ROTATE_RIGHT_MAX;

  // Pass current x and y rotate to x and y rotation variables used for animations in render.cpp
  if (x_rotate != nullptr)
	*x_rotate = current_xrotate;
  if (y_rotate != nullptr)
	*y_rotate = current_yrotate;

  // Rotate heading
  if ((xrotate != 0) OR (yrotate != 0)) {
    gx3d_GetRotateXMatrix (&mx, current_xrotate);
    gx3d_GetRotateYMatrix (&my, current_yrotate);
    gx3d_MultiplyMatrix (&mx, &my, &mxy);
    gx3d_MultiplyVectorMatrix (&start_heading, &mxy, &current_heading);  
    // Make sure heading is normalized
    gx3d_NormalizeVector (&current_heading, &current_heading);
  }
  
  // Set camera to same values
  camera_heading = current_heading;
  camera_xrotate = current_xrotate;
  camera_yrotate = current_yrotate;

/*____________________________________________________________________
|
| Move position?
|___________________________________________________________________*/

  if (move OR update_all) {
		if (move & POSITION_MOVE_FORWARD) {
			// Move 0.5 feet along the view vector
			gx3d_MultiplyScalarVector (move_amount, &current_heading, &v1);
		  gx3d_AddVector (&current_position, &v1, &current_position);
		}
		if (move & POSITION_MOVE_BACK) {
			// Move -0.5 feet along the view vector
			gx3d_MultiplyScalarVector (-move_amount, &current_heading, &v1);
		  gx3d_AddVector (&current_position, &v1, &current_position);
		}
		if (move & POSITION_MOVE_RIGHT) {
			// Compute the normalized right vector
			gx3d_VectorCrossProduct (&world_up, &forward_heading, &v_right);
			gx3d_NormalizeVector (&v_right, &v_right);
			// Move 0.5 feet along the right vector
			gx3d_MultiplyScalarVector (move_amount, &v_right, &v1);
		  gx3d_AddVector (&current_position, &v1, &current_position);
		}
		if (move & POSITION_MOVE_LEFT) {
			// Compute the normalized right vector
			gx3d_VectorCrossProduct (&world_up, &forward_heading, &v_right);
			gx3d_NormalizeVector (&v_right, &v_right);
			// Move -0.5 feet along the right vector
			gx3d_MultiplyScalarVector (-move_amount, &v_right, &v1);
		  gx3d_AddVector (&current_position, &v1, &current_position);
		}
		*position_changed = true;
	}
  // Re-initialize forward_heading
  forward_heading = {0, 0, 1};

/*____________________________________________________________________
|
| Enforce position contraints
|___________________________________________________________________*/

  // Stay on ground level
  current_position.y = 0;

  // Stay on origin x-axis
  current_position.z = 0;

  // Stay within bounds
  if (current_position.x > BOUNDARY_X)
	  current_position.x = BOUNDARY_X;
  else if (current_position.x < -BOUNDARY_X)
	  current_position.x = -BOUNDARY_X;
/*____________________________________________________________________
|
| Update camera
|___________________________________________________________________*/

  gx3dVector from;

	if ((xrotate != 0) OR (yrotate != 0) OR *position_changed) {

    // Move camera to current position
    from = current_position;
    // Move camera up to eye level
    from.y += camera_eye_level;

    // Compute a point the camera is looking at
		gx3d_MultiplyScalarVector (CAMERA_DISTANCE, &camera_heading, &v1);
	  gx3d_AddVector (&from, &v1, &to);

    // Back camera up
    gx3d_MultiplyScalarVector (-camera_tether_distance, &camera_heading, &v1);
    gx3d_AddVector (&from, &v1, &from);

    // Don't allow camera to go below ground (y=0)
    gx3dPlane plane; 
    gx3dVector intersection;
    static gx3dVector p1 = { 0, 0, 0 };
    static gx3dVector p2 = { 1, 0, 0 };
    static gx3dVector p3 = { 0, 0, 1 };
    gx3d_GetPlane (&p1, &p2, &p3, &plane);
    gx3dRay ray;
    ray.origin = from;
    ray.direction = camera_heading;
	if (gx3d_Intersect_Ray_Plane(&ray, camera_tether_distance, &plane, 0, &intersection) == gxRELATION_INTERSECT) {
		from = intersection;
		from.y += 0.05; // slightly lifts up the camera so it does not intersect with the ground plane
	}

    // Set new camera	position
	  gx3d_ComputeViewMatrix (&m, &from, &to, &world_up);
  	gx3d_SetViewMatrix (&m);

    *camera_changed = true;
  }
  
/*____________________________________________________________________
|
| Return new position and heading
|___________________________________________________________________*/

  *new_position = current_position;
  *new_heading  = current_heading;
}
