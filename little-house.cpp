#include <BuzzerESP32.h>
#include <ESP32Servo.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h>                              // Required for 16 MHz Adafruit Trinket
#endif
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <dht11.h>
#include "MFRC522_I2C.h"

// IIC pins default to GPIO21 and GPIO22 of ESP32
// 0x28 is the i2c address of SDA, if doesn't match，please check your address with i2c.
#define MFRC522_ADDRESS 0x28
MFRC522 mfrc522(MFRC522_ADDRESS);   // create MFRC522.

// LCD setup
#define LCD_ADDRESS 0x27
#define LCD_COLS    16
#define LCD_ROWS    2
LiquidCrystal_I2C mylcd(LCD_ADDRESS, LCD_COLS, LCD_ROWS); // Set the LCD I2C address

// Buzzer setup
#define BUZZER_PIN 25
BuzzerESP32 buzzer(BUZZER_PIN); // Initialize buzzer on GPIO25

Servo myservo;
dht11 DHT11; // Initialize dht11


// Type definition for fan direction
enum FanDirection {
  FORWARD,
  REVERSE
};

// Define RFID keys
const char* RFID_KEY_BLUE = "d91a009d";
const char* RFID_CARD_WHITE = "eb80b732";

// Pin definitions
#define btn1 16
#define btn2 27
#define led_yellow 12
#define person_sensor 14
#define window_servo 5
#define water 34
#define gas 23

#define LED_PIN 26 // Which pin on the Arduino is connected to the NeoPixels?
#define LED_COUNT 4 // How many NeoPixels are attached to the Arduino?
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800); // Declare our NeoPixel strip object:

#define fan_forward 19    // Fan control pin 1
#define fan_backward 18    // Fan control pin 2
int speed_val = 130;  // Initial fan speed (PWM value) 0 - 230

// const int DHT11PIN = 17; // Temperature and humidity sensor pin ||||| <--  TO DELETE
#define DHT11PIN 17 // Temperature and humidity sensor pin

// LEDC configuration for yellow LED
#define LED_YELLOW_FREQ    5000   // 5 kHz
#define LED_YELLOW_RES     8      // 8-bit resolution (0–255)

// Define the constants and variables
boolean gas_detected = false;
boolean window_gas_state = false; // false - closed, true - opened
String RFID_UID = "";

// functions declarations
void blindYellowLed();
void breathingYellowLed();
void playSound();
void setWindowServo();
void closeWindowServo();
void openWindowServo();

// NeoPixel function declarations
void colorWipe(uint32_t color, int wait);
void theaterChase(uint32_t color, int wait);
void rainbow(int wait);
void theaterChaseRainbow(int wait);
void setFan(int speed, FanDirection dir);
void stopFan();
void manageLeds(int mode);
void printLineWithScroll(int row, String text, unsigned long scrollDelay);
void setMessageLCD(String messages[], int count, unsigned long delayTime);
void ShowReaderDetails();
void RFIDReader();

void setup() {
  // 9600 
  // 115200 used to debug RFID reader
  Serial.begin(115200); // Uncomment this line to enable serial monitor
  Wire.begin();       // Initialize I2C bus | required for both LCD and MFRC522
 
  mfrc522.PCD_Init();   // Init MFRC522
  Serial.println("RFID Reader initialized");
  ShowReaderDetails();            // dispaly PCD - MFRC522 read carder
  Serial.println("Scan a card...");

  pinMode(btn1, INPUT);
  pinMode(btn2, INPUT);
  pinMode(led_yellow, OUTPUT);  //Set pin to output mode
  pinMode(person_sensor, INPUT); // 0 - no person, 1 - person detected
  setWindowServo();
  pinMode(water, INPUT);

  #if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
    clock_prescale_set(clock_div_1); // These lines are specifically to support the Adafruit Trinket 5V 16 MHz.
  #endif
  strip.begin();                   // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();                    // Turn OFF all pixels ASAP
  strip.setBrightness(50);         // Set BRIGHTNESS to about 1/5 (max = 255)

  pinMode(fan_forward, OUTPUT);
  pinMode(fan_backward, OUTPUT);

  mylcd.init();
  mylcd.backlight();

  pinMode(gas, INPUT);
}

void loop() {
  // Serial.println("Printing on Serial Monitor:");
  ledcAttach(led_yellow, LED_YELLOW_FREQ, LED_YELLOW_RES);

  boolean btn1_val = digitalRead(btn1);
  boolean btn2_val = digitalRead(btn2);
  boolean person_sensor_val = digitalRead(person_sensor);
  int water_sensor_val = analogRead(water);
  boolean gas_val = digitalRead(gas);
   //Define two temperature and humidity values
   int Temperature;
   int Humidity;
   //Obtain data
   int chk = DHT11.read(DHT11PIN);
   Temperature = DHT11.temperature;
   Humidity = DHT11.humidity;

  RFIDReader();

  // code logic implementation
  if (btn1_val == 0) {
    blindYellowLed();
    setFan(speed_val, FORWARD);
  }
  else if (btn2_val == 0) {
    breathingYellowLed();
    stopFan();
  }

  if (person_sensor_val == 1) {

  }
  else {

  }

  if (gas_val == 0) {
    gas_detected = true;
    manageLeds(5); // Magenta
    setFan(speed_val, REVERSE);
    playSound();
    window_gas_state = true;
    openWindowServo();
  }
  else if (gas_detected == true) {
    gas_detected = false;
    manageLeds(0); // Off
    stopFan();
    if (window_gas_state == true) {
      closeWindowServo();
      window_gas_state = false;
    }
  }

  if (RFID_UID.equals(RFID_KEY_BLUE) || RFID_UID.equals(RFID_CARD_WHITE)) {
      String messages[] = {
      person_sensor_val ? "Presence: Yes" : "Presence: No",
      gas_val ? "Gas: Safe" : "Gas: Leak",
      "Water: " + String(water_sensor_val),
      "Temperature: " + String(Temperature) + "C",
      "Humidity: " + String(Humidity) + "%"
    };
    setMessageLCD(messages, 5, 2000);
  }

}

// HELPER FUNCTIONS
void blindYellowLed()
{
  ledcWrite(led_yellow, 255); // ON
  delay(200);
  ledcWrite(led_yellow, 0);   // OFF
  delay(200);
}

void breathingYellowLed()
{
  for(int i = 0; i < 255; i++)  // For loop: increment variable i until it reaches 255
  {
    ledcWrite(led_yellow, i); // ON
    delay(3);
  }

  for(int i = 255; i > 0; i--)  // For loop: decrement variable i until it reaches 0
  {
    ledcWrite(led_yellow, i);
    delay(3);
  }
  ledcWrite(led_yellow, 0);   // OFF
}

void playSound()
{
  buzzer.setTimbre(30); // Set timbre (sound quality)
  buzzer.playTone(300, 250);  // D4
  buzzer.playTone(0, 0);      // Turn off buzzer
  delay(50);

  buzzer.setTimbre(50); // Set timbre (sound quality)
  buzzer.playTone(300, 250);  // E4
  delay(50);

  buzzer.setTimbre(70); // Set timbre (sound quality)
  buzzer.playTone(400, 250);  // F4
  delay(50);

  buzzer.playTone(0, 0);      // Turn off buzzer

}

// Window servo functions
void setWindowServo() {
  // Allow allocation of all timers
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  myservo.setPeriodHertz(50);    // standard 50 hz servo
  myservo.attach(window_servo, 1000, 2000); // attaches the servo on pin 18 to the servo object
  // using default min/max of 1000us and 2000us
  // different servos may require different min/max settings
  // for an accurate 0 to 180 sweep
  delay(200); // waits for the servo to get ready
}

void closeWindowServo() {
  myservo.write(0);              // tell servo to go to position in variable 'pos'
  delay(1000);                   // waits 1 second for the servo to reach the position
}

void openWindowServo() {
  myservo.write(176);             // tell servo to go to position in variable 'pos'
  delay(1000);                   // waits 1 second for the servo to reach the position
}

// NeoPixel functions 
void colorWipe(uint32_t color, int wait) {
  for(int i=0; i<strip.numPixels(); i++) {           // For each pixel in strip...
    strip.setPixelColor(i, color);                   // Set pixel's color (in RAM)
    strip.show();                                    // Update strip to match
    delay(wait);                                     // Pause for a moment
  }
}

void theaterChase(uint32_t color, int wait) {
  for(int a=0; a<10; a++) {                         // Repeat 10 times...
    for(int b=0; b<3; b++) {                        // 'b' counts from 0 to 2...
      strip.clear();                                // Set all pixels in RAM to 0 (off)
      for(int c=b; c<strip.numPixels(); c += 3) {    // 'c' counts up from 'b' to end of strip in steps of 3...
        strip.setPixelColor(c, color);               // Set pixel 'c' to value 'color'
      }
      strip.show();                                 // Update strip with new contents
      delay(wait);                                  // Pause for a moment
    }
  }
}

void rainbow(int wait) {
  for(long firstPixelHue = 0; firstPixelHue < 5*65536; firstPixelHue += 256) {
    for(int i=0; i<strip.numPixels(); i++) {        // For each pixel in strip...
      int pixelHue = firstPixelHue + (i * 65536L / strip.numPixels());
      strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue)));
    }
    strip.show();                                   // Update strip with new contents
    delay(wait);                                   // Pause for a moment
  }
}

void theaterChaseRainbow(int wait) {
  int firstPixelHue = 0;                           // First pixel starts at red (hue 0)
  for(int a=0; a<30; a++) {                        // Repeat 30 times...
    for(int b=0; b<3; b++) {                       // 'b' counts from 0 to 2...
      strip.clear();                               // Set all pixels in RAM to 0 (off)
      for(int c=b; c<strip.numPixels(); c += 3) {  // 'c' counts up from 'b' to end of strip in increments of 3...
        int      hue   = firstPixelHue + c * 65536L / strip.numPixels();
        uint32_t color = strip.gamma32(strip.ColorHSV(hue)); // hue -> RGB
        strip.setPixelColor(c, color);             // Set pixel 'c' to value 'color'
      }
      strip.show();                               // Update strip with new contents
      delay(wait);                               // Pause for a moment
      firstPixelHue += 65536 / 90;               // One cycle of color wheel over 90 frames
    }
  }
}

void manageLeds(int mode) {  
  switch(mode)
  {
    case 0: colorWipe(strip.Color(0,   0,   0), 50); break;      // Off
    case 1: colorWipe(strip.Color(255,  0,   0), 50); break;     // Red
    case 2: colorWipe(strip.Color(0,   255,   0), 50); break;    // Green
    case 3: colorWipe(strip.Color(0,   0,   255), 50); break;    // Blue
    case 4: colorWipe(strip.Color(255, 255,   0), 50); break;    // Yellow
    case 5: colorWipe(strip.Color(255, 0,   255), 50); break;    // Magenta
    case 6: colorWipe(strip.Color(255, 255, 255), 50); break;    // White
    case 7: theaterChase(strip.Color(127, 127, 127), 50); break; // White, half brightness
    case 8: theaterChase(strip.Color(127,   0,   0), 50); break; // Red, half brightness
    case 9: theaterChase(strip.Color(  0,   0, 127), 50); break; // Blue, half brightness
    case 10: rainbow(10); break;                                 // Flowing rainbow cycle along the whole strip
    case 11: theaterChaseRainbow(50); break;                     // Rainbow-enhanced theaterChase variant
  }
}

// Fan control functions
void setFan(int speed, FanDirection dir)
{
  speed = constrain(speed, 0, 255);
  stopFan();

  if(dir == FORWARD)
  {
    digitalWrite(fan_forward, LOW);
    analogWrite(fan_backward, speed);
  }
  else
  {
    digitalWrite(fan_backward, LOW);
    analogWrite(fan_forward, speed);
  }
}

void stopFan()
{
  analogWrite(fan_forward, 0);
  analogWrite(fan_backward, 0);
}

// LCD display function
void printLineWithScroll(int row, String text, unsigned long scrollDelay)
{
  mylcd.setCursor(0, row);

  if(text.length() <= LCD_COLS)
  {
    mylcd.print(text);
    mylcd.print("                "); // Clear rest
  }
  else
  {
    for(int i = 0; i <= text.length() - LCD_COLS; i++)
    {
      mylcd.setCursor(0, row);
      mylcd.print(text.substring(i, i + LCD_COLS));
      delay(scrollDelay);
    }
  }
}

void setMessageLCD(String messages[], int count, unsigned long delayTime)
{
  mylcd.clear();

  // 1 message → fixed
  if(count == 1)
  {
    printLineWithScroll(0, messages[0], delayTime);
    return;
  }

  // 2 messages → fixed
  if(count == 2)
  {
    printLineWithScroll(0, messages[0], delayTime);
    printLineWithScroll(1, messages[1], delayTime);
    return;
  }

  // More than 2 → show pages of 2
  for(int i = 0; i < count; i += 2)
  {
    mylcd.clear();

    printLineWithScroll(0, messages[i], delayTime);

    if(i + 1 < count)
      printLineWithScroll(1, messages[i + 1], delayTime);

    delay(delayTime);
  }
}

// RFID reader details function
void RFIDReader() {
  RFID_UID = "";
  // Look for new card
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  // Select the card
  if (!mfrc522.PICC_ReadCardSerial()) {
    // set a message on lcd 
    setMessageLCD(
      (String[]){"Error reading", "RFID card"}, 2, 2000
    );
    return;
  }

  // Print UID
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    if (mfrc522.uid.uidByte[i] < 0x10) {
      // Append leading zero for single digit hex values
      RFID_UID += "0";
    }
    // Append UID byte to RFID_UID string
    RFID_UID += String(mfrc522.uid.uidByte[i], HEX);
  }
  // Serial.println("Card UID: " + RFID_UID);

  // Normalize UID to lowercase to ensure consistent comparisons
  RFID_UID.toLowerCase();

  // Halt PICC and stop encryption
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}

void ShowReaderDetails() {
  byte v = mfrc522.PCD_ReadRegister(mfrc522.VersionReg);
  Serial.print("MFRC522 Version: 0x");
  Serial.println(v, HEX);

  if (v == 0x00 || v == 0xFF) {
    Serial.println("WARNING: RFID reader not detected!");
  }
}
