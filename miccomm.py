#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Mon Aug 28 11:02:58 2017

@author: ptutak
"""
import serial

def writeMicText(myMicSerial,text):
    myMicSerial.write([len(text)])
    myMicSerial.write(bytes(text,encoding='utf-8'))

class MySerial(serial.Serial):
    def writeText(self,text):
        self.write([len(text)+1])
        self.write(bytes(text+'\0',encoding='utf-8'))
    def printText(self,text):
        self.write([len(text)+2])
        self.write(bytes(text+'\n\0',encoding='utf-8'))
