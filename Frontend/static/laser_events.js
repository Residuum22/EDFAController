const laser_module_desired_temperature_offset = 5;
const laser_module_desired_monitor_diode_offset = 250;
const uri = "http://localhost:8000"


function post_call(url, json_body, callback) {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            if (callback)
                callback()
        }
    };
    xhttp.open("POST", uri + url, true);
    xhttp.setRequestHeader("Content-type", "application/json");
    xhttp.send(JSON.stringify(json_body));
}

function enable_disable_laser_module(laser_id) {
    let laser_name = "laser_module" + laser_id + "_on_off_switch";
    let state = document.getElementById(laser_name).checked;
    let url = "/enable_disable_laser_module/" + laser_id + "?" + "state="+ state 
    post_call(url, null, null);
}

function update_laser_temp(temp, laser_id) {
    document.getElementById("laser" + laser_id + "_temp").innerHTML = "Temperature: " + temp + "°";
}




function increment_laser_desired_temperature(laser_module_id) {
    let laser_name = "laser_module_" + laser_module_id + "_desired_temperature";
    let current_desired_temperature = document.getElementById(laser_name).innerHTML;
    let current_desired_temperature_int = parseInt(current_desired_temperature.match(/\d/g).join(""));

    if (current_desired_temperature_int < 70) {
        current_desired_temperature_int = current_desired_temperature_int + laser_module_desired_temperature_offset;
    }
    document.getElementById(laser_name).innerHTML = "Lézerek kívánt hőmérséklete: " + current_desired_temperature_int + " °C";
}

function decrement_laser_desired_temperature(laser_module_id) {
    let laser_name = "laser_module_" + laser_module_id + "_desired_temperature";
    let current_desired_temperature = document.getElementById(laser_name).innerHTML;
    let current_desired_temperature_int = parseInt(current_desired_temperature.match(/\d/g).join(""));

    if (current_desired_temperature_int > 40) {
        current_desired_temperature_int = current_desired_temperature_int - laser_module_desired_temperature_offset;
    }
    document.getElementById(laser_name).innerHTML = "Lézerek kívánt hőmérséklete: " + current_desired_temperature_int + " °C";
}

function increment_monitor_diode_desired_current(laser_id, laser_type) {
    let laser_name = "laser_module_" + laser_id + "_" + laser_type + "_monitor_diode_desired_current";
    let current_desired_monitor_diode_current = document.getElementById(laser_name).innerHTML;
    let current_desired_monitor_diode_current_int = parseInt(current_desired_monitor_diode_current.match(/\d/g).join(""));

    if (current_desired_monitor_diode_current_int < 3000) {
        current_desired_monitor_diode_current_int = current_desired_monitor_diode_current_int + laser_module_desired_monitor_diode_offset;
    }

    document.getElementById(laser_name).innerHTML = "Kívánt monitor dióda árama: " + current_desired_monitor_diode_current_int + " µA";
}

function decrement_monitor_diode_desired_current(laser_id, laser_type) {
    let laser_name = "laser_module_" + laser_id + "_" + laser_type + "_monitor_diode_desired_current";
    let current_desired_monitor_diode_current = document.getElementById(laser_name).innerHTML;
    let current_desired_monitor_diode_current_int = parseInt(current_desired_monitor_diode_current.match(/\d/g).join(""));

    if (current_desired_monitor_diode_current_int > 0) {
        current_desired_monitor_diode_current_int = current_desired_monitor_diode_current_int + laser_module_desired_monitor_diode_offset;
    }

    document.getElementById(laser_name).innerHTML = "Kívánt monitor dióda árama: " + current_desired_monitor_diode_current_int + " µA";
}

function laser_emergency_stop() {
    document.getElementById("laser1_current").innerHTML = "Laser diode current: ~0 mA";
}

