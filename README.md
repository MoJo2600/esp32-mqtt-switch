# ESP32 MQTT Switch

Simple script that pushes a message to an MQTT topic when the state of a switch changes. In the meantime the ESP32 will got to deepsleep and wait for the next event. Depending on the type of battery you are going to use, the system will last for month.

You can put a ESP32 module connected to a switch inside your letter box and receive a notification when someone opens the lid.

## TODO

[] Send the battery voltage with the request

## Configuration

Change the settings in `credentials.h` and then build the firmware and flash it to your ESP32.