#include <Arduino.h>
#include <TrackerMove.h>
#include <WiFiLogger.h>
#include <driver/timer.h>

#define SENSOR_NE 33
#define SENSOR_NW 34
#define SENSOR_SE 32
#define SENSOR_SW 35

#define WIFI_LED 3 // Pin diody LED Wi-Fi

TrackerMove tracker;
bool trackingEnabled = false;
// volatile bool timerFlag = false;
String startupLog; // Bufor na logi startowe


// Deklaracja funkcji
void sensorRead(int& ne, int& nw, int& se, int& sw);
int performTrackingAzimuth();
int performTrackingElevation();

void setup() {
  Logger.begin("SolarTracker", "SolarTracker");
  tracker.begin();

  pinMode(WIFI_LED, OUTPUT);
  digitalWrite(WIFI_LED, HIGH); // Wyłącz diodę LED Wi-Fi

  pinMode(SENSOR_NE, INPUT);
  pinMode(SENSOR_NW, INPUT);
  pinMode(SENSOR_SE, INPUT);
  pinMode(SENSOR_SW, INPUT);
}

void loop() {
  Logger.handleClient();

  String cmd = Logger.readCommand();
  if (cmd == "homing") {
    Logger.println("Wykonywanie homingu...");
    trackingEnabled = false;
    tracker.homing();
  } else if (cmd == "start") {
    Logger.println("Tryb śledzenia aktywowany.");
    trackingEnabled = true;
  } else if (cmd == "stop") {
    Logger.println("Tryb śledzenia dezaktywowany.");
    trackingEnabled = false;
  } else if (cmd == "move") {
    if (trackingEnabled) {
      Logger.println("Dezaktywuj tryb śledzenia przed ręcznym ustawieniem pozycji trackera.");
      return;
    }
    Logger.println("Ręczne ustawienie pozycji trackera.\nPodaj azymut:");
    float azimuth = 0.0f;
    while (true) {
      String azStr = Logger.readCommand();
      if (azStr.length() > 0) {
        azimuth = azStr.toFloat();
        break;
      }
      delay(10);
    }

    Logger.println("Podaj elewację:");
    float elevation = 0.0f;
    while (true) {
      String elStr = Logger.readCommand();
      if (elStr.length() > 0) {
        elevation = elStr.toFloat();
        break;
      }
      delay(10);
    }
    tracker.moveTracker(azimuth, elevation);
  } else if (cmd == "status") {
    if (!trackingEnabled) {
      Logger.println("Tryb śledzenia jest wyłączony.");
    } else {
      Logger.println("Tryb śledzenia jest aktywny.");
    }
    Logger.print("Aktualna pozycja trackera: Azymut = ");
    Logger.print(String(tracker.getCurrentAzimuth()));
    Logger.print(", Elewacja = ");
    Logger.println(String(tracker.getCurrentElevation()));
  }

  const int hysteresis = 400; // Próg histerezy

  if (trackingEnabled) {
    int diffAzimuth = performTrackingAzimuth();
    int diffElevation = performTrackingElevation();

    // Azymut
    if(diffAzimuth > hysteresis) {
      tracker.moveAzimuth(tracker.getCurrentAzimuth() - 3);
    } else if (diffAzimuth < -hysteresis) {
      tracker.moveAzimuth(tracker.getCurrentAzimuth() + 3);
    } else if (diffAzimuth < hysteresis && diffAzimuth > -hysteresis) {
      // Nie ruszamy trackerem
      Logger.println("Azymut: Nie ruszamy trackerem"); 
          // Elewacja
      if(diffElevation > hysteresis) {
        tracker.moveElevation(tracker.getCurrentElevation() - 3);
      } else if (diffElevation < -hysteresis) {
        tracker.moveElevation(tracker.getCurrentElevation() + 3);
      } else if (diffElevation < hysteresis && diffElevation > -hysteresis) {
        // Nie ruszamy trackerem
        Logger.println("Elewacja: Nie ruszamy trackerem");
    }
    }
  }
  delay(1000); // Opóźnienie dla stabilności
}
    

void sensorRead(int& ne, int& nw, int& se, int& sw) {
  ne = analogRead(SENSOR_NE);
  nw = analogRead(SENSOR_NW);
  se = analogRead(SENSOR_SE);
  sw = analogRead(SENSOR_SW);

  Logger.print("SENSORY: NE: ");
  Logger.print(String(ne));
  Logger.print(", NW: ");
  Logger.print(String(nw));
  Logger.print(", SE: ");
  Logger.print(String(se));
  Logger.print(", SW: ");
  Logger.println(String(sw));
}

int performTrackingAzimuth() {
  int ne, nw, se, sw;
  sensorRead(ne, nw, se, sw);

  int east = (ne + se) / 2;
  int west = (nw + sw) / 2;
  return east - west;
}

int performTrackingElevation() {
  int ne, nw, se, sw;
  sensorRead(ne, nw, se, sw);

  int north = (ne + nw) / 2;
  int south = (se + sw) / 2;
  return north - south;
}