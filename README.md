# EIACH

### Pins in Use

> WARNING using SI4463: Not all pins on the Mega and Mega 2560
> support change interrupts, so only the following
> can be used for RX: 10, 11, 12, 13, 14, 15,
> 50, 51, 52, 53, A8 (62), A9 (63), A10 (64),
> A11 (65), A12 (66), A13 (67), A14 (68), A15 (69).

|   Pins    |   Usage                       |
|-----------|-------------------------------|
|   10-12   |   Transmitter 0               |
|   50-52   |   Transmitter 1 (base only)   |

### Bidirectional Urgent Response Protocol (BURP)

#### Problems
* protocol_functions.h and protocol_settings.h must be synced manually in in base/ and pylon/
* Only one software serial at a time is possible