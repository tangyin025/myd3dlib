import turtle
import numpy
import random
import math

p = numpy.array([0, 0])
turtle.up()
turtle.goto(p)
turtle.dot(3)
queue = [p]
grid = {(0, 0): p}
inner = 10
cellsize = inner * math.sqrt(0.5)

def near(p, gx, gy):
    for x in range(gx - 2, gx + 3):
        for y in range(gy - 2, gy + 3):
            g = grid.get((x, y))
            if g is not None:
                dist = p - g
                if dist.dot(dist) < inner * inner:
                    return True
    return False

while len(queue) > 0:
    center = queue.pop(0)
    for i in range(0, 30, 1):
        theta = random.random() * math.pi * 2
        A = 2 / (inner * inner * 4 - inner * inner)
        r = math.sqrt(2 * random.random() / A + inner * inner)
        p = center + numpy.array([r * math.cos(theta), r * math.sin(theta)])
        gx = math.floor(p[0] / cellsize)
        gy = math.floor(p[1] / cellsize)
        if p[0] > -100 and p[0] < 100 and p[1] > -100 and p[1] < 100 and not near(p, gx, gy):
            assert (gx, gy) not in grid
            grid[(gx, gy)] = p
            queue.append(p)
            turtle.goto(p)
            turtle.dot(3)

turtle.done()
