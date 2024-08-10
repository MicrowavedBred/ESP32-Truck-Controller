#include <WiFi.h>
#include <esp_task_wdt.h>

const char* ssid = "SSID NAME";
const char* password = "PASSWORD";

const int startButtonPin = 16;  // Pin for the start button
const int accessoryRelayPin = 25;  // Pin for the accessory relay
const int ignitionRelayPin = 33;  // Pin for the ignition relay
const int redLedPin = 19;  // Pin for the red LED
const int greenLedPin = 18;  // Pin for the green LED
const int blueLedPin = 5;  // Pin for the blue LED

// States for the truck's modes
enum TruckMode {
  OFF,
  ACCESSORY,
  IGNITION
};

TruckMode truckMode = OFF;

// Debouncing variables
int lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;  // 50ms debounce delay

// Timer variables
unsigned long buttonHoldTime = 0;
const unsigned long longPressThreshold = 1500;  // 1.5 seconds
unsigned long ignitionTimer = 0;
const unsigned long ignitionTimeout = 3000;  // 3 seconds
bool longPressHandled = false;

// Breathing variables
unsigned long breathingTimer = 0;
const unsigned long breathingPeriod = 2000;  // 2 seconds
float breathingPhase = 0;

// Flag to indicate if a phone is connected
bool phoneConnected = false;

void setup() {
  Serial.begin(115200);
  pinMode(startButtonPin, INPUT_PULLUP);
  pinMode(accessoryRelayPin, OUTPUT);
  pinMode(ignitionRelayPin, OUTPUT);
  pinMode(redLedPin, OUTPUT);
  pinMode(greenLedPin, OUTPUT);
  pinMode(blueLedPin, OUTPUT);

  digitalWrite(accessoryRelayPin, HIGH);
  digitalWrite(ignitionRelayPin, HIGH);

  // Set clock speed to 40 MHz
  setCpuFrequencyMhz(80);

  WiFi.softAP(ssid, password);
  Serial.println("AP started");
  Serial.println("IP address: ");
  Serial.println(WiFi.softAPIP());
}

void loop() {
  // Check if a phone is connected
  int numClients = WiFi.softAPgetStationNum();
  phoneConnected = numClients > 0;

 if (!phoneConnected && truckMode == OFF) {
    // Turn off LEDs
    analogWrite(redLedPin, 0);
    analogWrite(greenLedPin, 0);
    analogWrite(blueLedPin, 0);
    delay(1000);

  } else {
    // Read the start button state
    int buttonState = digitalRead(startButtonPin);

    // Check if the button state has changed
    if (buttonState != lastButtonState) {
      lastDebounceTime = millis();
      longPressHandled = false;
    }

    // Check if the button has been pressed for long enough
    if ((millis() - lastDebounceTime) > debounceDelay) {
      if (buttonState == LOW) {
        // Check if the button is being held for a long press
        if (buttonHoldTime == 0) {
          buttonHoldTime = millis();
        }
        if (millis() - buttonHoldTime > longPressThreshold && !longPressHandled) {
          // Long press detected
          if (truckMode == OFF) {
            truckMode = ACCESSORY;
            Serial.println("Mode changed to: ACCESSORY");
          } else if (truckMode == ACCESSORY) {
            truckMode = OFF;
            Serial.println("Mode changed to: OFF");
          }
          longPressHandled = true;
        }
      } else {
        // Button released, check if it was a short press
        if (buttonHoldTime != 0 && millis() - buttonHoldTime < longPressThreshold) {
          // Short press detected
          if (truckMode == OFF) {
            truckMode = IGNITION;
            Serial.println("Mode changed to: IGNITION");
            ignitionTimer = millis();
          } else if (truckMode == ACCESSORY) {
            truckMode = IGNITION;
            Serial.println("Mode changed to: IGNITION");
            ignitionTimer = millis();
          } else if (truckMode == IGNITION) {
            truckMode = OFF;
            Serial.println("Mode changed to: OFF");
          }
        }
        buttonHoldTime = 0;
      }
    }

    // Update the last button state
    lastButtonState = buttonState;

    // Update the breathing phase
    breathingPhase = (millis() - breathingTimer) / (float)breathingPeriod * 2 * PI;
    if (breathingPhase > 2 * PI) {
      breathingPhase -= 2 * PI;
      breathingTimer = millis();
    }

    // Control the relays and RGB LED based on the truck mode
    switch (truckMode) {
      case OFF:
        digitalWrite(accessoryRelayPin, HIGH);
        digitalWrite(ignitionRelayPin, HIGH);
        analogWrite(redLedPin, (int)(128 + 127 * sin(breathingPhase)));
        analogWrite(greenLedPin, 0);
        analogWrite(blueLedPin, 0);
        break;
      case ACCESSORY:
        digitalWrite(accessoryRelayPin, LOW);
        digitalWrite(ignitionRelayPin, HIGH);
        analogWrite(redLedPin, 0);
        analogWrite(greenLedPin, 0);
        analogWrite(blueLedPin, (int)(128 + 127 * sin(breathingPhase)));
        break;
      case IGNITION:
        digitalWrite(accessoryRelayPin, LOW);
        if (millis() - ignitionTimer < ignitionTimeout) {
          digitalWrite(ignitionRelayPin, LOW);
        } else {
          digitalWrite(ignitionRelayPin, HIGH);
        }
        analogWrite(redLedPin, 0);
        analogWrite(greenLedPin, (int)(128 + 127 * sin(breathingPhase)));
        analogWrite(blueLedPin, 0);
        break;
    }

    // Print the current mode
    static unsigned long lastPrintTime = 0;
    if (millis() - lastPrintTime > 1000) {
      lastPrintTime = millis();
      Serial.print("Current mode: ");
      switch (truckMode) {
        case OFF:
          Serial.println("OFF");
          break;
        case ACCESSORY:
          Serial.println("ACCESSORY");
          break;
        case IGNITION:
          Serial.println("IGNITION");
          break;
      }
    }

    // Print the phone connection status
    static unsigned long lastPhonePrintTime = 0;
    if (millis() - lastPhonePrintTime > 1000) {
      lastPhonePrintTime = millis();
      Serial.print("Phone connected: ");
      Serial.println(phoneConnected);

        Serial.println();
  Serial.print("CPU Frequency is: ");
  Serial.print(getCpuFrequencyMhz()); //Get CPU clock
  Serial.print(" Mhz");
  Serial.println();
    }
  }
}
