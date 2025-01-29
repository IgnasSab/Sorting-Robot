import serial 
import os
import time
import glob

open_ports = glob.glob("/dev/ttyA*")

print(open_ports)


for port in open_ports:
    ser = serial.Serial(port, 9600, timeout = 1)
    ser.reset_input_buffer()
    print("Testing port: " + port)
    time.sleep(2)
    
    ser.write(b"i\n")
    line = ser.readline().decode('utf-8').rstrip()
    if (line == 'Conveyor'):
        print("found conveyor: " + port)
    
        
    else:
        print("not conveyor: " + port)
        open_ports.remove(port)
        break

ser = serial.Serial(open_ports[0], 9600, timeout = 1)
ser.reset_input_buffer()

time.sleep(2)
print(open_ports[0] + " is conveyor")

for i in range(100):
    
    if(i % 2 == 0):
        ser.write(b"0\n")
    else:
        ser.write(b"1\n")

    line = ser.readline().decode('utf-8').rstrip()
    print(line)
    
    time.sleep(2)
    