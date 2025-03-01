# Prototype of Glucose Monitoring using NHS3152 and NFC

This repository provides an experimental and simplified setup for a non-continuous glucose monitoring system prototype using the NHS3152 development board, an NFC antenna, and an electrochemical sensor, such as a glucose oxidase (GOx) electrode. This project proposes a single-arm patch executed by a mobile phone through NFC, allowing for on-demand glucose measurements.

![Non-Continuous Glucose Monitoring Prototype using NHS3152 and NFC](glucoNFC.gif?raw=true "Non-Continuous Glucose Monitoring Prototype using NHS3152 and NFC")

## Introduction

Glucose monitoring is a critical aspect of managing diabetes, traditionally relying on either continuous glucose monitors (CGMs) or regular finger-prick tests. CGMs, while effective, can be expensive and invasive for many users. On the other hand, finger-pricking, although more accessible, requires frequent and painful interactions, which can be a significant problem in regular monitoring.

This project introduces a novel approach to glucose monitoring. Using the NHS3152 development board together with a simple electrochemical setup and NFC technology, this system can provide a less invasive, cost-effective solution for glucose monitoring. By momentarily powering an electrode via NFC, the system measures glucose levels through electrochemical reactions catalyzed by glucose oxidase (GOx) and transmits the results directly to a smartphone.

The key advantages of this approach include:

- **Non-invasiveness:** Unlike finger-pricking, this method does not require blood samples.
- **Cost-effectiveness:** It utilizes affordable components and simple technology compared to expensive CGMs.
- **Ease of use:** Measurements can be taken and read using a smartphone, using NFC communication.
- **Potential for screening:** The simplicity and accessibility of this system make it a useful tool for diabetes screening and regular monitoring, especially in resource-limited settings.

The goal is to provide a practical alternative for diabetes patients, offering a balance of affordability, ease, and less frequent invasiveness, potentially transforming how glucose monitoring is approached in various healthcare scenarios.

## Methods

### Hardware Setup

This hardware setup is based on commercially available electrochemical test strips and an NFC-capable development board based on the NXP NHS3152 IC. The development board is equipped with an integrated NFC reader/writer, a low-power ARM processor, and six analog lines that can be connected to the I2D (current-to-digital) hardware block.

### Test Strip Preparation and Connection

The electrochemical test strips used in this example are designed to test an electrical signal that can be proportional to the blood glucose concentration. We soldered wires to the appropriate contacts on the test strip and connected them directly to the analog inputs of the NXP NHS3152 IC.

### Data Transmission and Processing

The circuit is powered up by a nearby smartphone that has NFC capabilities. That wakes up the processor. After each measurement, the NHS3152 IC transmits the raw data via NFC to a smartphone. In this example, tapping with an NFC-mobile phone will cause the NFC tag with the value of the electrode to show. No additional app is needed, because this is native behavior of NFC on a phone. For future development, an app can capture this data, perform a linear regression to calculate the glucose concentration from the electrical signal, store, and display the result to the user.

## Project Overview

The goal of this project is to demonstrate a potential application of the NHS3152 chip in medical devices, specifically for glucose monitoring. Unlike continuous monitoring systems, this setup requires no battery and uses the NFC capabilities of smartphones to power and read the sensor.

### Used Components

- **NHS3152 Development Board:** A programmable IC designed for sensor reading and data processing, suitable for low-power applications.
- **NFC Antenna (included in Development Board):** Used to wirelessly power the system and handle data transmission.
- **Testing Sensor, emulating electrochemical measurements.**

### Proposed Components

- **NHS3152 Development Board:** A programmable IC designed for sensor reading and data processing, suitable for low-power applications.
- **NFC Antenna:** Used to wirelessly power the system and handle data transmission.
- **Glucose Oxidase (GOx):** An enzyme used to react with glucose, altering the electrical characteristics measurable by this sensor setup.
- **Electrodes:** Custom-built using standard materials to interact with GOx and produce measurable changes in current in response to glucose concentrations.

### How It Works

- **Powering the System:** The NFC antenna in a smartphone provides enough power to activate the NHS3152 when brought into proximity.
- **Measurement Process:** Glucose oxidase coated on the electrode reacts with glucose in the interstitial fluid. This reaction alters the current between the working and reference electrodes. The NHS3152 chip measures these changes and converts them into a digital signal.
- **Data Reading:** The NFC feature transmits the measured data back to the smartphone, where an app can process and display the glucose reading.

### Setup and Configuration

#### Development Boards and Test Setup

This project utilizes several key hardware components to build and test the glucose monitoring system. Below are descriptions of each component and its role in the setup:

##### NHS3152 Development Board

The NHS3152 is an integrated system-on-chip solution that combines a Cortex-M0 processor with a configurable analog front end, specifically tailored for sensor applications requiring low power and NFC connectivity. This development board is crucial for reading the electrochemical changes induced by glucose oxidase and converting these changes into a readable digital format.

- **NHS3152 General Starter Kit** (https://www.nxp.com/products/rfid-nfc/nfc-hf/ntag-smartsensor/nhs3152-general-starter-kit:NHS3152TEMOADK ~135 Euros/ 1pc)

##### MCU Link

The MCU Link is a debugging tool required to program and interface the NHS3152 development board with a PC. It handles the uploading of code, debugging, and monitoring of the system during development and testing. It provides a bridge between the NHS3152 and the PC via a USB connection.

- **MCU Link Debug Probe** (https://www.nxp.com/design/design-center/software/development-software/mcuxpresso-software-and-tools-/mcu-link-debug-probe:MCU-LINK ~17 Euros/1 pc)

##### Testing Sensor

For initial testing and validation, a specialized sensor from BVT was used. This sensor mimics the electrical properties expected in a glucose monitoring scenario, providing a stable and reproducible signal for calibration and testing.

- **TS Testing Sensor** (https://bvt.cz/produkt/testing-sensor/ ~27 Euros/1 pc)
- **KA1.S.\* Simple Connector for Electrochemical Sensors** (https://bvt.cz/produkt/ka1-s/ ~36 Euros/1 pc)

### Hardware Setup

Electrode Configuration:

- **Working Electrode (WE):** Connected to AN0_1, measures the reaction current.
- **Reference Electrode (RE):** Connected to AN0_0, provides a stable reference voltage for the WE.
- The auxiliary electrode can be omitted as it does not contribute significantly to this specific application.

### Software Development

#### Development Environment

**MCUXpresso IDE:** Ensure you have version v11.7.1_9221 of the MCUXpresso IDE installed. This IDE is used for compiling and uploading the firmware to the NHS3152 development board. You can download it from NXP's official website (https://www.nxp.com/design/design-center/software/development-software/mcuxpresso-software-and-tools-/mcuxpresso-integrated-development-environment-ide:MCUXpresso-IDE).

#### Source Code

The src directory contains all necessary code for initializing and reading from the NHS3152 chip. However, the main code is simplistic, as presented below. This code initializes and runs a simple glucose monitoring system, setting up the electrodes, reading the glucose-induced voltage changes, and sending the data via NFC to a nearby device, such as a smartphone.


```
#include "board.h"
#include "ndeft2t/ndeft2t.h" // For NFC communication : NDEF messages
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void setup_DAC_for_Auxiliary(void) {
	// Pin Configuration for Auxiliary Electrode (AN0_0)
	Chip_IOCON_SetPinConfig(NSS_IOCON, IOCON_ANA0_0, IOCON_FUNC_1);

	// Initialize ADC/DAC
	Chip_ADCDAC_Init(NSS_ADCDAC0);

	// Set DAC output to AN0_0
	Chip_ADCDAC_SetMuxDAC(NSS_ADCDAC0, ADCDAC_IO_ANA0_0);

	// Set DAC to continuous mode
	Chip_ADCDAC_SetModeDAC(NSS_ADCDAC0, ADCDAC_CONTINUOUS);

	// Output desired voltage on AN0_0 
    // (73% of reference voltage, assuming 12-bit DAC, adjust as needed)
	Chip_ADCDAC_WriteOutputDAC(NSS_ADCDAC0, 3000);
}

void setup_DAC_for_Working_Electrode(void) {
	// Initialization for Working Electrode (AN0_1) ADC
	Chip_IOCON_SetPinConfig(NSS_IOCON, IOCON_ANA0_1, IOCON_FUNC_1);

	// Set ADC input to AN0_1
	Chip_ADCDAC_SetMuxADC(NSS_ADCDAC0, ADCDAC_IO_ANA0_1);
}

int read_Working_Electrode_value(void) {
	// Start ADC conversion
	Chip_ADCDAC_StartADC(NSS_ADCDAC0);

	// Polling method: Wait until ADC conversion is complete
	while (!(Chip_ADCDAC_ReadStatus(NSS_ADCDAC0) & ADCDAC_STATUS_ADC_DONE))
		;

	// Return ADC value
	return Chip_ADCDAC_GetValueADC(NSS_ADCDAC0);
}

void send_NFC_data(const char *data) {
	uint8_t instanceBuffer[NDEFT2T_INSTANCE_SIZE] __attribute__((aligned(4)));
	uint8_t messageBuffer[NFC_SHARED_MEM_BYTE_SIZE ] __attribute__((aligned(4)));
	NDEFT2T_CREATE_RECORD_INFO_T createRecordInfo;
	uint8_t locale[] = "en";

	Chip_NFC_Init(NSS_NFC);
	NDEFT2T_Init();
	NDEFT2T_CreateMessage((void*) instanceBuffer, messageBuffer,
			NFC_SHARED_MEM_BYTE_SIZE, true);

	createRecordInfo.shortRecord = 1;
	createRecordInfo.pString = locale;

	if (NDEFT2T_CreateTextRecord((void*) instanceBuffer, &createRecordInfo)) {
		if (NDEFT2T_WriteRecordPayload((void*) instanceBuffer, (uint8_t*) data,
				strlen(data))) {
			NDEFT2T_CommitRecord((void*) instanceBuffer);
		}
	}

	NDEFT2T_CommitMessage((void*) instanceBuffer);
}

int main(void) {
	Board_Init();

	setup_DAC_for_Auxiliary();
	setup_DAC_for_Working_Electrode();
	int adcInput = read_Working_Electrode_value();

    // Determining the linear response of commercial test strips to different blood glucose 
    // concentrations allows you to construct a calibration curve, which you can use to 
    // interpolate the glucose concentration from the voltage readings you'll get from the NXP ADC.

    // Linear regression is a straightforward approach to determine the parameters of the relationship 
    // between the sensor readings and the glucose concentrations 
    // (the slope "m" and the intercept "b" in the equation y=mx+b).

    // By using the equation obtained from linear regression, you can then calculate the glucose 
    // concentration from any given sensor reading.

    // Here, for simplicity, the code returns the Working Electrode value.

	char str_adcInput[16];
	itoa(adcInput, str_adcInput, 10);

	send_NFC_data(str_adcInput);

	return 0;
}
```

## Disclaimers

This project is an early-stage prototype demonstrating the conceptual implementation of a simplified glucose monitoring system. The provided code provides the initialization and operation of this system, including the setup of electrodes, measurement of glucose-induced voltage changes, and transmission of data via NFC to a proximal device, such as a smartphone.

Please note the following:

- Prototype Stage: The system is in the prototype stage and has not undergone comprehensive clinical testing. Its accuracy, reliability, and stability are not guaranteed and require extensive calibration and testing procedures.
- **Not for Medical Use**: This system is not intended for medical use or to replace professional healthcare advice. It should not be used for diagnosing or treating any health problems or diseases.
- **Development Purposes**: The primary purpose of this prototype is for research and development. It serves as a basis for further exploration and refinement in non-invasive glucose monitoring technology.
- Use at Own Risk: Any use of this system is at the user's own risk. The developers assume no responsibility for any direct or indirect harm that may arise from its use.

# License

This project is licensed under the MIT License.
