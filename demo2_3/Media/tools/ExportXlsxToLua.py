#!/usr/bin/python
# -*- coding: UTF-8 -*-
import os
import re
from openpyxl import load_workbook

files=os.listdir("../tables_xlsx")
for name in files:
    m=re.match("(.+)\.xlsx",name)
    if m:
        module=m.group(1)
        o=open("../tables/"+module+".lua","w",encoding='utf-8')
        o.write("module(\""+module+"\", package.seeall)\n")
        o.write("data={\n")
        wb=load_workbook("../tables_xlsx/"+module+".xlsx")
        ws=wb.active
        for row in ws.iter_rows(min_row=2,values_only=True):
            o.write("{")
            i=0
            for value in row:
                if i>0:
                    o.write(", ")
                i=i+1
                if type(value)==int:
                    o.write(str(value))
                elif type(value)==str:
                    o.write('"'+value+'"')
            o.write("},\n")
        o.write("}\n")
        o.close()
        print("../tables/"+module+".lua")