import sys, os
import numpy as np
from lxml import etree
from scipy.interpolate import spline

import matplotlib as mpl
mpl.use('PDF')
import matplotlib.pyplot as plt
plt.ioff()

# stackchart
x = np.arange(150)

fig, ax = plt.subplots()
ax.stackplot(x, y)
plt.show()

fig, ax = plt.subplots()
ax.stackplot(x, y1, y2, y3)
plt.show()