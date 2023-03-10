from fastapi.testclient import TestClient
from app.app import app

client = TestClient(app)



def test_laser1_send_data():
    with client.websocket_connect("/laser1_data") as websocket:
        websocket.send_text("53")
        data = websocket.receive_text()
        assert data == "53"