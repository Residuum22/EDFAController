const WS_HOST = "ws://localhost:8000"

var Socket = new WebSocket(WS_HOST + "/frontend_get_module_data");

Socket.onopen = function (e) {
    // Do nothing
};

Socket.onmessage = function (evt) {
    let json_msg = JSON.parse(JSON.stringify(evt.data))
    let module_number = json_msg.module_number

    update_laser_module_temp(module_number, 976, json_msg.laser_976nm.temperature)
    update_laser_module_temp(module_number, 1480, json_msg["laser_1480nm"]["temperature"])
};

Socket.onclose = function (event) {
    if (event.wasClean) {
        alert(`[close] Connection closed cleanly, code=${event.code} reason=${event.reason}`);
    } else {
        // e.g. server process killed or network down
        // event.code is usually 1006 in this case
        alert('[close] Connection died');
    }
};

Socket.onerror = function (error) {
    alert(`[error]`);
};
