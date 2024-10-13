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

function send_voa_desired_current() {
    let current_attenuation = document.getElementById("voa_attenuation").value;
    let current_attenuation_number = parseFloat(current_attenuation);

    if (current_attenuation_number <= 15) {
        current_attenuation_number = current_attenuation_number;
        send_att_backend(current_attenuation_number)
    }
    else
    {
        window.alert(`A VOA csillapítás csak 0 és 15 dB között lehet!`);
    }
}

function reset_voa_attenuation() {
    document.getElementById("voa_attenuation").innerHTML = "Csillapítás: " + 0 + " dB";
    send_att_backend(0)
}

