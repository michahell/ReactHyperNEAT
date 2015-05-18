/*
 * File:         $Id: mod_supervisor7.cpp $
 * Author:       Andrei A. Rusu
 * MODIFIED NEW VERSION
 */

#include <iostream>
#include <cstdio>
#include <cstring>
#include <sstream>
#include <ctime>
#include <cstdlib>
#include <unistd.h>
#include <numeric>
#include <stdexcept>
#include <vector>

#include <tinyxmlplus.h>

#include <webots/robot.h>
#include <webots/supervisor.h>
#include <webots/emitter.h>
#include <webots/receiver.h>
#include <webots/touch_sensor.h>

#include <boost/unordered_map.hpp>

#include "../ExperimentDefinition/AdditionalSettings.h"

using namespace std;

#define CONTROL_STEP 256
// #define CONTROL_STEP 512

/// !!!IMPORTANT! number of controllers (to wait for before quitting)
#define N 18

/// how much time to wait after deadline for termination notifications
static float termination_extra_time = 5;

/* each module is equipped with a bunch of devices */
static WbDeviceTag receiver;


int main() {

  boost::unordered_map<int, const char*> unique_collisions;

	//// necessary to initialize Webots
	wb_robot_init();

	/// get and enable devices
	receiver = wb_robot_get_device("receiver");
	wb_receiver_enable(receiver, CONTROL_STEP);

	/// say hello
  std::cout << "supervisor initialised" << std::endl;

	int count = (N * (N + 1)) / 2 - 8 - 9 + 15 + 16;
	int countdown = N;

	for (double time = 0.0; time < (deadline + termination_extra_time); time += CONTROL_STEP / 1000.0) {
		if (time > deadline) {
			/// the other controllers should be done by now,
			// so start counting termination notifications
			while (wb_receiver_get_queue_length(receiver) > 0) {
				// get id of controller signaling termination
				std::istringstream buf ( static_cast<const char*>(wb_receiver_get_data(receiver)) );
				int id;
				buf >> id;

				// update count
				count -= id;

				countdown--;
				/* fetch next packet */
				wb_receiver_next_packet(receiver);
			}

			// check count, if < 0 error, if 0 ok, both ways exit
			if (countdown == 0) { //count == 0 && 
				// success
				std::cout << "Success: all controllers have exited.\n";
				// exit the for loop and the simulation
				break;
			} else if (countdown < 0) { // count < 0 || 
				// exit the for loop and the simulation
				break;
			}
		}
		/// step the simulation forward
		if (wb_robot_step(CONTROL_STEP) == -1 || countdown <= 0) { // || count <= 0 
			break;
		}
	}

	if (countdown != 0) { // count != 0 || 
		// error
		std::cout << "Error: did not receive termination ids correctly. Exiting...\n";
		std::cout << "Countdown: " << countdown << endl; // "; Count: " << count << 
		// exit the for loop and the simulation
	}

	/// step the simulation forward (WHY AN EXTRA wb_robot_step ??)
	//	wb_robot_step(CONTROL_STEP);

	//// Before you leave, say goodbye
	std::cout << "Bye from supervisor" << endl;


	/////      TERMINATE THE SIMULATION
	wb_supervisor_simulation_quit((EXIT_SUCCESS));
	//wb_robot_step(CONTROL_STEP);
	wb_robot_cleanup();

	return 0;
}
