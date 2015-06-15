/*
 * File:         $Id: mod_ctrler7.cpp$
 * Date:         March 15th, 2010
 * Description:  Module controller for a quadrupled modular robot.
 * Author:       Andrei A. Rusu
 * MODIFIED NEW VERSION
 *
 */

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cmath>

#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <memory>
#include <string>
#include <vector>

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>

#include <webots/robot.h>
#include <webots/servo.h>
#include <webots/connector.h>
#include <webots/emitter.h>
#include <webots/receiver.h>
#include <webots/gyro.h>
#include <webots/gps.h>
#include <webots/touch_sensor.h>
#include <webots/distance_sensor.h>
#include <tinyxmlplus.h>

#include "NEAT.h"
#include "NEAT_NetworkIndexedLink.h"

#include "HCUBE_Defines.h"

#include "../ExperimentDefinition/AdditionalSettings.h"

using namespace std;
using namespace NEAT;
// using namespace boost;
using namespace HCUBE;

// comment to speed up controller
#define CTRLER_DEBUG
//#define CTRLER_DEBUG_WRITE_NETWORK
//

#define CPPN_BIAS 0.3

//* // comment to make biases 0
#define USE_BIASES
// */

#define WEIGHT_SCALER 0.5 // scales the remaining (over the threshold) normalised weight
#define WEIGHT_TRESHOLD 0.3 // treshold a weight has to be over to be considered

#define CONTROL_STEP 256
// #define CONTROL_STEP 512

#define W 0.95  // distance from origin is penalized exponentially with base W and power (distance_travelled/distance_from_origin) - 1
#define FITNESS_RECORDER_ID 4 // id of module equiped with a gps device to measure distances
char name[32] = "";
int control_loop_iteration = -1;
string xmlFileName;
fstream file;


bool heterogeneous_controller = true;
char *thread_id;

// #define screen cout << fixed << "["<< thread_id <<"]: " << name <<" [" <<control_loop_iteration << ", " << heterogeneous_controller << "]: "
#define screen cout << fixed

static double t = 0.0; /* time elapsed since simulation start [s] */
static int id = -1; /* this module's ID */

/* each module is equipped with a bunch of devices */
static WbDeviceTag servo, back_connector, front_connector, top_connector,
		bottom_connector, emitter, receiver_front, receiver_back, receiver_top,
		receiver_bottom, gyro, gps, distance_sensors[6], bumper;

// additional receiver for physics data
static WbDeviceTag physics_receiver;

NEAT::FastBiasNetwork<double> substrate;

const int numNodes = 3;
const int numNodesX = numNodes, numNodesY = numNodes;

template<class T> T signum(T n) {
	if (n < 0)
		return -1;
	if (n > 0)
		return 1;
	return 0;
}

map<Node, string> nameLookup;

// block collision count structure
struct blockCollisions {
  std::string defNameString;
  unsigned int collisions;
};

// setup a vector for block collision counts
std::vector<blockCollisions> collVector;

// the amounts of collisions and touch times
double totalCollisions = 0;
double totalTouchTime = 0;

// setup a map for block collision touch moment counts
typedef std::map<std::string, unsigned int> map_coll;
// std::map<std::string, unsigned int> collMap;
map_coll collMap;

// stringstream for parsing messages from physics plugin
stringstream message_parse_ss;


TiXmlElement* getIndividualXml(TiXmlDocument& cppn)
{

	cppn.LoadFile();

	if (cppn.Error())
	{
		throw std::runtime_error(std::string("XML error on load: ") + cppn.ErrorDesc() );
	}
	TiXmlElement *root = cppn.FirstChildElement();

	if (root == NULL)
	{
		throw std::runtime_error("XML root is NULL");
	}

	return root;
}

void writeFitnessToXml(string xmlCompleteFileName, const double fitness, const double distance, const double bodyheight)
{
	screen << "Writing fitness: " << fitness << " to file: "	<< xmlCompleteFileName << " ... ";
	TiXmlDocument cppn(xmlCompleteFileName);
	TiXmlElement *root = getIndividualXml(cppn);

	root->SetDoubleAttribute("Fitness", fitness);
  root->SetDoubleAttribute("Distance", distance);
  root->SetDoubleAttribute("Bodyheight", bodyheight);

	cppn.SaveFile();
	screen << "done" << endl;
}

void writeCollisionsToXml(string xmlCompleteFileName, map_coll collMap)
{
  TiXmlDocument cppn(xmlCompleteFileName);
  TiXmlElement *root = getIndividualXml(cppn);

  // create new collisions element
  TiXmlElement * collisions_node = new TiXmlElement( "collisions" );
  root->LinkEndChild( collisions_node );

  BOOST_FOREACH( map_coll::value_type &map, collMap )
  {
    screen << "map value contents:  " << map.first << " and " << map.second << endl;
    // add a collision element after last element in root
    TiXmlElement * collision_node = new TiXmlElement("collision");
    collisions_node->LinkEndChild(collision_node);

    // convert int to string.
    stringstream ss;
    ss << map.second;
    string collision_touchtime = ss.str();

    // SetAttribute (const std::string &name, const std::string &_value)
    collision_node->SetAttribute("obstacle", map.first);
    collision_node->SetAttribute("touchtime", collision_touchtime);

    totalCollisions++;
    totalTouchTime += map.second;
  }

  screen << "collisions totals:  " << totalCollisions << ", touchtime: " << totalTouchTime << endl;

  // add total number of collisions
  collisions_node->SetDoubleAttribute("total", totalCollisions);
  
  // adding total touchmoments number
  collisions_node->SetDoubleAttribute("totaltouchtime", totalTouchTime);

  cppn.SaveFile();
  screen << "done" << endl;
}

boost::shared_ptr<NEAT::GeneticIndividual> readCppnFromXml(string xmlCompleteFileName) {
	screen << "Reading CPPN from file: "	<< xmlCompleteFileName << endl;
	TiXmlDocument cppn(xmlCompleteFileName);
	TiXmlElement *root = getIndividualXml(cppn);

	return boost::shared_ptr<GeneticIndividual>(new GeneticIndividual(root));
}

void generateSubstrate() {
	screen << "Generating substrate...";
	boost::object_pool<NEAT::NetworkNode> networkNodePool;
	boost::object_pool<NEAT::NetworkLink> networkLinkPool;

	// pointer to heap allocated memory for nodes and links
	NEAT::NetworkNode *nodes = NULL;
	NEAT::NetworkLink *links = NULL;
	double *nodeBiases = NULL;

	try {
		// allocate heap space for the 3 layers of nodes
		nodes = (NEAT::NetworkNode*) malloc(sizeof(NEAT::NetworkNode)
				* numNodesX * numNodesY * 3);
		// and all the 9*9*9*9*2 possible links
		links = (NEAT::NetworkLink*) malloc(sizeof(NEAT::NetworkLink)
				* numNodesX * numNodesY * numNodesX * numNodesY * 2);
		// and the 3 sheets of biases
		nodeBiases = new double[numNodesX * numNodesY * 3];
	} catch (const std::exception &e) {
		screen << e.what() << endl;
		CREATE_PAUSE(string("Error!: ") + toString(__LINE__));
	}

	// will contain all the node names indexed by Node (Index3) objects
	nameLookup.clear();

	screen << "Creating nodes...\n";

	int nodeCounter = 0;

	/// register all 3 layers of nodes to the nameLookup map
	for (int y1 = 0; y1 < numNodesY; y1++) {
		for (int x1 = 0; x1 < numNodesX; x1++) {
			for (int z = 0; z < 3; z++) {
				// nodes are indexed from ( -numNodesX/2 ... +numNodesX/2 ) on X
				int xmod = x1 - numNodesX / 2;
				// nodes are indexed from ( -numNodesY/2 ... +numNodesY/2 ) on Y
				int ymod = y1 - numNodesY / 2;
				// create Index3 object
				Node node(xmod, ymod, z);
				/// Node name
				nameLookup[node] = (toString(xmod) + string("/") + toString(
						ymod) + string("/") + toString(z));
				/// run constructor on NEAT::NetworkNode object
				new (&nodes[nodeCounter]) NetworkNode(nameLookup[node]);
				nodeBiases[nodeCounter] = 0.0;
				nodeCounter++;
			}
		}
	}

	screen << "Creating links...\n";

	int linkCounter = 0;

	// for every node in the source layer
	for (int y1 = 0; y1 < numNodesY; y1++) {
		for (int x1 = 0; x1 < numNodesX; x1++) {

			// for every node in the destination layer
			for (int y2 = 0; y2 < numNodesY; y2++) {
				for (int x2 = 0; x2 < numNodesX; x2++) {

					// for every source index
					for (int z1 = 0; z1 < 2; z1++) {
						// for this source's destination index
						int z2 = z1 + 1;
						{
							try {
								//// create the link in the NEAT network
								/// sheets are intertwined, node order is column major
								new (&links[linkCounter++]) NetworkLink(
										&nodes[y1 * numNodesX * 3 + x1 * 3 + z1],
										&nodes[y2 * numNodesX * 3 + x2 * 3 + z2],
										0.0);
							} catch (const char *c) {
								screen << c << endl;
								CREATE_PAUSE(string("Error!: ") + toString(
										__LINE__));
							} catch (string& s) {
								screen << s << endl;
								CREATE_PAUSE(string("Error!: ") + toString(
										__LINE__));
							} catch (const std::exception &e) {
								screen << e.what() << endl;
								CREATE_PAUSE(string("Error!: ") + toString(
										__LINE__));
							} catch (...) {
								screen << "AN EXCEPTION HAS OCCURED!\n";
								CREATE_PAUSE(string("Error!: ") + toString(
										__LINE__));
							}
						}
					}
				}
			}
		}
	}

	screen << "Creating FastBiasNetwork\n";

	substrate = NEAT::FastBiasNetwork<double>(nodes, nodeCounter, links,
			linkCounter, nodeBiases);

	screen << "Testing new substrate\n";
	substrate.getLink("-1/-1/0", "-1/-1/1");

	screen << "done!\n";

	for (int a = 0; a < nodeCounter; a++) {
		nodes[a].~NetworkNode();
	}

	for (int a = 0; a < linkCounter; a++) {
		links[a].~NetworkLink();
	}

	free(nodes);
	free(links);
	delete[] nodeBiases;
}

void populateSubstrate(boost::shared_ptr<const NEAT::GeneticIndividual> individual) {
	NEAT::FastNetwork<double> network = individual->spawnFastPhenotypeStack<double> ();
	// for all nodes in the source layer
	for (int y1 = 0; y1 < numNodesY; y1++) {
		for (int x1 = 0; x1 < numNodesX; x1++) {

			// compute source node's coordinates in the NEAT nodeLookup
			const int x1mod = x1 - numNodesX / 2;
			const int y1mod = y1 - numNodesY / 2;

			// compute source node's coordinates in the CPPN
			const double x1normal = (x1 - numNodesX / 2) / double((numNodesX
					- 1) / 2);
			const double y1normal = (y1 - numNodesY / 2) / double((numNodesY
					- 1) / 2);

			// for all nodes in the target layer
			for (int y2 = 0; y2 < numNodesY; y2++) {
				for (int x2 = 0; x2 < numNodesX; x2++) {
				   for(int z1normal = -1; z1normal <= 0; z1normal+=1) {
					// compute target node's coordinates in the NEAT nodeLookup
					const int x2mod = x2 - numNodesX / 2;
					const int y2mod = y2 - numNodesY / 2;

					// compute target node's coordinates in the CPPN
					const double x2normal = (x2 - numNodesX / 2)
							/ double((numNodesX - 1) / 2);
					const double y2normal = (y2 - numNodesY / 2)
							/ double((numNodesY - 1) / 2);

					double z2normal = z1normal + 1;

					//// set up the CPPN input

					network.reinitialize();
					network.setValue("X1", x1normal);
					network.setValue("Y1", y1normal);
					network.setValue("Z1", x2normal);

					network.setValue("X2", y2normal);
					network.setValue("Y2", x2normal);
					network.setValue("Z2", y2normal);

					/// work transparently with Deltas
					if (network.hasNode("DeltaX") && network.hasNode("DeltaY") && network.hasNode("DeltaZ")) {
						network.setValue("DeltaX", x2normal - x1normal);
						network.setValue("DeltaY", y2normal - y1normal);
						network.setValue("DeltaZ", z2normal - z1normal);
					}

					/// work transparently with homogeneous & heterogeneous controllers
					if (heterogeneous_controller == true && network.hasNode(
							"T1") && network.hasNode("T2")) {
						//						screen << "Working with heterogeneous controllers"<<endl;
						double t1normal;
						double t2normal;

						double bodyLength(3.0); // Was 3.0 for original layout

						if (id <= 7) {
							t1normal = (id - 4) / bodyLength;
							t2normal = 0.25;
						} else {
							t1normal = (id - 13) / bodyLength;
							t2normal = -0.25;
						}

						network.setValue("T1", t1normal);
						network.setValue("T2", t2normal);

						if (network.hasNode("DeltaT")) {
							network.setValue("DeltaT", sqrt(t1normal * t1normal +  t2normal * t2normal)); //  (t2normal - t1normal)
						}
					} else {
						//						screen << "Working with homogeneous controllers"<<endl;
						heterogeneous_controller = false;
					}

					if (network.hasNode("Bias")) {
						// set CPPN bias
						network.setValue("Bias", CPPN_BIAS);
					}

					// propagate CPPN
					network.update();

					// get the weights for the two links between selected source-target
					// coordinates, between layers A & B, and B & C
					double output = network.getValue("Output");

					NetworkIndexedLink<double>* link;

					/// get NEAT network link object
					link = substrate.getLink(nameLookup[Node(x1mod, y1mod, z1normal + 1)],
							nameLookup[Node(x2mod, y2mod, z2normal + 1)]);

					/// set link value according to threshold

					output = pow(output, 171);

					if (fabs(output) > WEIGHT_TRESHOLD) {
						if (output > 0.0)
							link->weight = output * WEIGHT_SCALER;
						else
							link->weight = output * WEIGHT_SCALER;
					} else {
						link->weight = (0.0);
					}

				#ifdef USE_BIASES
				if(network.hasNode("Bias_out")){
					/// set the node's  biases using just the (x1,y1) values provided
					/// by the CPPN for the biases
					if (x2 == 0 && y2 == 0 && z2normal == 1)  {
						double nodeBias;
						// bias for target node
						nodeBias = network.getValue("Bias_out");
						substrate.setBias(nameLookup[Node(x1mod, y1mod, z2normal + 1)],
								nodeBias);

					}
				}
				#endif
			   }

			 }
			}
		}
	}
}

void setupNetwork() {
	//////// 			CREATE NETWORK OBJECT
	// Initialize NEAT
  
  // stringstream ss;
  // char const * homedir = getenv("HOME");
  // ss << homedir << "/" << datfile << endl;
  // const char * paramsDat = ss.str().c_str();

	// NEAT::Globals::init(paramsDat);
  // NEAT::Globals::init("/Users/michahell/Documents/projects_c++/experimentSuite/experiment_arena/ExperimentDefinition/ExperimentDefinitionParams.dat");
  // hardcoded path, uses symlink to point to the correct directory..
  NEAT::Globals::init("/Users/mtw800/experimentSuite/experiment_arena/ExperimentDefinition/ExperimentDefinitionParams.dat");

	generateSubstrate();
	boost::shared_ptr<GeneticIndividual> cppn = readCppnFromXml(xmlFileName);
	populateSubstrate(cppn);

#ifdef CTRLER_DEBUG
#ifdef CTRLER_DEBUG_WRITE_NETWORK
	file << "Controller network: " << endl;
	NetworkIndexedLink<double> * plink;
	const int numLinks = substrate.getLinkCount();
	for (int i = 0; i < numLinks; i++) {
		plink = substrate.getLink(i);
		if (plink->weight == 0.0)
			continue;
		file << (plink->fromNode) << " ---- ( " << (plink->weight) << " ) ---> " << (plink->toNode) << endl;
	}
	file << endl;
#endif
#endif
}

template<class T, int size>
class mesh2D {
public:
	T values[size][size];
	mesh2D(T val = 0) {
		for (int i = 0; i < size; i++)
			for (int j = 0; j < size; j++)
				values[i][j] = val;
	}
	mesh2D(const mesh2D<T, size> &source) {
		for (int i = 0; i < size; i++)
			for (int j = 0; j < size; j++)
				this->values[i][j] = source.values[i][j];
	}
};

template<class T, int size>
ostream &operator <<(ostream &stream, mesh2D<T, size> mesh) {
	stream << "mesh2D.values:" << endl;
	for (int i = 0; i < size; i++) {
		for (int j = 0; j < size; j++)
			stream << fixed << mesh.values[i][j] << "\t";
		stream << endl;
	}
	return stream;
}

mesh2D<double, numNodes> propagate_network(
		const mesh2D<double, numNodes> &input) {
#ifdef CTRLER_DEBUG
		file << "Network input: " << input << endl;
#endif

	/// prepare substrate network
	substrate.reinitialize();
	substrate.dummyActivation();
	// for all nodes in the input layer, in NEAT coordinates
	for (int y = (-numNodesY / 2); y <= numNodesY / 2; y++) {
		for (int x = (-numNodesX / 2); x <= numNodesX / 2; x++) {
			// compute C matrix coordinates
			int boardx = (x + (numNodesX / 2));
			int boardy = (y + (numNodesY / 2));
			/// set the right value to the input nodes (in layer A)
			//substrate.setValue(nameLookup[Node(x, y, 0)],
			//		(input.values[boardx][boardy] - normFactor) / stDev);
			substrate.setValue(nameLookup[Node(x, y, 0)], (input.values[boardx][boardy]));
		}

	}

  /* substrate.update(2) gave a compiler error for some reason */
	/* substrate.update(2); */
  substrate.update();

	/// write the output in coresponding object
	mesh2D<double, numNodes> output;
	int count = 0;
	for (int y = (-numNodesY / 2); y <= numNodesY / 2; y++) {
		for (int x = (-numNodesX / 2); x <= numNodesX / 2; x++) {
			int boardx = (x + (numNodesX / 2));
			int boardy = (y + (numNodesY / 2));
			/// read output from layer C
			output.values[boardx][boardy] = substrate.getValue(nameLookup[Node(
					x, y, 2)]);
			count++;
		}
	}
	// verify number of read output nodes
	if (count != (numNodesX * numNodesY)) {
		screen << "Error: not enough values written in output layer. Exiting..." << endl;
		exit(0);
	}

#ifdef CTRLER_DEBUG
		file << "Network output: " << output << endl;
#endif

	return output;
}

// VALUES FOR THE 3 NEIGHBORING SERVOS
mesh2D<double, 1> front_self, self, back_self, top_self, bottom_self;

int main()
{
	//// read environmental variable to get the network file
  #ifdef XML1
    char* cppnXmlFile (getenv("CPPN_XML_FILE1"));
  #endif
  
  #ifdef XML2
	 char* cppnXmlFile (getenv("CPPN_XML_FILE2"));
  #endif

  #ifdef XML3
   char* cppnXmlFile (getenv("CPPN_XML_FILE3"));
  #endif

  #ifdef XML4
   char* cppnXmlFile (getenv("CPPN_XML_FILE4"));
  #endif

  #ifdef XML5
   char* cppnXmlFile (getenv("CPPN_XML_FILE5"));
  #endif

	if (cppnXmlFile == NULL)
	{
		std::cout << "No XML file specified in CPPN_XML_FILE environment.\nAborting\n";
		return EXIT_FAILURE;
	}
	// else

	xmlFileName = cppnXmlFile;		// implicit char->string conversion? using copy constructor?

	screen << "loading CPPN file: \n" << xmlFileName << std::endl;

	/* necessary to initialize Webots */
	wb_robot_init();

	/*
	 * Find module id from robot name
	 * The robot name is used to identify each module
   * example : "module_4"
	 */
	strcpy(name, wb_robot_get_name());
	id = atoi(name + 7);
  // screen << "current module ID: " << id << endl;


  #ifdef CTRLER_DEBUG
	stringstream stream;
  
  // get CPPN name from xml filename
  unsigned found = xmlFileName.find_last_of("/");
  string cppnFileName = xmlFileName.substr(found + 1);
  screen << "xml file name without path: " << cppnFileName << endl;

	stream << cppnFileName << "_controller_debug_" << id << ".log";
	file.open(stream.str().c_str(), fstream::out);
	if (!file.is_open()) {
		screen << "Error: could not open debug log output file. ";
		exit(-1);
	}
	file << "Starting controller log:" << endl << endl;
  #endif

	// create network for this controller using the CPPN
	setupNetwork();

	screen << "mod_ctrler7: Here! \n";
	fflush( stdout);

	/* find hardware devices */
	servo = wb_robot_get_device("servo");
	front_connector = wb_robot_get_device("front_connector");
	back_connector = wb_robot_get_device("back_connector");
	top_connector = wb_robot_get_device("top_connector");
	bottom_connector = wb_robot_get_device("bottom_connector");
	emitter = wb_robot_get_device("emitter");

	receiver_front = wb_robot_get_device("receiver_front");
	receiver_back = wb_robot_get_device("receiver_back");
	receiver_top = wb_robot_get_device("receiver_top");
	receiver_bottom = wb_robot_get_device("receiver_bottom");

	// enable distance sensors
	char sonar_name[7] = "sonar0";
	for (int i = 0; i < 6; i++) {
		sonar_name[5] = '0' + i;
    // screen << "has sensor? " << wb_robot_get_device(sonar_name) << endl;
		distance_sensors[i] = wb_robot_get_device(sonar_name);
		// wb_distance_sensor_enable(distance_sensors[i], CONTROL_STEP);
    wb_distance_sensor_disable(distance_sensors[i]);
	}

	bumper = wb_robot_get_device("bumper");
	wb_touch_sensor_enable(bumper, CONTROL_STEP);

	// every module has a gyro
	gyro = wb_robot_get_device("gyro");
  // screen << "robot has gyro? " << gyro << endl;
	if (id == FITNESS_RECORDER_ID) {
		// only the first module has GPS, to compute fitness
		gps = wb_robot_get_device("gps");
    // only this module has the physics receiver
    physics_receiver = wb_robot_get_device("receiver_physics");
	}

	///  lock connectors
	wb_connector_lock(front_connector);
	wb_connector_lock(back_connector);

	if (id == 4 || id == 13) {
		wb_connector_lock(top_connector);
		wb_connector_lock(bottom_connector);
	} else {
		wb_connector_unlock(top_connector);
		wb_connector_unlock(bottom_connector);
	}

	////// 		ENABLE RECEIVERS
	wb_receiver_enable(receiver_front, CONTROL_STEP);
	if (receiver_back)
		wb_receiver_enable(receiver_back, CONTROL_STEP);
	if (receiver_top && receiver_bottom) {
		wb_receiver_enable(receiver_top, CONTROL_STEP);
		wb_receiver_enable(receiver_bottom, CONTROL_STEP);
	}
	wb_gyro_enable(gyro, CONTROL_STEP);
	if (id == FITNESS_RECORDER_ID) {
		wb_gps_enable(gps, CONTROL_STEP);
    wb_receiver_enable(physics_receiver, CONTROL_STEP);
    wb_receiver_set_channel(physics_receiver, physics_channel);
    screen << "##### physics_receiver channel set to: " << wb_receiver_get_channel(physics_receiver) << endl;
	}
	wb_servo_enable_position(servo, CONTROL_STEP);

	double distance_travelled = 0;
	double average_height = 0;
	// initial position of the controller
	double *initial_position = NULL, *old_position = NULL, old_speeds[3], old_distances[6];

	// intial joint angle [-1 ... 1] for [-pi/2 ... pi/2]
	double alpha = 0.001;
	// current target angle [-1 ... 1] for [-pi/2 ... pi/2]
	double target_angle = alpha;

	// initial angular speed [-1 ... 1] for [-10 ... 10]
	double omega = 0.5;
	// initial amplitude [-1 ... 1] for [-pi/2 ... pi/2]
	double amplitude = 0.5;
	// phase difference
  double phase = 1.0 / id;
  // reset amount of the target angle
	double reset = 0.0;

	// set initial distance

	if (id == FITNESS_RECORDER_ID) {
		// GET gps VALUES
		const double * pos1 = wb_gps_get_values(gps);
		// only module 1 should use these vectors
		initial_position = new double[3];
		old_position = new double[3];

    // check for finite amount (not NaN or infinite):
    if (std::isfinite(pos1[0]) && std::isfinite(pos1[1]) && std::isfinite(pos1[2])) {
      old_position[0] = initial_position[0] = pos1[0];
      old_position[1] = initial_position[1] = pos1[1];
      old_position[2] = initial_position[2] = pos1[2];
    } else {
  		// set double position values to zero manually.
  		old_position[0] = initial_position[0] = 0.0;
  		old_position[1] = initial_position[1] = 0.0;
  		old_position[2] = initial_position[2] = 0.0;
    }

    screen << "##### initial GPS coords. x: " << pos1[0] << " y: " << pos1[1] << " z: " << pos1[2] << endl;

	}

	// init old_speed

	// get current gyro values and write them to the old_speeds vector
	const double *speeds = wb_gyro_get_values(gyro);
	old_speeds[0] = speeds[0];
	old_speeds[1] = speeds[1];
	old_speeds[2] = speeds[2];

	// get current distance sensor reading and write them to the old_distances vector
	// for (int i = 0; i < 6; i++)
	// 	old_distances[i] = wb_distance_sensor_get_value(distance_sensors[i]);

	/*
	 * <<<<<<<<<<<<<    MAIN CONTROL LOOP      >>>>>>>>>>>>>>>>>>
	 */
	while (wb_robot_step(CONTROL_STEP) != -1) {
		//increment iteration number
		control_loop_iteration++;
    // control_loop_iteration++

    #ifdef CTRLER_DEBUG
		file << endl << "CONTROL LOOP ITERATION: " << control_loop_iteration
		<< endl << endl;
    #endif

		/* exit control loop and prepare for termination */
		if (t >= deadline) {
			break;
		}

    #ifdef CTRLER_DEBUG
  		//////////////// EXPERIMENTAL
  		// read distance sensor values
      // file << "Distances: ";
      // for (int i = 0; i < 6; i++) file << wb_distance_sensor_get_value(distance_sensors[i]) << ";\t";
      // file << endl;
      file << "Bumper: " << wb_touch_sensor_get_value(bumper) << endl;

      // to screen
      // screen << "robot " << id << " dists: ";
      // for (int i = 0; i < 6; i++) screen << wb_distance_sensor_get_value(distance_sensors[i]) << "  ";
      // screen << "Bumper: " << wb_touch_sensor_get_value(bumper) << endl;
      // screen << endl;
    #endif

		const double *pos = NULL;
		if (id == FITNESS_RECORDER_ID) {
			// GET GPS VALUES
			pos = wb_gps_get_values(gps);

      // #ifdef CTRLER_DEBUG
      //   screen << "##### current GPS coords. x: " << pos[0] << " y: " << pos[1] << " z: " << pos[2] << endl;
      // #endif

			// update the distance sum
			distance_travelled += sqrt(pow((pos[0] - old_position[0]), 2)
					+ pow((pos[2] - old_position[2]), 2));

			//// update old position
			old_position[0] = pos[0];
			old_position[1] = pos[1];
			old_position[2] = pos[2];

			// add current Y position (height)
			average_height += pos[1];

      // get any collision messages from physics plugin
      if (control_loop_iteration > 0) {
        while(wb_receiver_get_queue_length(physics_receiver) > 0) {

          int message_size = wb_receiver_get_data_size(physics_receiver);
          const char * message = (const char *) wb_receiver_get_data(physics_receiver);
          // screen << "##### physics plugin message received. size: " << message_size << ", msg: " << message << endl;
          // parse message, format:   OBSTACLE_3 1
          // clear string stream buffer
          message_parse_ss.str(std::string());
          // parse received message
          message_parse_ss << message << endl;
          string message_string = message_parse_ss.str();
          vector<string> message_split;
          
          // split on spaces
          boost::split(message_split, message_string, boost::is_any_of(" "));

          // store in the collision map  ->  std::map<std::string, unsigned int> collMap
          collMap[message_split[0]] = atoi(message_split[1].c_str());

          // move on to next head packet or nothing
          wb_receiver_next_packet(physics_receiver);
        }
      }
		}

		/////////////////////      UPDATE SELF VALUE     ///////////////////////////////

		///// the bottom left and right squares get the distance sensor information

		/// processed distance sensor information: 1 for something approaching, -1 departing, 0 for no variation
		const double threshold = 0.50;


		// double distances[6];
		// for (int i = 0; i < 6; i++)
		// 	distances[i] = wb_distance_sensor_get_value(distance_sensors[i]);

		// int activity[6] = {0,0,0,0,0,0};
		// activity[0] = (distances[0] - old_distances[0])
		// 		> threshold ? signum(distances[0] - old_distances[0]) : 0;

		// activity[1]  = (distances[1] - old_distances[1])
		// 		> threshold ? signum(distances[1] - old_distances[1]) : 0;

		// activity[2]  = (distances[2] - old_distances[2])
		// 		> threshold ? signum(distances[2] - old_distances[2]) : 0;

		// activity[3]  = (distances[3]
		// 		- old_distances[3]) > threshold ? signum(distances[3]
		// 		- old_distances[3]) : 0;

		// activity[4]  = (distances[4]
		// 		- old_distances[4]) > threshold ? signum(distances[4]
		// 		- old_distances[4]) : 0;

		// activity[5]  = (distances[5]
		// 		- old_distances[5]) > threshold ? signum(distances[5]
		// 		- old_distances[5]) : 0;

		// bool activated = false;
		// for (int i=0; i<6 && !activated;  i++)
		// 	if(activity[i] == 1)
		// 		activated = true;

		// self.values[0][0] = (activated? -1.0 : 0.0); // * (sin(10.0 * t));

		// update distance sensor information for next iteration
		// for (int i = 0; i < 6; i++)
		// 	old_distances[i] = distances[i];

		/////////////////////      END UPDATE SELF VALUE     ///////////////////////////////

		// send my current alpha
		if (wb_emitter_send(emitter, &self,
				sizeof(mesh2D<double, numNodes / 3> )) != 1) {
			screen << "Error: Sending my own package failed." << endl;
		}

		// get the front_self alpha
		if (control_loop_iteration > 0) {
			if (wb_receiver_get_queue_length(receiver_front) > 0) {
				if (wb_receiver_get_queue_length(receiver_front) != 1) {
					screen << "Warning: front receiver queue has "
							<< wb_receiver_get_queue_length(receiver_front)
							<< " packages." << endl;
				}
				front_self
						= *((mesh2D<double, numNodes / 3>*) wb_receiver_get_data(
								receiver_front));
				/* fetch next packet */
				wb_receiver_next_packet(receiver_front);
			} else {
				screen << "Error: empty front buffer." << endl;
			}
		}

		// get the back_self alpha
		if (receiver_back//id != 1 && id != 7 && id != 4 && id != 10 && id != 16 && id != 13
				&& control_loop_iteration > 0) {
			if (wb_receiver_get_queue_length(receiver_back) > 0) {
				if (wb_receiver_get_queue_length(receiver_back) != 1) {
					screen << "Warning: back receiver queue has "
							<< wb_receiver_get_queue_length(receiver_back)
							<< " packages." << endl;
				}
				back_self
						= *((mesh2D<double, numNodes / 3>*) wb_receiver_get_data(
								receiver_back));
				/* fetch next packet */
				wb_receiver_next_packet(receiver_back);
			} else {
				screen << "Error: empty back buffer." << endl;
			}
		}

		// get the top_self alpha
		if (receiver_top && receiver_bottom
				&& control_loop_iteration > 0) {
			if (wb_receiver_get_queue_length(receiver_top) > 0) {
				if (wb_receiver_get_queue_length(receiver_top) != 1) {
					screen << "Warning: top receiver queue has "
							<< wb_receiver_get_queue_length(receiver_top)
							<< " packages." << endl;
				}
				top_self
						= *((mesh2D<double, numNodes / 3>*) wb_receiver_get_data(
								receiver_top));
				/* fetch next packet */
				wb_receiver_next_packet(receiver_top);
			} else {
				screen << "Error: empty top buffer." << endl;
			}

			// handle bottom receiver
			if (wb_receiver_get_queue_length(receiver_bottom) > 0) {
				if (wb_receiver_get_queue_length(receiver_bottom) != 1) {
					screen << "Warning: bottom receiver queue has "
							<< wb_receiver_get_queue_length(receiver_bottom)
							<< " packages." << endl;
				}
				bottom_self = *((mesh2D<double, numNodes / 3>*) wb_receiver_get_data(receiver_bottom));
				/* fetch next packet */
				wb_receiver_next_packet(receiver_bottom);
			} else {
				screen << "Error: empty bottom buffer." << endl;
			}
		}
    #ifdef CTRLER_DEBUG
    		file << "Self:" << endl << self << endl;
    		file << "Front:" << front_self << endl;
    		file << "Back:" << back_self << endl;
    		file << "Top:" << top_self << endl;
    		file << "Bottom:" << bottom_self << endl;
    #endif

		////// 	COMPUTE THE NEW ALPHA_SELF

		/// create input
		mesh2D<double, numNodes> input(0);


		//////////////////////////// SUBSTRATE INPUT ////////////////////////////

		// input.values[1][0] = front_self.values[0][0];
		// input.values[1][2] = back_self.values[0][0];
		// input.values[0][1] = top_self.values[0][0];
		// input.values[2][1] = bottom_self.values[0][0];
		// input.values[1][1] = self.values[0][0];

		//////////////////////////// SUBSTRATE OUTPUT ////////////////////////////

		/// compute substrate outputs
		mesh2D<double, numNodes> output = propagate_network(input);


		// update angle, omega, amplitude, phase
		// intial joint angle [-1 ... 1] for [-pi/2 ... pi/2]
		alpha = output.values[1][1];
		// initial angular speed [-1 ... 1] for [-10 ... 10]
		omega = output.values[2][1];
		// initial amplitude [-1 ... 1] for [-pi/2 ... pi/2]
		amplitude = output.values[0][1];
		// update phase
		phase = output.values[1][2];
    // CPG diminishing / resetting output, [0...1]
    reset = output.values[1][0];

    // target_angle = -1*((1.5708 * alpha) +  (1.5708 * amplitude) * sin(10.0 * M_PI * omega * t + id * (CONTROL_STEP / 1000.0)));
    target_angle = (1.5708 * alpha) +  (1.5708 * amplitude) * sin(10.0 * M_PI * omega * t + id * (CONTROL_STEP / 1000.0));
		// target_angle = (1 - reset) * ( (1.5708 * alpha) +  (1.5708 * amplitude) * sin(10.0 * M_PI * omega * t + id * (CONTROL_STEP / 1000.0)) );


		/// set servo position
		wb_servo_set_position(servo, target_angle);

    #ifdef CTRLER_DEBUG
    		file << "alpha: " << alpha << "; omega: " << omega << "; amplitude: "
    		<< amplitude <<"; phase: "<< phase << ": servo value: " << target_angle << /*" row: " << row <<*/ endl;
    #endif

		/* computed elapsed time */
		t += CONTROL_STEP / 1000.0;
	} // end control's while loop


	// if (id == FITNESS_RECORDER_ID) {
	// 	// take the average in average_height (recordings start from iteration -1)
	// 	average_height /= (control_loop_iteration + 1);
	// }

	if (id == FITNESS_RECORDER_ID) {
    
    screen << "fitness / collision recorder adding results to cppn... " << endl;

    // iterate over the collision data
    writeCollisionsToXml(xmlFileName, collMap);

    // take the average in average_height (recordings start from iteration -1)
    average_height /= (control_loop_iteration + 1);

		// write distance traveled
		const double *pos = wb_gps_get_values(gps);

		double distance_from_origin = 999999;

		while (distance_from_origin > 100000) {
      // calculated euclidian distance
			distance_from_origin = sqrt(pow(pos[0] - initial_position[0], 2) + pow(pos[2] - initial_position[2], 2));
      // screen << "euclidian distance: " << pos[0] << " - " << initial_position[0] << " and " << pos[2] << " - " << initial_position[2] << endl;
			if (distance_from_origin > 100000) {
				stringstream stream;
				stream << "Error in computing fitness: " << distance_from_origin
						<< "; values: (" << initial_position[0] << "; "
						<< initial_position[1] << "; " << initial_position[2]
						<< ") to (" << pos[0] << "; " << pos[1] << "; " << pos[2]
						<< ")";

				screen << stream.str() << endl;
				//retrying
				sleep(1);
			}
		}

		///////////       COMPUTES THE FITNESS FUNCTION   ///////////
		// const double fitness = exp(distance_from_origin // how far the robot got
  //       * pow(W, (distance_travelled / distance_from_origin) - 1) // how much it spend getting there
  //       + (average_height)); // how high did it keep the body on average (less than 1.0m)

    // the following need to be minimized
    unsigned int maxTouchTime = 1000;
    unsigned int maxCollisions = 10;

    // calculate the collision penalisation scalar
    // double collision_penal_scalar = 1;
    // if (totalCollisions > 0 && totalCollisions <= maxCollisions) {
    //   collision_penal_scalar = 1 - (totalCollisions / maxCollisions);
    // } else {
    //   collision_penal_scalar = 0;
    // }
    // screen << "collision penal scalar = " << collision_penal_scalar << endl;

    // MINUS the amount of collisions
    // const double fitness = exp( (distance_from_origin // how far the robot got
    //     * pow(W, (distance_travelled / distance_from_origin) - 1) // how much it spend getting there
    //     + (average_height)) // how high did it keep the body on average (less than 1.0m)
    //     * collision_penal_scalar); // the fitness score gets multiplied by collision penal scalar

    unsigned int collisionTouchTimePoints = 0;
    if(maxTouchTime - totalTouchTime > 0) {
      collisionTouchTimePoints = maxTouchTime - totalTouchTime;
    } else if (maxTouchTime - totalTouchTime <= 0) {
      collisionTouchTimePoints = 0;
    }

    unsigned int collisionAmountPoints = 0;
    if(maxCollisions - totalCollisions > 0) {
      collisionAmountPoints = maxCollisions - totalCollisions;
    } else if(maxCollisions - totalCollisions <= 0) {
      collisionAmountPoints = 0;
    }

    // if more then 2 meters is travelled ( the first ring of obstacles HAD to be crossed) THEN collision points kick in:
    // else, the robot just failed to cover ground and should not get any points at all
    // (instead of getting full points because no obstacles had been encountered...)
    double finalCollisionPoints = 0;
    if(distance_from_origin > 1 || totalCollisions > 0) {
      finalCollisionPoints = ((collisionAmountPoints * collisionTouchTimePoints) / 1000) * distance_from_origin;
    }

    const double fitness = exp(distance_from_origin // how far the robot got
        * pow(W, (distance_travelled / distance_from_origin) - 1) // how much it spend getting there
        + (average_height)) // how high did it keep the body on average (less than 1.0m)
        + finalCollisionPoints; // + added the collision fitness
        

    // DISTANCE ONLY
    // const double fitness = exp(distance_from_origin);
    // const double fitness = exp(distance_from_origin * pow(W, (distance_travelled / distance_from_origin) - 1));

    // BODY HEIGHT ONLY
    // const double fitness = average_height * 10; // how high did it keep the body on average (less than 1.0m)

    screen << "Distance from origin / traveled / average height: " << endl;
    screen << distance_from_origin << " / " << distance_travelled << " / " << average_height << endl;
    
    screen << "FINAL collisions points: " << endl;
    screen << "((" << collisionAmountPoints << " * " << collisionTouchTimePoints << ") / 1000 ) * " << distance_from_origin << " = " << finalCollisionPoints << endl;

    screen << "fitness: " << endl;
    screen << fitness << endl;


		fflush(stdout);

		/// Fitness is the distance between the initial and final positions of the first controller
		// scaled down exponentially by the ratio of distance travelled
		writeFitnessToXml(xmlFileName, fitness, distance_from_origin, average_height);

	}

#ifdef CTRLER_DEBUG
	//close debug log file
	file.close();
#endif

	std::ostringstream os;
	os << id << std::ends;
	//// signal termination to supervisor
	// change emitter channel to 0 (supervisor channel)
	wb_emitter_set_channel(emitter, 0);
	// send one (int*) package with this controller's id
	if (wb_emitter_send(emitter, os.str().c_str(), os.str().length()) != 1) {
		screen << "Error: Sending termination signal (my id) to supervisor."
				<< endl;
	} else {
		// send was successful
		screen << "Sent id to supervisor. Awaiting termination!" << endl;
	}
	fflush(stdout);

	//*
	// continue until the step function returns -1, then exit
	while (wb_robot_step(CONTROL_STEP) != -1)
		control_loop_iteration++;
	// */

	screen << "Bye!" << endl;
	fflush(stdout);

	wb_robot_cleanup();

	return 0;
}

