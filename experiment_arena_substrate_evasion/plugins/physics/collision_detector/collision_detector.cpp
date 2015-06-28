/*
 * File:          
 * Date:          
 * Description:   
 * Author:        
 * Modifications: 
 */

#include <ode/ode.h>
#include <plugins/physics.h>
#include <vector>
#include <sstream>
#include <iostream>
#include <cstdio>
#include <fstream>

#include "../../../ExperimentDefinition/AdditionalSettings.h"

// using namespace std;

/*
 * Note: This plugin will become operational only after it was compiled and associated with the current world (.wbt).
 * To associate this plugin with the world follow these steps:
 *  1. In the Scene Tree, expand the "WorldInfo" node and select its "physics" field
 *  2. Then hit the [...] button at the bottom of the Scene Tree
 *  3. In the list choose the name of this plugin (same as this file without the extention)
 *  4. Then save the .wbt by hitting the "Save" button in the toolbar of the 3D view
 *  5. Then revert the simulation: the plugin should now load and execute with the current simulation
 */

// block collision count structure
struct blockCollisions {
  const char* defName;
  std::string defNameString;
  dGeomID id;
  unsigned int collisions;
  bool collidedwith;
};

// total collisions for this simulation
unsigned int totalCollisions = 0;

// setup a vector for block collision counts
std::vector<blockCollisions> collVector;

// stringstream to convert datatypes for message sending
std::stringstream message_ss;

// floor geom id
dGeomID floorID;

// amount of blocks to check for collision
int num_blocks = 68; // 105;

// webots node DEF names
std::string webots_obstacle_base_name = "OBSTACLE_";

// collision contact points, only 2 because it's not important (we just need to know that a collision happened)
// see http://ode-wiki.org/wiki/index.php?title=Manual:_Collision_Detection
dContactGeom collisionContacts[2];

std::string getGeomClassName(int geomClassNum) {
  std::string classNames[10] = {"Sphere", "Box", "Capped cylinder", "Regular flat-ended cylinder", "Infinite plane (non-placeable)", "Geometry transform", "Ray", "Triangle mesh", "Simple space", "Hash table based space"}; 
  return classNames[geomClassNum];
}


void webots_physics_init(dWorldID world, dSpaceID space, dJointGroupID contactJointGroup) {
  /*
   * Get ODE object from the .wbt model, e.g.
   *   dBodyID body1 = dWebotsGetBodyFromDEF("MY_ROBOT");
   *   dBodyID body2 = dWebotsGetBodyFromDEF("MY_SERVO");
   *   dGeomID geom2 = dWebotsGetGeomFromDEF("MY_SERVO");
   * If an object is not fousnd in the .wbt world, the function returns NULL.
   * Your code should correcly handle the NULL cases because otherwise a segmentation fault will crash Webots.
   *
   * This function is also often used to add joints to the simulation, e.g.
   *   dJointID joint = dJointCreateBall(world, 0);
   *   dJointAttach(joint, body1, body2);
   *   ...
   */


  std::cout << "physics plugin :: initialising..." << std::endl;

  // get the floor geom id
  floorID = dWebotsGetGeomFromDEF("GROUND");

  if (floorID == NULL) {
    std::cout << "physics plugin :: error: floor node not detected..." << std::endl;
  } else {
    std::cout << "physics plugin :: floor geomID found." << std::endl;
  }

  // vector index starts at zero:
  unsigned int v = 0;

  for(int i = 1; i < (num_blocks + 1); i++) {
    // vector index starts at 0, so:
    v = i - 1;

    // get const char* webots defname
    std::stringstream ss;
    ss << webots_obstacle_base_name << i;
    std::string defNameStr = ss.str();
    const char* defName = defNameStr.c_str();

    // get the geomID
    dGeomID id = dWebotsGetGeomFromDEF(defName);

    // handle node not found = NULL case
    if(id != NULL) {
      // create and add a blockCollision struct to the collisions vector
      collVector.push_back(blockCollisions());
      // store the geomID together with the defname and 0 collisions in the vector
      collVector[v].defNameString = defNameStr;
      collVector[v].defName = defName;
      collVector[v].id = id;
      collVector[v].collisions = 0;
      collVector[v].collidedwith = false;
      // std::cout << "added webots node with DEF name: " << defName << " to blockCollision vector." << std::endl;
    } else {
      std::cout << "physics plugin :: could not find webots node with DEF name: " << defName << std::endl;
    }
  }

  std::cout << "physics plugin :: done adding blocks to blockCollision vector." << std::endl;

}

void webots_physics_step() {
  /*
   * Do here what needs to be done at every time step, e.g. add forces to bodies
   *   dBodyAddForce(body1, f[0], f[1], f[2]);
   *   ...
   */
}

void webots_physics_draw() {
  /*
   * This function can optionally be used to add OpenGL graphics to the 3D view, e.g.
   *   // setup draw style
   *   glDisable(GL_LIGHTING);
   *   glLineWidth(2);
   * 
   *   // draw a yellow line
   *   glBegin(GL_LINES);
   *   glColor3f(1, 1, 0);
   *   glVertex3f(0, 0, 0);
   *   glVertex3f(0, 1, 0);
   *   glEnd();
   */
}


int webots_physics_collide(dGeomID g1, dGeomID g2) {

  /*
   * This function needs to be implemented if you want to overide Webots collision detection.
   * It must return 1 if the collision was handled and 0 otherwise. 
   * Note that contact joints should be added to the contactJointGroup, e.g.
   *   n = dCollide(g1, g2, MAX_CONTACTS, &contact[0].geom, sizeof(dContact));
   *   ...
   *   dJointCreateContact(world, contactJointGroup, &contact[i])
   *   dJointAttach(contactJoint, body1, body2);
   *   ...
   */
  
  // check all the block geoms
  for(int i = 0; i < num_blocks - 1; i++) {
    // if g1 is a block AND g2 is NOT the floor (or the inverse), we need to check for collision contacts !
    if ( (g1 == collVector[i].id && g2 != floorID) || (g2 == collVector[i].id && g1 != floorID) ) {
      // collide the geoms
      int contactCount = dCollide(g1, g2, 2, collisionContacts, sizeof(dContactGeom));
      // there is only a collision if ODE can generate 'contact points'.
      if(contactCount > 0) {

        // increment the collision counter for this block IFF no collisions have been made this simulation step
        collVector[i].collisions++;
        totalCollisions++;
        
        // notify of the collision if this is the first only! (massive spam otherwise.)
        if (collVector[i].collidedwith == false) {
          // this block has been collided with so set its collidedwith param to true
          collVector[i].collidedwith = true;

          // notify console
          if(g1 == collVector[i].id) {
            std::cout << "physics plugin :: collision with block : " << collVector[i].defNameString << " and : " << getGeomClassName(dGeomGetClass(g2)) << std::endl;
          } else {
            std::cout << "physics plugin :: collision with block : " << collVector[i].defNameString << " and : " << getGeomClassName(dGeomGetClass(g1)) << std::endl;
          }
        }

        // clear string stream buffer
        message_ss.str(std::string());
        // construct message
        message_ss << collVector[i].defNameString << " " << collVector[i].collisions << std::endl;
        const char * buff = message_ss.str().c_str();
        // send message to controller
        int buffersize = sizeof(buff) * strlen(buff);
        dWebotsSend(physics_channel, buff, buffersize);
        // std::cout << "physics plugin :: sent message to controller, buffer size = " << buffersize << " -> " << buff << std::endl;

      }
    }
  }

  // Let webots handle ALL collisions.
  return 0;

}

void webots_physics_cleanup() {
  /*
   * Here you need to free any memory you allocated in above, close files, etc.
   * You do not need to free any ODE object, they will be freed by Webots.
   */

  // REPORT the collisions
  std::cout << "physics plugin :: reporting. " << std::endl;
  std::cout << "physics plugin :: total collisions : " << totalCollisions << std::endl;
  for(int i = 0; i < num_blocks - 1; i++) {
    if(collVector[i].collisions > 0) {
      std::cout << "physics plugin :: # of collisions with " << collVector[i].defNameString << " : " << collVector[i].collisions << std::endl;
    }
  }

  // write collision data to a file located in : physics_logfile
  std::cout << "physics plugin :: writing collision data... " << std::endl;
  std::ofstream physics_log;
  physics_log.open(physics_logfile, std::ofstream::out); // | std::ofstream::app
  physics_log << totalCollisions << std::endl;
  for(int i = 0; i < num_blocks - 1; i++) {
    if(collVector[i].collisions > 0) {
      physics_log << collVector[i].defNameString << " " << collVector[i].collisions << std::endl;
    }
  }
  physics_log.close();
  std::cout << "physics plugin :: collision data written. " << std::endl;

  // exiting message
  std::cout << "physics plugin :: cleanup called. exiting... " << std::endl;
}
