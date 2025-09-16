/* device_test.h - Brandon Media OS Device Driver Testing Module
 * Neural Device Matrix Validation System
 */

#ifndef KERNEL_DEVICE_TEST_H
#define KERNEL_DEVICE_TEST_H

#include <stdint.h>

/* Test Functions */
void run_device_driver_tests(void);
void get_test_statistics(int *total, int *passed, int *failed);

#endif /* KERNEL_DEVICE_TEST_H */