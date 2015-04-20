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
};

// total collisions for this simulation
unsigned int totalCollisions = 0;

// list of robot segments
std::vector<dGeomID> robotSegments;
// and their names
std::vector<std::string> robotSegmentNames;

// list of dGeomID's that are ignored since initial collision happened (to stop the wall of text of collisions)
std::vector<dGeomID> ignoreList;

// setup a vector for block collision counts
std::vector<blockCollisions> collVector;

// floor geom id
dGeomID floorID;

// amount of blocks to check for collision
int num_blocks = 105;

// amount of robot segment to test collisions with (sparse array so hardcoded for now)
int num_robot_segments = 14;
int robot_segments[14] = {1, 2, 3, 4, 5, 6, 7, 10, 11, 12, 13, 14, 15, 16};

// webots node DEF names
std::string webots_obstacle_base_name = "OBSTACLE_";
// std::string webots_robot_segment_name = "MODULE_";
std::string webots_robot_segment_name = "BUMPER_";

// collision contact points, only 2 because it's not important (we just need to know that a collision happened)
// see http://ode-wiki.org/wiki/index.php?title=Manual:_Collision_Detection
dContactGeom collisionContacts[2];


bool isRobotSegment(dGeomID id) {
  bool result = false;
  // quit checking early if this is the floor
  if (id == floorID) return false;

  // check if it is a robot segment
  for(int i = 0; i < num_robot_segments; i++) {
    if (robotSegments[i] == id) {
      result = true;
      break;
    }
  }

  return result;
}

std::string getRobotSegmentName(dGeomID id) {
  std::string robotSegmentName = "NOT FOUND";

  // check if it is a robot segment
  for(int i = 0; i < num_robot_segments; i++) {
    if (robotSegments[i] == id) {
      robotSegmentName = robotSegmentNames[i];
      break;
    }
  }

  return robotSegmentName;
}

bool isIgnored(dGeomID id) {
  bool result = false;

  // check if the ID is on the ignore list
  if(ignoreList.size() > 0) {
    for(int i = 0; i < ignoreList.size(); i++) {
      if (id == ignoreList[i]) {
        result = true;
        break;
      }
    }
  }

  return result;
}

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
      // std::cout << "added webots node with DEF name: " << defName << " to blockCollision vector." << std::endl;
    } else {
      std::cout << "could not find webots node with DEF name: " << defName << std::endl;
    }
  }

  std::cout << "physics plugin :: done adding blocks to blockCollision vector." << std::endl;

  
  for(int i = 1; i < num_robot_segments + 3; i++) {
    // get const char* webots defname
    std::stringstream ss;
    ss << webots_robot_segment_name << i;
    std::string defNameStr = ss.str();
    const char* defName = defNameStr.c_str();

    // get the geomID
    dGeomID id = dWebotsGetGeomFromDEF(defName);

    // handle node not found = NULL case
    if(id != NULL) {
      // create and add a robot segment struct to the robot segments vector
      robotSegments.push_back(id);
      robotSegmentNames.push_back(defNameStr);
      std::cout << "added webots node with DEF name: " << defNameStr << " to robot segments vector, isRobotSegment(): " << isRobotSegment(id) << std::endl;
    } else {
      std::cout << "could not find webots node with DEF name: " << defNameStr << std::endl;
    }
  }

  std::cout << "physics plugin :: done adding robot segments to robot segments vector." << std::endl;

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
  
  // THIS DOESN'T WORK
  // if ( isRobotSegment(g1) || isRobotSegment(g2) ) {
  //   // if it's not a collision with the floor
  //   if (g1 != floorID && g2 != floorID) {
  //     // robot segment is colliding with something else than the floor
  //     std::cout << "collision with block. g1 robot? = " << isRobotSegment(g1) << ". g2 robot? = " << isRobotSegment(g2) << std::endl;
  //   }
  //   std::cout << "collision with something. g1 robot? = " << isRobotSegment(g1) << ". g2 robot? = " << isRobotSegment(g2) << std::endl;
  // }

  // check all the block geoms
  for(int i = 0; i < num_blocks - 1; i++) {
    // if g1 is a block AND g2 is NOT the floor (or the inverse), we need to check for collision contacts !
    if ( (g1 == collVector[i].id && g2 != floorID) || (g2 == collVector[i].id && g1 != floorID) ) {
      // collide the geoms
      int contactCount = dCollide(g1, g2, 2, collisionContacts, sizeof(dContactGeom));
      // there is only a collision if ODE can generate 'contact points'.
      if(contactCount > 0) {
        // increment the collision counter for this block
        collVector[i].collisions++;
        totalCollisions++;
        // is this a collision with a robot segment?
        if(isRobotSegment(g1)) {
          std::cout << collVector[i].defNameString << " collision with robot segment : " << getRobotSegmentName(g1) << std::endl;
        } else if(isRobotSegment(g2)) {
          std::cout << collVector[i].defNameString << " collision with robot segment : " << getRobotSegmentName(g2) << std::endl;
        }
        // notify of the collision if this is the first!
        if (collVector[i].collisions == 1) {
          std::cout << "collision with block : " << collVector[i].defNameString << " and something else." << std::endl;
          // what are we colliding with?
          std::cout << "something else being: " << std::endl;
          if(g1 == collVector[i].id) {
            std::cout << "something else being: " << std::endl;
            std::cout << g2 << std::endl;
          } else {
            std::cout << g1 << std::endl;
          }
          // is this a space?
          // std::cout << "is g1 a dSpace? " << dGeomIsSpace(g1) << ". is g2 a dSpace? " << dGeomIsSpace(g2) << std::endl;
          // what geom class are we dealing with?
          // std::cout << "what geom class is g1? " << dGeomGetClass(g1) << ". and g2? " << dGeomGetClass(g2) << std::endl;
          // translate names?
          // std::cout << "those are respectively: " << getGeomClassName(dGeomGetClass(g1)) << ". and " << getGeomClassName(dGeomGetClass(g2)) << std::endl;
        }
        // we do not need to know of further collisions with this block
        // so let us add the block geomID to an ignoreList:
        // if (g1 == collVector[i].id) {
        //   ignoreList.push_back(g1);
        // } else {
        //   ignoreList.push_back(g2);
        // }
        // // notify of ignore list add
        // std::cout << collVector[i].defNameString << " added to ignore list. " << std::endl;
      }
    }
    // else if (collVector[i].id == g2) {

    // }
  }

  // Let webots handle ALL collisions.
  return 0;

}

void webots_physics_cleanup() {
  /*
   * Here you need to free any memory you allocated in above, close files, etc.
   * You do not need to free any ODE object, they will be freed by Webots.
   */
   std::cout << "cleanup called. exiting... " << std::endl;
}
