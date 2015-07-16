/*
 * File:         
 * Author:       Michael Trouw
 * additional settings
 */

// Number of seconds until shutdown
#ifdef VIEWSIMULATION
  float deadline = 60;
#endif

#ifndef VIEWSIMULATION
  float deadline = 5;
#endif

// HyperNEAT params location
const char * datfile = "ExperimentDefinitionParams.dat";

// collision log file location
const char * physics_logfile = "/plugins/physics/collision_detector/collision_log.txt";

// experiment name
const char * experimentName = "experiment_arena_offspringtest";

// physics sender / receiver communication channel
float physics_channel = 20;
