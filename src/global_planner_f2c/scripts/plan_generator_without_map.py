#!/usr/bin/env python3
import csv
import os
import fields2cover as f2c

file_path = csv_file = os.path.expanduser("~/rover_ws/src/maps/xy_tri_final.csv")
boundary = f2c.VectorPoint()

with open(file_path, 'r') as file:
    reader = csv.reader(file, delimiter = ',')
    for line in reader:
        x = float(line[0])
        y = float(line[1])
        boundary.append(f2c.Point(x, y))

cell = f2c.Cell(f2c.LinearRing(boundary))
cells = f2c.Cells(cell)
print(type(cells))
#f2c.Visualizer.figure()
#f2c.Visualizer.plot(cells)
#f2c.Visualizer.save("hex0.png")

robot = f2c.Robot(0.4824, 1.0)
r_w = robot.getCovWidth()

const_hl = f2c.HG_Const_gen()
bf = f2c.SG_BruteForce()
obj = f2c.OBJ_FieldCoverage()



mid_hl = const_hl.generateHeadlands(cells, 5.0 * robot.getWidth())
swaths = bf.generateBestSwaths(obj, r_w, mid_hl.getGeometry(0))
print(type (swaths))


snake_sorter = f2c.RP_Snake()
swaths = snake_sorter.genSortedSwaths(swaths)

#robot.setMinTurningRadius(0)
path_planner = f2c.PP_PathPlanning()
dubins = f2c.PP_DubinsCurves()
path = path_planner.planPath(robot, swaths, dubins)
discretize_path = path.discretizeSwath(0.1)
print(discretize_path.size())

f2c.Visualizer.figure()
f2c.Visualizer.plot(cells)
f2c.Visualizer.plot(mid_hl)
f2c.Visualizer.plot(swaths)
f2c.Visualizer.plot(discretize_path)
f2c.Visualizer.save("tri_final.png")
discretize_path.saveToFile("tri_final.csv")