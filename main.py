import uvicorn

if __name__ == "__main__":
    uvicorn.run("Backend.app.app:app", port=5000, log_level="info", reload=True)