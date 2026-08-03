#ifndef PROCESSING_HPP
#define PROCESSING_HPP

#include "Arduino.h"
#include "esp_camera.h"

inline void getRGB(uint8_t *r, uint8_t *g, uint8_t *b, const uint8_t byte1, uint8_t byte2);
float howFar(const int width, const int height);
float whatAngle(const int centerX);
inline bool isTargetColor(const uint8_t r, const uint8_t g, const uint8_t b);
bool setCoords (camera_fb_t *fb, float *smoothedDistance, float *smoothedAngle);

#endif