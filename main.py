#/bin/python

import uvicorn

if __name__ == "__main__":
    uvicorn.run("Backend.app.app:app", port=8000, log_level="info", reload=True, host="0.0.0.0")