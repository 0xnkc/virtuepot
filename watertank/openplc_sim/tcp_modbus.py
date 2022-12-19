from pymodbus.client.sync import ModbusTcpClient
from datetime import datetime, time
import random
import time



#     // Create a tank
# tank = WaterTank {
#         level: 1000.0,
#         areal: 1000000.0,
#         height: 2000.0,
#         inflow: 20.0,
#         inflow_mean: 20.0,
#         inflow_stddev: 3.0,
#         max_inflow: 40.0,
#         outflow: 20.0,
#         max_outflow: 40.0,
#         set_level: 1000.0,
#     };

host = '192.168.0.26'  #ip of your modbus
port = 502
client = ModbusTcpClient(host, port)
while True:
    client.connect()
    data = random.randint(25,35)

# To Write values to multiple registers
    list = []
    for i in range(9):
         data = random.randint(10,40)
         list.append(data)

    # read_values = client.read_holding_registers(3, 0, count=2)
    # print(f"Read values: {read_values}")

    wr = client.write_registers(4,data)
    # write to multiple registers using list of data
    # wr = client.write_registers(20,list,unit=1)
    time.sleep(0.1)
    