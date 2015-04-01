#!/usr/bin/env python

import sys, os
import numpy as np
import matplotlib.pyplot as plt
from lxml import etree
from scipy.interpolate import spline

# import settings and options
print("running with lxml.etree")
np.set_printoptions(threshold=np.nan, precision=3)

# get the results folder we want to analyse
experiment_path = str(sys.argv[1])
experiment_folder = experiment_path.split("/")[2]
fig_folder = 'plots/' + experiment_folder
num_generations = 150
num_individuals = 10
y_axis_fitness = 25
y_axis_distance = 5

# numpy arrays to hold fitness, distance, bodyheight etc. values
np_fitness = np.zeros((num_generations, num_individuals))
np_distance = np.zeros((num_generations, num_individuals))
np_best_fitness_from_generation = np.zeros(num_generations)
np_best_dist_from_generation = np.zeros(num_generations)
np_fitness_avg = np.zeros(num_generations)
np_distances_avg = np.zeros(num_generations)

def generate_raw_stat_matrices():
  for fn in os.listdir(experiment_path):
    abspath = os.path.join(experiment_path, fn);
    # print "parsing " + abspath

    curr_generation = int(fn.split("_")[1])
    curr_individual = int(fn.split("_")[2].split(".")[0])
    
    tree = etree.parse(abspath)
    root = tree.getroot()

    # todo store values in two dimensional dict
    try:
      fitness = float(str(root.get("Fitness")))
      np_fitness[curr_generation][curr_individual] = fitness;
    except ValueError, e:
      print 'error: ' + str(e);

    try:
      distance = float(str(root.get("Distance")))
      np_distance[curr_generation][curr_individual] = distance;
    except ValueError, e:
      print 'error: ' + str(e);


def generate_gen_best():
  for num_gen in xrange(0, num_generations-1):
    np_best_fitness_from_generation[num_gen] = find_best_fitness_individual_generation(num_gen)
    np_best_dist_from_generation[num_gen] = find_best_distance_individual_generation(num_gen)


def generate_gen_avg():
  for num_gen in xrange(0, num_generations-1):
    np_fitness_avg[num_gen] = np.average(np_fitness[num_gen])
    np_distances_avg[num_gen] = np.average(np_distance[num_gen])


def find_best_individual():
  best_string = ""
  best_fitness = np.amax(np_fitness)
  best_distance = np.amax(np_distance)
  best_fitness_index = np.where(np_fitness==best_fitness)
  best_distance_index = np.where(np_distance==best_distance)
  print best_fitness_index
  print best_distance_index
  best_string = "best fitness:" + str(best_fitness), "distance:" + str(best_distance), "CPPN:" + str(best_fitness_index)
  return best_string


def find_best_fitness_individual_generation(num_generation):
  np_curr_gen = np.zeros(num_generations)
  np_curr_gen = np_fitness[num_generation]
  # print np_curr_gen
  max_fitness = np.amax(np_curr_gen)
  return max_fitness


def find_best_distance_individual_generation(num_generation):
  np_curr_gen = np.zeros(num_generations)
  np_curr_gen = np_distance[num_generation]
  # print np_curr_gen
  max_distance = np.amax(np_curr_gen)
  return max_distance


def print_best_individual():
  print "\nSimulation best individual: "
  print find_best_individual()


def print_generation_best():
  print "\nGeneration best individuals: "
  for num_gen in xrange(0, num_generations-1):
    print str(num_gen) + " --> " + str(np_best_fitness_from_generation[num_gen]) + ", dist: " + str(np_best_dist_from_generation[num_gen]);


def plot_stat_fitness(stat, description, point_type, file_name):
  x_range = xrange(0, num_generations)
  fig = plt.figure()
  fig.suptitle(description, fontsize=16)
  plt.xlabel('generation', fontsize=14)
  plt.ylabel('fitness', fontsize=14)
  plt.plot(x_range, stat, point_type)
  plt.axis([0, num_generations, 0, y_axis_fitness])
  plt.show()
  fig.savefig(fig_folder + file_name)

def plot_stat_distance(stat, description, point_type, file_name):
  x_range = xrange(0, num_generations)
  fig = plt.figure()
  fig.suptitle(description, fontsize=16)
  plt.xlabel('generation', fontsize=14)
  plt.ylabel('distance', fontsize=14)
  plt.plot(x_range, stat, point_type)
  plt.axis([0, num_generations, 0, y_axis_distance])
  plt.show()
  fig.savefig(fig_folder + file_name)


def plot_stat_distance_avg(stat, description, point_type, file_name):
  x_range = xrange(0, num_generations)
  fig = plt.figure()
  fig.suptitle(description, fontsize=16)
  plt.xlabel('generation', fontsize=14)
  plt.ylabel('distance', fontsize=14)
  plt.axis([0, num_generations, 0, y_axis_distance])
  line_space = np.linspace(0, num_generations, num_generations*10)
  avg_smooth = spline(x_range, stat, line_space)
  plt.plot(x_range, stat, point_type, line_space, avg_smooth, 'b')
  plt.show()
  fig.savefig(fig_folder + file_name)





# Generate raw numpy arrays from cppn xml files
generate_raw_stat_matrices()

# generate some stats
generate_gen_best()

# generate some stats
generate_gen_avg()

# print some stats
print_best_individual()
# print_generation_best()

# plot some stats
plot_stat_fitness(np_best_fitness_from_generation, 'generation best individuals', 'ro', '__plot_gen_best.jpg')
plot_stat_fitness(np_fitness, 'all individual fitness', 'bo', '__plot_gen_all.jpg')
plot_stat_fitness(np_fitness_avg, 'average generation fitness', 'go', '__plot_gen_avg.jpg')

plot_stat_distance(np_best_dist_from_generation, 'generation best distances', 'mo', '__plot_gen_dist_best.jpg')
plot_stat_distance(np_distance, 'all individual distances', 'co', '__plot_gen_dist_all.jpg')
plot_stat_distance_avg(np_distances_avg, 'average generation distances', 'yo', '__plot_gen_dist_avg.jpg')

