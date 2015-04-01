#!/usr/bin/env python

import sys, os, re
import numpy as np
import matplotlib.pyplot as plt
from lxml import etree

# import settings and options
print("running with lxml.etree")
np.set_printoptions(threshold=np.nan, precision=3)

# [mod_ctrler7_1] robot 12 dists: 0.000000  1.000233  0.947317  0.000000  0.000000  0.845146  
# [mod_ctrler7_2] robot 2 dists: 0.000000  0.850488  0.000000  0.000000  0.988482  1.028320  

# all robots are numbered
num_robots = range(17)
timesteps = 1500
parallel_controllers = [1]


for controller in parallel_controllers:
  controller = '[mod_ctrler7_{0}]'.format(controller)
  for robot in num_robots:
    
    # then loop through file and parse its dist sensor readouts
    fh = open('log_dists_example_reasonable_2.txt', 'r')
    curr_line = 0

    # create np array to hold all readout values
    np_dist_readouts = np.zeros((timesteps, 6))
    str_matcher = controller + ' robot ' + str(robot) + ' dists:'
    for line in fh:
      if str_matcher in str(line).lower():
        # get the numbers in an array
        numbers_list = line.replace(str_matcher, '').split()
        # put those numbers inside the np matrix
        for index, num in enumerate(numbers_list):
          np_dist_readouts[curr_line][index] = numbers_list[index]

        curr_line = curr_line + 1

    # print how many sensor readouts there are
    print  str(controller) + ' robot ' + str(robot) + ' --> ' + str(curr_line)

    # np matrix is now made into a graph
    fig = plt.figure()
    fig.suptitle(str(controller) + ' robot ' + str(robot) + ' sonar distance sensors (6) readouts', fontsize=14)
    plt.xlabel('readout', fontsize=14)
    plt.ylabel('level', fontsize=14)
    plotje = plt.plot(xrange(0, timesteps), np_dist_readouts)
    plt.axis([0, timesteps, 0, 1.5])
    plt.setp(plotje, linewidth=2.0)

    plt.show()
    fig.savefig('__test.jpg')