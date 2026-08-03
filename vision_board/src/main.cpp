#include "Arduino.h"
#include "esp_camera.h"
#include "camera_config.hpp"
#include "camera_processing.hpp"

void setup() {
    Serial.begin(115200);
    while(!Serial) { ; }
    
    Serial.println("\n--- ESP32-CAM Test Start ---");

    if (initCamera()) {
        Serial.println("Camera init SUCCESS!");
    } else {
        Serial.println("Camera init FAILED!");
    }
}

void loop() {

    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
        return;
    }
    static float distance = 0.0f;
    static float angle = 0.0f;
    if (setCoords(fb, &distance, &angle)) {
        Serial.printf("distance = %f, angle = %f\n", distance, angle);
    } else {
        Serial.printf("Cannot find the object\n");
        distance = 0.0f;
        angle = 0.0f;
    }

    esp_camera_fb_return(fb);

    delay(10);
}