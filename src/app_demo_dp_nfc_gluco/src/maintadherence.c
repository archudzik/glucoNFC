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

	// Output desired voltage on AN0_0 (73% of reference voltage, assuming 12-bit DAC)
	Chip_ADCDAC_WriteOutputDAC(NSS_ADCDAC0, 3000);
}

void setup_DAC_for_Working_Electrode(void) {
	// Initialization for Working Electrode (AN0_1) ADC
	Chip_IOCON_SetPinConfig(NSS_IOCON, IOCON_ANA0_1, IOCON_FUNC_1);

	// Set ADC input to AN0_1
	Chip_ADCDAC_SetMuxADC(NSS_ADCDAC0, ADCDAC_IO_ANA0_1);
}

uint16_t read_Working_Electrode_value(void) {
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

	char str_adcInput[16];
	itoa(adcInput, str_adcInput, 10);

	send_NFC_data(str_adcInput);

	return 0;
}
