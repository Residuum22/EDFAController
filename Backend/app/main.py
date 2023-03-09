from fastapi import FastAPI, Request, WebSocket, WebSocketDisconnect
from pydantic import BaseModel


class laser_data(BaseModel):
    temp : int
    max_temp : int

app = FastAPI()

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
async def root() -> list[laser_data]:
    return [
        laser_data(temp=50, max_temp=80),
        laser_data(temp=45, max_temp=82)
        ]
        

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