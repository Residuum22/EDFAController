# Laser controller board firmware

Configure path + python

Settings.json
```json
"idf.espIdfPathWin": "",
"idf.toolsPathWin": "",
"idf.pythonBinPathWin": "",
"idf.flashType": "JTAG",
    "idf.adapterTargetName": "esp32",
    "idf.openOcdConfigs": [
        "interface/jlink.cfg",
        "config/custom_board.cfg"
    ]
```
Example
```json
"idf.espIdfPathWin": "C:\\Users\\mihal\\esp\\esp-idf",
    "idf.toolsPathWin": "C:\\Espressif\\tools",
    "idf.pythonBinPathWin": "c:\\Espressif\\python_env\\idf5.0_py3.8_env\\Scripts\\python",
```

## Setting up development

TODO: Set this into the main repo
Get idf 5.0.1