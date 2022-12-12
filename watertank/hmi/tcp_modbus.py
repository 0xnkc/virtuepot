from pymodbus.client.sync import ModbusTcpClient
from datetime import datetime, time
import random
import time

host = '164.92.214.167'  #ip of your raspberry pi
port = 502
client = ModbusTcpClient(host, port)
while True:
    client.connect()
    data = random.randint(25,35)

# To Write values to multiple registers
    list = []
    for i in range(9):
         data = random.randint(25,55)
         list.append(data)

    #wr = client.write_registers(20,data)
    # write to multiple registers using list of data
    wr = client.write_registers(20,list,unit=1)
    time.sleep(0.1)
    print(data)