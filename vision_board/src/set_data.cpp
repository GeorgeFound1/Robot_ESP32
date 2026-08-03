#include "protocol.hpp"

uint8_t calculateChecksum(const uint8_t *data, size_t length) {
    uint8_t crc = 0;
    for (size_t i = 0; i < length; i++) {
        crc ^= data[i]; 
    }
    return crc;
}

void dataPackage(float dist, float ang, bool detect, CameraData *data) {
    data->angle = ang;
    data->distance = dist;
    data->detected = detect;
    data->startByte = START_MARKER;
    data->checksum = calculateChecksum((const uint8_t*)data, sizeof(CameraData) - 1);

}