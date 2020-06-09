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

while s.in_waiting > 0:
    line = s.readline().decode()
    print("Get: " + line + "\n")

"""
collect = np.arange(20)     # array size = 20 
t1 = np.arange(1, 21)       # time axis for collect
i = 0
while i < 20 and s.in_waiting > 0 :
    temp = s.readline().decode()
    TT = temp.split()
    if len(TT) == 1 and TT[0].isnumeric() == True:
        collect[i] = int(TT[0])
    else:
        print(temp + "is not int")
        continue
    if i == 0:      # initialize
        print("This is i = 0. Initialize")
        t = np.arange(0, 1, collect[0])
        x = np.arange(collect[0])
        y = np.arange(collect[0])
        z = np.arange(collect[0])
        tilt = np.arange(collect[0])
    else:           # concatenate
        print("This is i = " + i + ". Continue to concatenate")
        temp = np.arange(i, i+1, collect[i])
        t = np.concatenate((t, temp))
        tempx = np.arange(collect[i])
        tempy = np.arange(collect[i])
        tempz = np.arange(collect[i])
        tempTilt = np.arange(collect[i])
    j = 0
    while j < collect[i] :
        line = s.readline().decode()
        # seperate the line into 4 parts
        Tp = line.split()   # split by whitespace
        if len(Tp) == 4:
            if i == 0:
                x[j] = float(Tp[0])
                y[j] = float(Tp[1])
                z[j] = float(Tp[2])
                tilt[j] = int(Tp[3])
            else:
                tempx[j] = float(Tp[0])
                tempy[j] = float(Tp[1])
                tempz[j] = float(Tp[2])
                tempTilt[j] = int(Tp[3])
            j = j + 1
        else : 
            print("sth wrong with" + line)
            continue
    if i > 0:
        x = np.concatenate((x, tempx))
        y = np.concatenate((y, tempy))
        z = np.concatenate((z, tempz))
        tilt = np.concatenate((tilt, tempTilt))
    i = i + 1

fig, ax = plt.subplots(2, 1)
ax[0].plot(t, x, 'r')
ax[0].plot(t, y, 'y')
ax[0].plot(t, z, 'b')

ax[0].legend("xyz",loc='center left', bbox_to_anchor=(1, 0.5))
ax[0].set_xlabel('Time')
ax[0].set_ylabel('Acc Vector')
ax[1].stem(t1, tilt,'r') 

ax[1].set_xlabel('Time')
ax[1].set_ylabel('tilt')
plt.show()
"""
s.close()