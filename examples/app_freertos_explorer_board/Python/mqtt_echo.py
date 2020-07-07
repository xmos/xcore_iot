#!\usr\bin\env python

import paho.mqtt.client as mqtt
import argparse

topic = 'echo'

def on_message(client, userdata, message):
    print('Got message:')
    print(message.payload)
    client.publish('echo/b', payload=message.payload, qos=0, retain=False)
    return;


def run(args):
    client = mqtt.Client('host_client')

    client.user_data_set(args)
    client.on_message = on_message

    client.connect(args.broker, port=1883, keepalive=60, bind_address='')
    client.subscribe('echo/a', qos=0)

    try:
        client.loop_forever()
    except KeyboardInterrupt:
        print('Quitting')

    finally:
        client.unsubscribe(topic)
        client.disconnect()


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('-b', '--broker', default='localhost', help='MQTT Broker host')

    args = parser.parse_args()
    
    run(args)
