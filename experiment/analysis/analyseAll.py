#!/usr/bin/env python

import sys, os
from analyse import *
from analyseOne import *

# config
num_experiments = 30
folder_paths = []
folder_names = []
all_experiment_data = []

# numpy arrays
np_all_fitness = np.zeros((num_experiments, num_generations, num_individuals))
np_all_distance = np.zeros((num_experiments, num_generations, num_individuals))

# get a list of all dirs without the 'old' and 'sample' dirs
def get_cppn_experiment_folders():
  global folder_paths, folder_names
  for folder_name in os.listdir("../CPPNarchive/"):
    folder_path = os.path.join(os.getcwd() + "/../CPPNarchive/", folder_name)
    if os.path.isdir(folder_path) and str(folder_name) not in ['old', 'samples']:
      folder_paths.append(folder_path)
      folder_names.append(folder_name)


if __name__ == "__main__":
  print 'working...'
  
  get_cppn_experiment_folders()
  for index, folder_name in enumerate(folder_names):
    # print str(folder)
    print 'parsing ' + str(folder_name) + '...'
    experiment_result_data = analyse_one_experiment(folder_name, folder_paths[index])
    # print warning(str(experiment_result_data))
    all_experiment_data.append(experiment_result_data)

  print notify('combining all individual experiment results...')
  print '...'
  print warning(str(all_experiment_data))
  print '...'
