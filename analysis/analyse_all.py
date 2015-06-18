#!/usr/bin/env python

from analyse_one import *

# config
folder_paths = []
folder_names = []
all_experiment_data = []
num_simulations = 0
plots_folder = 'plots/'

# structures for merging all experiment results
all_experiment_names = []
all_experiments_best_individuals_fitness = []
all_experiments_best_individuals_distance = []
all_experiments_best_individuals_collisions = []

# get a list of all dirs without the 'old' and 'sample' dirs
def get_cppn_experiment_folders(current_exp):
  global folder_paths, folder_names, fig_folder, experiment_path, experiment_folder

  # reset folders
  folder_paths = []
  folder_names = []

  experiment_path = str(current_exp) + "/CPPNarchive/"
  experiment_folder = str(current_exp)
  fig_folder = 'plots/' + experiment_folder

  print notify('starting analysis of experiment: ' + str(current_exp))
  # add experiment name to list
  all_experiment_names.append(current_exp)

  # print experiment_path
  path = os.path.abspath(os.path.dirname(__file__)) + '/../' + experiment_path
  # print warning(path)
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


# sets boxplot styles for average fitness boxplots
def set_bp_style_fitness(bp):
  for box in bp['boxes']:
    # change outline color 
    box.set( color='#ffffff', linewidth=0.5) #7570b3, 0.5
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
    median.set(color='#000000', linewidth=1.5) #b2df8a

  # change the style of fliers and their fill
  for flier in bp['fliers']:
    flier.set(color='#e7298a', alpha=0.5)
    # marker='o', 


# sets boxplot styles for average distance boxplots
def set_bp_style_distance(bp):
  for box in bp['boxes']:
    # change outline color 
    box.set( color='#7570b3', linewidth=2)
    # change fill color
    box.set( facecolor = '#1b9e77' )

  # change color and linewidth of the whiskers
  for whisker in bp['whiskers']:
    whisker.set(color='#7570b3', linewidth=1)

  # change color and linewidth of the caps
  for cap in bp['caps']:
    cap.set(color='#7570b3', linewidth=2)

  # change color and linewidth of the medians
  for median in bp['medians']:
    median.set(color='#b2df8a', linewidth=2)

  # change the style of fliers and their fill
  for flier in bp['fliers']:
    flier.set(marker='o', color='#e7298a', alpha=0.5)


# sets boxplot styles for average distance boxplots
def set_bp_style_comparison_distance(bp):
  for box in bp['boxes']:
    # change outline color 
    box.set( color='#527CBF', linewidth=2)
    # change fill color
    box.set( facecolor = '#ABC8F5' )

  # change color and linewidth of the whiskers
  for whisker in bp['whiskers']:
    whisker.set(color='#7570b3', linewidth=1)

  # change color and linewidth of the caps
  for cap in bp['caps']:
    cap.set(color='#0F2F99', linewidth=2)

  # change color and linewidth of the medians
  for median in bp['medians']:
    median.set(color='#b2df8a', linewidth=2)

  # change the style of fliers and their fill
  for flier in bp['fliers']:
    flier.set(marker='o', color='#e7298a', alpha=0.5)


def plot_all_fitness():
  global np_all_fitness
  
  print notify('creating all fitness boxplot...')

  for num_experiment, dict in enumerate(all_experiment_data):
    print 'integrating experiment : ' + dict['experiment_name']
    for num_gen, gen_fitness_vec in enumerate(dict['np_fitness']):
      curr_individual = 0;
      for fitness in np.nditer(gen_fitness_vec):
        np_all_fitness[num_gen][(num_experiment * num_individuals) + curr_individual] = fitness
        curr_individual += 1
        # print str(num_experiment) + ' / ' + str(num_gen) + ' : ' + str((num_experiment * num_individuals) + curr_individual) + ' : ' + str(fitness)

  fig = plt.figure()
  plt.xlabel('Generations')
  plt.ylabel('Fitness')
  plt.title('5 generations\' all fitness', fontsize=12)
  plt.suptitle(experiment_folder, y=1.00, fontsize=14)
  # plot
  bp = plt.boxplot(np_all_fitness.T, patch_artist=True, widths=0.8) 
  # change outline color, fill color and linewidth of the boxes
  set_bp_style_fitness(bp)
  fig.savefig(fig_folder + '__boxplot_exp_all_fitness', bbox_inches='tight')


def plot_all_avg_fitness():
  global np_all_avg_fitness
  
  print notify('creating average fitness boxplot...')

  for num_experiment, dict in enumerate(all_experiment_data):
    print 'integrating experiment : ' + dict['experiment_name']
    for num_gen, average_fitness in enumerate(dict['np_fitness_avg']):
      np_all_avg_fitness[num_gen][num_experiment] = average_fitness

  # create boxplot
  fig = plt.figure()
  plt.xlabel('Generations')
  plt.ylabel('Fitness')
  plt.title('5 generations\' average fitness', fontsize=12)
  plt.suptitle(experiment_folder, y=1.00, fontsize=14)
  # plot
  bp = plt.boxplot(np_all_avg_fitness.T, patch_artist=True, widths=0.8)
  # change outline color, fill color and linewidth of the boxes
  set_bp_style_fitness(bp)
  fig.savefig(fig_folder + '__boxplot_exp_all_avg_fitness', bbox_inches='tight')
  # also plot a separate scatter plot
  # plot_stat_2(np_all_avg_fitness, '5 generations\' average fitness', 'bo', fig_folder, '__plot_exp_all_avg_fitness', 'generation', 'fitness', 20, False)


def plot_all_avg_distance():
  global np_all_avg_distance
  
  print notify('creating average distance boxplot...')

  for num_experiment, dict in enumerate(all_experiment_data):
    print 'integrating experiment : ' + dict['experiment_name']
    for num_gen, average_distance in enumerate(dict['np_distances_avg']):
      np_all_avg_distance[num_gen][num_experiment] = average_distance

  # create boxplot
  fig = plt.figure()
  plt.xlabel('Generations')
  plt.ylabel('Distance')
  plt.title('5 generations\' average distance', fontsize=12)
  plt.suptitle(experiment_folder, y=1.00, fontsize=14)
  # plot
  bp = plt.boxplot(np_all_avg_distance.T, patch_artist=True, widths=0.8)
  # change outline color, fill color and linewidth of the boxes
  set_bp_style_fitness(bp)
  fig.savefig(fig_folder + '__boxplot_exp_all_avg_distance', bbox_inches='tight')


def plot_all_collisions():
  global np_all_collision
  
  print notify('creating all collisions boxplot...')

  for num_experiment, dict in enumerate(all_experiment_data):
    print 'integrating experiment : ' + dict['experiment_name']
    for num_gen, gen_collision_vec in enumerate(dict['np_collisions']):
      curr_individual = 0;
      for collision in np.nditer(gen_collision_vec):
        np_all_collision[num_gen][(num_experiment * num_individuals) + curr_individual] = collision
        curr_individual += 1
        # print str(num_experiment) + ' / ' + str(num_gen) + ' : ' + str((num_experiment * num_individuals) + curr_individual) + ' : ' + str(fitness)

  fig = plt.figure()
  plt.xlabel('Generations')
  plt.ylabel('Collisions')
  plt.title('5 generations\' all collisions', fontsize=12)
  plt.suptitle(experiment_folder, y=1.00, fontsize=14)
  # plot
  bp = plt.boxplot(np_all_collision.T, patch_artist=True, widths=0.8) 
  # change outline color, fill color and linewidth of the boxes
  set_bp_style_fitness(bp)
  fig.savefig(fig_folder + '__boxplot_exp_all_collisions', bbox_inches='tight')


def plot_all_collisions_divided_by_distance():
  global np_all_collision_divided
  
  print notify('creating all collisions divided by distance boxplot...')

  for num_experiment, dict in enumerate(all_experiment_data):
    print 'integrating experiment : ' + dict['experiment_name']

    for num_gen, gen_collision_vec in enumerate(dict['np_collisions']):
      curr_individual = 0;
      for collisions in np.nditer(gen_collision_vec):
        distance = dict['np_distance'][num_gen][curr_individual]
        collisions_divided = (collisions / distance) if collisions > 0 else 0
        np_all_collision_divided[num_gen][(num_experiment * num_individuals) + curr_individual] = collisions_divided
        curr_individual += 1

  fig = plt.figure()
  plt.xlabel('Generations')
  plt.ylabel('Collisions divided by distance travelled')
  plt.title('5 generations\' all collisions / distance', fontsize=12)
  plt.suptitle(experiment_folder, y=1.00, fontsize=14)
  # plot
  bp = plt.boxplot(np_all_collision_divided.T, patch_artist=True, widths=0.8) 
  # change outline color, fill color and linewidth of the boxes
  set_bp_style_fitness(bp)
  fig.savefig(fig_folder + '__boxplot_exp_all_collisions_div_distance', bbox_inches='tight')


def plot_all_collision_touchtimes():
  global np_all_collision_touchtimes
  
  print notify('creating all collisions boxplot...')

  for num_experiment, dict in enumerate(all_experiment_data):
    print 'integrating experiment : ' + dict['experiment_name']
    for num_gen, gen_collisiontouchtime_vec in enumerate(dict['np_collisions_time']):
      curr_individual = 0;
      for touchtime in np.nditer(gen_collisiontouchtime_vec):
        np_all_collision_touchtimes[num_gen][(num_experiment * num_individuals) + curr_individual] = touchtime
        curr_individual += 1
        # print str(num_experiment) + ' / ' + str(num_gen) + ' : ' + str((num_experiment * num_individuals) + curr_individual) + ' : ' + str(fitness)

  fig = plt.figure()
  plt.xlabel('Generations')
  plt.ylabel('Collision touchtimes')
  plt.title('5 generations\' all collision touch times', fontsize=12)
  plt.suptitle(experiment_folder, y=1.00, fontsize=14)
  # plot
  bp = plt.boxplot(np_all_collision_touchtimes.T, patch_artist=True, widths=0.8) 
  # change outline color, fill color and linewidth of the boxes
  set_bp_style_fitness(bp)
  fig.savefig(fig_folder + '__boxplot_exp_all_collision_touchtimes', bbox_inches='tight')


def plot_all_collision_touchtimes_divided_by_distance():
  global np_all_collision_touchtimes_divided
  
  print notify('creating all collision touchtimes divided by distance boxplot...')

  for num_experiment, dict in enumerate(all_experiment_data):
    print 'integrating experiment : ' + dict['experiment_name']

    for num_gen, gen_collisiontouchtime_vec in enumerate(dict['np_collisions_time']):
      curr_individual = 0;
      for touchtime in np.nditer(gen_collisiontouchtime_vec):
        distance = dict['np_distance'][num_gen][curr_individual]
        touchtime_divided = (touchtime / distance) if touchtime > 0 else 0
        np_all_collision_touchtimes_divided[num_gen][(num_experiment * num_individuals) + curr_individual] = touchtime_divided
        curr_individual += 1

  fig = plt.figure()
  plt.xlabel('Generations')
  plt.ylabel('Collision touchtimes divided by distance travelled')
  plt.title('5 generations\' all collision touch times / distance', fontsize=12)
  plt.suptitle(experiment_folder, y=1.00, fontsize=14)
  # plot
  bp = plt.boxplot(np_all_collision_touchtimes_divided.T, patch_artist=True, widths=0.8) 
  # change outline color, fill color and linewidth of the boxes
  set_bp_style_fitness(bp)
  fig.savefig(fig_folder + '__boxplot_exp_all_collision_touchtimes_div_distance', bbox_inches='tight')


def plot_and_gather_mean_best_fitness():
  global all_experiments_best_individuals_fitness, all_experiments_best_individuals_distance, all_experiments_best_individuals_collisions
  print notify('plotting and gathering champion fitness...')
  experiment_all_champion_fitness = np.zeros(len(all_experiment_data))
  # experiment_all_champion_distance = np.zeros(len(all_experiment_data))
  # experiment_all_champion_collisions = np.zeros(len(all_experiment_data))

  # append all champion fitnesses
  for num_experiment, dict in enumerate(all_experiment_data):
    print 'integrating experiment : ' + dict['experiment_name']
    experiment_all_champion_fitness[num_experiment] = dict['np_experiment_champion'][0]
    # experiment_all_champion_distance[num_experiment] = dict['np_experiment_champion'][1]
    # experiment_all_champion_collisions[num_experiment] = dict['np_experiment_champion'][2]

  # print warning('champion distance\':' + str(experiment_all_champion_distance))

  # save champion fitnesses in dict
  all_experiments_best_individuals_fitness.append(experiment_all_champion_fitness)

  fig = plt.figure()
  plt.xlabel(experiment_folder)
  plt.ylabel('champion fitness')
  plt.title(str(len(all_experiment_data)) + ' generations champion fitness', fontsize=12)
  # plot
  bp = plt.boxplot(experiment_all_champion_fitness, patch_artist=True, widths=0.3) 
  # # change outline color, fill color and linewidth of the boxes
  set_bp_style_fitness(bp)
  fig.savefig(fig_folder + '__boxplot_exp_champion_fitness', bbox_inches='tight')


def plot_comparison_mean_best_fitness():
  comparison_experiments_string = ', '.join(all_experiment_names)
  print warning('all experiments analyzed: ' + comparison_experiments_string)
  
  most_experiments = 0
  for exp_champions in all_experiments_best_individuals_fitness:
    most_experiments = max(most_experiments, len(exp_champions))

  # fig = plt.figure()
  fig, ax1 = plt.subplots(figsize=(10,6))

  # Add a horizontal grid to the plot, but make it very light in color
  # so we can use it for reading data values but not be distracting
  ax1.yaxis.grid(True, linestyle='-', which='major', color='lightgrey', alpha=0.5)

  plt.xlabel('Experiment variations')
  plt.ylabel('champion fitness')
  plt.title('no inputs, baseline, substrate, freeze output experiments.', fontsize=12)
  plt.suptitle(str(most_experiments) + ' generations champion fitness', y=1.00, fontsize=14)
  
  # get some fake data
  fake_data = []
  for experiment in all_experiments_best_individuals_fitness:
    fake_data.append([champFitness + 3 for champFitness in experiment]);

  # plot
  bp = plt.boxplot(all_experiments_best_individuals_fitness, patch_artist=True, widths=0.8)
  set_bp_style_fitness(bp) # change outline color, fill color and linewidth of the boxes
  
  # plot distance data
  # bp = plt.boxplot(fake_data, patch_artist=True, widths=0.5)
  # set_bp_style_comparison_distance(bp) # change outline color, fill color and linewidth of the boxes

  # set x axes and xticks
  xtickNames = plt.setp(ax1, xticklabels=all_experiment_names)
  plt.setp(xtickNames, rotation=45, fontsize=12)

  
  fig.savefig(plots_folder + 'experiments__boxplot_comparison_champion_fitness', bbox_inches='tight')


def run_experiment_analysis():
  global np_all_fitness, np_all_distance, np_all_avg_fitness, np_all_avg_distance
  global np_all_collision, np_all_collision_touchtimes
  global np_all_collision_divided, np_all_collision_touchtimes_divided
  global all_experiment_data

  # define number of simulations conducted for each experiment
  num_simulations = len(folder_names)
  print notify('number of experiments done: ' + str(num_simulations))

  # reset and fill numpy arrays with zeroes
  np_all_fitness = np.zeros((num_generations, num_individuals * num_simulations))
  np_all_distance = np.zeros((num_generations, num_individuals * num_simulations))
  np_all_avg_fitness = np.zeros((num_generations, num_simulations))
  np_all_avg_distance = np.zeros((num_generations, num_simulations))
  np_all_collision = np.zeros((num_generations, num_individuals * num_simulations))
  np_all_collision_touchtimes = np.zeros((num_generations, num_individuals * num_simulations))
  # divided by distance travelled
  np_all_collision_divided = np.zeros((num_generations, num_individuals * num_simulations))
  np_all_collision_touchtimes_divided = np.zeros((num_generations, num_individuals * num_simulations))

  # reset all_experiment_data
  all_experiment_data = []

  # iterate through folders to parse the data
  for index, folder_name in enumerate(folder_names):
    print 'parsing ' + str(folder_name) # + ', path: ' + folder_paths[index]
    experiment_result_data = analyse_one_experiment(folder_name, folder_paths[index])
    all_experiment_data.append(experiment_result_data)

  print notify('combined all individual experiment results.')
  print notify('length of master dict: ' + str(len(all_experiment_data)))

  #  generate all fitness boxplot graph (num experiments * 10 individuals)
  # plot_all_fitness()

  #  generate average fitness boxplot graph
  # plot_all_avg_fitness()

  #  generate average distance boxplot graph
  # plot_all_avg_distance()

  # collisions
  # plot_all_collisions()

  # collision touchtime
  # plot_all_collision_touchtimes()

  # collision touchtimes divided by distance travelled
  plot_all_collision_touchtimes_divided_by_distance()

  # collisions divided by distance travelled
  plot_all_collisions_divided_by_distance()

  # gather mean best fitness
  plot_and_gather_mean_best_fitness()

  print notify('plotted all results, experiment analysis done.')


def run_comparison_analysis():

  # mean best fitness comparison
  plot_comparison_mean_best_fitness()


if __name__ == "__main__":
  # get cli arguments = all experiment folders (except the folders old and plots)
  num_exp = len(sys.argv) - 1
  print notify(str(num_exp) + ' experiments given to analyse.')
  if (num_exp == 1):
    get_cppn_experiment_folders(sys.argv[1])
    run_experiment_analysis()
  elif (num_exp > 1):
    # for each experiment conduct analysis
    # and save best individuals for later mean best fitness comparison
    for index, exp_name in enumerate(sys.argv):
      if index > 0:
        get_cppn_experiment_folders(exp_name)
        run_experiment_analysis()

    # finally, run_comparison_analysis
    run_comparison_analysis()