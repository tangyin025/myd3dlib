import random
import math

class PoissonDisc:
    def __init__(self,min,max,start,inner):
        self.min=min
        self.max=max
        self.inner=inner
        self.cellsize=inner*math.sqrt(0.5)
        self.queue=[start]
        self.grid={self.pos2grid(start):start}

    def pos2grid(self,pos):
        return (int(math.floor(pos[0]/self.cellsize)),int(math.floor(pos[1]/self.cellsize)))

    def near(self,p,g):
        for x in range(g[0]-2,g[0]+3):
            for y in range(g[1]-2,g[1]+3):
                c=self.grid.get((x,y))
                if c is not None:
                    if (math.pow(c[0]-p[0],2)+math.pow(c[1]-p[1],2))<self.inner*self.inner:
                        return True
        return False

    def sample(self):
        while self.queue:
            center=self.queue.pop(0)
            for i in range(0,30,1):
                theta=random.random()*math.pi*2
                A=2/(self.inner*self.inner*4-self.inner*self.inner)
                r=math.sqrt(2*random.random()/A+self.inner*self.inner)
                p=(center[0]+r*math.cos(theta),center[1]+r*math.sin(theta))
                g=self.pos2grid(p)
                if p[0]>self.min[0] and p[0]<self.max[0] and p[1]>self.min[1] and p[1]<self.max[1] and not self.near(p,g):
                    assert g not in self.grid
                    self.grid[g]=p
                    self.queue.append(p)
