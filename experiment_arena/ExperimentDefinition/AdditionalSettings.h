/*
 * File:         
 * Author:       Michael Trouw
 * additional settings
 */

// Number of seconds until shutdown
float deadline = 10;

// HyperNEAT params location
// const char * datfile = "/Users/michahell/Documents/projects_c++/experimentSuite/experiment_arena/ExperimentDefinition/ExperimentDefinitionParams.dat";
// const char * datfile = "/ExperimentDefinition/ExperimentDefinitionParams.dat";
const char * datfile = "ExperimentDefinitionParams.dat";

// collision log file location
const char * physics_logfile = "/plugins/physics/collision_detector/collision_log.txt";

// experiment name
const char * experimentName = "experiment_arena";

// physics sender / receiver communication channel
float physics_channel = 20;