/*
 * Copyright 2016-2020 NXP
 * This software is owned or controlled by NXP and may only be used strictly
 * in accordance with the applicable license terms.  By expressly accepting
 * such terms or by downloading, installing, activating and/or otherwise using
 * the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to
 * be bound by the applicable license terms, then you may not retain, install,
 * activate or otherwise use the software.
 */

#ifndef __STORAGE_H_
#define __STORAGE_H_

/**
 * @defgroup MODS_NSS_STORAGE storage: NVM Storage module
 * @ingroup MODS_NSS
 * The storage module allows an application to store samples <strong> all of identical size</strong> in non-volatile
 * memories EEPROM and FLASH.
 *
 * It will:
 * - abstract away where and how the samples are stored
 * - provide an easy interface via which samples can be written to and read from
 * - utilize all bits, placing all samples back to back, maximizing storage capacity
 * - minimize FLASH program operations, thereby avoiding the time penalty, voltage drop and current consumption that
 *  comes with every FLASH program operation
 * - move data from EEPROM to FLASH automatically whenever necessary to make sure the newest data is always in EEPROM
 * - allow application-specific compression of the data just before moving data from EEPROM to FLASH
 * - allow application-specific decompression of the data before they are read out again
 * - recover its full state based on the contents of the non-volatile memory alone
 *
 * The storage module will use general purpose registers, EEPROM and FLASH to store bits of data. The usage of the
 * three types of memories is prioritized. When a new sample is to be stored:
 * - first an attempt is made to store it in the general purpose registers ("cached").
 * - when this is not possible, the storage module tries to store it (together with all the cached samples) in EEPROM
 *  @n After moving data to EEPROM, the general purpose registers are re-used.
 * - only when a sufficient amount of samples are stored in EEPROM, all the data is moved to FLASH.
 *  @n After moving data to FLASH, the EEPROM is re-used.
 * Writing samples always means appending them to the already written samples; it is not possible to edit the already
 * written stream of bits.
 * When reading out, the user can control the starting read position using a sequence number. It is automatically
 * deduced where the corresponding sample is written, whether it is compressed, and what needs to be done to be able to
 * return the requested sample(s).
 *
 * @par Hardening
 *  The storage module provides a ready-made solution to maximize storage of samples in persistent memory. It can:
 *  - store more than 10.000 samples without hitting the write endurance limit of both EEPROM and FLASH
 *  - recover state from unexpected resets
 *  - recover all or most of the samples in case of data corruption (this may occur when the battery is degraded and
 *      can no longer supply the minimum voltage when the load increases during a write operation in EEPROM or FLASH)
 *  It is not possible to fully recover data under all circumstances. The storage module guarantees that only the last
 *  few samples may get lost. The number is dependent on the number of the reserved general purpose registers and the
 *  size of a sample, both of which (and more) can be tweaked using diversity settings.
 *
 *
 * @par Diversity
 *  This module supports diversity settings. Some settings define the type and size of the sample. Others define the
 *  EEPROM and FLASH regions placed under control of this module. The rest of the settings control the behavior of the
 *  module. Check @ref MODS_NSS_STORAGE_DFT for all diversity settings.
 *  @n
 *  It is expected that each application that requires this module includes it and configures the diversity settings of
 *  the module according to its specific needs.
 *
 * @par Memory Requirements
 *  The storage module requires a large chunk of SRAM, called its workarea - see #STORAGE_WORKAREA and
 *  #STORAGE_WORKAREA_SIZE. This is used for two purposes:
 *  - When storing samples, and a move from EEPROM to FLASH is required, the assigned compress callback - see
 *      #STORAGE_COMPRESS_CB - is given a pointer inside this SRAM memory. The output is then stored in FLASH.
 *  - When reading samples from FLASH, the assigned decompress callback - see #STORAGE_DECOMPRESS_CB - is called as
 *      little as possible: its output is cached in the work area to speed up subsequent reads.
 *  If two operations in your code require such a big chunk of memory, you can overlap them if they don't have to
 *  operate concurrently. Diversity setting #STORAGE_WORKAREA can be used for this.
 *
 * @par How to use the module
 *  -# Define the best diversity settings for your application or accept the default ones.
 *  -# Initialize the EEPROM driver and the storage module, in that order.
 *  -# Read and write samples as necessary, in any order or quantity that is required for your use case.
 *  -# De-initialize the storage module and the EEPROM driver, in that order.
 *
 * @par Example
 *  @snippet storage_mod_example_1.c storage_mod_example_1
 *
 * @warning These functions are not re-entrant. Calling these functions from multiple threads or in an interrupt is
 *  highly discouraged.
 * @warning The storage module requires the exclusive use of @em at @em least @em one register in the always-on domain
 *  (see #STORAGE_FIRST_ALON_REGISTER). Under no circumstance may the reserved registers be touched from outside the
 *  storage module.
 * @warning Although the storage module is able to recover after a power loss or going to Power-off, it is slow in doing
 *  so. The slow recovery time only occurs once as long as no changes to the NVM are made (e.g. by calling
 *  #Storage_Write).
 * @{
 */

#include "board.h"
#include "storage_dft.h"

/* ------------------------------------------------------------------------- */

/**
 * Whenever data is about to be moved from EEPROM to FLASH, the application is notified via a callback of this
 * prototype. It then has a chance to compress the data before it is written to FLASH.
 * The application is in charge of:
 * - reading the data from EEPROM using @c eepromByteOffset as starting point,
 * - compressing exactly @c bitCount bits,
 * - storing the end result in @c out and
 * - returning the size of the data written in @c pOut, expressed in @b bits.
 * @param eepromByteOffset The absolute offset in bytes to the EEPROM where the first sample is stored.
 * @param bitCount An exact total of #STORAGE_BLOCK_SIZE_IN_SAMPLES samples are stored. They are
 *  packed together, i.e. without any padding bits. This argument will @b always equal
 *  @c #STORAGE_UNCOMPRESSED_BLOCK_SIZE_IN_BITS.
 * @param pOut A pointer to SRAM where the compressed data must be stored in. The buffer has a size of
 *  @c #STORAGE_UNCOMPRESSED_BLOCK_SIZE_IN_BYTES bytes.
 * @return The size of the compressed data @b in @b bits. When @c 0 is returned, or a value bigger than or equal to
 *  @c #STORAGE_UNCOMPRESSED_BLOCK_SIZE_IN_BITS, the uncompressed data will be stored instead, and the corresponding
 *  decompression callback of type #pStorage_DecompressCb_t will not be called when reading out the data later.
 * @warning It is @b not allowed to call any function of this module during the lifetime of the callback.
 * @see STORAGE_COMPRESS_CB
 */
typedef int (*pStorage_CompressCb_t)(int eepromByteOffset, int bitCount, void * pOut);

/**
 * Whenever data is read from FLASH, the application is notified via a callback of this prototype. It then has a chance
 * to decompress the data before it is used to fulfill the read request #Storage_Read.
 * The application is in charge of:
 * - reading the data from FLASH using @c data as starting point,
 * - decompressing one block of compressed data stored from that point with size @c bitCount,
 * - storing the end result - #STORAGE_BLOCK_SIZE_IN_SAMPLES samples - in @c out. The samples @b must be written
 *  packed together, i.e. without any padding bits.
 * @param pData The absolute byte address to FLASH memory where the start of the (compressed) data block is found.
 * @param bitCount The size in bits of the (compressed) data block.
 * @param pOut A pointer to SRAM where the compressed data must be stored in. The buffer has a size of
 *  @c #STORAGE_UNCOMPRESSED_BLOCK_SIZE_IN_BYTES bytes.
 * @return The number of bits written to in @c pOut.
 *  If @c #STORAGE_UNCOMPRESSED_BLOCK_SIZE_IN_BITS is returned, the operation is assumed to be
 *  successful and the decompressed samples are now available. @c Any other value indicates a decompression failure:
 *  the samples stored in that block can @b not be retrieved any more, and the call to #Storage_Read which initiated
 *  this callback will fail.
 * @warning It is @b not allowed to call any function of this module during the lifetime of the callback.
 * @see STORAGE_DECOMPRESS_CB
 */
typedef int (*pStorage_DecompressCb_t)(const uint8_t * pData, int bitCount, void * pOut);

/* ------------------------------------------------------------------------- */

/**
 * This function must be the first function to call in this module after going to deep power down or power-off power
 * save mode. Not calling this function will result at best in random data being written and read, and possibly generate
 * hard faults.
 * @pre The value written in #STORAGE_FIRST_ALON_REGISTER and beyond is either exactly what was stored after leaving
 *  #Storage_DeInit, or equal to @c 0.
 * @pre EEPROM is initialized and is ready to be used.
 * @post When this function returns, #Storage_Seek still needs to be called before being able to read samples. This is
 *  also required when reading from the start of the memory, i.e. when reading the oldest stored sample referred to as
 *  index 0.
 * @warning This function can run for a long time before completion when it is forced to scan the assigned EEPROM
 *  region to recover the data that is stored in EEPROM and/or FLASH. The time spent is depending on both the assigned
 *  EEPROM region and the number of samples stored in EEPROM: the longer the region, the more time spent; the more
 *  samples stored in EEPROM, the less time spent recovering. Under worst case conditions using a system clock of
 *  0.5 MHz, this may last more than 10 msec.
 *  This penalty only occurs under these combined conditions:
 *  - the IC went to power-off, losing all information stored in the register #STORAGE_FIRST_ALON_REGISTER and beyond.
 *  - data was added to the storage module after leaving a previous power-off mode
 */
void Storage_Init(void);

/**
 * This function must be the last function to call in this module before going to deep power down or power-off power
 * save mode.
 * @pre EEPROM is still initialized and ready to be used.
 * @post Possibly, an EEPROM flush was necessary, but that has finished when this function returns.
 * @warning Loss of power before or during this call may result in loss of some or all of the newly added samples.
 */
void Storage_DeInit(void);

/**
 * @return The total number of samples currently stored, in EEPROM and FLASH combined.
 */
int Storage_GetCount(void);

/**
 * Resets the storage module to a pristine state.
 * @param checkFlash The contents in FLASH must have been erased before it can be written to. An erase operation is
 *  costly and time-consuming and is preferably avoided.
 *  - When @c true is given, @b all words of the FLASH memory assigned for sample storage are checked.
 *      If one checked word does not contain the erased value (@c 0xFFFFFFFF), all FLASH memory is erased. This
 *      can possibly take up to 3 erase cycles. Aligning #STORAGE_FLASH_FIRST_PAGE and #STORAGE_FLASH_LAST_PAGE to
 *      sector boundaries can reduce this to the minimum of 1 erase cycle.
 *  - When @c false is given, FLASH memory is not checked and not erased.
 * @note Provide @c true as argument for @c checkFlash when the intention is to store new samples afterwards.
 */
void Storage_Reset(bool checkFlash);

/**
 * Stores @c n samples.
 * @pre EEPROM is initialized
 * @param pSamples Pointer to the start of the array where to copy the samples from. For each element of the array,
 *  only the #STORAGE_BITSIZE LSBits are copied.
 * @param n The size of the array @c pSamples points to, in number of #STORAGE_TYPE elements.
 * @return The number of samples written. This value may be @c 0 or any number of samples less than or equal to @c n.
 * @note When a value less than @c n is returned, at least one of these errors occurred:
 *  - There is insufficient storage capacity
 *  - Compressing of samples was necessary during the call, but that operation yielded an error.
 * @note A prior call to #Storage_Seek is @b not required, as writing will always @b append the new samples.
 * @post A later call to #Storage_DeInit is necessary to ensure the data can survive Deep power down state.
 * @warning Data is not guaranteed to be stored in EEPROM or FLASH: reset can lose some of the last samples written.
 */
int Storage_Write(STORAGE_TYPE * pSamples, int n);

/**
 * Determines which sample is read out next in a future call to #Storage_Read. This call is
 * required to be called once before calling #Storage_Read one or multiple times.
 * @param n Must be a positive number. A value of @c 0 indicates the oldest sample, which was written first.
 * @return @c true when the sought for sequence number was found; @c false otherwise.
 * @post the next call to #Storage_Read will either return at least one sample - the value
 *  which was stored as the @c n-th sample - or fail - when less than @c n samples are being stored at the time of
 *  calling this function.
 * @note After initialization, the default sequence number is @em not @c 0. First, a call to this function is required
 *  before #Storage_Read can retrieve samples.
 */
bool Storage_Seek(int n);

/**
 * Reads @c n samples from persistent storage, starting from the sequence number set in #Storage_Seek.
 * @pre EEPROM is initialized
 * @pre A prior successful call to #Storage_Seek is required before this function can succeed.
 * @note Multiple reads can be issued after calling #Storage_Seek once, each time fetching samples in sequence.
 * @param [out] pSamples : Pointer to an array of @n elements, where the read samples are copied to. Upon successful
 *  completion, each element will contain one sample, where only the #STORAGE_BITSIZE LSBits are used per
 *  element; the remainder MSBits are set to 0.
 * @param n : The size of the array @c samples points to, in number of #STORAGE_TYPE elements.
 * @return The number of samples read. This value may be @c 0 or any number of samples less than @c n.
 *  The remainder of the elements with a higher index may have been written to, but must be ignored.
 * @note When a value less than @c n is returned, at least one of these errors occurred:
 *  - There was no prior successful call to #Storage_Seek
 *  - Decompressing of samples was necessary during the call, but that operation yielded an error.
 *  - There are no more samples stored.
 */
int Storage_Read(STORAGE_TYPE * pSamples, int n);

/**
 * @cond STORAGE_DOC
 * @mainpage storage: NVM Storage module
 * @copydoc MODS_NSS_STORAGE
 * @endcond
 */
#endif /** @} */
