#!/usr/bin/env python

from analyse_one import *

# config
folder_paths = []
folder_names = []
all_experiment_data = []
num_simulations = 0


# get a list of all dirs without the 'old' and 'sample' dirs
def get_cppn_experiment_folders(current_exp):
  global folder_paths, folder_names, fig_folder, experiment_path, experiment_folder

  experiment_path = str(current_exp) + "/CPPNarchive/"
  experiment_folder = str(current_exp)
  fig_folder = 'plots/' + experiment_folder

  print notify('starting analysis of experiment: ' + str(current_exp))

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


def plot_all_best_individuals_fitness():

  print notify('creating best fitness individuals boxplot...')

  for num_experiment, dict in enumerate(all_experiment_data):
    print 'integrating experiment : ' + dict['experiment_name']


def run_analysis():
  global np_all_fitness, np_all_distance, np_all_avg_fitness, np_all_avg_distance
  global np_all_collision, np_all_collision_touchtimes
  global np_all_collision_divided, np_all_collision_touchtimes_divided

  # define number of simulations conducted for each experiment
  num_simulations = len(folder_names)
  print notify('number of experiments done: ' + str(num_simulations))

  # numpy arrays
  np_all_fitness = np.zeros((num_generations, num_individuals * num_simulations))
  np_all_distance = np.zeros((num_generations, num_individuals * num_simulations))
  np_all_avg_fitness = np.zeros((num_generations, num_simulations))
  np_all_avg_distance = np.zeros((num_generations, num_simulations))
  np_all_collision = np.zeros((num_generations, num_individuals * num_simulations))
  np_all_collision_touchtimes = np.zeros((num_generations, num_individuals * num_simulations))

  # divided by distance travelled
  np_all_collision_divided = np.zeros((num_generations, num_individuals * num_simulations))
  np_all_collision_touchtimes_divided = np.zeros((num_generations, num_individuals * num_simulations))


  # iterate through folders to parse the data
  for index, folder_name in enumerate(folder_names):
    print 'parsing ' + str(folder_name) # + ', path: ' + folder_paths[index]
    experiment_result_data = analyse_one_experiment(folder_name, folder_paths[index])
    all_experiment_data.append(experiment_result_data)

  print notify('combined all individual experiment results.')
  print notify('length of master dict: ' + str(len(all_experiment_data)))


  #  generate all fitness boxplot graph (num experiments * 10 individuals)
  plot_all_fitness()

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

  print notify('plotted all results, single analysis done.')


if __name__ == "__main__":
  # get cli arguments = all experiment folders (except the folders old and plots)
  num_exp = len(sys.argv) - 1
  print notify(str(num_exp) + ' experiments given to analyse.')
  if (num_exp == 1):
    get_cppn_experiment_folders(sys.argv[1])
    run_analysis()
  elif (num_exp > 1):
    # define np array for each experiments best individuals:
    # TODO
    # for each experiment conduct analysis
    for exp_name in enumerate(sys.argv, index):
      if index > 1:
        get_cppn_experiment_folders(exp_name)
        run_analysis()