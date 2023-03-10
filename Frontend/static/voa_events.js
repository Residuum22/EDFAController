const voa_position_offset = 5

function update_voa_pos(position) {
    document.getElementById("voa_pos").innerHTML = "Position: " + position + "°";
}

function increment_voa_pos() {
    let current_pos = document.getElementById("voa_pos").innerHTML;
    let current_pos_int = parseInt(current_pos.match(/\d/g).join(""));

    if (current_pos_int < 360) {
        current_pos_int = current_pos_int + voa_position_offset;
    }

    document.getElementById("voa_pos").innerHTML = "Position: " + current_pos_int + "°";
}

function decrement_voa_pos() {
    let current_pos = document.getElementById("voa_pos").innerHTML;
    let current_pos_int = parseInt(current_pos.match(/\d/g).join(""));

    if (current_pos_int > 0) {
        current_pos_int = current_pos_int - voa_position_offset;
    }

    document.getElementById("voa_pos").innerHTML = "Position: " + current_pos_int + "°";
}

function reset_voa_position() {
    document.getElementById("voa_pos").innerHTML = "Position: " + 0 + "°";
}

