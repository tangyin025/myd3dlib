import turtle
import numpy
import random
import math
from PathFinding import AStar

# 绘制矩形框
ts=turtle.getscreen()
ts.bgpic("../terrain/project2 Height Output 1025.png")
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

# 鼠标点击处理
def onmouseclick(x,y):
    turtle.goto(x,y)
    turtle.dot(3)

# 输出统计信息
turtle.goto(rect[0]-50,0)
turtle.write("total:%d"%(len(grid)),False,"center")
# turtle.getcanvas().postscript(file="aaa.eps")
ts.onclick(onmouseclick)
turtle.down()
turtle.mainloop()
