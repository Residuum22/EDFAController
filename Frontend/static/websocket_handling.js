const WS_HOST = "ws://localhost:8000"

var Socket = new WebSocket(WS_HOST + "/frontend_get_module_data");

Socket.onopen = function (e) {
    // Do nothing
};

Socket.onmessage = function (evt) {
    let json_msg = JSON.parse(JSON.parse(evt.data))
    console.log(typeof(json_msg))
    let module_number = json_msg["module_number"]

    switch (module_number) {
        case 1:
            update_laser_id_temp(json_msg['laser_976_1']['laser_id'], json_msg['laser_976_1']['temperature'])
            update_laser_id_temp(json_msg['laser_976_2']['laser_id'], json_msg['laser_976_2']['temperature'])

            update_laser_id_laser_current(json_msg['laser_976_1']['laser_id'], json_msg['laser_976_1']['laser_current'])
            update_laser_id_laser_current(json_msg['laser_976_2']['laser_id'], json_msg['laser_976_2']['laser_current'])

            update_laser_id_monitor_diode_current(json_msg['laser_976_1']['laser_id'], json_msg['laser_976_1']['monitor_diode_current'])
            update_laser_id_monitor_diode_current(json_msg['laser_976_2']['laser_id'], json_msg['laser_976_2']['monitor_diode_current'])
            break;
        
        case 2:
        case 3:
            update_laser_id_temp(json_msg['laser_1480_1']['laser_id'], json_msg['laser_1480_1']['temperature'])
            update_laser_id_temp(json_msg['laser_1480_2']['laser_id'], json_msg['laser_1480_2']['temperature'])

            update_laser_id_laser_current(json_msg['laser_1480_1']['laser_id'], json_msg['laser_1480_1']['laser_current'])
            update_laser_id_laser_current(json_msg['laser_1480_2']['laser_id'], json_msg['laser_1480_2']['laser_current'])

            update_laser_id_monitor_diode_current(json_msg['laser_1480_1']['laser_id'], json_msg['laser_1480_1']['monitor_diode_current'])
            update_laser_id_monitor_diode_current(json_msg['laser_1480_2']['laser_id'], json_msg['laser_1480_2']['monitor_diode_current'])
            break;
        default:
            break;
    }
};

Socket.onclose = function (event) {
    // if (event.wasClean) {
    //     alert(`[close] Connection closed cleanly, code=${event.code} reason=${event.reason}`);
    // } else {
    //     // e.g. server process killed or network down
    //     // event.code is usually 1006 in this case
    //     alert('[close] Connection died');
    // }
};

Socket.onerror = function (error) {
    // alert(`[error]`);
};
