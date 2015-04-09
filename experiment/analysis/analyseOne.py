#!/usr/bin/env python

from analyse import *
from pylab import *
import random

def analyse_one_experiment(folder_name, folder_path):
  # store the experiment folder path
  set_experiment_path(folder_name, folder_path)

  # Generate raw numpy arrays from cppn xml files
  generate_raw_stat_matrices()

  # generate generations best 
  generate_gen_best()

  # generate generation averages
  generate_gen_avg()

  # temp override the generated matrices:
  np_fitness = np.random.power(2.0, (150, 10)) + 3
  np_distance = np.random.power(2.0, (150, 10)) + 3
  np_fitness_avg = (15 + random.random() * 23) * np.random.power(4 + random.random() * 2, 150)
  np_fitness_avg = np.sort(np_fitness_avg, axis=None, kind='mergesort')
  np_distances_avg = 12 * np.random.power(1 + random.random() * 4, 150)
  np_distances_avg = np.sort(np_distances_avg, axis=None, kind='mergesort')

  # print np_distances_avg


  # test if plotting images works headless
  # plot_stat_fitness(np_best_fitness_from_generation, 'generation best individuals', 'ro', '__plot_gen_best', False)
  # plot_stat_fitness(np_fitness, 'all individual fitness', 'bo', '__plot_gen_all', False)
  # plot_stat_fitness(np_fitness_avg, 'average generation fitness', 'go', '__plot_gen_avg', True)

  # return a dict with all the numpy arrays
  return {
    'experiment_name' : folder_name,
    'np_fitness' : np_fitness, 
    'np_distance' : np_distance,
    'np_best_fitness_from_generation' : np_best_fitness_from_generation,
    'np_best_dist_from_generation' : np_best_dist_from_generation,
    'np_fitness_avg' : np_fitness_avg,
    'np_distances_avg' : np_distances_avg
  }


if __name__ == "__main__":
  print 'working...'
  
  # we need exactly 1 CLI argument additional to the pythonic 'self' argument..
  if len(sys.argv) != (1 + 1):
    print len(sys.argv)
    print sys.argv
    print 'Provide the CPPN dir to analyse!'
    sys.exit(0)

  get_experiment_path()

  # Generate raw numpy arrays from cppn xml files
  generate_raw_stat_matrices()

  # generate generations best 
  generate_gen_best()

  # generate generation averages
  generate_gen_avg()

  # plot some stats
  plot_stat_fitness(np_best_fitness_from_generation, 'generation best individuals', 'ro', '__plot_gen_best.jpg', True)
  plot_stat_fitness(np_fitness, 'all individual fitness', 'bo', '__plot_gen_all.jpg', True)
  plot_stat_fitness(np_fitness_avg, 'average generation fitness', 'go', '__plot_gen_avg.jpg', True)

  plot_stat_distance(np_best_dist_from_generation, 'generation best distances', 'mo', '__plot_gen_dist_best.jpg', True)
  plot_stat_distance(np_distance, 'all individual distances', 'co', '__plot_gen_dist_all.jpg', True)
  plot_stat_distance_avg(np_distances_avg, 'average generation distances', 'yo', '__plot_gen_dist_avg.jpg', True)

