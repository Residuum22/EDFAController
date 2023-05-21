from fastapi import FastAPI, Request, WebSocket, WebSocketDisconnect
from pydantic import BaseModel
from fastapi.templating import Jinja2Templates
from fastapi.staticfiles import StaticFiles
from fastapi_mqtt.config import MQTTConfig
from fastapi_mqtt.fastmqtt import FastMQTT
from time import time
import json
import asyncio
from enum import Enum

# ------------------------------------------------------------------------------
# Models
# ------------------------------------------------------------------------------

# Laser module data report
class laser_module_data_report(BaseModel):
    laser_id: str
    temperature: int
    monitor_diode_current: int
    laser_current: int
    
class laser_module_data_report_module_976(BaseModel):
    module_number: int
    time_stamp: float
    laser_976_1: laser_module_data_report
    laser_976_2: laser_module_data_report
    
class laser_module_data_report_module_1480(BaseModel):
    module_number: int
    time_stamp: float
    laser_1480_1: laser_module_data_report
    laser_1480_2: laser_module_data_report
    
# Laser module data set
class laser_module_data_set(BaseModel):
    laser_id: str
    desired_temperature: int
    desired_monitor_diode_current: int

class laser_module_data_set_module_976(BaseModel):
    enabled: bool
    report_interval: int
    laser_976_1: laser_module_data_set
    laser_976_2: laser_module_data_set
    
class laser_module_data_set_module_1480(BaseModel):
    enabled: bool
    report_interval: int
    laser_1480_1: laser_module_data_set
    laser_1480_2: laser_module_data_set
    
# VOA module data report
class voa_module_data_set(BaseModel):
    attenuation: int


# ------------------------------------------------------------------------------
# Public -> database later
# ------------------------------------------------------------------------------
laser_module_1_report = laser_module_data_report_module_976(
                        module_number=1, time_stamp=0,
                        laser_976_1={"laser_id": "LD1","temperature": 0, "monitor_diode_current": 0, "laser_current": 0},
                        laser_976_2={"laser_id": "LD6","temperature": 0, "monitor_diode_current": 0, "laser_current": 0}
                        )

laser_module_2_report = laser_module_data_report_module_1480(
                        module_number=2, time_stamp=0,
                        laser_1480_1={"laser_id": "LD2","temperature": 0, "monitor_diode_current": 0, "laser_current": 0},
                        laser_1480_2={"laser_id": "LD4","temperature": 0, "monitor_diode_current": 0, "laser_current": 0}
                        )

laser_module_3_report = laser_module_data_report_module_1480(
                        module_number=3, time_stamp=0,
                        laser_1480_1={"laser_id": "LD3","temperature": 0, "monitor_diode_current": 0, "laser_current": 0},
                        laser_1480_2={"laser_id": "LD4","temperature": 0, "monitor_diode_current": 0, "laser_current": 0}
                        )

laser_module_1_set = laser_module_data_set_module_976(
                        enabled=False, report_interval=1, 
                        laser_976_1={"laser_id": "LD1","desired_temperature": 50, "desired_monitor_diode_current": 0}, 
                        laser_976_2={"laser_id": "LD6","desired_temperature": 50, "desired_monitor_diode_current": 0}
                        )

laser_module_2_set = laser_module_data_set_module_1480(
                        enabled=False, report_interval=1, 
                        laser_1480_1={"laser_id": "LD2","desired_temperature": 50, "desired_monitor_diode_current": 0}, 
                        laser_1480_2={"laser_id": "LD4","desired_temperature": 50, "desired_monitor_diode_current": 0}
                        )

laser_module_3_set = laser_module_data_set_module_1480(
                        enabled=False, report_interval=1, 
                        laser_1480_1={"laser_id": "LD3","desired_temperature": 50, "desired_monitor_diode_current": 0}, 
                        laser_1480_2={"laser_id": "LD6","desired_temperature": 50, "desired_monitor_diode_current": 0}
                        )

voa_module_set = voa_module_data_set(attenuation=0)

async def websocket_send_data(ws: WebSocket, data: json):
    try:
        if ws is not None:
            print(f"Sending data to: {ws} with data {data}")
            await asyncio.wait_for(ws.send_json(data), timeout=1.0)
    except Exception:
        return
# ------------------------------------------------------------------------------
# Instances
# FastAPI, jinja2
# ------------------------------------------------------------------------------
app = FastAPI()
templates = Jinja2Templates(directory="Frontend/public")  # Future use
app.mount("/static", StaticFiles(directory="Frontend/static"), name="static")
app.mount("/public", StaticFiles(directory="Frontend/public"), name="public")
app.mount("/img", StaticFiles(directory="Frontend/public/assets/img"), name="img")


mqtt_config = MQTTConfig()
mqtt = FastMQTT(config=mqtt_config)
mqtt.init_app(app)

@app.on_event("startup")
async def startup():
    print("Server starting...")


@app.on_event("shutdown")
async def shutdown():
    print("Server closing...")

# ------------------------------------------------------------------------------
# Instances end
# ------------------------------------------------------------------------------

# ------------------------------------------------------------------------------
# HTTP Start
# ------------------------------------------------------------------------------
@app.get("/")
async def root(request: Request):
    laser_modules = {1,2,3}
    return templates.TemplateResponse("index.html", {"request": request, "laser_modules": laser_modules})

@app.get("/help")
async def root(request: Request):
    return templates.TemplateResponse("edfa.html", {"request": request})

@app.get("/get_laser_module_data/{id}")
async def get_laser_module_data(id: int):
    match id:
        case 1:
            return laser_module_1_report
        case 2:
            return laser_module_2_report
        case 3:
            return laser_module_3_report


@app.post("/enable_disable_laser_module/{id}")
async def enable_disable_laser_module(id: int, state: bool):
    print(f"Laser module {id} state change to {state}")
    match id:
        case 1:
            laser_module_1_set.enabled = state
            await websocket_send_data('/laser_module/1/enable', laser_module_1_set.json())
        case 2:
            laser_module_2_set.enabled = state
            await websocket_send_data('/laser_module/2/enable',
                                laser_module_2_set.json())
        case 3:
            laser_module_3_set.enabled = state
            await websocket_send_data('/laser_module/3/enable',
                                laser_module_3_set.json())


@app.post("/set_laser_module_desired_temperature/{id}")
async def set_laser_module_desired_temperature(id: int, desired_temperature: int):
    if desired_temperature not in range(0, 71):
        return
    match id:
        case 1:
            laser_module_1_set.laser_976.desired_temperature = desired_temperature
            laser_module_1_set.laser_976.desired_temperature = desired_temperature
            mqtt.publish('/laser_module/1/set_temp', json.dumps(laser_module_1_set))
        case 2:
            laser_module_2_set.laser_1480.desired_temperature = desired_temperature
            laser_module_2_set.laser_1480.desired_temperature = desired_temperature
            mqtt.publish('/laser_module/2/set_temp', json.dumps(laser_module_2_set))
        case 3:
            laser_module_3_set.laser_1480.desired_temperature = desired_temperature
            laser_module_3_set.laser_1480.desired_temperature = desired_temperature
            mqtt.publish('/laser_module/3/set_temp', json.dumps(laser_module_3_set))
      
            
@app.post("/set_voa_module_attenuation")
async def set_voa_module_attenuation(attenuation: int):
    voa_module_set.attenuation = attenuation
    mqtt.publish("/voa_attenuation", json.dumps(voa_module_set))
# ------------------------------------------------------------------------------
# HTTP End
# ------------------------------------------------------------------------------


# ------------------------------------------------------------------------------
# Websocket Start
# ------------------------------------------------------------------------------
# Websocket for frontend
@app.websocket("/frontend_get_module_data")
async def websocket_frontend_get_all_module_data(websocket: WebSocket):
    await websocket.accept()
    global frontend_get_websocket
    frontend_get_websocket = websocket
    try:
        while True:
            # Do nothing just keep alive the websocket connection with the webserver.
            _ = await websocket.receive_text()

    except WebSocketDisconnect:
        return

    finally:
        frontend_get_websocket = None
        
# ------------------------------------------------------------------------------
# Websocket End
# ------------------------------------------------------------------------------


# ------------------------------------------------------------------------------
# MQTT Start
# ------------------------------------------------------------------------------
@mqtt.on_connect()
def connect(client, flags, rc, properties):
    print("Connected: ", client, flags, rc, properties)

@mqtt.on_message()
async def message(client, topic, payload, qos, properties):
    # print("Received message: ",topic, payload.decode(), qos, properties)
    return 0

@mqtt.subscribe("/report_laser_module_data")
async def message_to_topic(client, topic, payload, qos, properties):
    print("Received message to specific topic: ", topic, payload.decode(), qos, properties)
    json_data = payload.decode()
    json_data["time_stamp"] = time.time()
    match json_data["module_number"]:
        case 1:
            laser_module_1_report = json_data
            await websocket_send_data(frontend_get_websocket, json.dumps(laser_module_1_report))
        case 2:
            laser_module_2_report = json_data
            await websocket_send_data(frontend_get_websocket, json.dumps(laser_module_2_report))
        case 3:
            laser_module_3_report = json_data
            await websocket_send_data(frontend_get_websocket, json.dumps(laser_module_3_report))

@mqtt.on_disconnect()
def disconnect(client, packet, exc=None):
    print("Disconnected")

@mqtt.on_subscribe()
def subscribe(client, mid, qos, properties):
    # print("subscribed", client, mid, qos, properties)
    return
