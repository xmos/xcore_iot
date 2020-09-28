# XCOREAI Explorer Board Demo for AWS IoT Core
The XMOS **XCOREAI Explorer Board** provides connectivity to Amazon IoT Core.

Combined with Alexa Voice Service (AVS) and AWS Lambda, this demo allows you to quickly control GPIO using a custom Alexa Skill.  This example provides instructions on how to configure the development board, create your Alexa skill, and tie an Alexa skill intent to AWS IoT Core using a Lambda function.

To find out more, visit:
- [IoT Core](https://docs.aws.amazon.com/iot/?id=docs_gateway)
- [Lambda](https://docs.aws.amazon.com/lambda/?id=docs_gateway)
- [Alexa Skills Kit](https://developer.amazon.com/en-US/docs/alexa/ask-overviews/build-skills-with-the-alexa-skills-kit.html)

## Prerequisites
This document assumes familiarity with the XMOS xCORE architecture, FreeRTOS, MQTT, TLS, and the xC language.

You will need:
- **XCOREAI Explorer Board**
- WiFi access point with internet connectivity

You will also need an Amazon Developer account: https://developer.amazon.com

## Hardware setup
Setup your hardware by attaching the xTAG to the Explorer board and providing power.

## Example App and Cloud Setup
Brief instructions are below.  Refer to the Amazon documentation for the latest information concerning IoT Core, Lambda, and AVS.

#### IoT Core Setup
In this section, we will set up the device credentials and security policies.  For those unfamiliar with IoT Core, more detail on some steps can be found in this [AWS tutorial](https://docs.aws.amazon.com/iot/latest/developerguide/iot-moisture-setup.html)
1. Sign in to the [IoT Core console](https://console.aws.amazon.com/iot/home) with your developer account credentials.
2. In the navigation pane, expand **Secure** and select **Policies**.
3. Create a new policy, with a name of your choosing and the following parameters:
> Action: iot:* <br/>
> Resource: * <br/>
> Effect: Allow <br/>

    *Note: This policy should NOT be used outside of this demo.  Refer to [AWS documentation](https://docs.aws.amazon.com/iot/latest/developerguide/iot-policies.html) to setup policies where devices have appropriate access.*
4. In the navigation pane, expand **Manage** and select **Things**.
5. Create a new single Thing and provide a name.  Type, Group, and attributes are not required.  Press **Next**.
6. Press **Create certificate** in the generate **One-click certificate creation** section.  Download the thing certificate, private key, and CA certificate as you will need them later.  Refer to [AWS documentation](https://docs.aws.amazon.com/iot/latest/developerguide/server-authentication.html?icmpid=docs_iot_console#server-authentication-certs) if unsure which CA to download.  Activate the certificate when prompted.  Attach the policy created in step 3 and register the new Thing.
7. In the navigation pane, select **Settings** and take note of your endpoint URL.
> [*prefix*].iot.[*endpoint location*].amazonaws.com

#### Lambda Function setup
In this section, we will set up a Lambda function to bridge the gap between AVS and AWS IoT Core.
1. Sign in to the [Lambda console](https://console.aws.amazon.com/lambda/home) with your developer account credentials.
2. In the navigation pane, select **Functions**.
3. Select **Create function** and create a new function using **Author from scratch**, desired runtime, Node.js 10.x in this example, and select **Create a new role with basic Lambda permissions**.
4. Select the **Permissions** tab and click the link to the role created in the **Execution role** section.  This will open up the IAM management console.
5. In the **Permissions** tab under **Permissions policies** press **Attach policies**.  Search for AWSIoTFullAccess and attach this policy.

    *Note: Refer to [AWS IAM Documentation](https://docs.aws.amazon.com/IAM/latest/UserGuide/introduction.html) to adjust this policy to end user specific needs.*
6. Return back to the Lambda function page.  In the navigation pane, select **Configuration**.
7. In the **Function code** section, select the **Actions** tab and **Upload a .zip file**.  Browse and upload the ExplorerBoardControlLambda.zip found in this example app folder.
8. Once imported, update iotData with your AWS IoT Core endpoint address:

    ```var iotData = new AWS.IotData({endpoint:'[prefix].iot.[endpoint location].amazonaws.com'});```

Please proceed to the Alexa Skill Setup section, as you will need to create a new Alexa Skill first, to complete the Lambda function.

9. Select **Add trigger**.  Expand the trigger selection and select **Alexa Skills Kit**.  Paste in your Alexa Skill ID.
10. In the **Function code** section, update APP_ID with your Alexa Skill ID:

    ```var APP_ID = 'amzn1.ask.skill.[your skill id]]';```

11. Copy the Lambda function ARN, which should be located at the top right.  It will be of the format:

    ```arn:aws:lambda:[region]:[idnumber]:function:[function name]]```

Proceed to the Alexa Skill Setup, step 5.

#### Alexa Skill Setup
In this section, we will set up the Alexa skill.  For those unfamiliar with AVS, documentation can be found [here](https://developer.amazon.com/en-US/docs/alexa/ask-overviews/build-skills-with-the-alexa-skills-kit.html).
1. Sign in to the [AVS console](https://developer.amazon.com/alexa/console/ask) with your developer account credentials.
2. Select **Create Skill**, provide a name, choose the **Custom model**, and **Provision your own**.
3. Select the **Hello World Skill** template and create the skill.
4. In the navigation pane, select **Endpoint**.  Copy the value of the **Your Skill ID** section.

Return back to the Lambda setup section, starting at step 9.

5. In the **Endpoint** configuration, in the **AWS Lambda ARN** section, paste your Lambda function ARN into the **Default Region**.  Select **Save Endpoints**.
6. In the navigation pane, expand the **Interaction Model** menu and go to **JSON Editor**.  Drag and drop the .json file found in the aws_resources folder in this example directory.  This will populate the invocation and intents for the skill.
7. In the top pane press **Save Model** followed by **Build Model**.

#### Testing Alexa to IoT Core
In this section, we will verify the AVS to IoT Core setup.
1. Sign in to the [IoT Core console](https://console.aws.amazon.com/iot/home) with your developer account credentials.
2. In the navigation pane, select **Test**.
3. Set the **Subscription topic** to ```explorer/ledctrl``` and press **Subscribe to topic**.  Keep this window open, as you will be referencing it later to verify the Alexa skill and Lambda functionality.
4. Sign in to the [AVS console](https://developer.amazon.com/alexa/console/ask) with your developer account credentials, and open your skill.
5. In the top navigation pane, select **Test**.
6. In the **Skill testing is enabled in** dropdown, select **Development**.
7. From here you can type or use a microphone.  Submit the query ```"Alexa open x. core a. i."```.  Alexa should respond with ```"Hello. How may I help you? "```.  From here you can request things like ```"Turn on led zero"```, ```"Turn off led two"```, etc.  In the IoT Core Console window, opened in step 3, you should see the MQTT messages, such as:
```
{
      "LED": "0",
      "status": "on"
}
```

## Prerequisites for building

[XMOS Toolchain 15.0.1](https://www.xmos.com/software/tools/) or newer.

Install [CMake](https://cmake.org/download/) version 3.10 or newer.

Set environment variable for the XMOS AIoT SDK:

    > export XMOS_AIOT_SDK_PATH=<path to>/aiot_sdk

#### Explorer Board Configuration and Build
In this section, we will configure the demo software to connect to the proper MQTT broker.
1. In appconf.h, set appconfMQTT_HOSTNAME to your IoT endpoint URL:

    ```#define appconfMQTT_HOSTNAME "[prefix].iot.[endpoint location].amazonaws.com"```

2. In the example application root directory, run:

        > mkdir build
        > cd build
        > cmake ../ -DBOARD=XCORE-AI-EXPLORER
        > make -j

    This will create the iot_aws.xe binary in the bin folder.

3. Before the application can be run, the flash must be populated with a filesystem containing the crypto credentials for TLS and the WiFi connection details.  From the example application root directory, go to filesystem_support.  Paste the certificates and keys downloaded in the **IoT Core** setup of your **Thing** into the ```aws``` folder.  Rename the CA certificate file ```ca.pem``` , the device certificate file ```client.pem```, and the device private key file ```client.key```.  Run the Python script wifi_profile.py to create the WiFi configuration file.
4. With the development board and xTag connected, run the flash_image.sh bash script.  This will create a filesystem and flash it to the device.
    *Note: This script requires sudo, as it mounts and unmounts a disk.*


## Running the Demo
To run the demo navigate to the bin folder and use the command:

```xrun iot_aws.xe```

For debug output use:

```xrun --xscope iot_aws.xe```
