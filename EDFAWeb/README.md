# EDFA_WEB

## Getting started

### Mosquitto MQTT server

For the project, the Mosquitto MQTT must be installed on your system and it needs to run in the background, because the
web and the embedded devices are communication throught that.

### Backend prerequisites

`$ pip install -r Backend/requirements.txt`

### Frontend prerequisites

To create the dependencies for the project npm must be installed. 

`$ sudo apt intall npm`

After the installation of the package manager. You need to navigate to the `Frontend` folder and run `npm install` command.

Because the backend will run on your local machine in your local network, you need to configure the 

To start webserver run this command:


UNIX
```bash
python3 ./main.py
```

Windows
```bash
py ./main.py
```

