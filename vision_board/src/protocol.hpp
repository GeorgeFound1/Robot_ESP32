#ifndef PROTOCOL_HPP
#define PROTOCOL_HPP

#include <Arduino.h>

#pragma pack(push, 1)
struct CameraData {
    uint8_t startByte;  
    bool detected;  
    float distance;
    float angle;
    uint8_t checksum;
};
#pragma pack(pop)

const uint8_t START_MARKER = 0xAA;

void dataPackage(float distance, float angle, bool detected, CameraData *data);
uint8_t calculateChecksum(const uint8_t* data, size_t length);

#endif