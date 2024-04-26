/*
 * Copyright 2016-2017,2019 NXP
 * This software is owned or controlled by NXP and may only be used strictly
 * in accordance with the applicable license terms.  By expressly accepting
 * such terms or by downloading, installing, activating and/or otherwise using
 * the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to
 * be bound by the applicable license terms, then you may not retain, install,
 * activate or otherwise use the software.
 */

#ifndef __COMPRESS_H_
#define __COMPRESS_H_

/** @defgroup MODS_NSS_COMPRESS compress: compression / decompression module
 * @ingroup MODS_NSS
 * This block of code allows an application to perform losless compression and uncompression on arbitrary chunks of
 * data. @n
 * It will:
 * - abstract away how the samples are compressed and uncompressed,
 * - provide an easy interface via which data of any length compressed and uncompressed,
 *
 * The Compress module will use both EEPROM and FLASH to store bits of data. The newest samples are always stored in
 * EEPROM; when the reserved space in EEPROM is full, all the data is moved to FLASH, and the EEPROM is reused.
 * Writing samples always means appending them to the already written samples; it is not possible to edit the already
 * written stream of bits.
 * When reading back again, the user can control the starting read position using a sequence number. It is automatically
 * deduced where the corresponding sample is written, whether it is compressed, and what needs to be done to be able to
 * return the requested sample(s).
 *
 * It is expected that each application that requires this module includes it and configures the diversity settings of
 * the module according to its specific needs.
 *
 * @par Diversity
 *  This module supports diversity: settings to tweak the effectiveness of the compression.
 *  Check @ref MODS_NSS_COMPRESS_DFT for all diversity parameters.
 *
 * @par Memory Requirements
 *  The memory requirements are defined by the diversity settings. Check #COMPRESS_WINDOW_BITS and #COMPRESS_USE_INDEX.
 *
 * @par How to use the module
 *  -# To compress, simply call Compress_Encode. No preparation or initialization is necessary.
 *  -# To uncompress, call Compress_Decod. No preparation or initialization is necessary.
 *  -# Compress_Decode o Compress_Encode is an identity function.
 *
 * @par Example
 *  @snippet compress_mod_example_1.c compress_mod_example_1
 *
 * @{
 */

#include "board.h"
#include "compress_dft.h"

/* ------------------------------------------------------------------------- */

/**
 * Compresses a contiguous array of bytes.
 * @param input pointer to the array where all bytes to encode can be found. No alignment is enforced.
 * @param inputLength The number of bytes to encode, starting from @c input.
 * @param output pointer to the array where the encoded end result will be written to. No alignment is enforced.
 * @param outputLength Size in bytes of the available @c output array.
 * @return Size in bytes of the used @c output bytes. An output size of @c 0 indicates an error: the @c output buffer
 *  may or may not have been written to, but none of the bytes may be used for further processing.
 * @note The output, when a valid size has been returned, cannot be split up and decoded in chunks; decoding is only
 *  possible when providing the full array with all the now-encoded bytes.
 */
int Compress_Encode(const uint8_t * input, int inputLength, uint8_t * output, int outputLength);

/**
 * Uncompresses a contiguous array of bytes.
 * @param input pointer to the array where the encoded bytes to uncompress can be found. No alignment is enforced.
 * @param inputLength The number of bytes to decode, starting from @c input. This value must be equal to the returnvalue
 *  of a previous call to #Compress_Encode.
 * @param output pointer to the array where the decoded bytes will be written to. No alignment is enforced.
 * @param outputLength Size in bytes of the available @c output array.
 * @return Size in bytes of the used @c output bytes. An output size of @c 0 indicates an error: the @c output buffer
 *  may or may not have been written to, but none of the bytes may be used for further processing.
 * @note The output length, when a valid size has been returned, will be equal to the value of @c inputLength during
 *  a previous call to #Compress_Encode.
 */
int Compress_Decode(const uint8_t * input, int inputLength, uint8_t * output, int outputLength);

/** @} */
#endif
