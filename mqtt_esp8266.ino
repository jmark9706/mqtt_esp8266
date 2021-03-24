/*
 Basic ESP8266 MQTT example
 This sketch demonstrates the capabilities of the pubsub library in combination
 with the ESP8266 board/library.
 It connects to an MQTT server then:
  - publishes "hello world" to the topic "outTopic" every two seconds
  - subscribes to the topic "inTopic", printing out any messages
    it receives. NB - it assumes the received payloads are strings not binary
  - If the first character of the topic "inTopic" is an 1, switch ON the ESP Led,
    else switch it off
 It will reconnect to the server if the connection is lost using a blocking
 reconnect function. See the 'mqtt_reconnect_nonblocking' example for how to
 achieve the same result without blocking the main loop.
 To install the ESP8266 board, (using Arduino 1.6.4+):
  - Add the following 3rd party board manager under "File -> Preferences -> Additional Boards Manager URLs":
       http://arduino.esp8266.com/stable/package_esp8266com_index.json
  - Open the "Tools -> Board -> Board Manager" and click install for the ESP8266"
  - Select your ESP8266 in "Tools -> Board"
   Has LED output on pins 12, 13
   Note: currently only the yellow led on pin 13, trying to figure out the failure
   of the I2C data at times.  It is definitely caused by using pin 12 but it also
   may be due to other things.  Without 12 being used, it also fails after several hours.
   Note: added a yield() in the loop code.
*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <SparkFunBME280.h>

#define  BLUE 12
#define YELLOW 13
// Update these with values suitable for your network.

const char* ssid = "cirrus";
const char* password = "7138982048";
const char* mqtt_server = "mqtt.bekinected.com";
const char* SUB_TOPIC = "environ/sub";
const char* PUB_TOPIC = "environ/test";
 String humidity, baro, alt, temp, device;
 float baro_f;
 char msg[70];
 char * mp = msg;
 char uarr[40];
 int uleng;
WiFiClient espClient;
PubSubClient client(espClient);
 BME280 mySensor;
 
unsigned long lastMsg = 0;
 unsigned long now;
#define MSG_BUFFER_SIZE	(50)
// char msg[MSG_BUFFER_SIZE];
int value = 0;

// setup wifi
void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

// initialize I2C
void setup_I2C()
{
  Serial.println("Setup I2C");
 int sda = 5;
  int scl = 4;
  Wire.begin(sda, scl);

   while (mySensor.beginI2C() == false) //Begin communication over I2C
  {
    Serial.println("The sensor did not respond. Please check wiring.");
    delay (2000);
   // Serial.println("jump to elfin");
   // goto elfin2;
  }
}

// MQTT callback
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
 //   digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)
     digitalWrite(YELLOW, LOW);
  } else {
 //   digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
     digitalWrite(YELLOW, HIGH);
  }

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
   // String clientId = "ESP8266Client-";
  //  clientId += String(random(0xffff), HEX);
    String clientId = "Device_02";
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.print(clientId);
      Serial.println(" client connected");
      // get sensor readings
      read_sensors();
      format_payload();
      // Once connected, publish an announcement...
      client.publish(PUB_TOPIC, mp);
      // ... and resubscribe
      client.subscribe(SUB_TOPIC);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
// Read the sensors
void read_sensors()
{
   Serial.print("Humidity: ");
  humidity = mySensor.readFloatHumidity();
  Serial.print(mySensor.readFloatHumidity(), 0);
  baro = mySensor.readFloatPressure();
  Serial.print(" Pressure: ");
  Serial.print(mySensor.readFloatPressure(), 0);
  alt = mySensor.readFloatAltitudeMeters();
  Serial.print(" Alt: ");
  //Serial.print(mySensor.readFloatAltitudeMeters(), 1);
  Serial.print(mySensor.readFloatAltitudeFeet(), 1);
  temp = mySensor.readTempF();
  Serial.print(" Temp: ");
  //Serial.print(mySensor.readTempC(), 2);
  Serial.print(mySensor.readTempF(), 2);
  Serial.println();
}
void format_payload(){
      String humid = String(humidity);
      int hleng = humid.length() + 1;
      char harr[hleng];
      humid.toCharArray(harr,hleng);
     String dt =  String(temp);
     int sleng = dt.length()+1;
     char tarr[sleng];
     dt.toCharArray(tarr,sleng);
     // convert pressure to float
     baro_f = baro.toFloat();
     baro_f = baro_f / 100.;
     String baro_z = String(baro_f,0);
     int balng = baro_z.length() + 1;
     char barr[balng];
     baro_z.toCharArray(barr, balng);
     strcpy(mp,"temperature: ");
     strcat(mp,tarr);
     strcat(mp," humidity: ");
     strcat(mp,harr);
     strcat (mp,"%");
     strcat (mp, " Barometer: ");
     strcat (mp, barr);
     strcat (mp," mbar");
}
// SETUP starts here ===================================
void setup() {
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  pinMode(BLUE, OUTPUT);
  pinMode (YELLOW, OUTPUT);
  Serial.begin(115200);
  setup_wifi();
  setup_I2C();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  now = millis();
}

void loop() {
 // digitalWrite(YELLOW, HIGH);
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;
    ++value;
   // snprintf (msg, MSG_BUFFER_SIZE, "hello world #%ld", value);
  // digitalWrite(YELLOW, LOW);
   read_sensors();
   format_payload();
    Serial.print("Publish message: ");
    Serial.println(mp);
    client.publish(PUB_TOPIC, mp);
    yield();
    
  }
}
