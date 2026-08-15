# SPDX-License-Identifier: Apache-2.0
ROOT := $(abspath ../..)
BUILD_DIR := $(ROOT)/build/host
CC ?= cc
CFLAGS := -std=c11 -Wall -Wextra -Wpedantic -Werror -O2

.PHONY: all clean test test-stm32-facade test-gateway-default-deny test-cia302 check-vcan

all: $(BUILD_DIR)/test_can_port $(BUILD_DIR)/test_can_port_stm32 $(BUILD_DIR)/test_gateway_default_deny $(BUILD_DIR)/test_cia302_nmt_master

$(BUILD_DIR):
	mkdir -p $@

$(BUILD_DIR)/test_can_port: $(ROOT)/tests/host/test_can_port.c $(ROOT)/middleware/canopen/port/vcan_port.c $(ROOT)/middleware/canopen/port/can_port.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -DCAN_PORT_SOCKETCAN -I$(ROOT)/middleware/canopen/port $< $(ROOT)/middleware/canopen/port/vcan_port.c -o $@

$(BUILD_DIR)/test_cia302_nmt_master: $(ROOT)/tests/test_cia302_nmt_master.c $(ROOT)/middleware/canopen/core/cia302_nmt_master.c $(ROOT)/middleware/canopen/core/cia302_nmt_master.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -I$(ROOT)/middleware/canopen/core $^ -o $@

$(BUILD_DIR)/test_can_port_stm32: $(ROOT)/tests/test_can_port_stm32.c $(ROOT)/middleware/canopen/port/can_port.c $(ROOT)/middleware/canopen/port/can_port.h $(ROOT)/tests/fakes/stm32f7xx_hal.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -DCAN_PORT_STM32 -I$(ROOT)/tests/fakes -I$(ROOT)/middleware/canopen/port $< $(ROOT)/middleware/canopen/port/can_port.c -o $@

test-cia302: $(BUILD_DIR)/test_cia302_nmt_master
	$(BUILD_DIR)/test_cia302_nmt_master

test-stm32-facade: $(BUILD_DIR)/test_can_port_stm32
	$(BUILD_DIR)/test_can_port_stm32

$(BUILD_DIR)/test_gateway_default_deny: $(ROOT)/tests/test_gateway_default_deny.c $(ROOT)/App/Src/canopen_reference_gateway.c $(ROOT)/App/Inc/canopen_reference_gateway.h $(ROOT)/tests/fakes/CANopen.h $(ROOT)/tests/fakes/CO_gateway_ascii.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -DCANOPEN_REFERENCE_ENABLE_GATEWAY=1 -I$(ROOT)/tests/fakes -I$(ROOT)/App/Inc $< $(ROOT)/App/Src/canopen_reference_gateway.c -o $@

test-gateway-default-deny: $(BUILD_DIR)/test_gateway_default_deny
	$(BUILD_DIR)/test_gateway_default_deny

check-vcan:
	@test -d /sys/class/net/$${CAN_PORT_IFACE:-vcan0} || (echo "Missing $${CAN_PORT_IFACE:-vcan0}; run scripts/setup_vcan.sh" >&2; exit 1)

test: all test-stm32-facade test-gateway-default-deny check-vcan
	CAN_PORT_IFACE=$${CAN_PORT_IFACE:-vcan0} $(BUILD_DIR)/test_can_port
	PYTHONDONTWRITEBYTECODE=1 CAN_PORT_IFACE=$${CAN_PORT_IFACE:-vcan0} python3 $(ROOT)/tests/host/test_sdo.py

clean:
	rm -rf $(BUILD_DIR)
