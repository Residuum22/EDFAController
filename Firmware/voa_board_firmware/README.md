# Variable Optical Attenuator (VOA) firmware

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

## Setting up development

TODO: Set this into the main repo
Get idf 5.0.1