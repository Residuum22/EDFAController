from fastapi import FastAPI, Request, WebSocket, WebSocketDisconnect
from fastapi.responses import HTMLResponse
from fastapi.templating import Jinja2Templates
from fastapi.staticfiles import StaticFiles

app = FastAPI()
templates = Jinja2Templates(directory="../templates")

app.mount("/static/img", StaticFiles(directory="../static/img"), name="img")

@app.on_event("startup")
async def startup():
    print("Server starting...")

@app.on_event("shutdown")
async def shutdown():
    print("Server closing...")

@app.get("/")
async def root(request: Request):
    return templates.TemplateResponse("index.html", {"request": request})

@app.get("/index", response_class=HTMLResponse)
async def load_index_page(request: Request):
    return templates.TemplateResponse("index.html", {"request": request})

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