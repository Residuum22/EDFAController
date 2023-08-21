from fastapi import FastAPI, Request, WebSocket, WebSocketDisconnect
from pydantic import BaseModel
from fastapi.templating import Jinja2Templates
from fastapi.staticfiles import StaticFiles
from fastapi_mqtt.config import MQTTConfig
from fastapi_mqtt.fastmqtt import FastMQTT
from time import time
import json
import asyncio
from fastapi.encoders import jsonable_encoder

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
    enabled: bool
    desired_temperature: int
    desired_monitor_diode_current: int

class laser_module_data_set_module_976(BaseModel):
    report_interval: int
    laser_976_1: laser_module_data_set
    laser_976_2: laser_module_data_set
    
class laser_module_data_set_module_1480(BaseModel):
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
                        laser_1480_2={"laser_id": "LD3","temperature": 0, "monitor_diode_current": 0, "laser_current": 0}
                        )

laser_module_3_report = laser_module_data_report_module_1480(
                        module_number=3, time_stamp=0,
                        laser_1480_1={"laser_id": "LD4","temperature": 0, "monitor_diode_current": 0, "laser_current": 0},
                        laser_1480_2={"laser_id": "LD5","temperature": 0, "monitor_diode_current": 0, "laser_current": 0}
                        )

laser_module_1_set = laser_module_data_set_module_976(
                        report_interval=1, 
                        laser_976_1={"laser_id": "LD1", "enabled":False, "desired_temperature": 50, "desired_monitor_diode_current": 0}, 
                        laser_976_2={"laser_id": "LD6", "enabled":False, "desired_temperature": 50, "desired_monitor_diode_current": 0}
                        )

laser_module_2_set = laser_module_data_set_module_1480(
                        report_interval=1, 
                        laser_1480_1={"laser_id": "LD2", "enabled":False, "desired_temperature": 50, "desired_monitor_diode_current": 0}, 
                        laser_1480_2={"laser_id": "LD3", "enabled":False, "desired_temperature": 50, "desired_monitor_diode_current": 0}
                        )

laser_module_3_set = laser_module_data_set_module_1480(
                        report_interval=1, 
                        laser_1480_1={"laser_id": "LD4", "enabled":False, "desired_temperature": 50, "desired_monitor_diode_current": 0}, 
                        laser_1480_2={"laser_id": "LD5", "enabled":False, "desired_temperature": 50, "desired_monitor_diode_current": 0}
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

def send_laser_module_data(id: int):
    mqtt.publish(f'/laser_module_set/{id}', jsonable_encoder(laser_module_2_set))

@app.get("/")
async def root(request: Request):
    laser_module_instances = [
        {'laser_module_id': '1', 'laser_id': ['LD1', 'LD6']},
        {'laser_module_id': '2', 'laser_id': ['LD2', 'LD3']},
        {'laser_module_id': '3', 'laser_id': ['LD4', 'LD5']}
        ]
    return templates.TemplateResponse("index.html", {"request": request, 'laser_module_instances': laser_module_instances})

@app.get("/help")
async def root(request: Request):
    return templates.TemplateResponse("edfa.html", {"request": request})


@app.post('/enable_disable_laser_id/{id}')
async def enable_disable_laser_id(id: str, state: bool):
    print(f"Laser {id} state change to {state}")
    if id == 'LD1':
        laser_module_1_set.laser_976_1.enabled = state
        mqtt.publish('/laser_module_set/1', jsonable_encoder(laser_module_1_set))
    elif id == 'LD2':
        laser_module_2_set.laser_1480_1.enabled = state
        mqtt.publish('/laser_module_set/2', jsonable_encoder(laser_module_2_set))
    elif id == 'LD3':
        laser_module_2_set.laser_1480_2.enabled = state
        mqtt.publish('/laser_module_set/2', jsonable_encoder(laser_module_2_set))
    elif id == 'LD4':
        laser_module_3_set.laser_1480_1.enabled = state
        mqtt.publish('/laser_module_set/3', jsonable_encoder(laser_module_3_set))
    elif id == 'LD5':
        laser_module_3_set.laser_1480_2.enabled = state
        mqtt.publish('/laser_module_set/3', jsonable_encoder(laser_module_3_set))
    elif id == 'LD6':
        laser_module_1_set.laser_976_2.enabled = state
        mqtt.publish('/laser_module_set/1', jsonable_encoder(laser_module_1_set))

@app.post("/set_laser_module_desired_temperature/{id}")
async def set_laser_module_desired_temperature(id: int, desired_temperature: int):
    print(f'Desired temperature for module {id} is {desired_temperature}')
    if desired_temperature not in range(0, 71):
        return
    if id == 1:
        laser_module_1_set.laser_976_1.desired_temperature = desired_temperature
        laser_module_1_set.laser_976_2.desired_temperature = desired_temperature
        json_data = jsonable_encoder(laser_module_1_set)
        mqtt.publish('/laser_module_set/1', json.dumps(json_data))
    elif id == 2:
        laser_module_2_set.laser_1480_1.desired_temperature = desired_temperature
        laser_module_2_set.laser_1480_2.desired_temperature = desired_temperature
        json_data = jsonable_encoder(laser_module_2_set)
        mqtt.publish('/laser_module_set/2', json.dumps(json_data))
    elif id == 3:
        laser_module_3_set.laser_1480_1.desired_temperature = desired_temperature
        laser_module_3_set.laser_1480_2.desired_temperature = desired_temperature
        json_data = jsonable_encoder(laser_module_3_set)
        mqtt.publish('/laser_module_set/3', json.dumps(json_data))
      
@app.post("/set_laser_id_desired_mon_cur/{id}")
async def set_laser_id_desired_mon_cur(id: str, current: int):
    print(f"Laser {id} monitor diode current change to {current} uA")
    if id == 'LD1':
        laser_module_1_set.laser_976_1.desired_monitor_diode_current = current
        mqtt.publish('/laser_module_set/1', jsonable_encoder(laser_module_1_set))
    elif id == 'LD2':
        laser_module_2_set.laser_1480_1.desired_monitor_diode_current = current
        mqtt.publish('/laser_module_set/2', jsonable_encoder(laser_module_2_set))
    elif id == 'LD3':
        laser_module_2_set.laser_1480_2.desired_monitor_diode_current = current
        mqtt.publish('/laser_module_set/2', jsonable_encoder(laser_module_2_set))
    elif id == 'LD4':
        laser_module_3_set.laser_1480_1.desired_monitor_diode_current = current
        mqtt.publish('/laser_module_set/3', jsonable_encoder(laser_module_3_set))
    elif id == 'LD5':
        laser_module_3_set.laser_1480_2.desired_monitor_diode_current = current
        mqtt.publish('/laser_module_set/3', jsonable_encoder(laser_module_3_set))
    elif id == 'LD6':
        laser_module_1_set.laser_976_2.desired_monitor_diode_current = current
        mqtt.publish('/laser_module_set/1', jsonable_encoder(laser_module_1_set))
            
@app.post("/emergency_stop_laser_id/{id}")
async def emergency_stop_laser_id(id: str):
    if id == 'LD1':
        laser_module_1_set.laser_976_1.enabled = False
        laser_module_1_set.laser_976_1.desired_monitor_diode_current = 0
        mqtt.publish('/laser_module_set/1', jsonable_encoder(laser_module_1_set))
    elif id == 'LD2':
        laser_module_2_set.laser_1480_1.enabled = False
        laser_module_2_set.laser_1480_1.desired_monitor_diode_current = 0
        mqtt.publish('/laser_module_set/2', jsonable_encoder(laser_module_2_set))
    elif id == 'LD3':
        laser_module_2_set.laser_1480_2.enabled = False
        laser_module_2_set.laser_1480_2.desired_monitor_diode_current = 0
        mqtt.publish('/laser_module_set/2', jsonable_encoder(laser_module_2_set))
    elif id == 'LD4':
        laser_module_3_set.laser_1480_1.enabled = False
        laser_module_3_set.laser_1480_1.desired_monitor_diode_current = 0
        mqtt.publish('/laser_module_set/3', jsonable_encoder(laser_module_3_set))
    elif id == 'LD5':
        laser_module_3_set.laser_1480_2.enabled = False
        laser_module_3_set.laser_1480_2.desired_monitor_diode_current = 0
        mqtt.publish('/laser_module_set/3', jsonable_encoder(laser_module_3_set))
    elif id == 'LD6':
        laser_module_1_set.laser_976_2.enabled = False
        laser_module_1_set.laser_976_2.desired_monitor_diode_current = 0
        mqtt.publish('/laser_module_set/1', jsonable_encoder(laser_module_1_set))
        
@app.post("/set_voa_module_attenuation/{attenuation}")
async def set_voa_module_attenuation(attenuation: int):
    print(f'Attenuation set to attenuation to: {attenuation}')
    voa_module_set.attenuation = attenuation
    json_data = jsonable_encoder(voa_module_set)
    mqtt.publish("/voa_attenuation", json.dumps(json_data))
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
    #print("Received message to specific topic: ", topic, payload.decode(), qos, properties)
    json_data = json.loads(payload.decode())
    json_data["time_stamp"] = time()
    try:
        module_number = json_data["module_number"]
        if module_number == 1:
            laser_module_1_report = json_data
            await websocket_send_data(frontend_get_websocket, json.dumps(laser_module_1_report))
        elif module_number == 2:
            laser_module_2_report = json_data
            await websocket_send_data(frontend_get_websocket, json.dumps(laser_module_2_report))
        elif module_number == 3:
            laser_module_3_report = json_data
            await websocket_send_data(frontend_get_websocket, json.dumps(laser_module_3_report))
    except:
        print('No websocket found.')

@mqtt.on_disconnect()
def disconnect(client, packet, exc=None):
    print("Disconnected")

@mqtt.on_subscribe()
def subscribe(client, mid, qos, properties):
    # print("subscribed", client, mid, qos, properties)
    return
