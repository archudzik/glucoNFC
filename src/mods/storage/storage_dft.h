/*
 * Copyright 2016-2023 NXP
 * NXP confidential.
 * This software is owned or controlled by NXP and may only be used strictly
 * in accordance with the applicable license terms.  By expressly accepting
 * such terms or by downloading, installing, activating and/or otherwise using
 * the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to
 * be bound by the applicable license terms, then you may not retain, install,
 * activate or otherwise use the software.
 */

/**
 * @defgroup MODS_NSS_STORAGE_DFT Diversity Settings
 *  @ingroup MODS_NSS_STORAGE
 *
 * The application can adapt the storage module to better fit the different application scenarios through the use of
 * diversity settings in the form of defines below. Sensible defaults are chosen. To override the default settings,
 * place the defines with their desired values in the application app_sel.h header file. The compiler will pick up your
 * defines before parsing this file.
 *
 * Additional notes regarding some flags:
 * - By default, the assigned EEPROM region takes up 2KB and is located just below the read-only EEPROM rows. This
 *  storage space can be moved and resized by adapting #STORAGE_EEPROM_FIRST_ROW and #STORAGE_EEPROM_LAST_ROW.
 * - By default, the remaining FLASH memory after programming is used for data storage: the available free
 *  FLASH space is automatically determined. It can also be manually assigned: move and resize the storage space in
 *  FLASH by adapting #STORAGE_FLASH_FIRST_PAGE and #STORAGE_FLASH_LAST_PAGE.
 * - Data is stored decompressed in EEPROM; a chance is given to the application to compress the data before it is moved
 *  to FLASH. For this, define both #STORAGE_COMPRESS_CB and #STORAGE_DECOMPRESS_CB.
 * - Flash operations should be used sparingly. The amount of data transferred as one block from EEPROM to FLASH can be
 *  controlled using #STORAGE_BLOCK_SIZE_IN_SAMPLES. Too little data puts unnecessary strain on the battery and reduces
 *  compression abilities; too much data risks a portion of the FLASH to remain not utilized when a compressed block of
 *  samples no longer fits.
 * - The storage module requires the use of at least one general purpose register. This cannot be disabled.
 *
 * These flags may be overridden:
 * - #STORAGE_FIRST_ALON_REGISTER
 * - #STORAGE_SAMPLE_ALON_CACHE_COUNT
 * - #STORAGE_EEPROM_FIRST_ROW
 * - #STORAGE_EEPROM_LAST_ROW
 * - #STORAGE_FLASH_FIRST_PAGE
 * - #STORAGE_FLASH_LAST_PAGE
 * - #STORAGE_TYPE
 * - #STORAGE_BITSIZE
 * - #STORAGE_SIGNED
 * - #STORAGE_WORKAREA
 * - #STORAGE_WRITE_RECOVERY_EVERY_X_SAMPLES
 * - #STORAGE_BLOCK_SIZE_IN_SAMPLES
 * - #STORAGE_REDUCE_RECOVERY_WRITES
 * - #STORAGE_COMPRESS_CB
 * - #STORAGE_DECOMPRESS_CB
 *
 * These defines are fixed or derived from the above flags and may not be defined or redefined in an application:
 * - #STORAGE_EEPROM_ROW_COUNT
 * - #STORAGE_EEPROM_SIZE
 * - #STORAGE_MAX_SAMPLE_ALON_CACHE_COUNT
 * - #STORAGE_WORKAREA_SELF_DEFINED
 * - #STORAGE_WORKAREA_SIZE
 * - #STORAGE_MAX_BLOCK_SIZE_IN_SAMPLES
 * - #STORAGE_BLOCK_HEADER_SIZE
 * - #STORAGE_MAX_UNCOMPRESSED_BLOCK_SIZE_IN_BITS
 * - #STORAGE_MAX_LOSS_AFTER_CORRUPTION
 * - #STORAGE_UNCOMPRESSED_BLOCK_SIZE_IN_BITS
 * - #STORAGE_UNCOMPRESSED_BLOCK_SIZE_IN_BYTES
 *
 *  @par Choosing correct values
 *      The storage module has many diversity settings, and can be tweaked a lot. Storing data reliably and as
 *      efficient as possible is a complex operation, and every use case will need to adjust a few settings, not
 *      relying on the default values.
 *
 *      To start, determine the correct sizes of the three memory regions the storage module has exclusive access of:
 *      - General purpose registers: determined via #STORAGE_FIRST_ALON_REGISTER
 *          More general purpose registers means more data can be cached before writing to EEPROM. The more are
 *          assigned, the less EPROM flushes are performed, which prolongs life for batteries with a higher battery
 *          impedance (think printed batteries). But when things fail, all these samples will be lost.
 *      - EEPROM: #STORAGE_EEPROM_FIRST_ROW and #STORAGE_EEPROM_LAST_ROW
 *          By default, a conservative choice is made, leaving ~2 kB alone, for other uses of the application.
 *          The larger the assigned region, the more samples can be stored. This means less room left for the
 *          application to store its state, events and validation results.
 *      - FLASH: #STORAGE_FLASH_FIRST_PAGE and #STORAGE_FLASH_LAST_PAGE
 *          By default, all space not used by the program itself, is used for data storage by the storage module.
 *          Unless your application requires to write to flash for other purposes, it is advised to stick with the
 *          default settings.
 *
 *      Next, determine what will be stored. Each sample given to the storage module must have the same type. Both
 *      basic types and structures or unions are possible. Each sample will be placed back to back, without a single
 *      padding bit; if you know that a few MSBits are always 0, you can choose to store fewer bits, resulting in a
 *      larger storage capacity.
 *      - #STORAGE_TYPE
 *      - #STORAGE_BITSIZE
 *      - #STORAGE_SIGNED
 *
 *      Compression greatly increases the number of samples that can be stored, at the cost of the size of the
 *      compression library itself. Normally, the compression ratio is greater when more samples are compressed at
 *      once.
 *      The storage module allows you to insert function calls to compress and decompress a large number of
 *      samples automatically. @n
 *      Keep in mind that enlarging #STORAGE_UNCOMPRESSED_BLOCK_SIZE_IN_BYTES also enlarges
 *      #STORAGE_WORKAREA_SIZE, and the memory required for compressing and decompressing.
 *      - #STORAGE_BLOCK_SIZE_IN_SAMPLES
 *      - #STORAGE_COMPRESS_CB
 *      - #STORAGE_DECOMPRESS_CB
 *      If a compression is to be used which operates on a single sample at a time, the application can wrap the calls
 *      to the storage module and implement this outside the storage module. An example of this is a mapping operation
 *      which reduces the resolution in specific value ranges.
 *      Be sure to adapt the type and bitsize to match the encoded values.
 *
 *      Data recovery is paramount. Especially when at the end of the use case lifetime, when dealing with batteries
 *      which can't maintain a stable voltage any more, the risk of data corruption increases. The chance for data
 *      corruption to occur is highly dependent on your specific application, your choice of battery, and your
 *      environmental conditions. And apart from data corruption, the IC might also reset during flushing of EEPROM data,
 *      or during a FLASH program operation. When any of this happens, the storage module must be able to return as
 *      many samples as possible.
 *      @n The storage module ensures only a few most recent samples may get lost.
 *      - The samples stored in the general purposes registers were already mentioned. The number can be defined exactly
 *          using #STORAGE_SAMPLE_ALON_CACHE_COUNT
 *      - If data corruption occurs in EEPROM - or when data corruption in EEPROM is suspected - none of the data in
 *          the affected row can be used anymore. The storage module therefore stores a copy the last data row on a
 *          fixed location.
 *          Since the write endurance limit per EEPROM row is only 10.000, the storage module cannot copy this each
 *          time. Via the setting #STORAGE_WRITE_RECOVERY_EVERY_X_SAMPLES the application can decide, taking into
 *          account the maximum number of samples that are to be stored.
 *          If in your use case less than 30.000 samples are stored over the full lifetime of the product, this value
 *          would then be set to 3.
 *      - If data corruption occurs in FLASH, the program will avoid all FLASH operations thereafter. No samples are
 *          lost, but the total storage capacity diminishes, as the FLASH page where the corruption occurred and all
 *          higher FLASH sectors will not be used for data storage any more.
 *      - Writing recovery information in itself can also have an averse impact on the battery. The application is
 *          therefore given an additional choice: the number of EEPROM flushes due to writing recovery data can be
 *          halved, at the expense of an additional 8 bytes of samples which may get lost when data corruption occurs.
 *          If #STORAGE_REDUCE_RECOVERY_WRITES is enabled, only one row is reserved for recovery information, and only
 *          56 bytes are duplicated.
 *      The maximum amount of samples that may get lost due to an unexpected reset, a battery failure, or a data
 *      corruption, is captured in #STORAGE_MAX_LOSS_AFTER_CORRUPTION
 *      To recover all samples safe for the very last, at the expense of (a lot) more EEPROM flush operations, use these
 *      settings:
 *      @code
 *          #define STORAGE_SAMPLE_ALON_CACHE_COUNT 0
 *          #define STORAGE_WRITE_RECOVERY_EVERY_X_SAMPLES 1
 *          #define STORAGE_REDUCE_RECOVERY_WRITES 0
 *      @endcode
 * @{
 */
#ifndef __STORAGE_DFT_H_
#define __STORAGE_DFT_H_

/**
 * Helper. Performs integer division, rounding up.
 * @param n Must be a positive number.
 * @param d Must be a strict positive number
 * @internal
 */
#define STORAGE_IDIVUP(n, d) (((n)+(d)-1)/(d))

#ifdef STORAGE_CONFIG_ALON_REGISTER
    /** @warning Its use is discouraged. Present only for backward compatibility. Will be removed in a later SDK. */
    #define STORAGE_FIRST_ALON_REGISTER STORAGE_CONFIG_ALON_REGISTER
#endif

#ifndef STORAGE_FIRST_ALON_REGISTER
    /**
     * The storage module requires the use of at least one general purpose register. This is used for its own
     * housekeeping and allows it to finish initialization (#Storage_Init) fast.
     * In addition, @b all other general purposes registers with an index @b higher @b than given are also reserved for
     * the storage module. These will be used to minimize EEPROM flush operations by caching as many samples as possible
     * before committing them to EEPROM.
     * @pre These registers must be guaranteed not to be touched outside the storage module.
     * @note More information about the ALON registers can be found in the Power Management Unit driver.
     * @see STORAGE_SAMPLE_ALON_CACHE_COUNT
     */
    #define STORAGE_FIRST_ALON_REGISTER 4
#endif
#if !(STORAGE_FIRST_ALON_REGISTER >= 0) || !(STORAGE_FIRST_ALON_REGISTER <= 4)
    #error Invalid value for STORAGE_FIRST_ALON_REGISTER
#endif

/* ------------------------------------------------------------------------- */

#ifndef STORAGE_EEPROM_FIRST_ROW
    /**
     * The first EEPROM row assigned for sample storage. Starting from the first byte in this row, until the last byte
     * in #STORAGE_EEPROM_LAST_ROW, the storage module has full control: no other code may touch this EEPROM region.
     * @note By default, the EEPROM row 2 kB below the locked EEPROM rows will be chosen.
     */
    #define STORAGE_EEPROM_FIRST_ROW (EEPROM_NR_OF_RW_ROWS - (2048 / EEPROM_ROW_SIZE))
#endif
#if !(STORAGE_EEPROM_FIRST_ROW >= 0) || !(STORAGE_EEPROM_FIRST_ROW < EEPROM_NR_OF_RW_ROWS)
    #error Invalid value for STORAGE_EEPROM_FIRST_ROW
#endif

#ifndef STORAGE_EEPROM_LAST_ROW
    /**
     * The last EEPROM row assigned for sample storage. Starting from the first byte in #STORAGE_EEPROM_FIRST_ROW,
     * until the last byte in this row, the storage module has full control: no other code may touch this EEPROM region.
     * @note the size of the EEPROM region is not required to be a multiple of the FLASH sector size.
     */
    #define STORAGE_EEPROM_LAST_ROW (EEPROM_NR_OF_RW_ROWS - 1)
#endif
#if !(STORAGE_EEPROM_LAST_ROW >= STORAGE_EEPROM_FIRST_ROW) || !(STORAGE_EEPROM_LAST_ROW < EEPROM_NR_OF_RW_ROWS)
    #error Invalid value for STORAGE_EEPROM_LAST_ROW
#endif

/** The number of EEPROM rows assigned for sample storage. */
#define STORAGE_EEPROM_ROW_COUNT (STORAGE_EEPROM_LAST_ROW - STORAGE_EEPROM_FIRST_ROW + 1)
#if STORAGE_EEPROM_ROW_COUNT < 3
    #error STORAGE_EEPROM_LAST_ROW and STORAGE_EEPROM_FIRST_ROW must be farther apart: at least 3 EEPROM pages must be assigned for storage
#endif

/** The size of the assigned EEPROM storage in bytes */
#define STORAGE_EEPROM_SIZE (STORAGE_EEPROM_ROW_COUNT * EEPROM_ROW_SIZE)

/* ------------------------------------------------------------------------- */

#ifndef STORAGE_FLASH_FIRST_PAGE
    /**
     * The first FLASH page assigned for storage. Starting from the first byte in this page, until the last byte
     * in #STORAGE_FLASH_LAST_PAGE, the Storage module has full control: no other code will touch this FLASH region.
     * @note Two special values exist:
     *  - When equal to @c 0, the first empty FLASH page @b after the @c .text and @c .data
     *      sections will be used. This page is automatically determined during link time by the Storage module's code.
     *  - When a value strict higher than #STORAGE_FLASH_LAST_PAGE is provided, storage to FLASH is disabled. Only the
     *      EEPROM will be used to store the data.
     * @note The page is not required to be sector aligned.
     * @warning If a non-zero value is specified, it is the responsibility of the application programmer to ensure the
     *  FLASH location points to outside the @c .text and @c .data sections.
     */
    #define STORAGE_FLASH_FIRST_PAGE 0
#endif
#if !(STORAGE_FLASH_FIRST_PAGE >= 0) || (STORAGE_FLASH_FIRST_PAGE >= FLASH_NR_OF_RW_SECTORS * FLASH_PAGES_PER_SECTOR)
    #error Invalid value for STORAGE_FLASH_FIRST_PAGE
#endif

#ifndef STORAGE_FLASH_LAST_PAGE
    /**
     * The last FLASH page assigned for storage. Starting from the first byte in #STORAGE_FLASH_FIRST_PAGE,
     * until the last byte in this page, the Storage module has full control: no other code will touch this FLASH
     * region.
     * @note the size of the region is not required to be a multiple of the FLASH sector size.
     */
    #define STORAGE_FLASH_LAST_PAGE (FLASH_NR_OF_RW_SECTORS * FLASH_PAGES_PER_SECTOR - 1)
#endif
#if STORAGE_FLASH_LAST_PAGE >= FLASH_NR_OF_RW_SECTORS * FLASH_PAGES_PER_SECTOR
    #error Invalid value for STORAGE_FLASH_LAST_PAGE
#endif
#if STORAGE_FLASH_LAST_PAGE < STORAGE_FLASH_FIRST_PAGE
    #warning Flash will not be used for data storage
#endif
#if (STORAGE_FLASH_FIRST_PAGE == 0) && (STORAGE_FLASH_LAST_PAGE != (FLASH_NR_OF_RW_SECTORS * FLASH_PAGES_PER_SECTOR - 1))
    #error STORAGE_FLASH_FIRST_PAGE is determined at link time; STORAGE_FLASH_LAST_PAGE can thus not be checked at precompilation time.
    #error STORAGE_FLASH_LAST_PAGE must be set to the highest possible value in this case.
#endif

/* ------------------------------------------------------------------------- */

#ifndef STORAGE_TYPE
    /**
     * The type that is used to store one decompressed sample. When writing, samples are to be delivered using this
     * type - see #Storage_Write; when reading, samples are returned again using this type -
     * see #Storage_Read.
     */
    #define STORAGE_TYPE uint8_t
#endif

#ifndef STORAGE_BITSIZE
    /**
     * The number of bits that are to be stored for each sample. For each sample given using #Storage_Write
     * this number of LSBits are written;
     */
    #define STORAGE_BITSIZE 8
#endif
#if (STORAGE_BITSIZE <= 0)
    #error Invalid value for STORAGE_BITSIZE
#endif

/**
 * The maximum number of samples that can be cached in the ALON general purposes registers before they are committed to EEPROM.
 */
#define STORAGE_MAX_SAMPLE_ALON_CACHE_COUNT ((141 - STORAGE_FIRST_ALON_REGISTER * 32) / STORAGE_BITSIZE)
/* Magic value 141 is checked at compile time in storage.c */

#ifndef STORAGE_SAMPLE_ALON_CACHE_COUNT
    /**
     * The number of samples that are cached in the ALON general purposes registers before they are committed to EEPROM.
     */
    #define STORAGE_SAMPLE_ALON_CACHE_COUNT STORAGE_MAX_SAMPLE_ALON_CACHE_COUNT
#endif
#if STORAGE_SAMPLE_ALON_CACHE_COUNT > STORAGE_MAX_SAMPLE_ALON_CACHE_COUNT
    #error STORAGE_SAMPLE_ALON_CACHE_COUNT must be less than or equal to STORAGE_MAX_SAMPLE_ALON_CACHE_COUNT
#endif
#if (STORAGE_MAX_SAMPLE_ALON_CACHE_COUNT - STORAGE_SAMPLE_ALON_CACHE_COUNT) * STORAGE_BITSIZE >= 32
    #warning STORAGE_FIRST_ALON_REGISTER can be set higher, as the last general purpose register is reserved by but will not be used by the storage module
#endif


#ifndef STORAGE_SIGNED
    /**
     * Define or set to a non-zero value (e.g. @c 1) to indicate that #STORAGE_TYPE is a signed type.
     * - If defined, the bit at position @code (#STORAGE_BITSIZE - 1) @endcode will be treated as the sign bit:
     *  when reading out samples from EEPROM or FLASH, this bit will be propagated left up to the MSBit of
     *  #STORAGE_TYPE.
     * - If not defined, the MSBits at positions #STORAGE_BITSIZE and up will be set to @c 0.
     * @warning Setting this diversity flag to 1 while #STORAGE_TYPE is a structure, while raise compiler errors.
     * see #Storage_Read.
     */
    #define STORAGE_SIGNED 0
#endif

/* ------------------------------------------------------------------------- */

#ifndef STORAGE_WRITE_RECOVERY_EVERY_X_SAMPLES
    /**
     * Update the recovery information after adding @c X samples to @b non-volatile memory. This recovery information
     * is written on a fixed location. When an unexpected reset occurs or a corruption of the data while writing new
     * content, the recovery information is used to recover as much data as possible.
     * @note The recovery information is only written in the call to #Storage_DeInit. Depending on the number of calls
     *  to #Storage_Write and its arguments, it is possible more samples have been written than this number.
     * @pre maximum number of expected data samples / @c STORAGE_WRITE_RECOVERY_EVERY_X_SAMPLES < 10.000
     */
    #define STORAGE_WRITE_RECOVERY_EVERY_X_SAMPLES (1 + STORAGE_SAMPLE_ALON_CACHE_COUNT)
#endif
#if STORAGE_WRITE_RECOVERY_EVERY_X_SAMPLES <= 0
    #error STORAGE_WRITE_RECOVERY_EVERY_X_SAMPLES must be a strict positive number
#endif
#if STORAGE_WRITE_RECOVERY_EVERY_X_SAMPLES < STORAGE_SAMPLE_ALON_CACHE_COUNT
    #warning STORAGE_WRITE_RECOVERY_EVERY_X_SAMPLES is smaller than STORAGE_SAMPLE_ALON_CACHE_COUNT, which makes little sense.
    #warning Recovery information will be written in Storage_DeInit after STORAGE_SAMPLE_ALON_CACHE_COUNT samples.
#endif

#ifndef STORAGE_REDUCE_RECOVERY_WRITES
    /**
     * - If not defined, or defined to zero, two rows of the assigned EEPROM region for data storage are used for data
     *  recovery. When data corruption occurs, no extra loss of bytes will occur, only
     *  #STORAGE_SAMPLE_ALON_CACHE_COUNT and #STORAGE_WRITE_RECOVERY_EVERY_X_SAMPLES contribute to this.
     * - If defined to a non-zero value:
     *  the space used for recovery - where the duplicate data of the last row and the hint information is stored - is
     *  reduced to just one row. When data corruption occurs, an additional loss of up to 8 bytes can occur.
     */
    #define STORAGE_REDUCE_RECOVERY_WRITES 0
#endif
#if (STORAGE_REDUCE_RECOVERY_WRITES != 0) && (STORAGE_REDUCE_RECOVERY_WRITES != 1)
    #error Leave STORAGE_REDUCE_RECOVERY_WRITES undefined in app_sel.h, or define it to 0 (--> 2 recovery rows) or 1 (--> 1 recovery row)
#endif

/**
 * The maximum loss of samples that can occur. The storage module may not be able to return the most recently stored
 * samples after an EEPROM corruption occurs. A scenario where this can happen is when printed batteries are used,
 * and the battery has an internal impedance of 2 kOhm or higher, resulting in a barely sufficient voltage when
 * operating normally, and a below-spec voltage when extra load is generated due to an EEPROM flush operation.
 * @see STORAGE_SAMPLE_ALON_CACHE_COUNT
 * @see STORAGE_WRITE_RECOVERY_EVERY_X_SAMPLES
 * @see STORAGE_REDUCE_RECOVERY_WRITES
 */
#define STORAGE_MAX_LOSS_AFTER_CORRUPTION 1 \
        + STORAGE_SAMPLE_ALON_CACHE_COUNT \
        + STORAGE_WRITE_RECOVERY_EVERY_X_SAMPLES \
        + STORAGE_REDUCE_RECOVERY_WRITES * STORAGE_IDIVUP(8 * 8, STORAGE_BITSIZE)

/**
 * The maximum allowed value for #STORAGE_BLOCK_SIZE_IN_SAMPLES. After this many samples, the assigned EEPROM region is
 * completely filled and no new sample can be written to EEPROM before its contents are moved to FLASH first.
 * @hideinitializer
 */
#if STORAGE_REDUCE_RECOVERY_WRITES
    #define STORAGE_MAX_BLOCK_SIZE_IN_SAMPLES (((STORAGE_EEPROM_SIZE - 76) * 8) / STORAGE_BITSIZE)
    /* Magic value 76 is checked at compile time in storage.c */
#else
    #define STORAGE_MAX_BLOCK_SIZE_IN_SAMPLES (((STORAGE_EEPROM_SIZE - 140) * 8) / STORAGE_BITSIZE)
    /* Magic value 140 is checked at compile time in storage.c */
#endif

/** Defines the number of bits required to store one block of samples. */
#define STORAGE_MAX_UNCOMPRESSED_BLOCK_SIZE_IN_BITS (STORAGE_MAX_BLOCK_SIZE_IN_SAMPLES * STORAGE_BITSIZE)

/**
 * The size in bytes of the meta data stored just in front of the (compressed) data block in FLASH.
 */
#define STORAGE_BLOCK_HEADER_SIZE 2

#ifndef STORAGE_BLOCK_SIZE_IN_SAMPLES
    /**
     * After writing this number of samples to EEPROM, the module will try to compress them all at once - see
     * #STORAGE_COMPRESS_CB - and move them to FLASH - defined by #STORAGE_FLASH_FIRST_PAGE and #STORAGE_FLASH_LAST_PAGE.
     * @note This size determines the value of #STORAGE_WORKAREA_SIZE - the larger this number, the more SRAM is
     *  required, and the larger chunks the compression and decompression algorithms have to work with.
     * @note By default, the block size will be chosen such that an uncompressed block can fit in one FLASH sector,
     *  including the accompanying meta data.
     */
    #define STORAGE_BLOCK_SIZE_IN_SAMPLES (((1024 - STORAGE_BLOCK_HEADER_SIZE) * 8) / STORAGE_BITSIZE)
    #if STORAGE_BLOCK_SIZE_IN_SAMPLES > STORAGE_MAX_BLOCK_SIZE_IN_SAMPLES
        #undef STORAGE_BLOCK_SIZE_IN_SAMPLES
        #define STORAGE_BLOCK_SIZE_IN_SAMPLES STORAGE_MAX_BLOCK_SIZE_IN_SAMPLES
    #endif
#endif
#if ((STORAGE_BLOCK_SIZE_IN_SAMPLES <= 1) || (STORAGE_BLOCK_SIZE_IN_SAMPLES > STORAGE_MAX_BLOCK_SIZE_IN_SAMPLES))
    #error Invalid value for STORAGE_BLOCK_SIZE_IN_SAMPLES
#endif

/** Defines the number of bits required to store one block of samples. */
#define STORAGE_UNCOMPRESSED_BLOCK_SIZE_IN_BITS (STORAGE_BLOCK_SIZE_IN_SAMPLES * STORAGE_BITSIZE)

/** Defines the number of bytes required to store one block of samples. */
#define STORAGE_UNCOMPRESSED_BLOCK_SIZE_IN_BYTES STORAGE_IDIVUP(STORAGE_UNCOMPRESSED_BLOCK_SIZE_IN_BITS, 8)

/** The size in bytes of the required memory for this module */
#define STORAGE_WORKAREA_SIZE ((FLASH_PAGE_SIZE * 2) + STORAGE_UNCOMPRESSED_BLOCK_SIZE_IN_BYTES)

#ifdef STORAGE_WORKAREA
    #undef STORAGE_WORKAREA_SELF_DEFINED
#else
    /**
     * Used internally to know when to provide (static) memory for this.
     * @internal
     */
    #define STORAGE_WORKAREA_SELF_DEFINED 1
    /**
     * An array, or a pointer to an array of at least #STORAGE_WORKAREA_SIZE bytes.
     * During the lifetime of this module - starting from the start of the call to #Storage_Init until the end of
     * the call to #Storage_DeInit, this memory is under full control by this module.
     * @pre Must be word (32 bits) aligned.
     */
    #define STORAGE_WORKAREA sStorage_Workarea
#endif

/* ------------------------------------------------------------------------- */

#ifdef STORAGE_COMPRESS_CB
    #undef STORAGE_COMPRESS_CB_SELF_DEFINED
#else
    /**
     * The name of the function - @b not a function pointer - of type #pStorage_CompressCb_t that is able to
     * compress #STORAGE_BLOCK_SIZE_IN_SAMPLES packed samples of type #STORAGE_TYPE where #STORAGE_BITSIZE bits
     * are retained for each.
     * @note When not overridden, the default behavior is to store the data uncompressed, i.e. the data is copied from
     *  EEPROM to FLASH unmodified.
     * @hideinitializer
     */
    #define STORAGE_COMPRESS_CB Storage_DummyCompressCb
#endif

#ifdef STORAGE_DECOMPRESS_CB
    #undef STORAGE_DECOMPRESS_CB_SELF_DEFINED
#else
    /**
     * The name of the function - @b not a function pointer - of type #pStorage_DecompressCb_t that is able to
     * decompress #STORAGE_BLOCK_SIZE_IN_SAMPLES packed samples of type #STORAGE_TYPE where #STORAGE_BITSIZE bits
     * are retained for each.
     * @hideinitializer
     */
    #define STORAGE_DECOMPRESS_CB Storage_DummyDecompressCb
#endif

#endif /** @} */
