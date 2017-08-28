#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Mon Aug 28 11:02:58 2017

@author: ptutak
"""

def writeMicText(micSerial,text):
    micSerial.write([len(text)])
    micSerial.write(bytes(text,encoding='utf-8'))