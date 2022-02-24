import turtle
import numpy
import random
import math
import cv2
import xml.dom.minidom as minidom
from RandomSampling import PoissonDisc
from PathFinding import AStar

# 坐标转换
step=4
def tur2img(img,tp):
    return (int(math.floor((tp[0]+img.shape[1]/2)/step+0.5)*step),int(math.floor((img.shape[0]/2-tp[1])/step+0.5)*step))

def img2tur(img,ip):
    return (ip[0]-img.shape[1]/2,img.shape[0]/2-ip[1])

def img2wm(img,ip):
    return (ip[0]*7.8125,(ip[1]-img.shape[1])*7.8125)

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

# 泊松盘采样
disc=PoissonDisc((rect[0],rect[1]),(rect[2],rect[3]),turtle.pos(),50)
random.seed(3)
disc.sample()
i=0
for k,v in disc.grid.items():
    turtle.goto(v)
    turtle.dot(3)
    turtle.write(i,False,"center")
    i=i+1

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
        return dx*dx+dy*dy+dh*dh*320

img=cv2.imread("../../terrain/project2 Height Output 1025.png")

# 鼠标点击处理
all_paths=[]
def onmouseclick(x,y):
    finder=AStar2D(img,tur2img(img,turtle.pos()),tur2img(img,(x,y)),50000)
    if finder.solve():
        pos=finder.goal
        path=[]
        while pos in finder.came_from:
            path.append(pos)
            pos=finder.came_from[pos]
        turtle.down()
        for pos in reversed(path):
            turtle.goto(img2tur(img,pos))
        all_paths.append(path)
    else:
        print("failed",finder.start,finder.goal,len(finder.close))
        turtle.up()
        turtle.goto((x,y))

# 绘制统计信息
turtle.goto(rect[0]-50,0)
turtle.write("total:%d"%(len(disc.grid)),False,"center")
# turtle.getcanvas().postscript(file="aaa.eps")
ts.onclick(onmouseclick)
turtle.down()
turtle.mainloop()

# 输出路劲svg
dom=minidom.getDOMImplementation().createDocument(None,'svg',None)
root=dom.documentElement
lt=img2wm(img,(0,0))
rb=img2wm(img,(img.shape[0],img.shape[1]))
root.setAttribute('width',"%dpx"%rb[0])
root.setAttribute('height',"%dpx"%-lt[1])
root.setAttribute("viewBox","%d %d %d %d"%(lt[0],lt[1],rb[0],-lt[1]))
for path in all_paths:
    for pos in path:
        wmp=img2wm(img,pos)
        if 'path_str' not in dir():
            path_str="M{0},{1}".format(wmp[0],wmp[1])
        else:
            path_str=path_str+" L{0},{1}".format(wmp[0],wmp[1])
    element = dom.createElement('path')
    element.setAttribute('d', path_str)
    root.appendChild(element)
    del path_str
with open('aaa.svg','w',newline='\n',encoding='utf-8') as f:
    dom.writexml(f, addindent='\t', newl='\n',encoding='utf-8')