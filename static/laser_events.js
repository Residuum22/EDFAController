const laser_max_temp_offset = 5

function update_laser_temp(temp) {
    document.getElementById("laser1_temp").innerHTML = "Temperature: " + temp + "°";
}

function increment_laser_max_temp() {
    let current_max_temp = document.getElementById("laser1_max_temp").innerHTML;
    let current_pos_int = parseInt(current_max_temp.match(/\d/g).join(""));

    if (current_pos_int < 80) {
        current_pos_int = current_pos_int + laser_max_temp_offset;
    }

    document.getElementById("laser1_max_temp").innerHTML = "Maximum tempearature: " + current_pos_int + "°";
}

function decrement_laser_max_temp() {
    let current_max_temp = document.getElementById("laser1_max_temp").innerHTML;
    let current_pos_int = parseInt(current_max_temp.match(/\d/g).join(""));

    if (current_pos_int > 0) {
        current_pos_int = current_pos_int - laser_max_temp_offset;
    }

    document.getElementById("laser1_max_temp").innerHTML = "Maximum tempearature: " + current_pos_int + "°";
}

function increment_laser_current() {
    let current_max_temp = document.getElementById("laser1_current").innerHTML;
    let current_pos_int = parseInt(current_max_temp.match(/\d/g).join(""));

    if (current_pos_int < 1000) {
        current_pos_int = current_pos_int + laser_max_temp_offset;
    }

    document.getElementById("laser1_current").innerHTML = "Maximum tempearature: " + current_pos_int + "°";
}

function decrement_laser_current() {
    let current_max_temp = document.getElementById("laser1_current").innerHTML;
    let current_pos_int = parseInt(current_max_temp.match(/\d/g).join(""));

    if (current_pos_int > 0) {
        current_pos_int = current_pos_int - laser_max_temp_offset;
    }

    document.getElementById("laser1_current").innerHTML = "Maximum tempearature: " + current_pos_int + "°";
}

function laser_emergency_stop() {
    document.getElementById("laser1_current").innerHTML = "Laser diode current: ~0 mA";
}

