const laser_max_temp_offset = 20

function enable_disable_laser(laser_id) {
    let laser_name = "laser" + laser_id + "_on_off_switch";
    let state = document.getElementById(laser_name).checked;
    if (state == true)
        console.log("Enabling laser")
    else
        console.log("Disabling laser")
}

function update_laser_temp(temp, laser_id) {
    document.getElementById("laser" + laser_id + "_temp").innerHTML = "Temperature: " + temp + "°";
}

function increment_laser_max_temp(laser_id) {
    let laser_name = "laser" + laser_id + "_max_temp";
    let current_max_temp = document.getElementById(laser_name).innerHTML;
    let current_pos_int = parseInt(current_max_temp.match(/\d/g).join(""));

    if (current_pos_int < 80) {
        current_pos_int = current_pos_int + laser_max_temp_offset;
    }

    document.getElementById(laser_name).innerHTML = "Maximum tempearature: " + current_pos_int + "°";
}

function decrement_laser_max_temp(laser_id) {
    let laser_name = "laser" + laser_id + "_max_temp";
    let current_max_temp = document.getElementById("laser1_max_temp").innerHTML;
    let current_pos_int = parseInt(current_max_temp.match(/\d/g).join(""));

    if (current_pos_int > 0) {
        current_pos_int = current_pos_int - laser_max_temp_offset;
    }

    document.getElementById("laser1_max_temp").innerHTML = "Maximum tempearature: " + current_pos_int + "°";
}

function increment_laser_current(laser_id) {
    let laser_name = "laser" + laser_id + "_current";
    let current_max_temp = document.getElementById(laser_name).innerHTML;
    let current_pos_int = parseInt(current_max_temp.match(/\d/g).join(""));

    if (current_pos_int < 1000) {
        current_pos_int = current_pos_int + laser_max_temp_offset;
    }

    document.getElementById(laser_name).innerHTML = "Laser diode current: ~" + current_pos_int + "mA";
}

function decrement_laser_current(laser_id) {
    let laser_name = "laser" + laser_id + "_current";
    let current_max_temp = document.getElementById(laser_name).innerHTML;
    let current_pos_int = parseInt(current_max_temp.match(/\d/g).join(""));

    if (current_pos_int > 0) {
        current_pos_int = current_pos_int - laser_max_temp_offset;
    }

    document.getElementById(laser_name).innerHTML = "Laser diode current: ~" + current_pos_int + "mA";
}

function laser_emergency_stop() {
    document.getElementById("laser1_current").innerHTML = "Laser diode current: ~0 mA";
}

