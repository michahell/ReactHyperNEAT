#ifndef ModNeatExperiment7_H_
#define ModNeatExperiment7_H_

#include <iostream>
#include <cstdio>
#include <string>
#include "HCUBE_Defines.h"
#include "Experiments/HCUBE_Experiment.h"

namespace HCUBE {
  
  class ModNeatExperiment7: public Experiment {

  protected:
  	//	NEAT::FastBiasNetwork<double> substrate;
  	int numNodesX, numNodesY;
  	map<Node, string> nameLookup;

  private:
    int runSystemCommand(string command);
    void webotsEvaluateIndividual(string xmlFileName, unsigned int groupnr, string pathToWorldFile);
    std::string getDateTimeString();
    std::string createSubFolder(string pathExp, string folder, string subfolder);

  public:
  	ModNeatExperiment7(string _experimentName, int _thread_id);

  	virtual ~ModNeatExperiment7() {
  	}

  	virtual NEAT::GeneticPopulation * createInitialPopulation(int populationSize);

  	double processEvaluation(shared_ptr<NEAT::GeneticIndividual> individual, wxDC *drawContext, unsigned int groupnr);

    virtual int getGroupCapacity();

  	virtual void processGroup(shared_ptr<NEAT::GeneticGeneration> generation);

  	virtual inline bool performUserEvaluations() {
  		return false;
  	}

  	virtual inline bool isDisplayGenerationResult() {
  		return false;
  	}

  	void writeCppnToXml(shared_ptr<NEAT::GeneticIndividual> individual, string xmlCompleteFileName);

  	shared_ptr<NEAT::GeneticIndividual> readCppnFromXml(string xmlCompleteFileName);

  	double readDoubleFromXml(string xmlCompleteFileName, string attribute) ;

  	virtual Experiment* clone();

  };

}

#endif // ModNeatExperiment7_H_
