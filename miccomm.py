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

class mySerial(serial.Serial):
    def writeText(self,text):
        self.write([len(text)])
        self.write(bytes(text,encoding='utf-8'))
