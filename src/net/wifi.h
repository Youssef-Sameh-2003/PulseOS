#ifndef WIFI_H
#define WIFI_H

#include <stdint.h>
#include <stddef.h>

// Intel 3945ABG PCI IDs
#define WIFI_VENDOR_INTEL 0x8086
#define WIFI_DEVICE_3945ABG 0x4222

struct wifi_device {
    uint32_t mmio_base;
    uint8_t bus, slot, func;
    uint8_t mac_addr[6];
    int initialized;
    // Additional fields for firmware, queues, state, etc.
};

int wifi_probe(struct wifi_device *dev);
// Firmware is required for most modern WiFi chips; see notes!
int wifi_init(struct wifi_device *dev, const uint8_t *firmware, size_t fwlen);
int wifi_get_mac(struct wifi_device *dev);
int wifi_scan(struct wifi_device *dev);
int wifi_connect(struct wifi_device *dev, const char *ssid, const char *pass);
int wifi_send(struct wifi_device *dev, const void *data, size_t len);
int wifi_receive(struct wifi_device *dev, void *buf, size_t buflen);

#endif