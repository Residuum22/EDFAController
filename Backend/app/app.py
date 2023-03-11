from fastapi import FastAPI, Depends, Request, WebSocket, WebSocketDisconnect
from pydantic import BaseModel
from fastapi.templating import Jinja2Templates
from fastapi.staticfiles import StaticFiles
from time import time
import json
import asyncio

# ------------------------------------------------------------------------------
# Models
# ------------------------------------------------------------------------------


class laser_data_set(BaseModel):
    desired_temperature: int
    desired_monitor_diode_current: int


class laser_module_data_set(BaseModel):
    enabled: bool
    report_interval: int
    laser_976: laser_data_set
    laser_1480: laser_data_set


class laser_data_report(BaseModel):
    temperature: int
    monitor_diode_current: int
    laser_current: int


class laser_module_data_report(BaseModel):
    module_number: int
    time_stamp: float
    laser_976: laser_data_report
    laser_1480: laser_data_report


class voa_module_data_set(BaseModel):
    attenuation: int


# ------------------------------------------------------------------------------
# Public -> database later
# ------------------------------------------------------------------------------
laser_module_1_report = laser_module_data_report(module_number=1, time_stamp=0,
                                                 laser_976={
                                                     "temperature": 0, "monitor_diode_current": 0, "laser_current": 0},
                                                 laser_1480={
                                                     "temperature": 0, "monitor_diode_current": 0, "laser_current": 0}
                                                 )

laser_module_2_report = laser_module_data_report(module_number=2, time_stamp=0,
                                                 laser_976={
                                                     "temperature": 0, "monitor_diode_current": 0, "laser_current": 0},
                                                 laser_1480={
                                                     "temperature": 0, "monitor_diode_current": 0, "laser_current": 0}
                                                 )

laser_module_3_report = laser_module_data_report(module_number=3, time_stamp=0,
                                                 laser_976={
                                                     "temperature": 0, "monitor_diode_current": 0, "laser_current": 0},
                                                 laser_1480={
                                                     "temperature": 0, "monitor_diode_current": 0, "laser_current": 0}
                                                 )

laser_module_1_set = laser_module_data_set(enabled=False, report_interval=1, laser_976={
                                           "desired_temperature": 50, "desired_monitor_diode_current": 0}, laser_1480={"desired_temperature": 50, "desired_monitor_diode_current": 0})

laser_module_2_set = laser_module_data_set(enabled=False, report_interval=1, laser_976={
                                           "desired_temperature": 50, "desired_monitor_diode_current": 0}, laser_1480={"desired_temperature": 50, "desired_monitor_diode_current": 0})

laser_module_3_set = laser_module_data_set(enabled=False, report_interval=1, laser_976={
                                           "desired_temperature": 50, "desired_monitor_diode_current": 0}, laser_1480={"desired_temperature": 50, "desired_monitor_diode_current": 0})

voa_module_set = voa_module_data_set(attenuation=0)


async def websocket_send_data(ws: WebSocket, data: json):
    try:
        if ws is not None:
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
app.mount("/npm", StaticFiles(directory="Frontend/node_modules"), name="npm")


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
    return templates.TemplateResponse("index.html", {"request": request})


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
            await websocket_send_data(laser_module_1_websocket,
                                laser_module_1_set.json())
        case 2:
            laser_module_2_set.enabled = state
            await websocket_send_data(laser_module_2_websocket,
                                laser_module_2_set.json())
        case 3:
            laser_module_3_set.enabled = state
            await websocket_send_data(laser_module_3_websocket,
                                laser_module_3_set.json())
    return


@app.post("/set_laser_module_desired_temperature/{id}")
async def set_laser_module_desired_temperature(id: int, desired_temperature: int):
    if desired_temperature not in range(29, 71):
        return
    match id:
        case 1:
            laser_module_1_set.laser_976.desired_temperature = desired_temperature
            laser_module_1_set.laser_1480.desired_temperature = desired_temperature
            await websocket_send_data(laser_module_1_websocket,
                                laser_module_1_set.json())
        case 2:
            laser_module_2_set.laser_976.desired_temperature = desired_temperature
            laser_module_2_set.laser_1480.desired_temperature = desired_temperature
            await websocket_send_data(laser_module_2_websocket,
                                laser_module_2_set.json())
        case 3:
            laser_module_3_set.laser_976.desired_temperature = desired_temperature
            laser_module_3_set.laser_1480.desired_temperature = desired_temperature
            await websocket_send_data(laser_module_3_websocket,
                                laser_module_3_set.json())
# ------------------------------------------------------------------------------
# HTTP End
# ------------------------------------------------------------------------------
# ------------------------------------------------------------------------------
# Websocket Start
# ------------------------------------------------------------------------------


# Websocket for esp32

@app.websocket("/report_laser_module_data")
async def websocket_laser1_data(websocket: WebSocket):
    await websocket.accept()
    try:
        while True:
            json_data = await websocket.receive_json()
            json_data["time_stamp"] = time()
            match json_data["module_number"]:
                case 1:
                    laser_module_1_report = json_data
                case 2:
                    laser_module_2_report = json_data
                case 3:
                    laser_module_3_report = json_data
            # TODO send data to frontend with websocket
    except WebSocketDisconnect:
        return


@app.websocket("/set_laser_module_1")
async def websocket_laser1_data(websocket: WebSocket):
    await websocket.accept()
    global laser_module_1_websocket
    laser_module_1_websocket = websocket
    try:
        while True:
            _ = await websocket.receive_text()

    except WebSocketDisconnect:
        return

    finally:
        laser_module_1_websocket = None


@app.websocket("/set_laser_module_2")
async def websocket_laser1_data(websocket: WebSocket):
    await websocket.accept()
    global laser_module_2_websocket
    laser_module_2_websocket = websocket
    try:
        while True:
            _ = await websocket.receive_text()

    except WebSocketDisconnect:
        return

    finally:
        laser_module_2_websocket = None


@app.websocket("/set_laser_module_3")
async def websocket_laser1_data(websocket: WebSocket):
    await websocket.accept()
    global laser_module_3_websocket
    laser_module_3_websocket = websocket
    try:
        while True:
            _ = await websocket.receive_text()

    except WebSocketDisconnect:
        return

    finally:
        laser_module_3_websocket = None


@app.websocket("/set_voa_module")
async def websocket_laser1_data(websocket: WebSocket):
    await websocket.accept()
    global voa_module_websocket
    voa_module_websocket = websocket
    try:
        while True:
            _ = await websocket.receive_text()

    except WebSocketDisconnect:
        return

    finally:
        voa_module_websocket = None
# ------------------------------------------------------------------------------
# Websocket End
# ------------------------------------------------------------------------------
