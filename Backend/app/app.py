from fastapi import FastAPI, Request, WebSocket, WebSocketDisconnect
from pydantic import BaseModel
from fastapi.templating import Jinja2Templates
from fastapi.staticfiles import StaticFiles

#------------------------------------------------------------------------------
# Models
#------------------------------------------------------------------------------
class laser_data(BaseModel):
    temp : int
    max_temp : int
    monitor_diode_current : int
    laser_current : int
    
class laser_module(BaseModel):
    enabled: bool
    time_stamp: int
    laser_976: laser_data
    laser_1480: laser_data
    
class voa_data(BaseModel):
    time_stamp: int
    position: int
    
class laser_enable(BaseModel):
    status: bool

#------------------------------------------------------------------------------
# Public -> database later
#------------------------------------------------------------------------------
laser_module_1 = laser_module(enabled=False, time_stamp=0, 
                              laser_976={"temp": 0, "max_temp": 0, "monitor_diode_current": 0, "laser_current": 0}, 
                              laser_1480={"temp": 0, "max_temp": 0, "monitor_diode_current": 0, "laser_current": 0}
                              )

laser_module_2 = laser_module(enabled=False, time_stamp=0, 
                              laser_976={"temp": 0, "max_temp": 0, "monitor_diode_current": 0, "laser_current": 0}, 
                              laser_1480={"temp": 0, "max_temp": 0, "monitor_diode_current": 0, "laser_current": 0}
                              )

laser_module_3 = laser_module(enabled=False, time_stamp=0, 
                              laser_976={"temp": 0, "max_temp": 0, "monitor_diode_current": 0, "laser_current": 0}, 
                              laser_1480={"temp": 0, "max_temp": 0, "monitor_diode_current": 0, "laser_current": 0}
                              )
#------------------------------------------------------------------------------
# Instances
# FastAPI, jinja2
#------------------------------------------------------------------------------
app = FastAPI()
templates = Jinja2Templates(directory="Frontend/public") # Future use
app.mount("/static", StaticFiles(directory="Frontend/static"), name="static")
app.mount("/public", StaticFiles(directory="Frontend/public"), name="public")
app.mount("/img", StaticFiles(directory="Frontend/public/assets/img"), name="img")
app.mount("/npm", StaticFiles(directory="Frontend/node_modules"), name="npm")

@app.on_event("startup")
async def startup():
    print("Server starting...")

@app.on_event("shutdown")
async def shutdown():
    print("Server closing...")

#------------------------------------------------------------------------------
# HTTP
#------------------------------------------------------------------------------
@app.get("/")
async def root(request: Request):
    return templates.TemplateResponse("index.html", {"request": request})

@app.get("/get_laser1_data")
async def get_laser1_data() -> laser_module:
    return laser_module_1
            
@app.get("/get_laser2_data")
async def get_laser2_data() -> laser_module:
    return laser_module_2

@app.get("/get_laser3_data")
async def get_laser3_data() -> laser_module:
    return laser_module_3

@app.get("/get_voa_data")
async def get_voa_data() -> voa_data:
    return 

@app.post("/set_laser1_data")
async def set_laser1_data(laser1_data: laser_enable):
    if (laser1_data.status):
        print("Enabling laser1 module")
        laser_module_1.enabled = True
    else:
        print("Disabling laser1 module")
        laser_module_1.enabled = False
    print("Laser state:" + str(laser_module_1.enabled))
    return

@app.post("/set_laser2_data")
async def set_laser2_data(laser2_data: laser_enable):
    if (laser2_data.status):
        print("Enabling laser2 module")
        laser_module_2.enabled = True
    else:
        print("Disabling laser2 module")
        laser_module_2.enabled = False
    print("Laser state:" + str(laser_module_2.enabled))
    return

@app.post("/set_laser3_data")
async def set_laser3_data(laser3_data: laser_enable):
    if (laser3_data.status):
        print("Enabling laser3 module")
        laser_module_3.enabled = True
    else:
        print("Disabling laser3 module")
        laser_module_3.enabled = False
    print("Laser state:" + str(laser_module_3.enabled))
    return
#------------------------------------------------------------------------------
# Websocket
#------------------------------------------------------------------------------
@app.websocket("/laser1_data")
async def websocket_laser1_data(websocket: WebSocket):
    await websocket.accept()
    try:
        while True:
            data = await websocket.receive_text()
            await websocket.send_text(data)
            print("Laser 1: " + data)
            websocket.close()
            # add 
    except WebSocketDisconnect:
        await websocket.close()
        
@app.websocket("/laser2_data")
async def websocket_laser2_data(websocket: WebSocket):
    await websocket.accept()
    try:
        while True:
            data = await websocket.receive_text()
            await websocket.send_text(data)
            await websocket.send_text(data)
            print("Laser 2" + data)
            # add 
    except WebSocketDisconnect:
        await websocket.close()
           
@app.websocket("/laser3_data")
async def websocket_laser3_data(websocket: WebSocket):
    await websocket.accept()
    try:
        while True:
            data = await websocket.receive_text()
            await websocket.send_text(data)
            print("Laser 3" + data)
            # add 
    except WebSocketDisconnect:
        await websocket.close()
        
@app.websocket("/voa_data")
async def websocket_voa_data(websocket: WebSocket):
    await websocket.accept()
    try:
        while True:
            data = await websocket.receive_text()
            await websocket.send_text(data)
            print("VOA" + data)
            # add 
    except WebSocketDisconnect:
        await websocket.close()