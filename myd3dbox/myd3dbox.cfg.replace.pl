import re

def repl(m):
	l,t,w,h=int(m.group(1)),int(m.group(2)),int(m.group(3)),int(m.group(4))
	return "_rect=%d,%d,%d,%d"%(l,t,l+w,t+h)

a=open('myd3dbox.cfg','r')
b=open('aaa.txt','w')
for l in a.readlines():
	b.write(re.sub("_rect=(\d+),(\d+),(\d+),(\d+)",repl,l))