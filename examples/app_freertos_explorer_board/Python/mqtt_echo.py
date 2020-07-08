#!\usr\bin\env python

import paho.mqtt.client as mqtt
import argparse

topic = 'echo'

def on_message(client, userdata, message):
    print('Got message:')
    print(message.payload)
    print('Publish to echo/b:')
    print(message.payload)
    client.publish('echo/b', payload=message.payload, qos=1, retain=False)
    return;


def run(args):
    client = mqtt.Client('host_client')

    client.user_data_set(args)
    client.on_message = on_message

    client.tls_set(ca_certs=args.cacert)
    client.tls_insecure_set(True)

    client.connect(args.broker, port=8883, keepalive=60, bind_address='')
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
    parser.add_argument('-ca', '--cacert', default='./../filesystem_support/echo_client_certs/server.pem', help='CA certificate')
    parser.add_argument('-c', '--cert', default='./../filesystem_support/echo_client_certs/client.pem', help='Client certificate')
    parser.add_argument('-k', '--key', default='./../filesystem_support/echo_client_certs/client.key', help='Client key')

    args = parser.parse_args()
    
    run(args)
