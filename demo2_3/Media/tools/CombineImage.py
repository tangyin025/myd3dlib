from PIL import Image

for d in range(0,-1,-1):
	for i in range(0,pow(2,d)):
		for j in range(0,pow(2,d)):
			img=Image.new('RGB',(1,1))
			for si in range(i*2,(i+1)*2):
				for sj in range(j*2,(j+1)*2):
					simg=Image.open("project6 Bitmap Output 256_d{0}_x{1}_y{2}.png".format(d+1,si,sj))
					print(simg.filename,simg.mode,simg.width,simg.height)
					if img.width<simg.width*pow(2,d):
						img=img.resize((simg.width*pow(2,d+1),simg.height*pow(2,d+1)))
						print("resize",img.width,img.height)
					loc=(simg.width*si,simg.height*sj)
					img.paste(simg,loc)
			img.resize((int(img.width/2),int(img.height/2))).save("project6 Bitmap Output 256_d{0}_x{1}_y{2}.png".format(d,i,j))