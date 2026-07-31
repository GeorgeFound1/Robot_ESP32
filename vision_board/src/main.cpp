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
    config.pin_sccb_sda = SIOD_GPIO_NUM;
    config.pin_sccb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;

    config.xclk_freq_hz = 20000000;
    
    config.pixel_format = PIXFORMAT_RGB565; 

    config.frame_size = FRAMESIZE_VGA;
    
    config.fb_count = 1;

    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed with error 0x%x\n", err);
        return false;
    }

    for (int i = 0; i < 10; i++) {
        camera_fb_t * fb = esp_camera_fb_get();
        if (fb) esp_camera_fb_return(fb);
        delay(50);
    }

    sensor_t * s = esp_camera_sensor_get();
    if (s) {

        s->set_gain_ctrl(s, 0);       // Отключить автоусиление
        s->set_exposure_ctrl(s, 0);   // Отключить автоэкспозицию
        s->set_whitebal(s, 0);        // Отключить автобаланс белого
    }
    
    return true;
}

inline void getRGB(uint8_t *r, uint8_t *g, uint8_t *b, const uint8_t byte1, uint8_t byte2) {

    *r = byte1 & 0xF8;
    *g = (uint8_t)(((byte1 & 0x07) << 5) | ((byte2 & 0xE0) >> 3));
    *b = (uint8_t)((byte2 & 0x1F) << 3);

}

inline bool isTargetColor(const uint8_t h, const uint8_t s, const uint8_t v) {
    uint8_t max_h1 = 8;
    uint8_t min_h1 = 0;
    uint8_t max_h2 = 180;
    uint8_t min_h2 = 172;

    uint8_t min_s = 80;
    uint8_t min_v = 40;
    uint8_t max_v = 230;

    bool isHueTarget = (h >= min_h1 && h <= max_h1) || (h >= min_h2 && h <= max_h2);
    bool isTarget = isHueTarget && (s >= min_s) && (v >= min_v && v <= max_v);
    if (isTarget) {
        return true;
    }
    return false;
}

float howFar(const int width, const int height) {
    float focalLength = 540.0f; //temporarily
    const float realWidth = 1.0f; //same
    const float pixelWidth = (float)width; //same
    const float realHeight = 3.0f; //same
    const float pixelHeight = (float)height; //same

    if (width <= 0 || height <= 0) return 0.0f;
    
    float distanceX = (focalLength * realWidth) / pixelWidth;
    float distanceY = (focalLength * realHeight) / pixelHeight;

    Serial.printf("dx = %f\n", distanceX);
    Serial.printf("dy = %f\n", distanceY);

    return distanceY;

}

float whatAngle(const int centerX) {

    const float camFOV = 60.0f; 
    const int frameWidth = 640;   
    const int center = frameWidth / 2;

    const float degPerpixel = camFOV / frameWidth;

    int error = centerX - center;
    float angle = error * degPerpixel;
    return angle;
}

inline bool isTargetColor(const uint8_t r, const uint8_t g, const uint8_t b) {
    uint16_t sum = r + g + b;
    if (sum < 80) return false;

    float r_norm = (float)r / sum; 
    if (r_norm < 0.52f) return false;

    if (r < g + 45 || r < b + 45) return false;

    return true;
}

bool setCoords (camera_fb_t *fb, float *smoothedDistance, float *smoothedAngle) {

    if (!fb || !fb->buf || !smoothedDistance || !smoothedAngle) return false;

    uint16_t rowPixels[480] = {0};
    uint16_t colPixels[640] = {0};

    for (int y = 0; y < fb->height; y++) {
        for (int x = 0; x < fb->width; x++) {

            int index = (y * fb->width + x) * 2;
            uint8_t byte1 = fb->buf[index];
            uint8_t byte2 = fb->buf[index + 1];
            uint8_t r, g, b, h, s, v;
            getRGB(&r, &g, &b, byte1, byte2);

            if (isTargetColor(r, g, b)) {
                rowPixels[y]++;
                colPixels[x]++;
            }
        }
    }

    const int noiseThresholdY = 10; 
    const int noiseThresholdX = 4;
    const int consecutiveNeeded = 3;

    int minY = -1;
    int consecutiveCount = 0;
    for (int y = 0; y < fb->height; y++) {
        if (rowPixels[y] >= noiseThresholdY) {
            consecutiveCount++;
            if (consecutiveCount >= consecutiveNeeded) {
                minY = y - (consecutiveNeeded - 1);
                break;
            }
        } else {
            consecutiveCount = 0;
        }
    }

    int maxY = -1;
    consecutiveCount = 0;
    for (int y = fb->height - 1; y >= 0; y--) {
        if (rowPixels[y] >= noiseThresholdY) {
            consecutiveCount++;
            if (consecutiveCount >= consecutiveNeeded) {
                maxY = y + (consecutiveNeeded - 1);
                break;
            }
        } else {
            consecutiveCount = 0;
        }
    }

    int minX = -1;
    consecutiveCount = 0;
    for (int x = 0; x < fb->width; x++) {
        if (colPixels[x] >= noiseThresholdX) {
            consecutiveCount++;
            if (consecutiveCount >= consecutiveNeeded) {
                minX = x - (consecutiveNeeded - 1);
                break;
            }
        } else {
            consecutiveCount = 0;
        }
    }

    int maxX = -1;
    consecutiveCount = 0;
    for (int x = fb->width - 1; x >= 0; x--) {
        if (colPixels[x] >= noiseThresholdX) {
            consecutiveCount++;
            if (consecutiveCount >= consecutiveNeeded) {
                maxX = x + (consecutiveNeeded - 1);
                break;
            }
        } else {
            consecutiveCount = 0;
        }
    }

    if (minY == -1 || maxY == -1 || minX == -1 || maxX == -1) return false;

    int width = maxX - minX + 1;
    int height = maxY - minY + 1;


    if (height > 600 || width > 700) {
        return false;
    }

    int boxArea = width * height;

    int actualTargetPixels = 0;
    for (int y = minY; y <= maxY; y++) {
        actualTargetPixels += rowPixels[y];
    }

    float fillRatio = (float)actualTargetPixels / boxArea;

    if (boxArea > 500 && fillRatio < 0.1f) { 
        return false;
    }

    int centerX = (maxX + minX) / 2;

    float distance = howFar(width, height);
    float angle = whatAngle(centerX);
    const float alpha = 0.15f;

    if (*smoothedDistance == 0.0f) {
        *smoothedDistance = distance;
        *smoothedAngle = angle;
    } else {
        *smoothedDistance = alpha * distance + (1.0f - alpha) * (*smoothedDistance);
        *smoothedAngle = alpha * angle + (1.0f - alpha) * (*smoothedAngle);
    }

    return true;
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