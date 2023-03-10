from fastapi import FastAPI, Request, WebSocket, WebSocketDisconnect
from pydantic import BaseModel
from fastapi.templating import Jinja2Templates
from fastapi.staticfiles import StaticFiles

#------------------------------------------------------------------------------
# Models
#------------------------------------------------------------------------------
class laser_data(BaseModel):
    enabled : bool
    temp : int
    max_temp : int
    monitor_diode : int
    laser_current : int
    
class voa_data(BaseModel):
    position: int
    
class laser_enable(BaseModel):
    status: bool
    
    
#------------------------------------------------------------------------------
# Public -> database later
#------------------------------------------------------------------------------
laser1_976_instance = laser_data(enabled=False, temp=0, max_temp=50, monitor_diode=0, laser_current=0)
laser1_1480_instance = laser_data(enabled=False, temp=0, max_temp=50, monitor_diode=0, laser_current=0)

laser2_976_instance = laser_data(enabled=False, temp=0, max_temp=50, monitor_diode=0, laser_current=0)
laser2_1480_instance = laser_data(enabled=False, temp=0, max_temp=50, monitor_diode=0, laser_current=0)

laser3_976_instance = laser_data(enabled=False, temp=0, max_temp=50, monitor_diode=0, laser_current=0)
laser3_1480_instance = laser_data(enabled=False, temp=0, max_temp=50, monitor_diode=0, laser_current=0)


voa_data_instance = voa_data(position=0)


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
async def get_laser1_data() -> list[laser_data]:
    return [laser1_976_instance, laser1_1480_instance]
            
@app.get("/get_laser2_data")
async def get_laser2_data() -> list[laser_data]:
    return [laser2_976_instance, laser2_1480_instance]

@app.get("/get_laser3_data")
async def get_laser3_data() -> list[laser_data]:
    return [laser3_976_instance, laser3_1480_instance]

@app.get("/get_voa_data")
async def get_voa_data() -> voa_data:
    return voa_data_instance

@app.post("/set_laser1_data")
async def get_voa_data(laser1_data: laser_enable):
    if (laser1_data.status):
        print("Enabling laser1 module")
    else:
        print("Disabling laser1 module")
    return

@app.post("/set_laser2_data")
async def get_voa_data(laser2_data: laser_enable):
    if (laser2_data.status):
        print("Enabling laser2 module")
    else:
        print("Disabling laser2 module")
    return

@app.post("/set_laser3_data")
async def get_voa_data(laser3_data: laser_enable):
    if (laser3_data.status):
        print("Enabling laser3 module")
    else:
        print("Disabling laser3 module")
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