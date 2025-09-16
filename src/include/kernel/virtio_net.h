/* virtio_net.h - Brandon Media OS VirtIO Network Driver
 * Neural Network Interface Controller
 */

#ifndef KERNEL_VIRTIO_NET_H
#define KERNEL_VIRTIO_NET_H

#include <stdint.h>
#include "kernel/hal.h"

/* VirtIO Network Device Structure */
struct virtio_net_device {
    struct hal_device *hal_dev;
    struct pci_device *pci_dev;
    uint32_t io_base;
    uint8_t mac_addr[6];
    uint32_t features;
    struct virtio_queue rx_queue;
    struct virtio_queue tx_queue;
    int initialized;
    uint32_t rx_packets;
    uint32_t tx_packets;
    uint32_t rx_bytes;
    uint32_t tx_bytes;
};

/* VirtIO Network Function Prototypes */
void virtio_net_init(void);
void virtio_net_print_stats(void);
struct virtio_net_device *virtio_net_get_device(void);

/* Network packet functions (future implementation) */
int virtio_net_send_packet(const void *data, size_t len);
int virtio_net_receive_packet(void *buffer, size_t buffer_size);

#endif /* KERNEL_VIRTIO_NET_H */