import serial 
import os
import time
import glob

open_ports = glob.glob("/dev/ttyA*") # Finds all usb devices connected to the pi

print("Open ports: " + str(open_ports))

sort_port = ""
conveyor_port = ""

for port in open_ports: # Tests each connected device to identify the arduinos 
    test_ser = serial.Serial(port, 9600, timeout = 1)
    test_ser.reset_input_buffer()
    print("Testing port: " + port)
    time.sleep(2) # Important as the port inexplicably needs time to warm up to work, cannot be only 1 second, we tried
    
    test_ser.write(b"i\n") # Sends command to device to identify itself
    line = test_ser.readline().decode('utf-8').rstrip()
    if (line == 'Conveyor'): # The conveyor driving arduino will respond with the line "Conveyor"
        conveyor_port = port
    else: # And the sorting arduino will not respond at all
        sort_port = port
        

print("Port of sorting arduino: " + sort_port)
print("Port of conveyor arduino: " + conveyor_port)

sort_ser = serial.Serial(sort_port, 9600, timeout = 1)
sort_ser.reset_input_buffer()

conveyor_ser = serial.Serial(conveyor_port, 9600, timeout = 1)
conveyor_ser.reset_input_buffer()

filename = 'Patricks_pipe'
if os.path.exists(filename):
    os.remove(filename)
print("Pipe name: " + filename)
# This method provides a clear and easy way to send a data packet to a server via pipes
# It accepts @code{data}, which is always a string of comma-separated values
def update_server(data):
    with open(filename, "w") as pipe:
        try:
            print(data)
            pipe.write(data)
            pipe.write('\n')
        except BrokenPipeError as pipe_error:
            print("Broken pipe")
            os.remove(filename)

# Sends a 1 or 0 to the conveyor driving arduino based on what data is provided
def update_conveyor(data):
    if (data[3] == 1):
        conveyor_ser.write(b"1\n")
    else:
        conveyor_ser.write(b"0\n")


past_data = [0, 0, 0, 0, 0] # Initialize the array
update_conveyor(past_data);
while True:
    if sort_ser.in_waiting > 0: # Checks if there is any data to read
        line = sort_ser.readline().decode('utf-8') # Read data from arduino
        if (line.endswith('\n') and len(line) != 3): # Check if the package is not cropped AND the package has current sorting status
            data = [int(num) for num in line.strip().split(',')]
            if (data != past_data or data == [0, 0, 0, 0, 0]):
                update_server(",".join(map(str, data))) # Update the server on change (when the data is not the same as before)
                update_conveyor(data) # Update the conveyor driving arduino on change
            past_data = data.copy() # Store the current data so we can compare it to the new one later
        elif (line.endswith('\n') and len(line) == 3): # Check if the package is not cropped AND the package has error data
            update_server(line.strip()) # Send the error
        

                    
                

