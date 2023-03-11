from fastapi import FastAPI, Request, WebSocket, WebSocketDisconnect
from pydantic import BaseModel
from fastapi.templating import Jinja2Templates
from fastapi.staticfiles import StaticFiles
from time import time
import json

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


class voa_data(BaseModel):
    attenuation: int


class board_status(BaseModel):
    status: str


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
# HTTP
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
        case 2:
            laser_module_2_set.enabled = state
        case 3:
            laser_module_3_set.enabled = state

    # TODO httpx send data to esp32
    return


@app.post("/set_laser_module_desired_temperature/{id}")
async def set_laser_module_desired_temperature(id: int, desired_temperature: int) -> board_status:
    if desired_temperature not in range(30, 70):
        return board_status(status="ERROR")
    match id:
        case 1:
            laser_module_1_set.laser_976.desired_temperature = desired_temperature
            laser_module_1_set.laser_1480.desired_temperature = desired_temperature
            # TODO httpx send data
        case 2:
            laser_module_2_set.laser_976.desired_temperature = desired_temperature
            laser_module_2_set.laser_1480.desired_temperature = desired_temperature
            # TODO httpx send data
        case 3:
            laser_module_3_set.laser_976.desired_temperature = desired_temperature
            laser_module_3_set.laser_1480.desired_temperature = desired_temperature
            # TODO httpx send data
    return board_status(status="OK")
# ------------------------------------------------------------------------------
# Websocket
# ------------------------------------------------------------------------------


@app.websocket("/report_laser_module_data")
async def websocket_laser1_data(websocket: WebSocket):
    await websocket.accept()
    try:
        while True:
            data = await websocket.receive_text()
            json_data = json.loads(data)
            json_data["time_stamp"] = time()
            match json_data["module_number"]:
                case 1:
                    laser_module_1_report = json_data
                case 2:
                    laser_module_2_report = json_data
                case 3:
                    laser_module_3_report = json_data
            # add
    except WebSocketDisconnect:
        return
