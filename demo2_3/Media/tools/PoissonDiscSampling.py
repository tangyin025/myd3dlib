import turtle
import numpy
import random
import math
from PathFinding import AStar
import cv2

# 坐标转换
step=8
def tur2img(img,tp):
    return (int(math.floor((tp[0]+img.shape[1]/2)/step+0.5)*step),int(math.floor((img.shape[0]/2-tp[1])/step+0.5)*step))

def img2tur(img,ip):
    return (ip[0]-img.shape[1]/2,img.shape[0]/2-ip[1])

# 绘制矩形框
ts=turtle.getscreen()
ts.bgpic("../../terrain/project2 Height Output 1025.png")
rect=(-500,-500,500,500)
turtle.speed("fastest")
turtle.delay(0)
# turtle.tracer(False)
turtle.up()
turtle.goto(rect[0],rect[1])
turtle.down()
turtle.goto(rect[2],rect[1])
turtle.goto(rect[2],rect[3])
turtle.goto(rect[0],rect[3])
turtle.goto(rect[0],rect[1])
turtle.up()
turtle.colormode(255)
turtle.color((255,255,0),(255,0,0))

# 中心点为起点
random.seed(3)
p=numpy.array([0,0])
turtle.goto(p)
queue=[p]
grid={(0,0):p}
turtle.dot(3)
turtle.write(len(grid),False,"center")
inner=50
cellsize=inner*math.sqrt(0.5)

# 遍历最近节点
def near(p,gx,gy):
    for x in range(gx-2,gx+3):
        for y in range(gy-2,gy+3):
            g=grid.get((x,y))
            if g is not None:
                dist=p-g
                if dist.dot(dist)<inner*inner:
                    return True
    return False

# 泊松盘采样
while len(queue)>0:
    center=queue.pop(0)
    for i in range(0,30,1):
        theta=random.random()*math.pi*2
        A=2/(inner*inner*4-inner*inner)
        r=math.sqrt(2*random.random()/A+inner*inner)
        p=center+numpy.array([r*math.cos(theta),r*math.sin(theta)])
        gx=math.floor(p[0]/cellsize)
        gy=math.floor(p[1]/cellsize)
        if p[0]>rect[0] and p[0]<rect[2] and p[1]>rect[1] and p[1]<rect[3] and not near(p,gx,gy):
            assert (gx,gy) not in grid
            grid[(gx,gy)]=p
            queue.append(p)
            turtle.goto(p)
            turtle.dot(3)
            turtle.write(len(grid),False,"center")

# 寻路准备
class AStar2D(AStar):
    def __init__(self,img,start,goal,depth):
        super(AStar2D, self).__init__(start,goal,depth)
        self.img=img
    def heuristic_cost_estimate(self,start,goal):
        dx=goal[0]-start[0]
        dy=goal[1]-start[1]
        return dx*dx+dy*dy
    def get_neighbors(self,pos):
        neis=(
            (pos[0]-step,pos[1]-step),
            (pos[0],pos[1]-step),
            (pos[0]+step,pos[1]-step),
            (pos[0]-step,pos[1]),
            (pos[0]+step,pos[1]),
            (pos[0]-step,pos[1]+step),
            (pos[0],pos[1]+step),
            (pos[0]+step,pos[1]+step),
            (pos[0]-step,pos[1]-step*2),
            (pos[0]+step,pos[1]-step*2),
            (pos[0]-step*2,pos[1]-step),
            (pos[0]+step*2,pos[1]-step),
            (pos[0]-step*2,pos[1]+step),
            (pos[0]+step*2,pos[1]+step),
            (pos[0]-step,pos[1]+step*2),
            (pos[0]+step,pos[1]+step*2))
        ret_neis=[]
        for nei in neis:
            if nei[0]>=0 and nei[0]<self.img.shape[0] and nei[1]>=0 and nei[1]<self.img.shape[1]:
                ret_neis.append(nei)
        return ret_neis
    def dist_between(self,start,goal):
        dx=goal[0]-start[0]
        dy=goal[1]-start[1]
        dh=math.fabs(float(img[goal[1],goal[0],0])-float(img[start[1],start[0],0]))
        return dx*dx+dy*dy+dh*dh*120

img=cv2.imread("../../terrain/project2 Height Output 1025.png")

# 鼠标点击处理
def onmouseclick(x,y):
    finder=AStar2D(img,tur2img(img,turtle.pos()),tur2img(img,(x,y)),50000)
    if finder.solve():
        pos=finder.goal
        path=[]
        while pos in finder.came_from:
            path.append(pos)
            pos=finder.came_from[pos]
        for pos in reversed(path):
            turtle.goto(img2tur(img,pos))
    else:
        print("failed",finder.start,finder.goal,len(finder.close))
        turtle.goto((x,y))

# 输出统计信息
turtle.goto(rect[0]-50,0)
turtle.write("total:%d"%(len(grid)),False,"center")
# turtle.getcanvas().postscript(file="aaa.eps")
ts.onclick(onmouseclick)
turtle.down()
turtle.mainloop()
