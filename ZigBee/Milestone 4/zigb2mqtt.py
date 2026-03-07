#------------------------------------
#       LIBRARIES
#------------------------------------

from digi.xbee.devices import XBeeDevice
from digi.xbee.io import IOLine, IOMode
import paho.mqtt.client as mqtt
from datetime import datetime

#------------------------------------
#       CONFIGURATIONS
#------------------------------------

MQTT_IP = '172.18.132.157'
MQTT_PORT = 1883
GROUP_NAME = "Group6"
DEVICE_ID = "Device6"

PORT = "COM10"
BAUD_RATE = 9600
REMOTE_NODE_ID = "END_DEVICE"
IO_SAMPLING_RATE = 5  # seconds

#------------------------------------
#       MQTT CALLBACKS
#------------------------------------

def on_connect(client, userdata, flags, rc):
    print("MQTT connected, rc =", rc)

def on_message(client, userdata, msg):
    print("MQTT message:", msg.topic, msg.payload.decode("utf-8", errors="replace"))

# ✅ CHANGE 1: Add publish confirmation callback (optional but great)
def on_publish(client, userdata, mid):
    print("MQTT published, mid =", mid)

#------------------------------------
#       MQTT CLIENT SETUP
#------------------------------------

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

# ✅ CHANGE 2: Register on_publish callback (optional)
client.on_publish = on_publish

client.connect(MQTT_IP, MQTT_PORT, 60)

# ✅ CHANGE 3 (IMPORTANT): Start MQTT loop so publish actually works
client.loop_start()

#------------------------------------
#       MAIN METHOD
#------------------------------------

def main():
    print(" +----------------------------------------------+")
    print(" | XBee Python Library Handle IO Samples Sample |")
    print(" +----------------------------------------------+\n")

    device = XBeeDevice(PORT, BAUD_RATE)

    try:
        device.open()

        xbee_network = device.get_network()
        remote_device = xbee_network.discover_device(REMOTE_NODE_ID)
        if remote_device is None:
            print("Could not find the remote device")
            return

        remote_device.set_dest_address(device.get_64bit_addr())
        remote_device.set_io_sampling_rate(IO_SAMPLING_RATE)

        remote_device.set_io_configuration(IOLine.DIO4_AD4, IOMode.DIGITAL_IN)
        remote_device.set_dio_change_detection({IOLine.DIO4_AD4})

        remote_device.set_io_configuration(IOLine.DIO2_AD2, IOMode.ADC)
        remote_device.set_io_configuration(IOLine.DIO3_AD3, IOMode.ADC)

        def io_samples_callback(sample, remote, time):
            sample_str = (
                str(sample)
                .replace(" ", "")
                .replace("{", "")
                .replace("}", "")
                .replace("[", "")
                .replace("]", "")
                .replace("IOLine.", "")
                .replace("IOValue.", "")
            )

            parts = sample_str.split(",")
            data = {}

            print(datetime.now())
            for s in parts:
                if ":" not in s:
                    continue
                k, v = s.split(":", 1)
                data[k] = v

            for key, val in data.items():
                topic = f"{GROUP_NAME}/{DEVICE_ID}/{key}"
                payload = str(val)

                print(f"    Publishing -> {topic} : {payload}")

                # ✅ CHANGE 4: Capture publish result + QoS 1 for confirmation
                result = client.publish(topic, payload, qos=1)

                # result.rc: 0 success, non-zero error
                if result.rc != 0:
                    print("    ❌ Publish failed, rc =", result.rc)

            print()

        device.add_io_sample_received_callback(io_samples_callback)

        while True:
            pass

    finally:
        if device is not None and device.is_open():
            device.close()

        # ✅ CHANGE 5: Stop MQTT cleanly
        client.loop_stop()
        client.disconnect()

if __name__ == '__main__':
    main()