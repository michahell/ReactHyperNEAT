import sys, os
import numpy as np
from lxml import etree
from scipy.interpolate import spline

import matplotlib as mpl
mpl.use('PDF')
import matplotlib.pyplot as plt
plt.ioff()

import csv

# import settings and options
print("running with lxml.etree")
np.set_printoptions(threshold=np.nan, precision=3)


# get the results folder we want to analyse
experiment_path = ''
experiment_folder = ''
fig_folder = 'plots/'

# default experiment settings
num_generations = 150
num_individuals = 10

# numpy arrays to hold fitness, distance, bodyheight etc. values
np_fitness = np.zeros((num_generations, num_individuals))
np_distance = np.zeros((num_generations, num_individuals))
np_best_fitness_from_generation = np.zeros(num_generations)
np_best_dist_from_generation = np.zeros(num_generations)
np_fitness_avg = np.zeros(num_generations)
np_distances_avg = np.zeros(num_generations)
np_species = np.zeros((num_generations, num_individuals))
np_experiment_champ = np.zeros(4)

# other datatypes, python lists for quick prototyping etc.
unique_species = []

# added for arena experiment
np_collisions = np.zeros((num_generations, num_individuals))
np_collisions_avg = np.zeros(num_generations)

np_collisions_time = np.zeros((num_generations, num_individuals))
np_collisions_time_avg = np.zeros(num_generations)

# some colored output functions
def notify(text):
  return '\033[0;34m' + text + '\033[0m'


def warning(text):
  return '\033[0;33m' + text + '\033[0m'


def error(text):
  return '\033[0;31m' + text + '\033[0m'


def get_experiment_path():
  global experiment_path, experiment_folder, fig_folder

  experiment_path = str(sys.argv[1])
  experiment_folder = experiment_path.split("/")[3]
  print experiment_path, experiment_folder
  fig_folder = 'plots/' + experiment_folder


def set_experiment_path(folder_name, folder_path):
  global experiment_path, experiment_folder, fig_folder

  experiment_path = folder_path
  experiment_folder = folder_name
  fig_folder = 'plots/' + experiment_folder


def generate_raw_stat_matrices():
  global np_fitness, np_distance, np_collisions, np_collisions_time, np_species, np_experiment_champ

  for fn in os.listdir(experiment_path):
    abspath = os.path.join(experiment_path, fn);
    # print "parsing " + abspath

    # individuals range from 1 to 10, so we want to make this from 0...9
    curr_generation = int(fn.split("_")[1])
    curr_individual = int(fn.split("_")[2].split(".")[0]) - 1
    
    try:
      tree = etree.parse(abspath)
      root = tree.getroot()
      collisions_elem = tree.xpath('/Network/collisions')

      # try to get the fitness xml attribute
      try:
        fitness = float(str(root.get("Fitness")))
        np_fitness[curr_generation][curr_individual] = fitness;
      except ValueError, e:
        print error('ValueError: ' + str(e));
      
      # try to get the distance xml attribute
      try:
        distance = float(str(root.get("Distance")))
        np_distance[curr_generation][curr_individual] = distance;
      except ValueError, e:
        print error('error: ' + str(e));

      # try to get the speciesID xml attribute
      try:
        speciesID = int(str(root.get("SpeciesID")))
        np_species[curr_generation][curr_individual] = speciesID
        if speciesID not in unique_species:
          # print 'new species in gen. ' + str(curr_generation) + ', individual: ' + str(curr_individual) + ' = ' + str(speciesID)
          unique_species.append(speciesID)
      except ValueError, e:
        print error('error: ' + str(e));

      # try to get collision data
      try:
        collisions_total = float(str(collisions_elem[0].get("total")))
        collisions_touchtime = float(str(collisions_elem[0].get("totaltouchtime")))
        np_collisions[curr_generation][curr_individual] = collisions_total;
        np_collisions_time[curr_generation][curr_individual] = collisions_touchtime;

      except ValueError, e:
        print error('error: ' + str(e));
      except IndexError, e:
        print error('error: ' + str(e));

    except etree.XMLSyntaxError, e:
      print error('XMLSyntaxError: ' + str(e));
  
  # get champ
  np_experiment_champ[0] = np.amax(np_fitness)
  # np_experiment_champ[0] = np_fitness.argmax()
  # index = np.where( np_fitness == np_fitness.argmax() )
  # print np_experiment_champ[0], np_fitness.argmax(), str(index)
  # np_experiment_champ[1] =  np_fitness[index[0], index[1]]
  # np_experiment_champ[1] = np_distance[index[0], index[1]]


def generate_gen_best():
  global np_best_fitness_from_generation, np_best_dist_from_generation
  for num_gen in xrange(0, num_generations-1):
    np_best_fitness_from_generation[num_gen] = find_best_fitness_individual_generation(num_gen)
    np_best_dist_from_generation[num_gen] = find_best_distance_individual_generation(num_gen)


def generate_gen_avg():
  global np_fitness_avg, np_distances_avg
  for num_gen in xrange(0, num_generations-1):
    np_fitness_avg[num_gen] = np.average(np_fitness[num_gen])
    np_distances_avg[num_gen] = np.average(np_distance[num_gen])


def generate_collision_avg():
  global np_collisions_avg, np_collisions_time_avg
  for num_gen in xrange(0, num_generations-1):
    np_collisions_avg[num_gen] = np.average(np_collisions[num_gen])
    np_collisions_time_avg[num_gen] = np.average(np_collisions_time[num_gen])


def generate_species():
  global np_species
  # print unique_species
  # print '\n\r'
  # print len(unique_species)
  # print '\n\r'
  # print np_species

  # parse offspringTable.csv
  # with open('offspringTable.csv', 'rb') as csvfile:
  #   offspringTable = csv.reader(csvfile, delimiter = ' ', quotechar='|')
  #   for row in offspringTable:
  #     print ', '.join(row)
      # Spam, Spam, Spam, Spam, Spam, Baked Beans
      # Spam, Lovely Spam, Wonderful Spam


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


def plot_stat(stat, description, point_type, file_name, label_x, label_y, y_axis_max, show_plot):
  x_range = xrange(0, num_generations)
  fig = plt.figure()
  fig.suptitle(description, fontsize=16)
  plt.xlabel(label_x, fontsize=14)
  plt.ylabel(label_y, fontsize=14)
  plt.plot(x_range, stat, point_type)
  plt.axis([0, num_generations, 0, y_axis_max])
  if show_plot:
    plt.show()
  fig.savefig(fig_folder + file_name)


def plot_stat_2(stat, description, point_type, fig_folder, file_name, label_x, label_y, y_axis_max, show_plot):
  x_range = xrange(0, num_generations)
  fig = plt.figure()
  fig.suptitle(description, fontsize=16)
  plt.xlabel(label_x, fontsize=14)
  plt.ylabel(label_y, fontsize=14)
  plt.plot(x_range, stat, point_type)
  plt.axis([0, num_generations, 0, y_axis_max])
  if show_plot:
    plt.show()
  fig.savefig(fig_folder + file_name)


def plot_stat_line(stat, description, point_type, file_name, label_x, label_y, y_axis_max, show_plot):
  x_range = xrange(0, num_generations)
  fig = plt.figure()
  fig.suptitle(description, fontsize=16)
  plt.xlabel(label_x, fontsize=14)
  plt.ylabel(label_y, fontsize=14)
  plt.axis([0, num_generations, 0, y_axis_max])
  line_space = np.linspace(0, num_generations, num_generations*10)
  avg_smooth = spline(x_range, stat, line_space)
  plt.plot(x_range, stat, point_type, line_space, avg_smooth, 'b')
  if show_plot:
    plt.show()
  fig.savefig(fig_folder + file_name)


if __name__ == "__main__":
  print 'this is a support module and not meant to be executable.'