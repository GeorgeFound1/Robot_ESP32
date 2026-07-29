#include "Arduino.h"
#include "esp_camera.h"

#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

bool initCamera() {
    camera_config_t config;
    
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_sscb_sda = SIOD_GPIO_NUM;
    config.pin_sscb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;

    config.xclk_freq_hz = 20000000;
    
    config.pixel_format = PIXFORMAT_RGB565; 

    config.frame_size = FRAMESIZE_QQVGA;
    
    config.fb_count = 1;

    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed with error 0x%x\n", err);
        return false;
    }
    
    return true;
}

inline void rgbTohsv(const uint8_t r, const uint8_t g,const uint8_t b, uint8_t *h, uint8_t *s, uint8_t *v) {
    uint8_t rgb_min = min(r, min(g, b));
    uint8_t rgb_max = max(r, max(g, b));
    
    // 1. value (0-255)
    *v = rgb_max;
    if (*v == 0) {
        *h = 0;
        *s = 0;
        return;
    }

    // 2. saturation (0-255)
    int delta = rgb_max - rgb_min;
    *s = (255 * delta) / *v;
    if (*s == 0) {
        *h = 0;
        return;
    }

    // 3. hue (0-180 for uint8_t)
    long hue;
    if (rgb_max == r) {
        hue = 0 + 43 * (g - b) / delta;
    } else if (rgb_max == g) {
        hue = 85 + 43 * (b - r) / delta;
    } else {
        hue = 128 + 43 * (r - g) / delta;
    }

    if (hue < 0) {
        hue += 180;
    }
    
    *h = (uint8_t)hue;
}

inline void getRGB(uint8_t *r, uint8_t *g, uint8_t *b, const uint8_t byte1, uint8_t byte2) {

    *r = byte1 & 0xF8;
    *g = (uint8_t)(((byte1 & 0x07) << 5) | ((byte2 & 0xE0) >> 3));
    *b = (uint8_t)((byte2 & 0x1F) << 3);

}

inline bool isTargetColor(const uint8_t h, const uint8_t s, const uint8_t v) {
    uint8_t max_h1 = 10;
    uint8_t min_h1 = 0;
    uint8_t max_h2 = 180;
    uint8_t min_h2 = 165;

    uint8_t min_s = 100;
    uint8_t min_v = 50;

    bool isHueTarget = (h >= min_h1 && h <= max_h1) || (h >= min_h2 && h <= max_h2);
    bool isTarget = isHueTarget && (s >= min_s) && (v >= min_v);
    if (isTarget) {
        return true;
    }
    return false;
}

float howFar(const int width, const int height) {
    float focalLength = 0; //temporarily
    const float realWidth = 0; //same
    const float pixelWidth = (float)width; //same
    const float realHeight = 0; //same
    const float pixelHeight = (float)height; //same

    if (width <= 0 || height <= 0) return 0.0f;
    
    float distanceX = (focalLength * realWidth) / pixelWidth;
    float distanceY = (focalLength * realHeight) / pixelHeight;

    return (distanceX + distanceY) / 2.0f;

}

float whatAngle(const int centerX) {

    const float camFOV = 60.0f; 
    const int frameWidth = 160;    
    const int center = frameWidth / 2;

    const float degPerpixel = camFOV / frameWidth;

    int error = centerX - center;
    float angle = error * degPerpixel;
    return angle;
}

bool setCoords (camera_fb_t *fb, float *distance, float *angle) {

    if (!fb || !fb->buf || !distance || !angle) return false;
    *distance = 0.0f;
    *angle = 0.0f;

    long sumX = 0; 
    long sumY = 0;
    int pixelCount = 0; 
    int minX = fb->width; 
    int maxX = 0;
    int minY = fb->height;
    int maxY = 0;

    for (int y = 0; y < fb->height; y++) {
        for (int x = 0; x < fb->width; x++) {

            int index = (y * fb->width + x) * 2;
            uint8_t byte1 = fb->buf[index];
            uint8_t byte2 = fb->buf[index + 1];
            uint8_t r, g, b, h, s, v;
            getRGB(&r, &g, &b, byte1, byte2);
            rgbTohsv(r, g, b, &h, &s, &v);

            if (isTargetColor(h, s, v)) {
                pixelCount++;
                sumX += x;
                sumY += y;

                if (x < minX) minX = x;
                if (x > maxX) maxX = x;
                if (y < minY) minY = y;
                if (y > maxY) maxY = y;

            }
        }
    }

    if (pixelCount > 15) {
        int centerX = sumX / pixelCount;
        int centerY = sumY / pixelCount;

        int width = (maxX >= minX) ? (maxX - minX + 1) : 0;
        int height = (maxY >= minY) ? (maxY - minY + 1) : 0;

        *distance = howFar(width, height);
        *angle = whatAngle(centerX);
        return true;
    }

    return false;
}

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
    

}