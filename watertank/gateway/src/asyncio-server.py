#!/usr/bin/env python

import asyncio
from pymodbus.server.async_io import StartTcpServer

from pymodbus.device import ModbusDeviceIdentification
from pymodbus.datastore import ModbusSequentialDataBlock, ModbusSparseDataBlock
from pymodbus.datastore import ModbusSlaveContext, ModbusServerContext

#from pymodbus.transaction import ModbusRtuFramer, ModbusBinaryFramer

# configure the service logging
import logging
FORMAT = ('%(asctime)-15s %(threadName)-15s'
          ' %(levelname)-8s %(module)-15s:%(lineno)-8s %(message)s')
logging.basicConfig(format=FORMAT)
log = logging.getLogger()
log.setLevel(logging.CRITICAL)

import socket
import json

def talk_to_simulation(read_values, sock):
    # find out how to put this into another file
    #sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    #sock.connect(("127.0.0.1", 9977))

    payload = {
        "outflow": read_values[0], 
        "setpoint": read_values[1],  
    }
    payload_encoded = json.dumps(payload, separators=(',', ':'))    
    header = {
        "len": len(payload_encoded),
        "msg_type": "get_info",
    }
    header_encoded = json.dumps(header, separators=(',', ':'))
    header_len = len(header_encoded)

    sock.send(bytes([header_len]))
    sock.send(bytes(header_encoded, 'utf-8'))
    sock.send(bytes(payload_encoded, 'utf-8'))

    response_data = sock.recv(1024)

    response_data = json.loads(response_data.decode('utf-8').strip())

    for key in response_data:
        log.debug(f"{key}: {response_data[key]}")

    #sock.close()
    return response_data

# a worker for talking to the simulation
async def updating_writer(a, time, sock):
    while True:
        await asyncio.sleep(time)

        log.debug("updating the context")
        context = a[0]
        register = 4
        #slave_id = 0x00
        address = 0x01

        ## read some values 
        #read_values = context[slave_id].getValues(3, 0, count=2)
        read_values = context.getValues(3, 0, count=2)
        log.debug(f"Read values: {read_values}")

        # Update the values some way or another, for example from another server
        response_data = talk_to_simulation(read_values, sock)

        # Write the values back.
        values = [response_data["tank_level"], response_data["tank_inflow"]]
        log.debug(f"New values: {values}")
        context.setValues(register, address, values)

def run_server():

    # Initialize data store
    store = ModbusSlaveContext(
        di=ModbusSequentialDataBlock(0, [17]*100),
        co=ModbusSequentialDataBlock(0, [17]*100),
        hr=ModbusSequentialDataBlock(0, [32768]*100),
        ir=ModbusSequentialDataBlock(0, [17]*100))

    context = ModbusServerContext(slaves=store, single=True)

    # Server information
    identity = ModbusDeviceIdentification()
    identity.VendorName = 'Pymodbus'
    identity.ProductCode = 'PM'
    identity.VendorUrl = 'http://github.com/riptideio/pymodbus/'
    identity.ProductName = 'Pymodbus Server'
    identity.ModelName = 'Pymodbus Server'
    identity.MajorMinorRevision = '2.3.0'

    # run the server you want after creating tasks

    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect(("simulation", 9977))
    log.debug(f"Connected to: {sock}")

    loop = asyncio.get_event_loop()
    loop.create_task(start_servers(context, identity, loop))
    loop.create_task(updating_writer(context, 0.3, sock))
    loop.run_forever()
    sock.close()
    
async def start_servers(context, identity, loop):
        server = await StartTcpServer(context, identity=identity, address=("0.0.0.0", 5020),
                                   allow_reuse_address=True, defer_start=True, loop=loop)
        await server.serve_forever()

if __name__ == "__main__":
    run_server()

