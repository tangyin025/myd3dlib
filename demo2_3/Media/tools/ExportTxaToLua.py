#!/usr/bin/python
# -*- coding: UTF-8 -*-
import xml.sax
from openpyxl import Workbook

class TxaHandler(xml.sax.ContentHandler):
    def __init__(self,fname):
        self.l=[]
        self.wb=Workbook()
        self.ws=self.wb.active

    # 元素开始事件处理
    def startElement(self,tag,attributes):
        self.l.append(tag)

    # 元素结束事件处理
    def endElement(self,tag):
        assert(tag==self.l[-1])
        self.l.pop(-1)
        if tag=="array_item":
            self.ws.append([self.uv_start_x,self.uv_start_y,self.uv_end_x,self.uv_end_y])

    # 内容事件处理
    def characters(self, content):
        if self.l[-1]=="name":
            self.name=content
        elif self.l[-1]=="x":
            if self.l[-2]=="uv_start":
                self.uv_start_x=content
            elif self.l[-2]=="uv_end":
                self.uv_end_x=content
        elif self.l[-1]=="y":
            if self.l[-2]=="uv_start":
                self.uv_start_y=content
            elif self.l[-2]=="uv_end":
                self.uv_end_y=content

parser=xml.sax.make_parser()
model="icon_Equip1"
handler=TxaHandler(model)
parser.setContentHandler(handler)
parser.parse("../texture/"+model+".txa")
handler.wb.save(model+".xlsx")