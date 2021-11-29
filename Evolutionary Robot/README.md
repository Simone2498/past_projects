# AutoRoboEngine
Repository for the code assignments in 2020-2021-400-KEN4114 Maastricht Univeristy
Simone Grassi
David Schimmel

## How to use this code ##
Run environment to start the simulation.

- In Config.py is possible to change all the robot and simulation settings
- Run Experiments.py to run sequentially multiple experiments, with rendering and final graphs with bar errors  
- To run (with rendering) an already trained robot with all the controller weights saved as json, run Run_robot.py inserting the string and choosing the number of simulation steps
- To visualize a room and create new ones, run collision_managment.py
- checkpoint.py containts the intermediate best individuals devided by experiment and sampled every x generation. It is automaticaly created by Experiment.py.
