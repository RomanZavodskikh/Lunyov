#!/usr/bin/python3
import numpy
import np

y = np.matrix ( ((13, 21, 23), (-31, 0, 1), (43, 3, 0)) );
print(3)
for i in range(3):
    for j in range(3):
        print(y[i].transpose()[j].item(), end=" ")
    print()

print(numpy.linalg.det(y))
