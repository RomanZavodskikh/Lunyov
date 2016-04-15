#!/usr/bin/python3
import numpy
import np
import random
import math

random.seed()

SIZE=500
a=[]
for i in range(SIZE):
    b=[]
    for j in range(SIZE):
        b.append((random.randint(-100, 100))/(i+j+1))
    a.append(b)

y = np.matrix (a);
print(SIZE)
for i in range(SIZE):
    for j in range(SIZE):
        print(y[i].transpose()[j].item(), end=" ")
    print()

print(numpy.linalg.det(y))
