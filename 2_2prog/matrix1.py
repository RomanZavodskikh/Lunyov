#!/usr/bin/python3
import numpy
import np

y = np.matrix ( ((1, 2, 3), (2, 0, 4), (3, 4, 5)) );
print(3)
for i in range(3):
    for j in range(3):
        print(y[i].transpose()[j].item(), end=" ")
    print()

print(numpy.linalg.det(y))
