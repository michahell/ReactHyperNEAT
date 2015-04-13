#!/usr/bin/env python

from analyse_one import *

# config
folder_paths = []
folder_names = []
all_experiment_data = []
num_experiments = 0


# get a list of all dirs without the 'old' and 'sample' dirs
def get_cppn_experiment_folders():
  global folder_paths, folder_names, fig_folder, experiment_path, experiment_folder

  experiment_path = str(sys.argv[1]) + "/CPPNarchive/"
  experiment_folder = str(sys.argv[1])
  fig_folder = 'plots/' + experiment_folder

  # print experiment_path
  path = os.path.abspath(os.path.dirname(__file__)) + '/../' + experiment_path
  # print path
  foldercount = 0

  for folder_name in os.listdir(path):
    folder_path = os.path.join(path, folder_name)
    if os.path.isdir(folder_path) and str(folder_name) not in ['old', 'samples']:
      folder_paths.append(folder_path)
      folder_names.append(folder_name)
      foldercount = foldercount + 1

  if foldercount == 0:
    print warning('no CPPN experiment folders detected, exiting.')
    sys.exit()



def plot_all_avg_fitness():
  global np_all_avg_fitness
  
  print notify('creating average fitness boxplot...')

  for index, dict in enumerate(all_experiment_data):
    print 'integrating experiment : ' + dict['experiment_name']
    for index2, average_fitness in enumerate(dict['np_fitness_avg']):
      # print 'indexes : ' + str(index) + ' : ' + str(index2)
      np_all_avg_fitness[index][index2] = average_fitness

  fig = plt.figure()
  bp = plt.boxplot(np_all_avg_fitness, patch_artist=True, widths=0.8) 

  # change outline color, fill color and linewidth of the boxes
  for box in bp['boxes']:
    # change outline color 
    box.set( color='#7570b3', linewidth=0.5)
    # change fill color
    box.set( facecolor = '#1b9e77' )

  # change color and linewidth of the whiskers
  for whisker in bp['whiskers']:
    whisker.set(color='#7570b3', linewidth=0.5)

  # change color and linewidth of the caps
  for cap in bp['caps']:
    cap.set(color='#7570b3', linewidth=0.5)

  # change color and linewidth of the medians
  for median in bp['medians']:
    median.set(color='#b2df8a', linewidth=1)

  # change the style of fliers and their fill
  for flier in bp['fliers']:
    flier.set(color='#e7298a', alpha=0.5)
    # marker='o', 

  fig.savefig(fig_folder + 'boxplot_np_all_avg_fitness', bbox_inches='tight')


def plot_all_avg_distance():
  global np_all_avg_distance
  
  print notify('creating average distance boxplot...')

  for index, dict in enumerate(all_experiment_data):
    print 'integrating experiment : ' + dict['experiment_name']
    for index2, average_distance in enumerate(dict['np_distances_avg']):
      # print 'indexes : ' + str(index) + ' : ' + str(index2)
      np_all_avg_distance[index][index2] = average_distance

  fig = plt.figure()
  bp = plt.boxplot(np_all_avg_distance, notch=True, patch_artist=True)
  
  # change outline color, fill color and linewidth of the boxes
  for box in bp['boxes']:
    # change outline color 
    box.set( color='#7570b3', linewidth=2)
    # change fill color
    box.set( facecolor = '#1b9e77' )

  # change color and linewidth of the whiskers
  for whisker in bp['whiskers']:
    whisker.set(color='#7570b3', linewidth=2)

  # change color and linewidth of the caps
  for cap in bp['caps']:
    cap.set(color='#7570b3', linewidth=2)

  # change color and linewidth of the medians
  for median in bp['medians']:
    median.set(color='#b2df8a', linewidth=2)

  # change the style of fliers and their fill
  for flier in bp['fliers']:
    flier.set(marker='o', color='#e7298a', alpha=0.5)

  fig.savefig(fig_folder + 'boxplot_np_all_avg_distance', bbox_inches='tight')


if __name__ == "__main__":
  print 'working...'

  # get all experiment folders (except the folders old and plots)
  get_cppn_experiment_folders()

  # define number of experiments
  num_experiments = len(folder_names)
  print notify('number of experiments done: ' + str(num_experiments))

  # numpy arrays
  global np_all_fitness, np_all_distance, np_all_avg_fitness, np_all_avg_distance
  np_all_fitness = np.zeros((num_experiments, num_generations, num_individuals))
  np_all_distance = np.zeros((num_experiments, num_generations, num_individuals))
  np_all_avg_fitness = np.zeros((num_experiments, num_generations))
  np_all_avg_distance = np.zeros((num_experiments, num_generations))

  # iterate through folders to parse the data
  for index, folder_name in enumerate(folder_names):
    print 'parsing ' + str(folder_name) + '...'
    experiment_result_data = analyse_one_experiment(folder_name, folder_paths[index])
    all_experiment_data.append(experiment_result_data)

  print notify('combining all individual experiment results...')
  print notify('length of master dict: ' + str(len(all_experiment_data)))

  #  generate average fitness boxplot graph
  plot_all_avg_fitness()

  #  generate average distance boxplot graph
  plot_all_avg_distance()  

  print notify('plotted all results, done.')