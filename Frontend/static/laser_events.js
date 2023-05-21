const laser_module_desired_temperature_offset = 5;
const laser_module_desired_monitor_diode_offset = 250;
const HOST = "http://localhost:8000"


function post_call(path, json_body, callback) {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            if (callback)
                callback()
        }
    };
    xhttp.open("POST", HOST + path, true);
    xhttp.setRequestHeader("Content-type", "application/json");
    xhttp.send(JSON.stringify(json_body));
}

function send_desTemp_backend(laser_id, desTemp) {
    let path = "/set_laser_module_desired_temperature/" + laser_id + '?' + 'desired_temperature=' + desTemp;
    post_call_voa(path, null, null);
}

function send_desMonCur_backend(laser_id, desMonCur) {
    let path = "/set_laser_id_desired_mon_cur/" + laser_id + '?' + 'current=' + desMonCur;
    post_call_voa(path, null, null);
}

function send_enable_backend(laser_id, state) {
    let path = "/enable_disable_laser_id/" + laser_id + "?" + "state="+ state;
    post_call(path, null, null);
}

function send_emerg_stop_backend(laser_id) {
    let path = "/emergency_stop_laser_id/" + laser_id;
    post_call(path, null, null);
}

function enable_disable_laser_id(laser_id) {
    let laser_id_name = `laser_id_${laser_id}_on_off_switch`;
    let state = document.getElementById(laser_id_name).checked;
    send_enable_backend(laser_id, state);
}

function increment_laser_desired_temperature(laser_module_id) {
    let laser_name = "laser_module_" + laser_module_id + "_desired_temperature";
    let current_desired_temperature = document.getElementById(laser_name).innerHTML;
    let current_desired_temperature_int = parseInt(current_desired_temperature.match(/\d/g).join(""));

    if (current_desired_temperature_int < 70) {
        current_desired_temperature_int = current_desired_temperature_int + laser_module_desired_temperature_offset;
        send_desTemp_backend(laser_module_id, current_desired_temperature_int);
    }
    document.getElementById(laser_name).innerHTML = "Lézerek kívánt hőmérséklete: " + current_desired_temperature_int + " °C";
}

function decrement_laser_desired_temperature(laser_module_id) {
    let laser_name = "laser_module_" + laser_module_id + "_desired_temperature";
    let current_desired_temperature = document.getElementById(laser_name).innerHTML;
    let current_desired_temperature_int = parseInt(current_desired_temperature.match(/\d/g).join(""));

    if (current_desired_temperature_int > 40) {
        current_desired_temperature_int = current_desired_temperature_int - laser_module_desired_temperature_offset;
        send_desTemp_backend(laser_module_id, current_desired_temperature_int);
    }
    document.getElementById(laser_name).innerHTML = "Lézerek kívánt hőmérséklete: " + current_desired_temperature_int + " °C";
}

function increment_monitor_diode_desired_current(laser_id) {
    let laser_name = "laser_module_" + laser_id + "_monitor_diode_desired_current";
    let current_desired_monitor_diode_current = document.getElementById(laser_name).innerHTML;
    let current_desired_monitor_diode_current_int = parseInt(current_desired_monitor_diode_current.match(/\d/g).join(""));

    if (current_desired_monitor_diode_current_int < 3000) {
        current_desired_monitor_diode_current_int = current_desired_monitor_diode_current_int + laser_module_desired_monitor_diode_offset;
        send_desMonCur_backend(laser_id, current_desired_monitor_diode_current_int)
    }
    document.getElementById(laser_name).innerHTML = "Kívánt monitor dióda árama: " + current_desired_monitor_diode_current_int + " µA";
}

function decrement_monitor_diode_desired_current(laser_id) {
    let laser_name = "laser_module_" + laser_id + "_monitor_diode_desired_current";
    let current_desired_monitor_diode_current = document.getElementById(laser_name).innerHTML;
    let current_desired_monitor_diode_current_int = parseInt(current_desired_monitor_diode_current.match(/\d/g).join(""));

    if (current_desired_monitor_diode_current_int > 0) {
        current_desired_monitor_diode_current_int = current_desired_monitor_diode_current_int - laser_module_desired_monitor_diode_offset;
        send_desMonCur_backend(laser_id, current_desired_monitor_diode_current_int)
    }
    document.getElementById(laser_name).innerHTML = "Kívánt monitor dióda árama: " + current_desired_monitor_diode_current_int + " µA";
}

function laser_emergency_stop(laser_id) {
    laser_id.forEach(element => {
        let laser_name = "laser_module_" + element + "_monitor_diode_desired_current";
        document.getElementById(laser_name).innerHTML = "Kívánt monitor dióda árama: 0 µA";

        laser_name = "laser_id_" + element + "_on_off_switch";
        document.getElementById(laser_name).checked = 0;

        send_emerg_stop_backend(element)
    });
}

function update_laser_module_temp(laser_id, laser_type, temp) {
    document.getElementById(`laser_module_${laser_id}_${laser_type}_temperature`).innerHTML = `Hőmérséklet: ${temp} °C`;
}

function update_laser_module_monitor_diode_current(laser_id, laser_type, current) {
    document.getElementById(`laser_module_${laser_id}_${laser_type}_monitor_diode_current`).innerHTML = `Monitor dióda árama: ${current} µA`;
}

function update_laser_module_laser_current(laser_id, laser_type, current) {
    document.getElementById(`laser_module_${laser_id}_${laser_type}_laser_current`).innerHTML = `Lézer dióda becsült árama: ${current} mA`;
}