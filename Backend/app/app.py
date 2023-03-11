from fastapi import FastAPI, Request, WebSocket, WebSocketDisconnect
from pydantic import BaseModel
from fastapi.templating import Jinja2Templates
from fastapi.staticfiles import StaticFiles
from time import time
import json

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
    module_number: int
    time_stamp: float
    laser_976: laser_data
    laser_1480: laser_data
    
class voa_data(BaseModel):
    time_stamp: int
    position: int

#------------------------------------------------------------------------------
# Public -> database later
#------------------------------------------------------------------------------
laser_module_1 = laser_module(enabled=False, module_number=1, time_stamp=0, 
                              laser_976={"temp": 0, "max_temp": 0, "monitor_diode_current": 0, "laser_current": 0}, 
                              laser_1480={"temp": 0, "max_temp": 0, "monitor_diode_current": 0, "laser_current": 0}
                              )

laser_module_2 = laser_module(enabled=False, module_number=2, time_stamp=0, 
                              laser_976={"temp": 0, "max_temp": 0, "monitor_diode_current": 0, "laser_current": 0}, 
                              laser_1480={"temp": 0, "max_temp": 0, "monitor_diode_current": 0, "laser_current": 0}
                              )

laser_module_3 = laser_module(enabled=False, module_number=3, time_stamp=0, 
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

@app.get("/get_voa_data")
async def get_voa_data() -> voa_data:
    return 

@app.get("/get_laser_module_data/{id}")
async def get_laser_module_data(id: int):
    match id:
        case 1:
            return laser_module_1
        case 2:
            return laser_module_2
        case 3:
            return laser_module_3

@app.post("/enable_disable_laser_module/{id}")
async def enable_disable_laser_module(id : int, state: bool):
    print(f"Laser module {id} state change to {state}")
    match id:
        case 1:
            laser_module_1.enabled = state
        case 2:
            laser_module_2.enabled = state
        case 3:
            laser_module_3.enabled = state
    # TODO send state to the hardware boards
    return
#------------------------------------------------------------------------------
# Websocket
#------------------------------------------------------------------------------
@app.websocket("/set_laser_module_data")
async def websocket_laser1_data(websocket: WebSocket):
    await websocket.accept()
    try:
        while True:
            data = await websocket.receive_text()
            json_data = json.loads(data)
            json_data["time_stamp"] = time()
            match json_data["module_number"]:
                case 1:
                    laser_module_1 = json_data
                case 2:
                    laser_module_2 = json_data
                case 3:
                    laser_module_3 = json_data
            # add 
    except WebSocketDisconnect:
        return
        
@app.websocket("/set_voa_data")
async def websocket_voa_data(websocket: WebSocket):
    await websocket.accept()
    try:
        while True:
            data = await websocket.receive_text()
            json_data = json.loads(data)
            json_data["time_stamp"] = time()
            # add 
    except WebSocketDisconnect:
        return