import time
import serial
import random

ser = serial.Serial(port='/dev/ttyS0', baudrate = 115200, parity = serial.PARITY_NONE, stopbits = serial.STOPBITS_ONE, bytesize = serial.EIGHTBITS, timeout = 1)

print(ser)

num_data = 8
data_types = ["micros()", "revs", "ECT", "IAT", "MAP_INST", "MAP_AVG", "TPS", "RPM"]
print(data_types)

data_list = [""] * num_data
tmp_data_list = [""] * num_data
iterator = 0;

while True:
#     if (ser.is_open == False):
#         iterator = 0
#         tmp_data_list = [""] * num_data
#         continue
    data = ser.read();
    data = data.decode('utf-8');
    if (data == '\n'):
        data_list = tmp_data_list
        print(data_list, end="\r")
        print("\r", end="")
        tmp_data_list = [""] * num_data
        iterator = 0
    elif (data == ':'):
        iterator = iterator + 1
    else:
        tmp_data_list[iterator] += data

    
    