# Research Notes: Gateway and RTOS Integration Sources

These notes preserve the external findings used in `docs/07_profile_gateway_rtos_roadmap.md`.

| Source | Finding used in this project |
|---|---|
| [CANopenNode Gateway ASCII mapping](https://canopennode.github.io/CANopenNode/group__CO__CANopen__309__3.html) | CANopenNode exposes an ASCII CiA 309-3 gateway. It uses an application-owned input/output stream, an output callback registered through `CO_GTWA_initRead()`, command ingestion through `CO_GTWA_write()`, and non-blocking cyclic processing through `CO_GTWA_process()`. The documentation identifies a 200-byte default response buffer and a 1.2-second default state timeout. |
| [CANopenNode device support](https://canopennode.github.io/CANopenNode/md_doc_2deviceSupport.html) | Hardware-specific interfaces are intentionally outside the base CANopenNode project. The upstream documentation identifies a separate STM32 integration and lists an older community FreeRTOS integration; it recommends project-specific driver/README ownership. |
| [Upstream gateway header](https://github.com/CANopenNode/CANopenNode/blob/master/309/CO_gateway_ascii.h) | The public gateway header describes serial/terminal, stdio, or socket streams as possible application transports. It documents ASCII SDO and NMT commands and states that gateway commands are newline-terminated. |

The reference uses these findings only to define integration boundaries and source contracts. It does not claim that a board-specific UART, FreeRTOS task model, gateway authentication scheme, or network-security design has been validated.
