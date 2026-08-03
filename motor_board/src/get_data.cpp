#include "protocol.hpp"
#include "target_search.hpp"

uint8_t calculateChecksum(const uint8_t *data, size_t length) {
    uint8_t crc = 0;
    for (size_t i = 0; i < length; i++) {
        crc ^= data[i]; 
    }
    return crc;
}

bool dataUnpackage(CameraData *data, TargetData *target) {

    if (data->startByte != START_MARKER || data->checksum != calculateChecksum((const uint8_t*)data, sizeof(CameraData) - 1)) {
        return false;
    }

    target->detected = data->detected;
    target->distance = (double)data->distance;
    target->angle = (double)data->angle;

    return true;
}