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

#ifndef __COMPRESS_DFT_H_
#define __COMPRESS_DFT_H_

/** @defgroup MODS_NSS_COMPRESS_DFT Diversity Settings
 *  @ingroup MODS_NSS_COMPRESS
 *
 * The application can adapt the compress module to better fit the different application scenarios through the use of
 * diversity flags in the form of defines below. Sensible defaults are chosen; to override the default settings, place
 * the defines with their desired values in the application app_sel.h header file: the compiler will pick up your
 * defines before parsing this file.
 *
 * @{
 */

/**
 * The window size determines how far back in the input can be searched for repeated patterns. A value of 8 will only
 * use 2^8 == 256 bytes, while a value of 10 will use 2^10 == 1024 bytes. The latter uses more memory, but may also
 * compress more effectively by detecting more repetition.
 * It is a buffer size parameter that is explicit trade-off between compression effectiveness and working memory.
 * - required stack size for compression: 16 + 2<<w if no index is enabled.
 * - required stack size for compression: 16 + 4<<w if indexing is enabled.
 * - required stack size for decompression: 46 + 1<<w regardless of indexing.
 */
#ifndef COMPRESS_WINDOW_BITS
    #define COMPRESS_WINDOW_BITS 10
#endif
#if (COMPRESS_WINDOW_BITS < 4) || (COMPRESS_WINDOW_BITS > 11)
    #error COMPRESS_WINDOW_BITS must be in the range [4, 11]
#endif

/**
 * The lookahead size determines the maximum length for repeated patterns that are found.
 * If equal to 4, a 50-byte run of 'a' characters will be represented as several repeated 2^4 == 16-byte patterns,
 * whereas a larger value may be able to represent it all at once.
 * The number of bits used for the lookahead size is fixed, so an overly large lookahead size can reduce compression
 * by adding unused size bits to small patterns.
 */
#ifndef COMPRESS_LOOKAHEAD_BITS
    #define COMPRESS_LOOKAHEAD_BITS 4
#endif
#if (COMPRESS_LOOKAHEAD_BITS < 3) || (COMPRESS_LOOKAHEAD_BITS >= COMPRESS_WINDOW_BITS)
    #error COMPRESS_LOOKAHEAD_BITS must be in the range [3, COMPRESS_WINDOW_BITS[
#endif

/**
 * Enables indexing. Indexing greatly reduces the compression time - possibly up to a factor of 5 (!). It is not used
 * for decompression. Enabling it roughly doubles the required stack size for compression, plus requires temporarily
 * an extra 512 bytes while the index being built up.
 */
#ifndef COMPRESS_USE_INDEX
    #define COMPRESS_USE_INDEX 0
#endif

/* Dynamic allocation is explicitly disabled for compression. This is non-configurable. */
#undef HEATSHRINK_DYNAMIC_ALLOC

#endif /** @} */
