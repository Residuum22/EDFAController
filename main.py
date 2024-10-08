#!/usr/bin/python3

import uvicorn

if __name__ == "__main__":
    uvicorn.run("Backend.app.app:app", port=8000, log_level="info", reload=False, host="0.0.0.0")
