# SPDX-License-Identifier: Apache-2.0
ROOT := $(abspath ../..)
BUILD_DIR := $(ROOT)/build/host
CC ?= cc
CFLAGS := -std=c11 -Wall -Wextra -Wpedantic -Werror -O2

.PHONY: all clean test check-vcan

all: $(BUILD_DIR)/test_can_port

$(BUILD_DIR):
	mkdir -p $@

$(BUILD_DIR)/test_can_port: $(ROOT)/tests/host/test_can_port.c $(ROOT)/middleware/canopen/port/vcan_port.c $(ROOT)/middleware/canopen/port/can_port.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -DCAN_PORT_SOCKETCAN -I$(ROOT)/middleware/canopen/port $< $(ROOT)/middleware/canopen/port/vcan_port.c -o $@

check-vcan:
	@test -d /sys/class/net/$${CAN_PORT_IFACE:-vcan0} || (echo "Missing $${CAN_PORT_IFACE:-vcan0}; run scripts/setup_vcan.sh" >&2; exit 1)

test: all check-vcan
	CAN_PORT_IFACE=$${CAN_PORT_IFACE:-vcan0} $(BUILD_DIR)/test_can_port
	PYTHONDONTWRITEBYTECODE=1 CAN_PORT_IFACE=$${CAN_PORT_IFACE:-vcan0} python3 $(ROOT)/tests/host/test_sdo.py

clean:
	rm -rf $(BUILD_DIR)
