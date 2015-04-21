#include "Experiments/ExperimentDefinition.h"
#include <sstream>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <string>
#include <boost/filesystem.hpp>

#define REPEATS 1.0

#define MODULAR_HNEAT
#define USE_DELTAS
#define USE_BIASES

#define screen cout << fixed

// experiment folder path
string pathToExperiment = "/Users/michahell/Documents/projects_c++/experimentSuite/experiment_arena";                 
// WEBOTS world file name
string pathToWorldFile = pathToExperiment + "/worlds/arena_world_";
// CPPN archive folder name
string pathToArchive = "/CPPNarchive";
// analysis folder name
string pathToAnalysis = "/analysis";
// The current simulation's CPPN folder
string pathToCurrentCppnFolder = "";
// The current simulation's analysis folder
string pathToCurrentAnalysisFolder = "";

unsigned int currGeneration = 0;
unsigned int currIndividual = 0;

unsigned int popSize = 0;

namespace HCUBE {

  using namespace NEAT;

  extern int checkWin(int xBoardState, int oBoardState);

  // keep track of generations
  int countGeneration = 0;

  // C++ time and time struct objects used by getDateTimeString function
  time_t rawtime;
  struct tm * timeinfo;
  char buffer[80];
  string dateTimeString;

  inline double diffclock(const clock_t begin, const clock_t end) {
  	return ((end - begin) * 1000.0) / CLOCKS_PER_SEC;
  }

  std::string ModNeatExperiment7::getDateTimeString() {
    // refresh time
    time (&rawtime);
    timeinfo = localtime(&rawtime);
    // format timeinfo as string ( 28-01-2015 17:12:20 )
    strftime(buffer, 80, "%d-%m-%Y----%I-%M", timeinfo);
    dateTimeString = buffer;
    // prints 28-01-2015----17-12
    return dateTimeString;
  }

  std::string ModNeatExperiment7::createSubFolder(string pathExp, string folder, string experimentFolder) {
    // create boost path
    boost::filesystem::path cppnDir(pathExp + folder + experimentFolder);
    // make directory (if exists, it is not an error according to boost.)
    if(boost::filesystem::create_directory(cppnDir)) {
      screen << "folder created : " << experimentFolder << "\n";
    } else {
      screen << "folder already exists : " << experimentFolder << "\n";
    }
    return pathExp + folder + experimentFolder;
  }

  int ModNeatExperiment7::runSystemCommand(string command) {
    char finalCommand[command.length() + 1];
    strcpy(finalCommand, command.c_str());
    return ::system(finalCommand);
  }

  void ModNeatExperiment7::webotsEvaluateIndividual(string xmlFileName, unsigned int groupnr, string pathToWorldFile) {
    
    stringstream xml_env_name;
    xml_env_name << "CPPN_XML_FILE" << groupnr;
    string xml_env_name_string = xml_env_name.str();
    const char * xml_env_name_char = xml_env_name_string.c_str();
    screen << "env var name: " << xml_env_name_string << endl;

    // set CPPN environment variable
    ::setenv(xml_env_name_char, xmlFileName.c_str(), true);

    stringstream ss;
    ss << groupnr;
    string str_groupnr = ss.str();

    // calling the webots evaluation
    string command = "webots ";
    // time
    // command += "--minimize --mode=fast --stdout --stderr "; 
    command += pathToWorldFile + str_groupnr + ".wbt";
    // command += " > ";
    // command += xmlFileName;
    // command += ".log ";

    // screen << "Calling system(\"" << command << "\")" << endl;
    // screen << "Calling webots to evaluate individual..." << endl;

    // make system call (perform one WeBots individual simulation)
    int rez = runSystemCommand(command);
    if (rez != 0) {
      screen << "system() returned a non zero value: " << rez << "." << endl;
      // exit(-1);
    }
  }

  ModNeatExperiment7::ModNeatExperiment7(string _experimentName, int _thread_id) : 
  Experiment(_experimentName, _thread_id), numNodesX(9), numNodesY(9) {
  	screen << "Creating ModNeatExperiment7(" << _experimentName << ")" << endl;
  	// generateSubstrate();
    // create experiment CPPN folder
    string experimentFolder = "/" + getDateTimeString();
    pathToCurrentCppnFolder = createSubFolder(pathToExperiment, pathToArchive, experimentFolder);
  }

  int ModNeatExperiment7::getGroupCapacity() {
    // we want to process two individuals at a time, since we have 2 processor cores available.
    return 2;
  }

  GeneticPopulation* ModNeatExperiment7::createInitialPopulation(int populationSize) {
  	GeneticPopulation *population = new GeneticPopulation();
  	vector<GeneticNodeGene> genes;

  	// Set-up the CCPPN which encodes the substrate weights

  	////// CCPPN encoding a 3 layered sandwich substrate's weights
  	// Configuration of the CCPPN's Input layer
  	genes.push_back(GeneticNodeGene("X1", "NetworkSensor", 0, false)); // position on the X axis in the substrate's link start layers
  	genes.push_back(GeneticNodeGene("Y1", "NetworkSensor", 0, false)); // position on the Y axis in the substrate's link start layers
  	genes.push_back(GeneticNodeGene("Z1", "NetworkSensor", 0, false)); // position on the Z axis in the substrate's link start layers

  	genes.push_back(GeneticNodeGene("X2", "NetworkSensor", 0, false)); // position on the X axis in the substrate's link end layers
  	genes.push_back(GeneticNodeGene("Y2", "NetworkSensor", 0, false)); // position on the Y axis in the substrate's link end layers
  	genes.push_back(GeneticNodeGene("Z2", "NetworkSensor", 0, false)); // position on the Z axis in the substrate's link end layers
    
    #ifdef USE_BIASES
  	 genes.push_back(GeneticNodeGene("Bias", "NetworkSensor", 0, false)); // bias for the CPPN
    #endif // USE_BIASES

    #ifdef USE_DELTAS
  	 genes.push_back(GeneticNodeGene("DeltaX", "NetworkSensor", 0, false)); // offset between the two nodes on the X axis
  	 genes.push_back(GeneticNodeGene("DeltaY", "NetworkSensor", 0, false)); // offset between the two nodes on the Y axis
  	 genes.push_back(GeneticNodeGene("DeltaZ", "NetworkSensor", 0, false)); // offset between the two nodes on the Z axis
    #endif // USE_DELTAS

    #ifdef MODULAR_HNEAT
    	screen << "Laying out a heterogeneous controller CPPN" << endl;
    	genes.push_back(GeneticNodeGene("T1", "NetworkSensor", 0, false)); // position in the body
    	genes.push_back(GeneticNodeGene("T2", "NetworkSensor", 0, false)); // offset between the two nodes on the Z axis
      #ifdef USE_DELTAS
  	   genes.push_back(GeneticNodeGene("DeltaT", "NetworkSensor", 0, false)); // offset between the two nodes on the Z axis
      #endif  // USE_DELTAS
    #endif  // MODULAR_HNEAT

    #ifndef MODULAR_HNEAT
  	 screen << "Laying out a homogeneous controller CPPN" << endl;
    #endif

  	// Configuration of the CCPPN's Output layer
  	genes.push_back(GeneticNodeGene("Output", "NetworkOutputNode", 1, false, ACTIVATION_FUNCTION_SIGMOID)); // weight for link between layers A and B

    #ifdef USE_BIASES
  	 genes.push_back(GeneticNodeGene("Bias_out", "NetworkOutputNode", 1, false, ACTIVATION_FUNCTION_SIGMOID)); // bias for the second sandwich layer
    #endif // USE_BIASES

  	// Create the initial population
  	for (int a = 0; a < populationSize; a++) {
  		shared_ptr<GeneticIndividual> individual(new GeneticIndividual(genes, true, 1.0));

  		for (int b = 0; b < 0; b++) {
  			individual->testMutate();
  		}

  		population->addIndividual(individual);
  	}

  	screen << "Finished creating population\n";

    // store population and individual size
    popSize = population->getIndividualCount();
    // screen << "\n individual count: " << popSize << "\n";

  	return population;
  }


  double ModNeatExperiment7::processEvaluation(shared_ptr<NEAT::GeneticIndividual> individual, wxDC *drawContext, unsigned int groupnr) {
  	//	individual->print();
  	stringstream id_str;

    // CPPN xml file name
    id_str << pathToCurrentCppnFolder << "/cppn_" << currGeneration << "_" << currIndividual << ".xml";
    string xmlFileName = id_str.str();

    // Write CPPN to file
  	screen << "Writing file: " << "\n" << xmlFileName << endl;
  	writeCppnToXml(individual, xmlFileName);

    // evaluate individual (alternate worldfile based on groupnr)
    webotsEvaluateIndividual(xmlFileName, groupnr + 1, pathToWorldFile);

  	// read back the fitness
  	double fitness = 0.0;
  	fitness = readDoubleFromXml(xmlFileName, "Fitness");
  	screen << "Read back fitness: " << fitness << "\t\t\t\t\t\t\t\t(****) " << endl;

    // read back the fitness
    double collisions = 0.0;
    collisions = readDoubleFromXml(xmlFileName, "Collisions");
    screen << "Read back collisions: " << collisions << "\t\t\t\t\t\t\t\t(****) " << endl;

  	// fitness must be positive
  	fitness = (fitness > 0.0 ? fitness : 1.0E-8);

  	screen << "ModNeatExperiment7 Bye!\n\n\n";
  	fflush(stdout);

    return fitness;
  }

  void ModNeatExperiment7::processGroup(shared_ptr<NEAT::GeneticGeneration> generation) {
    
    // FORK a number of processes equal to the number of individuals we are going to process
    const int groupSize = getGroupSize();
    pid_t* pids = new pid_t[groupSize];

    for (int i = 0; i < groupSize; ++i) {
      // increment individual count if the current individual number is smaller than the population size.
      // if it is not, then it is the first individual of the next population.
      // this is an ugly workaround since it is not easily possible to get the current individual or generation
      // from HyperNEAT itself, thus it needs to be managed by ourselves.
      if(currIndividual == popSize) {
        // reset individual and increase generation number
        currIndividual = 1;
        currGeneration++;
      } else {
        // this is just a next individual in the current generation
        currIndividual++;
      }

      screen << "processing generation: " << currGeneration << ", individual: " << currIndividual << ", on thread: " << i << endl;
      shared_ptr<NEAT::GeneticIndividual> individual = group[i];
      // avoid 0 fitness
      if (individual->getFitness() <= 0) individual->setFitness(0.000001);

      // spawn child process
      pids[i] = fork();
      
      // child code
      if (pids[i] == 0) { 
        // code only executed by child process
        // evaluate individual and get fitness
        double fitness = processEvaluation(individual, NULL, i);
        
        // write fitness to a named textfile
        stringstream ss;
        ss << i;
        string temp_str = "simulation" + ss.str() + ".txt";

        // write fitness to textfile
        ofstream a_file (temp_str.c_str());
        a_file << fitness;
        a_file.close();

        exit(0); // exit child process successfully
      }

    }

    // WAIT for each individual process and only when all have exited, continue main process
    for (int i = 0; i < groupSize; ++i) {
      int status;
      while (-1 == waitpid(pids[i], &status, 0)) {
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
          screen << "Process " << i << " (pid " << pids[i] << ") failed" << endl;
          exit(1);
        }
      }
    }

    // READ back the fitness values and 
    for (int i = 0; i < groupSize; ++i) {
      // get individual to reward fitness too
      shared_ptr<NEAT::GeneticIndividual> individual = group[i];

      // Opens for reading the file
      stringstream ss;
      ss << i;
      string temp_str = "simulation" + ss.str() + ".txt";
      ifstream b_file (temp_str.c_str());
      // Reads one string from the file
      string fitness;
      b_file >> fitness;
      b_file.close();

      // reward this individual its fitness
      individual->reward(abs(::atof(fitness.c_str())) + 0.000001);
    }

  }


  /* ======================== XML FUNCTIONS ======================== */


  void ModNeatExperiment7::writeCppnToXml(shared_ptr<NEAT::GeneticIndividual> individual, string xmlCompleteFileName) {
  	TiXmlDocument doc(xmlCompleteFileName);
  	TiXmlElement *root = new TiXmlElement("Network");
  	individual->dump(root);
  	doc.LinkEndChild(root);
  	doc.SaveFile();
  }

  shared_ptr<NEAT::GeneticIndividual> ModNeatExperiment7::readCppnFromXml(string xmlCompleteFileName) {
  	TiXmlDocument doc1(xmlCompleteFileName);
  	doc1.LoadFile();
  	if (doc1.Error()) {
  		screen << "XML error. Exiting..." << endl;
  		fflush(stdout);
  		exit(-1);
  	}
  	TiXmlElement *root = doc1.FirstChildElement();

  	if (root == NULL) {
  		screen << "root is NULL. Exiting..." << endl;
  		fflush(stdout);
  		exit(-1);
  	}
  	return boost::shared_ptr<GeneticIndividual>(new GeneticIndividual(root));
  }

  double ModNeatExperiment7::readDoubleFromXml(string xmlCompleteFileName, string attribute) {
  	TiXmlDocument doc1(xmlCompleteFileName);
  	doc1.LoadFile();
  	if (doc1.Error()) {
  		screen << "XML error. Exiting..." << endl;
  		fflush(stdout);
  		exit(-1);
  	}
  	TiXmlElement *root = doc1.FirstChildElement();

  	if (root == NULL) {
  		screen << "root is NULL. Exiting..." << endl;
  		fflush(stdout);
  		exit(-1);
  	}

  	// return root->FirstAttribute()->DoubleValue();

  	double value;
  	const char* valueStr = root->Attribute(attribute.c_str(),	&value);

  	if (valueStr == NULL) {
  		screen << "attribute " << attribute << " is NULL. Exiting..." << endl;
  		fflush(stdout);
  		exit(-1);
  	}

  	return value;
  }


  /* ======================== EOF XML FUNCTIONS ======================== */


  #ifndef HCUBE_NOGUI
  void ModNeatExperiment7::createIndividualImage(wxDC &drawContext,shared_ptr<NEAT::GeneticIndividual> individual) {
  	if (lastIndividualSeen!=individual)
  	{
  		screen << "Repopulating substrate\n";
  		populateSubstrate(individual);
  		lastIndividualSeen = individual;
  	}

  	processEvaluation(individual,&drawContext,userBoardState);
  }

  bool ModNeatExperiment7::handleMousePress(wxMouseEvent& event,wxSize &bitmapSize) {
  	wxPoint clickPoint = event.GetPosition();
  	int x = (clickPoint.x/20) - 2;
  	int y = (clickPoint.y/20) - 2;

  	screen << "Clicked on row " << y << " and column " << x << endl;

  	if (x>=0&&y>=0&&x<3&&y<3)
  	TOGGLEBIT(userBoardState,(y*3)+x);

  	return true;
  }
  #endif

  Experiment* ModNeatExperiment7::clone() {
  	ModNeatExperiment7* experiment = new ModNeatExperiment7(*this);

  	return experiment;
  }

}