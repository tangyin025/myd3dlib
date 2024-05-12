# 在Maya中生成MarchingCubes模型集合
import maya.cmds as cmd
for i in range(1,255):
	names=[]
	for j in range(0,8):
		if 1<<j&i:
			names.append("aaa%d_%d"%(i,j))
			cmd.polyCube(w=1<<(1<<2^j)&i and 1 or 0.9,h=1<<(1<<1^j)&i and 1 or 0.9,d=1<<(1<<0^j)&i and 1 or 0.9,n=names[-1])
			cmd.move((1<<2&j and 0.5 or -0.5),1<<1&j and 0.5 or -0.5,1<<0&j and 0.5 or -0.5)
	if len(names)>1:
		cmd.polyCBoolOp(names,n="aaa%d"%(i))
	elif len(names)>0:
		cmd.rename(names[-1],"aaa%d"%(i))
	if len(names)>0:
		cmd.makeIdentity("aaa%d"%(i),a=1,t=1)
	# cmd.move(i*1.5)

# 重新布局
for i in range(1,255):
	cmd.move(i*1.5,0,0,"aaa%d"%(i))

# 克隆及并排摆放
import maya.cmds as cmds
for i in range(1,13):
    cmds.move(i*7, 0, 0, cmds.duplicate('pCube7'))