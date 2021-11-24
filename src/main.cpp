#include <Arduino.h>
#include <Wifi.h>
#include <PubSubClient.h>
#include <driver/rtc_io.h>
#include <Bounce2.h>
#include <credentials.h>

/*
Deep Sleep with External Wake Up
=====================================
This code displays how to use deep sleep with
an external trigger as a wake up source and how
to store data in RTC memory to use it over reboots

This code is under Public Domain License.
*/

#define LED_PIN GPIO_NUM_5
#define SWITCH_PIN GPIO_NUM_15

unsigned long start_time; 
unsigned long current_time; // millis() function returns unsigned long

RTC_DATA_ATTR int lastSwitchState = 0;

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("keysafe")) {
      Serial.println("connected");
      client.publish("tele/keysafe/LWT", "Online");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 1 seconds");
      delay(1000);
    }
  }
}

void sendMessage(int switchState) {
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.println("Connecting to WiFi..");
    }
    Serial.println("Connected to the WiFi network");


    client.setServer(MQTT_SERVER, 1883);
    while (!client.connected()) {
      reconnect();
    }

    client.publish("stat/keysafe/SWITCH", switchState ? "ON" : "OFF");
    client.publish("tele/keysafe/LWT", "Offline");

    //Go to sleep now
    Serial.println("Giving the mqtt client a second to finisch processing");
    start_time = millis();
    current_time = millis();
    while (current_time - start_time < 1000) {
      current_time = millis();  // reset the timer
      client.loop();
    }
    client.disconnect();
}

void setup(){
  pinMode(SWITCH_PIN, INPUT_PULLUP);
  bool switchState = digitalRead(SWITCH_PIN);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, 0);
  
  Serial.begin(115200);
  delay(1000); //Take some time to open up the Serial Monitor

  esp_err_t result;
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();

  rtc_gpio_deinit(SWITCH_PIN);
  
  Serial.println((String)"Switch pin is " + switchState);

  result = rtc_gpio_init(SWITCH_PIN);
  Serial.println("rtc_gpio_init: "+ String(result));
  result = rtc_gpio_pullup_en(SWITCH_PIN);
  Serial.println("rtc_gpio_pullup_en: "+ String(result));
  result = rtc_gpio_pulldown_dis(SWITCH_PIN);
  Serial.println("rtc_gpio_pullup_en: "+ String(result));

  if (wakeup_reason == ESP_SLEEP_WAKEUP_UNDEFINED) {
    Serial.println("Not woken up from deepsleep");
    Serial.println("Will wake up, when switch state is " + String(!switchState));
  } else {
    if (switchState != lastSwitchState) {
      Serial.println("Switch state changed, sending message");
      sendMessage(switchState);
      Serial.println("Will wake up, when switch state is " + String(!switchState));
    } else {
      Serial.println("Switch state NOT changed");
    }
  }

  lastSwitchState = switchState;
  digitalWrite(LED_PIN, 1);

  Serial.println("Good night");
  esp_sleep_enable_ext0_wakeup(SWITCH_PIN, !switchState); //1 = High, 0 = Low
  esp_deep_sleep_start();
}

void loop(){
  //This is not going to be called
}