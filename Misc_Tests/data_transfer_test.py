import serial
import time


ser = serial.Serial('/dev/ttyACM0', 9600, timeout=1)
ser.reset_input_buffer()

time.sleep(2)
for i in range(100): 
    print(i)
    ser.write(b"i\n")
    
    line = ser.readline().decode('utf-8').rstrip()
    print(line)
        
        