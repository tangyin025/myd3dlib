import cv2
import numpy as np
from matplotlib import pyplot as plt
for d in range(0,-1,-1):
	for x in range(0,pow(2,d)):
		for y in range(0,pow(2,d)):
			for sx in range(x*2,(x+1)*2):
				for sy in range(y*2,(y+1)*2):
					simg=cv2.imread("../terrain/project2 Bitmap Output 1025_d{0}_x{1}_y{2}.png".format(d+1,sx,sy))
					if 'img' not in dir():
						img=np.empty([simg.shape[0]*pow(2,d+1),simg.shape[1]*pow(2,d+1),simg.shape[2]],simg.dtype)
						print(img.shape,img.dtype)
					img[simg.shape[0]*sx:simg.shape[0]*(sx+1),simg.shape[1]*sy:simg.shape[1]*(sy+1)]=simg
			rimg=cv2.resize(img,None,fx=0.5,fy=0.5,interpolation=cv2.INTER_CUBIC)
			cv2.imwrite("../terrain/project2 Bitmap Output 1025_d{0}_x{1}_y{2}.png".format(d,x,y),rimg)
			# plt.imshow(rimg)
			# plt.show()
