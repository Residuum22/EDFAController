const laser_module_desired_temperature_offset = 5;
const laser_module_desired_laser_diode_offset = 20;
const HOST = "http://localhost:8000"

/**
 * This function is a generic function which can send data to
 * the backend. 
 */
function post_call(path, json_body, callback) {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            if (callback)
                callback()
        }
    };
    xhttp.open("POST", HOST + path, true);
    xhttp.setRequestHeader("Content-type", "text/plain");
    xhttp.send(JSON.stringify(json_body));
}

/**
 * Send desired temperature from the frontend to the backend
 */
function send_desTemp_backend(laser_id, desTemp) {
    let path = "/set_laser_module_desired_temperature/" + laser_id + '?' + 'desired_temperature=' + desTemp;
    post_call_voa(path, null, null);
}

/**
 * Send desired monitor current from the frontend to the backend
 */
function send_desLasCur_backend(laser_id, desMonCur) {
    let path = "/set_laser_id_desired_laser_cur/" + laser_id + '?' + 'current=' + desMonCur;
    post_call_voa(path, null, null);
}

/**
 * Disable laser in emergency case
 */
function send_emerg_stop_backend(laser_id) {
    let path = "/emergency_stop_laser_id/" + laser_id;
    post_call(path, null, null);
}

/**
 * Enable / disable laser end send to the backend
 */
function enable_disable_laser_id(laser_id) {
    let laser_id_name = `laser_id_${laser_id}_on_off_switch`;
    let state = document.getElementById(laser_id_name).checked;
    let path = "/enable_disable_laser_id/" + laser_id + "?" + "state=" + state;
    post_call(path, null, null);
}

/**
 * Increment desired temperature
 */
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

function send_laser_diode_desired_current(laser_id) {
    let laser_name = "laser_module_" + laser_id + "_laser_current";
    console.log(laser_name)
    let current_laser_diode_current = document.getElementById(laser_name).value;

    let current_laser_diode_current_int = parseInt(current_laser_diode_current);

    if (['LD1', 'LD6'].includes(laser_id)) {
        if (current_laser_diode_current_int <= 360) {
            send_desLasCur_backend(laser_id, current_laser_diode_current_int)
            document.getElementById(laser_name).value = current_laser_diode_current_int
        }
        else {
            window.alert(`A ${laser_id} árama csak 0 és 360 mA között lehet!`);
        }
    }
    else {
        if (current_laser_diode_current_int <= 700) {
            send_desLasCur_backend(laser_id, current_laser_diode_current_int)
            document.getElementById(laser_name).value = current_laser_diode_current_int
        }
        else {
            window.alert(`A ${laser_id} árama csak 0 és 700 mA között lehet!`);
        }
    }
}

function laser_emergency_stop(laser_id) {
    laser_id.forEach(element => {
        let laser_name = "laser_module_" + element + "_laser_diode_desired_current";
        document.getElementById(laser_name).innerHTML = "Lézer dióda árama: 0 mA";

        laser_name = "laser_id_" + element + "_on_off_switch";
        document.getElementById(laser_name).checked = 0;

        send_emerg_stop_backend(element)
    });
}

function update_laser_id_temp(laser_id, temp) {
    document.getElementById(`laser_id_${laser_id}_temperature`).innerHTML = `Hőmérséklet: ${temp} °C`;
}

function update_laser_id_monitor_diode_current(laser_id, current) {
    document.getElementById(`laser_id_${laser_id}_monitor_diode_current`).innerHTML = `Becsült monitor dióda árama: ${current} µA`;
}