# Houle, Daniel B
# CS420 Fall 2019
# HW2
import math
import csv
import glob
import matplotlib.pyplot as plt
from matplotlib import gridspec
from matplotlib.lines import Line2D

# method to flatten a multidimensional list up 1 dimension
def flattenList(dirty):
  clean = []
  for sub in dirty:
    for i in sub:
      clean.append(i)
  return clean

# All of the CSV files should be in the same directory as as this script
startPath = './'
# list to hold the info in to graph
nodes = {}
# loops over the given directory to find all of the files specified in the REGEX
  # REGEX Key:
  # _1.csv = sequential before distributed
  # _2.csv = distributed before sequential
for relPath in glob.glob(startPath + "*_1.csv"): 
  with open(relPath, 'r') as csvFile:
    info = csv.reader(csvFile,delimiter=',')
    # `fileName` is used as a key in the `nodes` list, and become the title of each subplot
    fileName = relPath.split('/')[-1].split('.')[0]
    # Flag to skip the first line of each CSV file as the labels don't matter here
    trip = True
    nodes[fileName] = []
    for row in info:
      if trip:
        trip = False
        continue
      nodes[fileName].append(row)  

# Calculate the dimensions of the subplots' graph
N = len(nodes) + 1
cols = 4
rows = int(math.ceil(N / float(cols)))
# used to place the subplots evenly
gs = gridspec.GridSpec(rows, cols)

# holds the subplots
tots = plt.figure()
i = 0
# this is the average of each node set
avg = {}
# loop over the `nodes` list
for key in sorted(nodes.keys()):
  current = nodes[key]
  runs = [] # becomes the X-axis
  l = [] # sequential search time
  d = [] # distributed search time
  s = [] # speed up value
  for q in current:
    # need to convert to appropriate number values to use them
    runs.append(int(q[0]))
    temp = [float(w) for w in q[1::]]
    l.append(temp[0])
    d.append(temp[1])
    s.append(temp[0]/temp[1])
  # add the position of the next subplot
  t = tots.add_subplot(gs[i])
  i = i + 1
  # state what lines need to be ploted, and give the subplot a title
  t.plot(runs,l, color='b')
  t.plot(runs,d, color='y')
  t.plot(runs,s, color='r')
  t.title.set_text(key)
  # split the key up, to just get the number of nodes used
  nKey = int(key.replace('nodes','').split('_')[0])
  # calculate the average of the speed up value for the current node set
  avg[nKey] = (sum(s)/len(s))

# This is used for the legend.
  # it is required, as many lines are being ploted, and would show up in the legend
  # as separate data
custom_lines = [Line2D([0], [0], color='b', lw=4),
                Line2D([0], [0], color='y', lw=4),
                Line2D([0], [0], color='r', lw=4)]
# Adding the subplot for the average speed up values
t = tots.add_subplot(gs[i])
t.plot(avg.keys(),avg.values(), color='r')
t.title.set_text('AVG Speed Up')
# adds the legend for the whole plot
tots.legend(handles=custom_lines,labels=['sequential', 'distributed', 'speed up'],loc='lower right',prop={'size': 18})
# Do the thing
plt.show()



