from fastapi import FastAPI, Request
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