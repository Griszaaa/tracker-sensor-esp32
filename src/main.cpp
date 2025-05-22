#include <Arduino.h>
#include <TrackerMove.h>
#include <WiFiLogger.h>
#include <driver/timer.h>

#define SENSOR_NE 33
#define SENSOR_NW 34
#define SENSOR_SE 32
#define SENSOR_SW 35

TrackerMove tracker;
bool trackingEnabled = false;
// volatile bool timerFlag = false;

// Konfiguracja timera
// const uint64_t interval_us = 5000000; // 10 sekund w mikrosekundach
// timer_config_t timerConfig = {
//     .alarm_en = TIMER_ALARM_EN,
//     .counter_en = TIMER_PAUSE,
//     .intr_type = TIMER_INTR_LEVEL,
//     .counter_dir = TIMER_COUNT_UP,
//     .auto_reload = TIMER_AUTORELOAD_EN,
//     .divider = 80 // 80 MHz / 80 = 1 MHz (1 tick = 1 us)
// };

// Deklaracja funkcji
void sensorRead(int& ne, int& nw, int& se, int& sw);
int performTrackingAzimuth();
int performTrackingElevation();
// bool IRAM_ATTR onTimer(void* arg);

void setup() {
    Logger.begin("SolarTrackerAP", "password123");
    tracker.begin();
    
    pinMode(SENSOR_NE, INPUT);
    pinMode(SENSOR_NW, INPUT);
    pinMode(SENSOR_SE, INPUT);
    pinMode(SENSOR_SW, INPUT);

    // Inicjalizacja timera
    // timer_init(TIMER_GROUP_0, TIMER_0, &timerConfig);
    // timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 0);
    // timer_set_alarm_value(TIMER_GROUP_0, TIMER_0, interval_us);
    // timer_enable_intr(TIMER_GROUP_0, TIMER_0);
    // timer_isr_callback_add(TIMER_GROUP_0, TIMER_0, &onTimer, NULL, 0);
    // timer_start(TIMER_GROUP_0, TIMER_0);
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
    delay(3000); // Opóźnienie dla stabilności
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

// bool IRAM_ATTR onTimer(void* arg) {
//   timerFlag = true;
//   return true;
// }