#!/usr/bin/env python3
"""Deterministic source-contract tests for the STM32F767 CANopen reference.

These checks deliberately validate electrical-interface and timing assumptions that
cannot be observed from a host-only executable. They prevent a source edit from
silently changing the documented 25 MHz HSE / 216 MHz system clock, 500 kbit/s
bxCAN timing, PA11/PA12 pin assignment, or 1 ms CANopen timer cadence.
"""
from __future__ import annotations

import re
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
MAIN = (ROOT / "Core" / "Src" / "main.c").read_text(encoding="utf-8")
CLOCK = (ROOT / "Core" / "Src" / "system_clock_reference.c").read_text(encoding="utf-8")
MSP = (ROOT / "Core" / "Src" / "stm32f7xx_hal_msp.c").read_text(encoding="utf-8")
FEATURES = (ROOT / "App" / "Inc" / "CO_driver_custom.h").read_text(encoding="utf-8")
PROFILE = (ROOT / "App" / "Inc" / "canopen_reference_config.h").read_text(encoding="utf-8")
BOARD = (ROOT / "App" / "Src" / "canopen_reference_board.c").read_text(encoding="utf-8")
CIA302_HEADER = (ROOT / "App" / "Inc" / "canopen_reference_cia302.h").read_text(encoding="utf-8")
CIA302_SOURCE = (ROOT / "App" / "Src" / "canopen_reference_cia302.c").read_text(encoding="utf-8")
APP_RUNTIME = (ROOT / "App" / "Src" / "CO_app_STM32_reference.c").read_text(encoding="utf-8")
CMAKE = (ROOT / "CMakeLists.txt").read_text(encoding="utf-8")


def source_integer(name: str) -> int:
    match = re.search(rf"^#define\s+{name}\s+([0-9]+)(?:U|UL|L)?$", MAIN, re.MULTILINE)
    if match is None:
        raise AssertionError(f"missing integer macro {name}")
    return int(match.group(1))


class FirmwareConfigurationTests(unittest.TestCase):
    def test_clock_tree_contract(self) -> None:
        """The documented 25 MHz HSE clock tree produces 216/54/108 MHz domains."""
        for expected in (
            "osc.PLL.PLLM = 25U;",
            "osc.PLL.PLLN = 432U;",
            "osc.PLL.PLLP = RCC_PLLP_DIV2;",
            "clk.AHBCLKDivider = RCC_SYSCLK_DIV1;",
            "clk.APB1CLKDivider = RCC_HCLK_DIV4;",
            "clk.APB2CLKDivider = RCC_HCLK_DIV2;",
        ):
            self.assertIn(expected, CLOCK)
        hse_hz = 25_000_000
        sysclk_hz = hse_hz // 25 * 432 // 2
        self.assertEqual(sysclk_hz, 216_000_000)
        self.assertEqual(sysclk_hz // 4, 54_000_000)

    def test_bxcan_bit_timing_is_500_kbit(self) -> None:
        """CAN1 uses 54 MHz PCLK1 with an exact 500 kbit/s nominal rate."""
        for expected in (
            "hcan1.Init.Prescaler = 6U;",
            "hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;",
            "hcan1.Init.TimeSeg1 = CAN_BS1_15TQ;",
            "hcan1.Init.TimeSeg2 = CAN_BS2_2TQ;",
            "hcan1.Init.AutoBusOff = ENABLE;",
            "hcan1.Init.AutoRetransmission = ENABLE;",
        ):
            self.assertIn(expected, MAIN)
        pclk1_hz = 54_000_000
        time_quanta = 1 + 15 + 2
        self.assertEqual(pclk1_hz // (6 * time_quanta), 500_000)
        self.assertEqual(pclk1_hz % (6 * time_quanta), 0)

    def test_tim7_is_exactly_one_millisecond(self) -> None:
        """TIM7 runs from the doubled APB1 timer clock when APB1 is divided by four."""
        timer_input_hz = source_integer("CANOPEN_REFERENCE_TIM7_INPUT_HZ")
        prescaler_div = source_integer("CANOPEN_REFERENCE_TIM7_PRESCALER_DIV")
        period_ticks = source_integer("CANOPEN_REFERENCE_TIM7_PERIOD_TICKS")
        self.assertEqual(timer_input_hz, 108_000_000)
        self.assertEqual(timer_input_hz // (prescaler_div * period_ticks), 1_000)
        self.assertEqual(timer_input_hz % (prescaler_div * period_ticks), 0)
        self.assertIn("htim7.Init.Prescaler = CANOPEN_REFERENCE_TIM7_PRESCALER_DIV - 1U;", MAIN)
        self.assertIn("htim7.Init.Period = CANOPEN_REFERENCE_TIM7_PERIOD_TICKS - 1U;", MAIN)
        self.assertIn("canopen_app_interrupt();", MAIN)

    def test_can_pins_and_interrupts_match_can1_contract(self) -> None:
        """The STM32 classic-CAN peripheral is wired to PA11/PA12 AF9 and IRQ-backed."""
        for expected in (
            "if (hcan->Instance != CAN1)",
            "__HAL_RCC_CAN1_CLK_ENABLE();",
            "gpio.Pin = GPIO_PIN_11 | GPIO_PIN_12;",
            "gpio.Alternate = GPIO_AF9_CAN1;",
            "HAL_NVIC_EnableIRQ(CAN1_TX_IRQn);",
            "HAL_NVIC_EnableIRQ(CAN1_RX0_IRQn);",
            "HAL_NVIC_EnableIRQ(CAN1_RX1_IRQn);",
            "HAL_NVIC_EnableIRQ(CAN1_SCE_IRQn);",
        ):
            self.assertIn(expected, MSP)

    def test_canopennode_feature_dependencies_are_complete(self) -> None:
        """Block-transfer options include every upstream prerequisite used by this reference."""
        for expected in (
            "#define CO_CONFIG_SDO_SRV             (0x02U | 0x04U | 0x4000U)",
            "#define CO_CONFIG_SDO_SRV_BUFFER_SIZE 1024U",
            "#define CO_CONFIG_SDO_CLI             (0x01U | 0x02U | 0x04U | 0x08U | 0x4000U)",
            "#define CO_CONFIG_SDO_CLI_BUFFER_SIZE 1024U",
            "#define CO_CONFIG_FIFO                (0x01U | 0x02U | 0x04U)",
            "#define CO_CONFIG_CRC16               0x01U",
            "#define CO_CONFIG_PDO                 (0x01U | 0x02U | 0x04U | 0x08U | 0x10U | 0x20U | 0x40U | 0x4000U)",
            "#define CO_CONFIG_LSS                 (0x01U | 0x02U)",
            "#define CO_CONFIG_LEDS                0x01U",
        ):
            self.assertIn(expected, FEATURES)

    def test_profile_selection_and_safe_board_defaults(self) -> None:
        """The checked-in personality is CiA 401 and weak board hooks default to de-energized."""
        self.assertIn("#define CANOPEN_REFERENCE_ENABLE_CIA401 1U", PROFILE)
        self.assertIn("#define CANOPEN_REFERENCE_ENABLE_CIA402 0U", PROFILE)
        self.assertIn("CANopenReferenceBoard_SetCanTransceiverEnabled(false);", BOARD)
        self.assertIn("CANopenReferenceHw_DriveSetEnable(false);", BOARD)
        self.assertIn("CANopenReferenceHw_WriteDigitalOutputs(0U);", BOARD)

    def test_cia302_master_is_explicitly_opt_in_and_mainline_ordered(self) -> None:
        """The CiA 302 master is disabled by default and receives every heartbeat before stack processing."""
        self.assertIn("option(CANOPEN_REFERENCE_ENABLE_CIA302_MASTER", CMAKE)
        self.assertIn('option(CANOPEN_REFERENCE_ENABLE_CIA302_MASTER "Build the opt-in CiA 302 NMT-master personality" OFF)', CMAKE)
        self.assertRegex(PROFILE, r"#define\s+CANOPEN_REFERENCE_ENABLE_CIA302_MASTER\s+0U")
        self.assertIn("CANopenReferenceCia302_PreProcess(now);", APP_RUNTIME)
        self.assertLess(APP_RUNTIME.index("CANopenReferenceCia302_PreProcess(now);"),
                        APP_RUNTIME.index("resetCommand = CO_process("))
        self.assertIn("CO_FLAG_READ(node->CANrxNew)", CIA302_SOURCE)
        self.assertIn("CANopenReferenceCia302_PreProcess", CIA302_HEADER)
        self.assertIn("event_count_heartbeat_timeout", CIA302_HEADER)


if __name__ == "__main__":
    unittest.main(verbosity=2)
