import paho.mqtt.client as mqtt
import requests

host = "10.1.1.145"
port = 1883

def on_connect(self, client, userdata, rc):
    print("MQTT Connected.")
    self.subscribe("sensors")

def on_message(client, userdata,msg):
    print(msg.payload.decode("utf-8", "strict"))
    url = f'http://{host}:3000/sensors'
    myobj = {'message': msg.payload.decode("utf-8", "strict")}
    headers = {'Content-Type': 'application/json'} 
    r = requests.post(url, json=myobj, headers=headers)
 
    print(r)

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message
client.connect(host)
client.loop_forever()