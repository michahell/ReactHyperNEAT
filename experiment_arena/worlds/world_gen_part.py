#!/usr/bin/env python

import sys, os, random, math, time
import pylab as pl

# config
circle_increments = 0.5
arena_start = 0.75
arena_end = 2.75
base_blocks_per_circle_area = 10
base_block_size = 0.05
ring_offset_x = (-0.129 / 2)
ring_offset_z = 0.25
arena_range = pl.frange(arena_start, arena_end, circle_increments)


# webots worldfile text being replaced

# pedestal_template_VRML = '''
# DEF PEDESTAL_TEMPLATE Shape {
#   appearance Appearance {
#     material Material {
#       diffuseColor 0.960784 0.984314 1
#     }
#   }
#   geometry DEF PEDESTAL_SHAPE Box {
#     size 0.16 0.002 0.035
#   }
# }

obstacle_VRML = '''
DEF OBSTACLE_%(obstacle_number)s Solid {
  translation %(trans_x)s %(trans_y)s %(trans_z)s 
  rotation 0 1 0 %(rot_alpha)s
  children [
    Shape {
      appearance Appearance {
        material Material {
          diffuseColor %(red)s %(green)s %(blue)s
        }
      }
      geometry DEF BLOCK_SHAPE_%(obstacle_number)s Box {
        size %(width)s %(height)s %(depth)s
      }
    }
  ]
  name "%(block_name)s"
  boundingObject USE BLOCK_SHAPE_%(obstacle_number)s
  physics DEF BLOCK_PHYSICS Physics {
    density -1
    mass 1
    bounce 0.05
    coulombFriction 0.9
    forceDependentSlip 0.5
  }
}'''

# pedestal
# pedestal_VRML = '''
# DEF PEDESTAL_%(obstacle_number)s Solid {
#   translation %(trans_x)s 0.001 %(trans_z)s
#   rotation 0 1 0 %(rot_alpha)s
#   children [
#     Shape {
#       appearance Appearance {
#         material Material {
#           diffuseColor 0.960784 0.984314 1
#         }
#       }
#       geometry DEF PEDESTAL_SHAPE_%(obstacle_number)s Box {
#         size 0.16 0.002 0.035
#       }
#     }
#   ]
# }'''

# obstacle_VRML = '''
# TouchSensor {
#   translation %(trans_x)s %(trans_y)s %(trans_z)s
#   rotation 0 1 0 %(rot_alpha)s
#   children [
#     Shape {
#       appearance Appearance {
#         material Material {
#           diffuseColor %(red)s %(green)s %(blue)s
#         }
#       }
#       geometry DEF BLOCK_SHAPE_%(obstacle_number)s Box {
#         size %(width)s %(height)s %(depth)s
#       }
#     }
#   ]
#   name "%(block_name)s"
#   boundingObject USE BLOCK_SHAPE_%(obstacle_number)s
#   physics Physics {
#     density -1
#     mass 1
#     bounce 0.05
#     coulombFriction 0.9
#     forceDependentSlip 0.5
#   }
# }
# '''


# some colored output functions
def notify(text):
  return '\033[0;34m' + text + '\033[0m'


def warning(text):
  return '\033[0;33m' + text + '\033[0m'


def error(text):
  return '\033[0;31m' + text + '\033[0m'


def copy_shape_and_substitute_for_each(index, arena_area, block_number, trans_x, trans_z):
  
  global obstacle_VRML
  print notify('generating shape ' + str(block_number) + ' for circle area ' + str(index) + ' at: ' + str(arena_area))

  #  block colors (simple blue tint)
  red = 0.1
  green = 0.2 + (index * 0.15)
  blue = 0.75 + (index * 0.05)

  # block height
  block_width = 0.15 # fixed widht, as of yet not important
  block_height = base_block_size + (index * 0.025)
  block_depth = 0.025 # fixed depth, as of yet not important
  trans_y = (block_height / 2) + 0.002

  # so block names start with obstacle 1
  index = index + 1 

  # random rotation
  rot_alpha = random.random() * math.pi
  
  # perform string replacement(s)
  # pedestal_VRML_replaced = pedestal_VRML % {
  #   'trans_x': str(trans_x), 'trans_z': str(trans_z),
  #   'rot_alpha': str(rot_alpha),
  #   'obstacle_number': str(block_number)
  # }

  obstacle_VRML_replaced = obstacle_VRML % {
    'trans_x': str(trans_x), 'trans_y': str(trans_y), 'trans_z': str(trans_z),
    'rot_alpha': str(rot_alpha),
    'red': str(red), 'green': str(green), 'blue': str(blue),
    'width': str(block_width), 'height': str(block_height), 'depth': str(block_depth),
    'obstacle_number': str(block_number),
    'block_name': 'block_bumper_' + str(block_number)
  }

  return obstacle_VRML_replaced # pedestal_VRML_replaced + 
  

if __name__ == "__main__":
  output = ''
  block_count_so_far = 1

  print notify('arena range: ' + str(arena_range))
  for index, circle_area in enumerate(arena_range):
    blocks_per_circle_area = int(base_blocks_per_circle_area * (circle_area + 0.5))
    print notify('blocks_per_circle_area: ' + str(blocks_per_circle_area))
    # calculate PI / (blocks_per_circle_area) amount of positions in a circle shape:
    circle_position_increments = (math.pi * 2) / blocks_per_circle_area
    for block_number in xrange(1, blocks_per_circle_area):
      trans_x = math.cos(circle_position_increments * block_number) * arena_range[index] + ring_offset_x
      trans_z = math.sin(circle_position_increments * block_number) * arena_range[index] + ring_offset_z
      # print trans_x, trans_z
      output = output + copy_shape_and_substitute_for_each(index, circle_area, block_count_so_far, trans_x, trans_z)
      # increment block number
      block_count_so_far = block_count_so_far + 1

  time.sleep(1)
  print output
