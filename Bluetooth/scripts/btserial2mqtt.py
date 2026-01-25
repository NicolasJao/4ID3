import serial
import json
import time
import paho.mqtt.client as mqtt

# -----------------------------
# Configuration
# -----------------------------
mqtt_ip = input("MQTT Broker IP: ").strip()
if not mqtt_ip:
    mqtt_ip = "172.18.129.252"

mqtt_port = input("MQTT Broker Port: ").strip()
if not mqtt_port:
    mqtt_port = 1883
else:
    mqtt_port = int(mqtt_port)

bluetooth_com_port = input("Bluetooth COM (e.g. COM6): ").strip()
if not bluetooth_com_port:
    bluetooth_com_port = "COM6"

print("\n-------")
print("CONFIGURATION")
print("-------")
print(f"IP: {mqtt_ip}")
print(f"PORT: {mqtt_port}")
print(f"BT COM: {bluetooth_com_port}")
print("-------\n")

# -----------------------------
# MQTT Callbacks
# -----------------------------
def on_connect(client, userdata, flags, rc):
    print(f"Connected to MQTT, status = {rc}")

def on_disconnect(client, userdata, rc):
    print("Disconnected from MQTT")

# -----------------------------
# MQTT Setup
# -----------------------------
client = mqtt.Client(protocol=mqtt.MQTTv311)
client.on_connect = on_connect
client.on_disconnect = on_disconnect

client.connect(mqtt_ip, mqtt_port, 60)
client.loop_start()

print(f"Connected to MQTT Broker at {mqtt_ip}:{mqtt_port}")

# -----------------------------
# Serial (Bluetooth) Setup
# -----------------------------
print(f"Connecting to Bluetooth COM {bluetooth_com_port}...")
time.sleep(1)

ser = serial.Serial(
    bluetooth_com_port,
    baudrate=9600,
    timeout=1
)

print(f"Bluetooth COM {bluetooth_com_port} opened")

# -----------------------------
# Main Loop
# -----------------------------
try:
    while True:
        frame = ser.readline().decode("utf-8", errors="ignore").strip()
        frame = frame.replace("\x00", "")  # remove Bluetooth NULL bytes

        if not frame:
            continue   # ignore empty lines cleanly

        print("FRAME:", repr(frame))

        try:
            sensor_data = json.loads(frame)
        except json.JSONDecodeError:
            print("Failed to parse JSON:", frame)
            continue

        # Expected JSON structure:
        # { "GroupA": { "DeviceA": { "Temp": "...", "Humidity": "...", "Luminosity": "..." } } }

        group_name = next(iter(sensor_data))
        device_id = next(iter(sensor_data[group_name]))

        for key, val in sensor_data[group_name][device_id].items():
            topic = f"{group_name}/{device_id}/{key}"
            client.publish(topic, str(val), qos=1)
            print(f"MQTT → {topic} = {val}")

        # Let MQTT process network traffic
        client.loop(timeout=0.01)

except KeyboardInterrupt:
    print("\nExiting...")

finally:
    ser.close()
    client.loop_stop()
    client.disconnect()
    print("Clean shutdown complete.")
