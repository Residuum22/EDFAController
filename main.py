import uvicorn

if __name__ == "__main__":
    uvicorn.run("app.app:app", port=5000, log_level="info", reload=True)