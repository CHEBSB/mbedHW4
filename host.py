import matplotlib.pyplot as plt
import numpy as np
import serial
import time

# XBee setting
serdev = '/dev/ttyUSB0'
s = serial.Serial(serdev, 9600)

s.write("+++".encode())
char = s.read(2)
print("Enter AT mode.")
print(char.decode())

s.write("ATMY 0x120\r\n".encode())
char = s.read(3)
print("Set MY 0x120.")
print(char.decode())

s.write("ATDL 0x220\r\n".encode())
char = s.read(3)
print("Set DL 0x220.")
print(char.decode())

s.write("ATID 0x1\r\n".encode())
char = s.read(3)
print("Set PAN ID 0x1.")
print(char.decode())

s.write("ATWR\r\n".encode())
char = s.read(3)
print("Write config.")
print(char.decode())

s.write("ATMY\r\n".encode())
char = s.read(4)
print("MY :")
print(char.decode())

s.write("ATDL\r\n".encode())
char = s.read(4)
print("DL : ")
print(char.decode())

s.write("ATCN\r\n".encode())
char = s.read(3)
print("Exit AT mode.")
print(char.decode())

t = 1
print("start sending RPC")
# the first call give no reply, idk
s.write("/getTimes/run\r".encode())
while t <= 20:
    # send RPC to remote
    s.write("/getTimes/run\r".encode())
    time.sleep(1)
    t = t + 1

collect = np.arange(20)     # array size = 20 
for i in range(20) :
    collect[i] = s.read()
    num = int(collect[i].decode())
    for j in range(num) :
        line = s.readline().decode()
        # seperate the line into 4 parts
        Tp = line.split()   # split by whitespace
        x[i] = float(Tp[0])
        y[i] = float(Tp[1])
        z[i] = float(Tp[2])
        tilt[i] = int(Tp[3])
s.close()