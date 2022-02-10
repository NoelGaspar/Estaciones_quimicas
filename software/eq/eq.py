# -*- coding: utf-8 -*-
import time
import serial
import json
import threading
import sys, getopt
import re


class eq:
    def __init__(self,id,com):
        self.id  =  id
        self.debug 			= False
        self.port_name 		= com
        self.baud_rate 		= 9600
        self.arduino_com  	= None    #serial com
        self.arduino_thread = None
        self.connected    	= False  #false = not connected,  true = connected
        self.incomming_msg  = ""

        self.test_start_time   = 0
        self.test_actual_time  = 0
        self.test_elapsed_time = 0

        self.actual_value = 0.0
        self.val_mv = 0.0
        self.temp = 0.0

    def printStatus(self):
        print(f'{self.id},{self.port_name},{self.connected}')

    def getValue(self):
        return self.actual_value

    def getmV(self):
        return self.val_mv

    def getTemp(self):
        return self.temp

    def connectSerialPort(self):
        try:
            self.arduino_com = serial.Serial(self.port_name,self.baud_rate,timeout=0)
            self.connected = True
            self.arduino_thread = threading.Thread(target = self.arduinoRCV,)
            self.arduino_thread.deamon = True
            self.arduino_thread.start()
            self.arduino_thread.join(0.1)
            print(f"station  {self.id} connected to port {self.port_name}")
        except Exception as e:
            self.connected = False
            print(f"failed to connect {self.id} to port {self.port_name}")
            print(e)

    def disconnectSerialPort(self):
        try:
            self.arduino_com.close()
            self.connected = False
        except Exception as e:
            print("Not COM available")

    def sendMsg(self, cmd, arg):
        msg = json.dumps({"cmd":cmd,"arg":arg})
        print(f'sending msg :{msg}')
        if self.arduino_com.isOpen():
            self.arduino_com.write(msg.encode('ascii'))
            self.arduino_com.flush()

   
    def arduinoRCV(self):
        while self.connected:
            try:
                if self.arduino_com.isOpen():
                    self.incomming_msg += self.arduino_com.readline().decode("utf-8")
                    if "}" in self.incomming_msg:
                    #if self.incomming_msg != "":
                        data =  json.loads(self.incomming_msg)
                        self.incomming_msg =""
                        print(f"data recived : {data}")
                        self.upDateValues(data)
                    else:
                        pass
            except  Exception as e:
                pass
                #print (e)

    def upDateValues(self,d):
        self.val_mv = d["mv"]
        self.actual_value = d["val"]
        self.temp = d["temp"]
        #print(f"new values updateds {self.val_mv},{self.actual_value},{self.temp}")

    def stopRcvr(self):
        print("Closing")
        self.connected = False
        self.arduino_com.close()

if __name__ == "__main__":
    COM = "/dev/ttyUSB0"

    try:
        opts,args = getopt.getopt(sys.argv[1:],"hp:i:")
    except:
        print("Comando no valido pruebe -h por ayuda")
        sys.exit(2)

    for opt ,arg in opts:
        if opt == '-h':
            print("main.py -h for help -p <com> for port to connect")
            sys.exit()
        elif opt =='-p':
            COM = arg
            print(f"trying to connect to port {COM}")
        elif opt == '-i':
            print("option i")

    nodo = eq("b1",COM)
    nodo.connectSerialPort()
    time.sleep(30)
    nodo.stopRcvr()
