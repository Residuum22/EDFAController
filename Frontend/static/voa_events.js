const voa_attenuation_delta = 1

function update_voa_pos(attenuation) {
    document.getElementById("voa_attenuation").innerHTML = "Csillapítás: " + attenuation + " dB";
}

function increment_voa_attenuation() {
    let current_attenuation = document.getElementById("voa_attenuation").innerHTML;
    let current_attenuation_number = parseInt(current_attenuation.match(/\d/g).join(""));

    if (current_attenuation_number < 15) {
        current_attenuation_number = current_attenuation_number + voa_attenuation_delta;
    }

    document.getElementById("voa_attenuation").innerHTML = "Csillapítás: " + current_attenuation_number + " dB";
}

function decrement_voa_attenuation() {
    let current_pos = document.getElementById("voa_attenuation").innerHTML;
    let current_pos_int = parseInt(current_pos.match(/\d/g).join(""));

    if (current_pos_int > 0) {
        current_pos_int = current_pos_int - voa_attenuation_delta;
    }

    document.getElementById("voa_attenuation").innerHTML = "Csillapítás: " + current_pos_int + " dB";
}

function reset_voa_attenuation() {
    document.getElementById("voa_attenuation").innerHTML = "Csillapítás: " + 0 + " dB";
}

