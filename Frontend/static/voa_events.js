const voa_attenuation_delta = 1

function post_call_voa(path, json_body, callback) {
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

function send_att_backend(atten) {
    let path = "/set_voa_module_attenuation/" + atten;
    post_call_voa(path, null, null);
}

function update_voa_pos(attenuation) {
    document.getElementById("voa_attenuation").innerHTML = "Csillapítás: " + attenuation + " dB";
    send_att_backend(attenuation)
}

function increment_voa_attenuation() {
    let current_attenuation = document.getElementById("voa_attenuation").innerHTML;
    let current_attenuation_number = parseInt(current_attenuation.match(/\d/g).join(""));

    if (current_attenuation_number < 15) {
        current_attenuation_number = current_attenuation_number + voa_attenuation_delta;
    }

    document.getElementById("voa_attenuation").innerHTML = "Csillapítás: " + current_attenuation_number + " dB";
    send_att_backend(current_attenuation_number)

}

function decrement_voa_attenuation() {
    let current_pos = document.getElementById("voa_attenuation").innerHTML;
    let current_pos_int = parseInt(current_pos.match(/\d/g).join(""));

    if (current_pos_int > 0) {
        current_pos_int = current_pos_int - voa_attenuation_delta;
    }

    document.getElementById("voa_attenuation").innerHTML = "Csillapítás: " + current_pos_int + " dB";
    send_att_backend(current_pos_int)
}

function reset_voa_attenuation() {
    document.getElementById("voa_attenuation").innerHTML = "Csillapítás: " + 0 + " dB";
    send_att_backend(0)
}

