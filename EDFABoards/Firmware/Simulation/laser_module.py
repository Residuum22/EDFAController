import asyncio
import websockets
import json
import random
import time

NUMBER_OF_SEND = 10

laser_module_1_data = {
    "module_number": 1,
    "time_stamp": 0,
    "laser_976nm": {
        "temperature": 0,
        "monitor_diode_current": 0,
        "laser_current": 0
    },
    "laser_1480nm": {
        "temperature": 0,
        "monitor_diode_current": 0,
        "laser_current": 0
    }
}

laser_module_2_data = {
    "module_number": 2,
    "time_stamp": 0,
    "laser_976nm": {
        "temperature": 0,
        "monitor_diode_current": 0,
        "laser_current": 0
    },
    "laser_1480nm": {
        "temperature": 0,
        "monitor_diode_current": 0,
        "laser_current": 0
    }
}

laser_module_3_data = {
    "module_number": 3,
    "time_stamp": 0,
    "laser_976nm": {
        "temperature": 0,
        "monitor_diode_current": 0,
        "laser_current": 0
    },
    "laser_1480nm": {
        "temperature": 0,
        "monitor_diode_current": 0,
        "laser_current": 0
    }
}


async def test1():
    async with websockets.connect('ws://localhost:8000/report_laser_module_data') as websocket:
        for i in range(0,NUMBER_OF_SEND):
            random.seed(time.time())
            
            laser_module_1_data["laser_976nm"]["temperature"] = random.randrange(30, 60, 1)
            laser_module_1_data["laser_1480nm"]["temperature"] = random.randrange(30, 60, 1)
            
            await websocket.send(json.dumps(laser_module_1_data))
            await asyncio.sleep(1)
        
            
async def test2():
    async with websockets.connect('ws://localhost:8000/report_laser_module_data') as websocket:
        for i in range(0,NUMBER_OF_SEND):
            await websocket.send(json.dumps(laser_module_2_data))
            await asyncio.sleep(1)
            
async def test3():
    async with websockets.connect('ws://localhost:8000/report_laser_module_data') as websocket:
        for i in range(0,NUMBER_OF_SEND):
            await websocket.send(json.dumps(laser_module_3_data))
            await asyncio.sleep(1)

async def main():
    task1 = asyncio.create_task(test1())
    #task2 = asyncio.create_task(test2())
    #task3 = asyncio.create_task(test3())
    
    await task1
    #await task2
    #await task3

asyncio.run(main())    
