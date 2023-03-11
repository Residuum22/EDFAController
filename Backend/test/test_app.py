from fastapi.testclient import TestClient
from app.app import app

client = TestClient(app)

def test_laser1_send_data():
    with client.websocket_connect("/laser1_data") as websocket:
        websocket.send_text("53")
        data = websocket.receive_text()
        assert data == "53"
        
def test_laser2_send_data():
    with client.websocket_connect("/laser2_data") as websocket:
        websocket.send_text("53")
        data = websocket.receive_text()
        assert data == "53"
        
def test_laser3_send_data():
    with client.websocket_connect("/laser3_data") as websocket:
        websocket.send_text("53")
        data = websocket.receive_text()
        assert data == "53"
        
def test_voa_send_data():
    with client.websocket_connect("/voa_data") as websocket:
        websocket.send_text("53")
        data = websocket.receive_text()
        assert data == "53"