Houle, Daniel B.
CS420 Fall 2019
HW2

I ran `mpisearch2_skel.c` for nodes numbering 3, 5, 7, 9, 11, 13, 15, 17, 19, and 21.
Each node set was run for a total of 10 runs. The sequential linear search time and the
  distributed linear search time are recorded in CSV files, in the CSV directory.
Each node set run, was also run a second time. The first test has the sequential search
  before the distributed; the CSV files are marked `*_1.csv`. The second test has the
  sequential search after the distributed; the CSV files are marked `*_2.csv`. 
The second test somehow showed a marked improvement in the sequential search time. The
  results of these tests are in screen shots of graphs that are also in the CSV directory.
  The graphs can be easily reproduced by going into the CSV directory and running the 
  `displayCharts.py` script. Please read some of the starting comments on how to change 
  the test files that the script deals with.
The "speed up" values are calculated in the python script, as well as the "average speed
  up" values, that are also ploted. There are commented out print() statements that will
  display the actual values as well.