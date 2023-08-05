from pymodbus.client.sync import ModbusTcpClient
from datetime import datetime, time
import random
import time


host = '192.168.0.26'  #ip of your modbus
port = 502
client = ModbusTcpClient(host, port)
while True:
    client.connect()
    data = random.randint(25,35)

# To Write values to multiple registers
    list = []
    for i in range(2):
         data = random.randint(10,40)
         list.append(data)

    # read_values = client.read_holding_registers(3, 0, count=2)
    # print(f"Read values: {read_values}")

    wr = client.write_registers(101,list,unit=1)
    # write to multiple registers using list of data
    # wr = client.write_registers(20,list,unit=1)
    time.sleep(0.1)
    