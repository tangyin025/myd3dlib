import re

def repl(m):
	h,l,t,r,b=m.group(1),int(m.group(2)),int(m.group(3)),int(m.group(4)),int(m.group(5))
	return "%s=%d,%d,%d,%d"%(h,l,t,r-1,b-1)

a=open('myd3dbox.cfg','r')
b=open('aaa.txt','w')
for l in a.readlines():
	b.write(re.sub("(checkbox_\w+_rect)=(\d+),(\d+),(\d+),(\d+)",repl,l))