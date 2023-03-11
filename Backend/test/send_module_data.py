import asyncio
import websockets
import json

NUMBER_OF_SEND = 10

laser_module_1_data = {
    "enabled": False,
    "module_number": 1,
    "time_stamp": 0,
    "laser_976nm": {
        "temperature": 22,
        "max_temperature": 50,
        "monitor_diode_current": 200,
        "laser_current": 200
    },
    "laser_1480nm": {
        "temperature": 0,
        "max_temperature": 0,
        "monitor_diode_current": 0,
        "laser_current": 0
    }
}

laser_module_2_data = {
    "enabled": False,
    "module_number": 1,
    "time_stamp": 0,
    "laser_976nm": {
        "temperature": 22,
        "max_temperature": 50,
        "monitor_diode_current": 200,
        "laser_current": 200
    },
    "laser_1480nm": {
        "temperature": 0,
        "max_temperature": 0,
        "monitor_diode_current": 0,
        "laser_current": 0
    }
}

laser_module_3_data = {
    "enabled": False,
    "module_number": 1,
    "time_stamp": 0,
    "laser_976nm": {
        "temperature": 22,
        "max_temperature": 50,
        "monitor_diode_current": 200,
        "laser_current": 200
    },
    "laser_1480nm": {
        "temperature": 0,
        "max_temperature": 0,
        "monitor_diode_current": 0,
        "laser_current": 0
    }
}

voa_data = {
    "time_stamp": 0,
    "position": 0
}


async def test1():
    async with websockets.connect('ws://localhost:8000/set_laser_module_data') as websocket:
        for i in range(0,NUMBER_OF_SEND):
            #response = await websocket.recv()
            #JSON_DATA["time_stamp"] = response
            await websocket.send(json.dumps(laser_module_1_data))
            await asyncio.sleep(1)
            
async def test2():
    async with websockets.connect('ws://localhost:8000/set_laser_module_data') as websocket:
        for i in range(0,NUMBER_OF_SEND):
            #response = await websocket.recv()
            #JSON_DATA["time_stamp"] = response
            await websocket.send(json.dumps(laser_module_2_data))
            await asyncio.sleep(1)
            
async def test3():
    async with websockets.connect('ws://localhost:8000/set_laser_module_data') as websocket:
        for i in range(0,NUMBER_OF_SEND):
            #response = await websocket.recv()
            #JSON_DATA["time_stamp"] = response
            await websocket.send(json.dumps(laser_module_3_data))
            await asyncio.sleep(1)
        
async def test4():
    async with websockets.connect('ws://localhost:8000/set_voa_data') as websocket:
        for i in range(0,NUMBER_OF_SEND):
            #response = await websocket.recv()
            #JSON_DATA["time_stamp"] = response
            await websocket.send(json.dumps(voa_data))
            await asyncio.sleep(1)

asyncio.get_event_loop().run_until_complete(test1())
asyncio.get_event_loop().run_until_complete(test2())
asyncio.get_event_loop().run_until_complete(test3())
asyncio.get_event_loop().run_until_complete(test4())
