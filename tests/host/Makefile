# SPDX-License-Identifier: Apache-2.0
ROOT := $(abspath ../..)
BUILD_DIR := $(ROOT)/build/host
SANITIZER_BUILD_DIR := $(ROOT)/build/host-sanitize
COVERAGE_BUILD_DIR := $(ROOT)/build/host-coverage
CC ?= cc
CFLAGS := -std=c11 -Wall -Wextra -Wpedantic -Werror -O2
SANITIZER_FLAGS := -fsanitize=address,undefined -fno-omit-frame-pointer -O1 -g
COVERAGE_FLAGS := --coverage -O0 -g

.PHONY: all clean test test-sanitize test-coverage test-stm32-facade test-gateway-default-deny test-cia302 test-lss-policy test-cia418 test-recovery check-vcan

all: $(BUILD_DIR)/test_can_port $(BUILD_DIR)/test_can_port_stm32 $(BUILD_DIR)/test_gateway_default_deny $(BUILD_DIR)/test_cia302_nmt_master $(BUILD_DIR)/test_lss_policy $(BUILD_DIR)/test_cia418_reference $(BUILD_DIR)/test_can_recovery

$(BUILD_DIR):
	mkdir -p $@

$(BUILD_DIR)/test_can_port: $(ROOT)/tests/host/test_can_port.c $(ROOT)/middleware/canopen/port/vcan_port.c $(ROOT)/middleware/canopen/port/can_port.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -DCAN_PORT_SOCKETCAN -I$(ROOT)/middleware/canopen/port $< $(ROOT)/middleware/canopen/port/vcan_port.c -o $@

$(BUILD_DIR)/test_cia302_nmt_master: $(ROOT)/tests/test_cia302_nmt_master.c $(ROOT)/middleware/canopen/core/cia302_nmt_master.c $(ROOT)/middleware/canopen/core/cia302_nmt_master.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -I$(ROOT)/middleware/canopen/core $^ -o $@

$(BUILD_DIR)/test_lss_policy: $(ROOT)/tests/test_lss_policy.c $(ROOT)/App/Src/canopen_reference_lss.c $(ROOT)/App/Inc/canopen_reference_lss.h $(ROOT)/tests/fakes/CANopen.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -I$(ROOT)/tests/fakes -I$(ROOT)/App/Inc $^ -o $@

$(BUILD_DIR)/test_cia418_reference: $(ROOT)/tests/test_cia418_reference.c $(ROOT)/App/Src/cia418_reference.c $(ROOT)/App/Inc/cia418_reference.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -I$(ROOT)/App/Inc $^ -o $@

$(BUILD_DIR)/test_can_recovery: $(ROOT)/tests/test_can_recovery.c $(ROOT)/App/Src/canopen_reference_can_recovery.c $(ROOT)/App/Inc/canopen_reference_can_recovery.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -I$(ROOT)/App/Inc $^ -o $@

$(BUILD_DIR)/test_can_port_stm32: $(ROOT)/tests/test_can_port_stm32.c $(ROOT)/middleware/canopen/port/can_port.c $(ROOT)/middleware/canopen/port/can_port.h $(ROOT)/tests/fakes/stm32f7xx_hal.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -DCAN_PORT_STM32 -DCAN_PORT_FAKE_F767 -I$(ROOT)/tests/fakes -I$(ROOT)/middleware/canopen/port $< $(ROOT)/middleware/canopen/port/can_port.c -o $@

test-cia302: $(BUILD_DIR)/test_cia302_nmt_master
	$(BUILD_DIR)/test_cia302_nmt_master

test-lss-policy: $(BUILD_DIR)/test_lss_policy
	$(BUILD_DIR)/test_lss_policy

test-cia418: $(BUILD_DIR)/test_cia418_reference
	$(BUILD_DIR)/test_cia418_reference

test-recovery: $(BUILD_DIR)/test_can_recovery
	$(BUILD_DIR)/test_can_recovery

test-stm32-facade: $(BUILD_DIR)/test_can_port_stm32
	$(BUILD_DIR)/test_can_port_stm32

$(BUILD_DIR)/test_gateway_default_deny: $(ROOT)/tests/test_gateway_default_deny.c $(ROOT)/App/Src/canopen_reference_gateway.c $(ROOT)/App/Inc/canopen_reference_gateway.h $(ROOT)/tests/fakes/CANopen.h $(ROOT)/tests/fakes/CO_gateway_ascii.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -DCANOPEN_REFERENCE_ENABLE_GATEWAY=1 -I$(ROOT)/tests/fakes -I$(ROOT)/App/Inc $< $(ROOT)/App/Src/canopen_reference_gateway.c -o $@

test-gateway-default-deny: $(BUILD_DIR)/test_gateway_default_deny
	$(BUILD_DIR)/test_gateway_default_deny

$(SANITIZER_BUILD_DIR):
	mkdir -p $@

$(SANITIZER_BUILD_DIR)/test_can_recovery: $(ROOT)/tests/test_can_recovery.c $(ROOT)/App/Src/canopen_reference_can_recovery.c $(ROOT)/App/Inc/canopen_reference_can_recovery.h | $(SANITIZER_BUILD_DIR)
	$(CC) $(CFLAGS) $(SANITIZER_FLAGS) -I$(ROOT)/App/Inc $< $(ROOT)/App/Src/canopen_reference_can_recovery.c -o $@

$(SANITIZER_BUILD_DIR)/test_cia302_nmt_master: $(ROOT)/tests/test_cia302_nmt_master.c $(ROOT)/middleware/canopen/core/cia302_nmt_master.c $(ROOT)/middleware/canopen/core/cia302_nmt_master.h | $(SANITIZER_BUILD_DIR)
	$(CC) $(CFLAGS) $(SANITIZER_FLAGS) -I$(ROOT)/middleware/canopen/core $^ -o $@

test-sanitize: $(SANITIZER_BUILD_DIR)/test_can_recovery $(SANITIZER_BUILD_DIR)/test_cia302_nmt_master
	ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 $(SANITIZER_BUILD_DIR)/test_can_recovery
	ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 $(SANITIZER_BUILD_DIR)/test_cia302_nmt_master

$(COVERAGE_BUILD_DIR):
	mkdir -p $@

$(COVERAGE_BUILD_DIR)/test_can_recovery: $(ROOT)/tests/test_can_recovery.c $(ROOT)/App/Src/canopen_reference_can_recovery.c $(ROOT)/App/Inc/canopen_reference_can_recovery.h | $(COVERAGE_BUILD_DIR)
	$(CC) $(CFLAGS) $(COVERAGE_FLAGS) -I$(ROOT)/App/Inc $< $(ROOT)/App/Src/canopen_reference_can_recovery.c -o $@

$(COVERAGE_BUILD_DIR)/test_cia302_nmt_master: $(ROOT)/tests/test_cia302_nmt_master.c $(ROOT)/middleware/canopen/core/cia302_nmt_master.c $(ROOT)/middleware/canopen/core/cia302_nmt_master.h | $(COVERAGE_BUILD_DIR)
	$(CC) $(CFLAGS) $(COVERAGE_FLAGS) -I$(ROOT)/middleware/canopen/core $^ -o $@

test-coverage: $(COVERAGE_BUILD_DIR)/test_can_recovery $(COVERAGE_BUILD_DIR)/test_cia302_nmt_master
	$(COVERAGE_BUILD_DIR)/test_can_recovery
	$(COVERAGE_BUILD_DIR)/test_cia302_nmt_master
	@mkdir -p $(COVERAGE_BUILD_DIR)/report
	@cd $(COVERAGE_BUILD_DIR) && gcov -b -c test_can_recovery-test_can_recovery.c 2>/dev/null > report/test_can_recovery.gcov.txt || true
	@cd $(COVERAGE_BUILD_DIR) && gcov -b -c test_cia302_nmt_master-test_cia302_nmt_master.c 2>/dev/null > report/test_cia302.gcov.txt || true
	@test -s $(COVERAGE_BUILD_DIR)/report/test_can_recovery.gcov.txt
	@test -s $(COVERAGE_BUILD_DIR)/report/test_cia302.gcov.txt

check-vcan:
	@test -d /sys/class/net/$${CAN_PORT_IFACE:-vcan0} || (echo "Missing $${CAN_PORT_IFACE:-vcan0}; run scripts/setup_vcan.sh" >&2; exit 1)

test: all test-stm32-facade test-gateway-default-deny check-vcan
	CAN_PORT_IFACE=$${CAN_PORT_IFACE:-vcan0} $(BUILD_DIR)/test_can_port
	PYTHONDONTWRITEBYTECODE=1 CAN_PORT_IFACE=$${CAN_PORT_IFACE:-vcan0} python3 $(ROOT)/tests/host/test_sdo.py

clean:
	rm -rf $(BUILD_DIR)
