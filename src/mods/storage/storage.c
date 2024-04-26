/*
 * Copyright 2016-2021 NXP
 * This software is owned or controlled by NXP and may only be used strictly
 * in accordance with the applicable license terms.  By expressly accepting
 * such terms or by downloading, installing, activating and/or otherwise using
 * the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to
 * be bound by the applicable license terms, then you may not retain, install,
 * activate or otherwise use the software.
 */

#include <string.h>
#include "storage.h"

/**
 * @file
 *
 *  @par Maintaining state
 *
 *  It is a crucial feature of this module to be able to resume operation at all times. Between de-initialization
 *  and re-initialization anything can happen, and no volatile memory can be trusted. Still, speed of operation is
 *  necessary. To be able to restore the exact state - done in #Storage_Init - three different recovery structures
 *  are used:
 *  - #RecoverInfo_t - stored in the general purpose registers
 *  - #Hint_t - stored on a fixed location in EEPROM
 *  - #Marker_t - stored on a variable location, immediately after the last written sample in EEPROM
 *  After recovery, state is maintained in SRAM using the structure #Storage_Instance_t, and in #Storage_DeInit at
 *  least two of the three recovery structures are updated.
 *  @n The recovery structures are linked to each other as explained below.
 *
 *  - #RecoverInfo_t
 *      This data is stored in the designated ALON register #STORAGE_FIRST_ALON_REGISTER and is by far the fastest
 *      and easiest way to recover the current state. It is always tried first. If its data is non-zero, the
 *      information is trusted and used.
 *  - #Hint_t
 *       This data is stored in the last page of the assigned EEPROM region, and also enables recovery of the
 *       current state in a fast way. However, this structure may be out of date as it may not be updated at every
 *       change.
 *       The reason for not updating is to ensure the maximum number of writes (endurance) is never reached.
 *       However, this data will always accurately tell
 *       - whether samples are stored or not
 *       - how much flash is occupied by data storage
 *       It also provides a possible location to #Marker_t, which may be outdated.
 *  - #Marker_t
 *       This data is always stored just after the last sample written in EEPROM. Its precise location is not known,
 *       as it is progressing together with the data. #RecoverInfo_t points to the start of this structure in the
 *       assigned EEPROM region; #Hint_t may point to it; if both are failing a full slow search is performed in the
 *       assigned EEPROM region.
 *
 *  When a full backward search is performed during data recovery, we rely on the length of the marker to eliminate
 *  false positives, i.e. to ensure no bit sequence exists that looks like a valid marker but are in reality one or
 *  more samples stored in EEPROM. The comments near the code in #FindMarker explain the length and value of the
 *  marker avoid finding false positives. Yet, although extremely improbable, there is still a very tiny possibility
 *  that real-life data being stored in EEPROM results in such a false positive.
 *  Even then, most applications will never have the need to perform a full backward search, evading this problem.
 *  If your application is more likely to hit this, due to the use case being sufficiently different, and due to the
 *  samples being sufficiently similar, this module can be adapted to erase the full EEPROM after moving all data to
 *  FLASH - see #MoveSamplesFromEepromToFlash. During our continuous tests using randomized data of different
 *  lengths, we have never encountered this problem and therefore decided not to suffer the extra cost (time and
 *  power consumption) of implementing this.
 *
 *  @note the pictures below use this legend:
 *      - @c aaa, @c bbb, ... @c fff: samples, with bitsize #STORAGE_BITSIZE
 *      - @c AA: ALON register, formatted according to #RecoverInfo_t
 *      - @c HH: hint, formatted according to #Hint_t
 *      - @c MMMMMMM: marker, formatted according to #Marker_t
 *      - @c BB: block information, of size #FLASH_DATA_HEADER_SIZE
 *      - @c xxxx: block bytes, the (possibly compressed) data bytes moved from EEPROM and now stored in FLASH
 *
 *  @par Recovering
 *
 *  In #Storage_DeInit, at least the ALON register (#RecoverInfo_t) and the EEPROM marker (#Marker_t) are updated.
 *  Assuming only one sample was added to the non-volatile EEPROM memory, the EEPROM contents would then change as
 *  visually depicted below:
 *
 *  @code
 *        EEPROM                            EEPROM
 *       with samples a..d                 with samples a..e
 *      +---------+                       +---------+
 *      | aaabbbc |                       | aaabbbc |
 *      | ccdddMM |     After calling     | ccdddee |
 *      | MMMMM   |     Storage_Write     | eMMMMMM |
 *      |         |     with n=1:         | MM      |
 *      |         |  ------------------>  |         |
 *      |      HH |                       |      HH |
 *      +---------+                       +---------+
 *
 *  @endcode
 *
 *  The ALON register will then point to the new location of the marker; while the hint will not be updated and will
 *  for sure contain faulty information.
 *
 *  @dot
 *      digraph "Alon, Marker OK" {
 *          node [shape=box];
 *          subgraph a {
 *              rank=same
 *              AA              [label="ALON register", URL="\ref STORAGE_FIRST_ALON_REGISTER"]
 *          }
 *          subgraph b {
 *              rank=same
 *              MMMMMMM         [label="Moving marker in EEPROM", URL="\ref Marker_t"]
 *              HH              [label="Hint information on a\nfixed EEPROM location", URL="\ref HINT_ABSOLUTE_BYTE_OFFSET"]
 *          }
 *          subgraph c {
 *              rank=same
 *              flash           [label="flashByteCursor", URL="\ref Storage_Instance_t.flashByteCursor"]
 *              random          [label="Wrong location", shape=point]
 *          }
 *
 *          AA -> MMMMMMM
 *          HH -> random
 *          MMMMMMM -> flash
 *      }
 *  @enddot
 *
 *  When power gets lost, the ALON register contents is reset to @c 0, and will then point to a faulty location as
 *  well.
 *
 *  @dot
 *      digraph "Marker OK" {
 *          node [shape=box];
 *          subgraph a {
 *              rank=same
 *              AA              [label="ALON register", URL="\ref STORAGE_FIRST_ALON_REGISTER"]
 *          }
 *          subgraph b {
 *              rank=same
 *              MMMMMMM         [label="Moving marker in EEPROM", URL="\ref Marker_t"]
 *              HH              [label="Hint information on a\nfixed EEPROM location", URL="\ref HINT_ABSOLUTE_BYTE_OFFSET"]
 *              NULL            [label="0"]
 *          }
 *          subgraph c {
 *              rank=same
 *              flash           [label="flashByteCursor", URL="\ref Storage_Instance_t.flashByteCursor"]
 *              random          [shape=point]
 *          }
 *
 *          AA -> NULL
 *          HH -> random
 *          MMMMMMM -> flash
 *      }
 *  @enddot
 *
 *  Only the marker will then still be correct, which can be recovered only through a full backward search. When
 *  that is done, after leaving #Storage_DeInit, all three recovery structures will be correctly linked.
 *
 *  @dot
 *      digraph "Alon, Hint, Marker OK" {
 *          node [shape=box];
 *          subgraph a {
 *              rank=same
 *              AA              [label="ALON register", URL="\ref STORAGE_FIRST_ALON_REGISTER"]
 *          }
 *          subgraph b {
 *              rank=same
 *              MMMMMMM         [label="Moving marker in EEPROM", URL="\ref Marker_t"]
 *              HH              [label="Hint information on a\nfixed EEPROM location", URL="\ref HINT_ABSOLUTE_BYTE_OFFSET"]
 *          }
 *          subgraph c {
 *              rank=same
 *              flash           [label="flashByteCursor", URL="\ref Storage_Instance_t.flashByteCursor"]
 *          }
 *
 *          AA -> MMMMMMM
 *          HH -> MMMMMMM
 *          MMMMMMM -> flash
 *      }
 *  @enddot
 *
 *  @par Data corruption
 *
 *  When the battery is degrading, two scenarios can cause a corruption:
 *  - the supplied voltage drops too much during a flash operation, hanging the IC or causing a reset. This can
 *      corrupt the FLASH portion being written to.
 *  - the supplied voltage drops too much during an EEPROM operation, hanging the IC or causing a reset. This can
 *      corrupt the EEPROM row being written to.
 *
 *  The storage module can be configured to be fully resilient against this, so that (almost) no data present in the
 *  non-volatile memories EEPROM and FLASH are lost; and that (almost) all samples can still be read out. Only
 *  the samples written after the last update to the hint structure cannot be recovered, as no duplicate information
 *  is yet available.
 *
 *  After a hard reset, the general purpose registers are reset to 0, and the samples temporarily stored there are
 *  lost as well. This brings the total number of minimum lost samples to
 *  #STORAGE_WRITE_RECOVERY_EVERY_X_SAMPLES + #STORAGE_SAMPLE_ALON_CACHE_COUNT - both are diversity settings the
 *  application can set.
 *
 *  @par Using EEPROM @b and FLASH
 *
 *  Both EEPROM and FLASH are used to store data - in the form of equisized samples. Since writing to EEPROM is
 *  cheaper, faster and less complicated than writing to FLASH, it is the preferred storage medium. Whenever the
 *  size of all the samples stored in EEPROM is large enough - see #STORAGE_BLOCK_SIZE_IN_SAMPLES - all that data is
 *  moved in one operation to FLASH. EEPROM is then completely empty, and new samples are then in EEPROM again.
 *
 *  The memory content changes are visually depicted below. If writing a new sample would increase the size to equal
 *  to or higher than #STORAGE_BLOCK_SIZE_IN_SAMPLES, first the EEPROM contents are moved, then the new sample is
 *  written.
 *
 *  @code
 *      Before writing sample f                 After writing sample f
 *
 *        EEPROM                                EEPROM
 *       with samples a..e                     with sample f only
 *      +---------+                           +---------+
 *      | aaabbbc |                           | fffMMMM |
 *      | cc..... |                           | MMM     |
 *      | ....... |                           |         |
 *      | ....ddd |  ---------------------->  |         |
 *      | eeeMMMM |                           |         |
 *      | MMM  HH |                           |      HH |
 *      +---------+                           +---------+
 *
 *        FLASH                                 FLASH
 *       with one block moved from EEPROM      with two blocks moved from EEPROM
 *      +---------+                           +---------+
 *      | BBxxxxx |                           | BBxxxxx |
 *      | xxxxxxx |                           | xxxxxxx |
 *      | x       |  ---------------------->  | xBBxxxx |
 *      |         |                           | xxxxx   |
 *      |         |                           |         |
 *      |         |                           |         |
 *      +---------+                           +---------+
 *  @endcode
 *
 *  @par Caching data
 *
 *  To reduce the number of EEPROM flushes, data is initially @b not stored in non-volatile memories EEPROM and
 *  FLASH. Instead, the reserved general purpose registers are used to cache the data. The value is preserved
 *  when entering and leaving a deep power down, but any inadvertent reset clears the registers, losing the data.
 *  This is not a disadvantage, since the cached data will be moved to non-volatile memory before writing the hint.
 *
 *  The benefit is a reduction of the required EEPROM program operations. This helps in avoiding the write
 *  endurance limit even when storing massive amounts of data, and also reduces the stress on the battery when it
 *  nears end-of-life.
 *
 *  @anchor storage_initializing_par
 *  @par Initializing
 *
 *  The implementation of #Storage_Init tries to recover the complete state as fast as possible. This can be done
 *  when either the recovery info or the hint information points to a valid marker structure. When a slow search
 *  is required, the initialization function make sure a next call will be fast.
 *  @n #Storage_Init implements this flowchart:
 *
 *  @dot
 *      digraph "Storage_Init" {
 *        start [label="", shape=circle];
 *
 *        subgraph cluster_rhf {
 *          rank=same
 *          label="Find Marker"
 *
 *          subgraph r {
 *            node_rv [label="Recovery info\nfrom ALON register\nvalid?", shape=diamond]
 *            node_rg [label="Read marker location\nin EEPROM pointed to\nby recovery info" shape=rectangle]
 *            node_rmv [label="Marker\nvalid?", shape=diamond]
 *          }
 *          subgraph h {
 *            node_hv [label="Hint info\nfrom fixed EEPROM\nlocation valid?", shape=diamond]
 *            node_hg [label="Read marker location\nin EEPROM pointed to\nby hint info" shape=rectangle]
 *            node_hmv [label="Marker\nvalid?", shape=diamond]
 *          }
 *          subgraph f {
 *            node_none [label="----------------------------------", shape=none, fontcolor=white]
 *            node_f [label="Start a slow search\nRead the entire EEPROM\nto find a valid marker", shape=rectangle]
 *            node_fmv [label="Marker\nfound?", shape=diamond]
 *          }
 *        }
 *
 *        subgraph c {
 *          node_c1 [label="Hint info\nvalid?", shape=diamond]
 *          node_c2 [label="Hint info\nvalid?", shape=diamond]
 *        }
 *
 *        subgraph cluster_i {
 *          label="              Initialize instance"
 *
 *          node_inito [label="No data present or\nno data can be recovered.\nWrite a new marker and a new hint", shape=rectangle]
 *          node_inith [label="Recover data from last row and\naccept potential loss of data.\nWrite a new marker", shape=rectangle]
 *          node_initm1 [label="All data is retained.\nWrite a new hint", shape=rectangle]
 *          node_initm2 [label="All data is retained", shape=rectangle]
 *        }
 *
 *        end [label="", shape=circle, style=filled, color=black];
 *
 *        start -> node_rv
 *        node_rv -> node_rg [label="yes"]
 *        node_rv -> node_hv [label=" no"]
 *        node_rg -> node_rmv
 *        node_rmv -> node_c2 [label=" yes"]
 *        node_rmv -> node_hv [label="no"]
 *        node_hv -> node_hg [label="yes"]
 *        node_hv -> node_f [label="no"]
 *        node_hg -> node_hmv
 *        node_hmv -> node_c2 [label="yes    "]
 *        node_hmv -> node_f [label="no"]
 *        node_f -> node_fmv
 *        node_fmv -> node_c2 [label="yes"]
 *        node_fmv -> node_c1 [label=" no"]
 *        node_c1 -> node_inith [label=" yes"]
 *        node_c1 -> node_inito [label="no"]
 *        node_c2 -> node_initm1 [label=" no"]
 *        node_c2 -> node_initm2 [label=" yes"]
 *        node_inito -> end
 *        node_inith -> end
 *        node_initm1 -> end
 *        edge [weight=2]
 *        node_initm2 -> end
 *      }
 *  @enddot
 */

/**
 * The absolute offset to the very first byte of the assigned EEPROM region.
 * @note Unless explicitly specified, all bit and byte offsets referring to the EEPROM region in the code are relative
 *  to this value.
 */
#define EEPROM_ABSOLUTE_FIRST_BYTE_OFFSET (STORAGE_EEPROM_FIRST_ROW * EEPROM_ROW_SIZE)

/**
 * The absolute offset to the very last byte of the assigned EEPROM region. */
#define EEPROM_ABSOLUTE_LAST_BYTE_OFFSET (((STORAGE_EEPROM_LAST_ROW + 1) * EEPROM_ROW_SIZE) - 1)

/** Translates a flash byte cursor relative to assigned FLASH region to a byte address. */
#define FLASH_CURSOR_TO_BYTE_ADDRESS(flashByteCursor) \
    ((uint8_t *)(FLASH_START + (STORAGE_FLASH_FIRST_PAGE * FLASH_PAGE_SIZE) + (flashByteCursor)))

/** Translates a FLASH byte cursor relative to assigned FLASH region to the number of the page the cursor refers to. */
#define FLASH_CURSOR_TO_PAGE(flashByteCursor) (STORAGE_FLASH_FIRST_PAGE + ((flashByteCursor) / FLASH_PAGE_SIZE))

/** Translates a FLASH page number to the start address of that page. */
#define FLASH_PAGE_TO_ADDRESS(type, flashPage) ((type)(FLASH_START + ((flashPage) * FLASH_PAGE_SIZE)))

/** The very first byte address of the assigned FLASH region. */
#define FLASH_FIRST_BYTE_ADDRESS FLASH_PAGE_TO_ADDRESS(uint8_t *, STORAGE_FLASH_FIRST_PAGE)

/** The very last byte address of the assigned FLASH region. */
#define FLASH_LAST_BYTE_ADDRESS (FLASH_PAGE_TO_ADDRESS(uint8_t *, STORAGE_FLASH_LAST_PAGE + 1) - 1)

/** The very first 32-bit word address of the assigned FLASH region. */
#define FLASH_FIRST_WORD_ADDRESS FLASH_PAGE_TO_ADDRESS(uint32_t *, STORAGE_FLASH_FIRST_PAGE)

/** The very last WORD address of the assigned FLASH region. */
#define FLASH_LAST_WORD_ADDRESS (FLASH_PAGE_TO_ADDRESS(uint32_t *, STORAGE_FLASH_LAST_PAGE + 1) - 1)

/**
 * The size in bytes of the meta data stored just in front of the (compressed) data block in FLASH. The LSBit of the
 * first byte after these header size contains the start of the (compressed) data block.
 * This header is used to give the decompress callback the correct arguments, and to deduce where to find the next
 * block.
 * @note Either this size is a value greater than 0 but less than #STORAGE_UNCOMPRESSED_BLOCK_SIZE_IN_BITS, which
 *  indicates the contents have been compressed; either this size is equal to #STORAGE_UNCOMPRESSED_BLOCK_SIZE_IN_BITS
 *  which indicates no compression/decompression algorithm was set or compression yielded bad results on this block and
 *  therefore the block was stored uncompressed.
 * @see FLASH_BLOCK_SIZE
 */
#define FLASH_DATA_HEADER_SIZE STORAGE_BLOCK_HEADER_SIZE

/**
 *  Given the header value of a (compressed) data block in FLASH, calculates the size in bytes of the complete block,
 *  @b including #FLASH_DATA_HEADER_SIZE.
 *  The start of a new block @b must @c always be stored on a 32-bit word boundary: this allows to continue writing the
 *  same page without erasing. As a consequence, up to 31 padding bits after the end of previous (compressed) data
 *  block are lost.
 *  @param bitCount The size in bits of the (compressed) data block, @b excluding #FLASH_DATA_HEADER_SIZE.
 *  @note
 *  - From the FLASH specification:
 *      <em>It is possible to write only a sub-set of the page (Y words), but the maximum number of
 *      times that a page can be written to, before an erase must be performed, is 16.
 *      WARNING: Writing a page more than 16 times without erasing may result in corrupting
 *      the page’s contents, due to write disturb (an intrinsic mechanism to the programming
 *      mechanism of the C14EFLASH)</em>
 *  - From the NVMC datasheet:
 *      <em>Due to the presence of ECC, it is not allowed to modify additional bits inside a memory
 *      word where some bits have already been programmed: the resulting ECC code in
 *      memory would then be the AND of the codes for the previous and new value written, and
 *      will most likely be inconsistent with the resulting data, potentially resulting in unwanted or
 *      missing bit corrections, or spurious error conditions. [C-NODPG]</em>
 */
#define FLASH_BLOCK_SIZE(bitCount) (4 * STORAGE_IDIVUP((bitCount) + FLASH_DATA_HEADER_SIZE * 8, 32))

/**
 * The first of two special values that are used in #Marker_t to be able to reconstruct the EEPROM and FLASH bit cursor
 * in case the battery has died - and thus the register value has been reset to zero.
 * @note These values are not chosen randomly: the code using them - #FindMarker - is depending on their specific bit
 * values.
 */
#define MARKER_HEADER ((int)0x0000FFFF)
#define MARKER_FOOTER ((int)0x7FFFFFFF) /**< The second special value. @see MARKER_HEADER */

/** The mask to use to zero out all possible 1 bits in #Marker_t.flashByteCursor */
#define MARKER_CURSOR_ZERO_MASK 0xFFFF8003

/* ------------------------------------------------------------------------- */

#if !STORAGE_FLASH_FIRST_PAGE
extern const int _etext; /**< Generated by the linker, used to calculate #STORAGE_FLASH_FIRST_PAGE */
extern const int _data; /**< Generated by the linker, used to calculate #STORAGE_FLASH_FIRST_PAGE */
extern const int _edata; /**< Generated by the linker, used to calculate #STORAGE_FLASH_FIRST_PAGE */
__attribute__ ((section(".noinit")))
static int sStorageFlashFirstPage; /* Initialized in #Storage_Init, RO afterwards. */
    #undef STORAGE_FLASH_FIRST_PAGE
    #define STORAGE_FLASH_FIRST_PAGE sStorageFlashFirstPage
#endif

/* ------------------------------------------------------------------------- */

/** @see Storage_Instance_t.readLocation */
typedef enum LOCATION {
    LOCATION_UNKNOWN, /**< A successful call to #Storage_Seek is required. */
#if STORAGE_SAMPLE_ALON_CACHE_COUNT > 0
    LOCATION_CACHE, /**< Memory in the assigned general purpose registers is targeted. */
#endif
    LOCATION_EEPROM, /**< Memory in the assigned EEPROM region is targeted. */
    LOCATION_FLASH /**< Memory in the assigned FLASH region is targeted. */
} LOCATION_T;

/** The size of the first bits of the cache, in the same general purpose register where #RecoverInfo_t is stored. */
#define FIRST_BITS_OF_CACHE_SIZE 13

/**
 * An instance of this structure is stored in the designated register of the ALON domain, and points to where to find
 * #Marker_t. This structure is stored in ALON and allows for the fastest initialization.
 * If all these values are 0, either no logging is ongoing, or the battery has gone empty. To discover which case
 * holds true, first #Hint_t is checked; if that fails too, the EEPROM is checked backwards for the location of a stored
 * instance of type #Marker_t (which is time consuming). Once found, the correct values for this structure can be
 * reconstructed.
 */
typedef struct RecoverInfo_s {
    unsigned int eepromBitCursor : 15; /**< @see Storage_Instance_t.eepromBitCursor */

    /**
     * The number of samples that are cached in the general purpose registers.
     * These are stored starting from @c firstBitsOfCache up to the last general purpose register.
     * A value of 0 indicates no samples are cached.
     */
    unsigned int sampleCacheCount : 4;

    /** A filler. It allows to perform a sanity check at compile time on the size of RecoverInfo_s. */
    unsigned int firstBitsOfCache : FIRST_BITS_OF_CACHE_SIZE;
} RecoverInfo_t;

/** Byte size of #Hint_t. Checked at compile time using #checkSizeOfHint. */
#define SIZE_OF_HINT 4

/** The absolute offset to a copy of the hint structure, to validate its contents. */
#define INVERSE_HINT_ABSOLUTE_BYTE_OFFSET ((EEPROM_ABSOLUTE_LAST_BYTE_OFFSET + 1 - SIZE_OF_HINT))

/** The absolute offset to #Hint_t at the end of the assigned EEPROM region. */
#define HINT_ABSOLUTE_BYTE_OFFSET (INVERSE_HINT_ABSOLUTE_BYTE_OFFSET - SIZE_OF_HINT)

    /** Byte size of the duplicate information. */
#if STORAGE_REDUCE_RECOVERY_WRITES
    #define SIZE_OF_DUPLICATE_DATA (EEPROM_ROW_SIZE - (2 * SIZE_OF_HINT))
    #if SIZE_OF_DUPLICATE_DATA != 56
        #error Unexpected value for SIZE_OF_DUPLICATE_DATA. Check the comments for magic values 56 and 8 that are related to this.
    #endif
#else
    #define SIZE_OF_DUPLICATE_DATA 64
#endif

    /** The absolute offset to the recovery information at the end of the assigned EEPROM region. */
#if STORAGE_REDUCE_RECOVERY_WRITES
    #define DUPLICATE_DATA_ABSOLUTE_BYTE_OFFSET (EEPROM_ABSOLUTE_LAST_BYTE_OFFSET + 1 - EEPROM_ROW_SIZE)
#else
    #define DUPLICATE_DATA_ABSOLUTE_BYTE_OFFSET (EEPROM_ABSOLUTE_LAST_BYTE_OFFSET + 1 - (2 * EEPROM_ROW_SIZE))
#endif

/**
 * An instance of this structure is stored at the fixed location #HINT_ABSOLUTE_BYTE_OFFSET, and points to where to
 * find #Marker_t. If all the information contained is correct (which may not be the case) it still allows for a fast
 * initialization.
 * - @c eepromBitCursor @b may contain correct information. If not, the EEPROM is checked backwards for the location of
 *  a stored instance of type #Marker_t (which is time consuming) and this hint structure is adapted (so a next
 *  initialization under the same circumstances is done quickly).
 * - The Hint_t instance also contains a duplicate reference to flashByteCursor. This will be used when the marker is no
 *  longer available or is corrupt.
 */
typedef struct Hint_s {
    /** @see Storage_Instance_t.eepromBitCursor */
    uint16_t eepromBitCursor;

    /** @see Storage_Instance_t.flashByteCursor */
    uint16_t flashByteCursor;
} Hint_t;

/**
 * An instance of this structure is stored in EEPROM, right after the end of the last sample in EEPROM.
 * It is used to re-create #sInstance upon initialization.
 * Since a number of false candidates may be proposed, it contains a @c header and a @c footer to have a near 100%
 * chance of detecting erroneous information.
 */
typedef struct Marker_s {
    const int header; /**< Must equal #MARKER_HEADER, or @c flashByteCursor is not valid. */

    /**
     * @see Storage_Instance_t.flashByteCursor
     * @note Usage
     *  - RO in #Storage_Init,
     *  - WO to set the value equal to #Storage_Instance_t.flashByteCursor in #Storage_DeInit, or to clear the
     *      marker from EEPROM in #Storage_Reset,
     *  - not to be read, not to be written to at all other times.
     */
    int flashByteCursor;

    const int footer; /**< Must equal #MARKER_FOOTER, or @c flashByteCursor is not valid. */
} Marker_t;

/**
 * The total number of bytes in EEPROM consumed by meta-data, to be able to keep track of what is stored in FLASH and
 * EEPROM, even after a power-off.
 * Apart from #Marker_t, there is an additional bit that marks whether the storage in FLASH and EEPROM is empty or at
 * least one sample is stored in EEPROM - @see #Hint_t
 */
#define SIZE_OF_MARKER 12

/**
 * Stores all meta data to perform the requested reads and writes in FLASH and EEPROM. Upon de-initialization, an
 * updated instance of #Marker_t is updated and stored in EEPROM, and an updated instance of #RecoverInfo_t is stored
 * in the ALON domain, using the information in this structure.
 */
typedef struct Storage_Instance_s {
    /**
     * The position of the first bit where new data can be stored in EEPROM, relative to #STORAGE_EEPROM_FIRST_ROW.
     * This doubles up as the start position of a stored instance of type #Marker_t in EEPROM.
     * There are at most @c EEPROM_NR_OF_RW_ROWS rows of EEPROM that can be used for storage, thus an EEPROM bit cursor
     * always has values less than 2**15: 15 bits are required to store this information.
     */
    int eepromBitCursor;

    /**
     * The position of the first byte where new data can be stored in FLASH, relative to #STORAGE_FLASH_FIRST_PAGE.
     * @pre Must be 32-bit word-aligned: multiple FLASH writes in the same page must occur at word boundaries. See
     *  #FLASH_BLOCK_SIZE
     * @note There are at most (FLASH_NR_OF_RW_SECTORS * FLASH_PAGES_PER_SECTOR - program size) pages of FLASH that
     *  can be used for storage, thus a FLASH byte cursor always has values less than 2**15; and due to the boundary
     *  requirement, the 2 LSBits must be 0 as well: 13 bits are thus required to store this information.
     *  Masking with #MARKER_CURSOR_ZERO_MASK must thus always yield a @c 0 value.
     */
    int flashByteCursor;

    /* ------------------------------------------------------------------------- */

    /**
     * Indicates the type of memory to @c readCursor targets. It also defines the type of @c readCursor:
     * a byte cursor or a bit cursor.
     */
    LOCATION_T readLocation;

    /**
     * - If @c readLocation equals #LOCATION_FLASH: the offset in bytes relative to
     *  #FLASH_FIRST_BYTE_ADDRESS where to read next in FLASH. This points to the two-byte header that precedes
     *  a (compressed) data block, stored in FLASH.
     * - If @c readLocation equals #LOCATION_EEPROM: the offset in bits relative to
     *  #EEPROM_ABSOLUTE_FIRST_BYTE_OFFSET where to read next in EEPROM. This points to the position of the LSBit of
     *  the next sample to read.
     * - If @c readLocation equals #LOCATION_CACHE: a relative index. The index of the sample that is stored
     *  in the cache. @c 0 indicates the oldest sample still present in the cache.
     * @note A negative value indicates nothing can be read. A call to #Storage_Seek is then required.
     */
    int readCursor;

    /**
     * A sequence number to help identify where the next sample, which must be read out, is located.
     * - In case of reading from FLASH, where (compressed) blocks of samples are stored, this is the absolute sequence
     *  number of the first sample in that block after decompressing.
     * - In case of reading from EEPROM, this is the absolute sequence number of the sample @c readCursor points to.
     * - In case of reading from sCache, this is the absolute sequence number of the sample @c readCursor points to.
     * The very first sample written via #Storage_Write has sequence number 0.
     * @note A negative value indicates nothing can be read. A call to #Storage_Seek is then required.
     */
    int readSequence;

    /**
     * The sequence number that was requested in the last call to #Storage_Seek.
     * - This value equals @c readSequence in case @c readLocation equals #LOCATION_EEPROM or #LOCATION_CACHE
     * - This value may be equal - but is unlikely to - in case @c readLocation equals #LOCATION_FLASH
     */
    int targetSequence;

    /* ------------------------------------------------------------------------- */

    /**
     * Determines what contents are available in #STORAGE_WORKAREA.
     * - The flash byte offset relative to #FLASH_FIRST_BYTE_ADDRESS where the header preceding the (compressed) data
     *  block can be found; these contents are decompressed and the samples, packed without padding bits, are available
     *  from the start of #STORAGE_WORKAREA.
     * - @c -1 if #STORAGE_WORKAREA does not contain samples after decompressing a data block stored in FLASH, or
     *  if #STORAGE_WORKAREA is used to compress data.
     */
    int cachedBlockOffset;
} Storage_Instance_t;

/**
 * The assigned EEPROM region cannot be used fully: there must be always room for #Marker_t and the last couple of
 * bytes are used to #Hint_t. To avoid corruption in both the marker and the hint, the hint is placed in a separate
 * row.
 * The total overhead is summed up here.
 */
#if STORAGE_REDUCE_RECOVERY_WRITES
    #define EEPROM_OVERHEAD_IN_BITS ((SIZE_OF_MARKER + EEPROM_ROW_SIZE) * 8)
#else
    #define EEPROM_OVERHEAD_IN_BITS ((SIZE_OF_MARKER + 2 * EEPROM_ROW_SIZE) * 8)
#endif

#if STORAGE_MAX_BLOCK_SIZE_IN_SAMPLES != (((STORAGE_EEPROM_ROW_COUNT * EEPROM_ROW_SIZE * 8) - EEPROM_OVERHEAD_IN_BITS) / STORAGE_BITSIZE)
    #error Internal memory storage model has changed - likely Marker_t or Hint_t - and no longer matches the define STORAGE_MAX_BLOCK_SIZE_IN_SAMPLES
#endif

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

/** If this construct doesn't compile, #STORAGE_TYPE can not hold #STORAGE_BITSIZE bits. */
int checkSizeOfSampleType[(sizeof(STORAGE_TYPE) * 8 < STORAGE_BITSIZE) ? -1 : 1]; /* Dummy variable since we can't use sizeof during precompilation. */

/** If this construct doesn't compile, the define #SIZE_OF_MARKER is no longer equal to @c sizeof(#Hint_t) */
int checkSizeOfHint[(SIZE_OF_HINT == sizeof(Hint_t)) ? 1 : -1]; /* Dummy variable since we can't use sizeof during precompilation. */

/** If this construct doesn't compile, the define #SIZE_OF_MARKER is no longer equal to @c sizeof(#Marker_t) */
int checkSizeOfMarker[(SIZE_OF_MARKER == sizeof(Marker_t)) ? 1 : -1]; /* Dummy variable since we can't use sizeof during precompilation. */

/** If this construct doesn't compile, adjust #FIRST_BITS_OF_CACHE_SIZE. RecoverInfo_t must be exactly 32 bits in size */
int checkSizeOfRecoverInfo[sizeof(RecoverInfo_t) == 4 ? 1 : -1]; /* Dummy variable since we can't use sizeof during precompilation. */

#if STORAGE_MAX_SAMPLE_ALON_CACHE_COUNT != (((5 - STORAGE_FIRST_ALON_REGISTER) * 32 - (32 - FIRST_BITS_OF_CACHE_SIZE)) / STORAGE_BITSIZE)
    /* 5: there are 5 general purpose registers */
    /* 32: one word == 32 bits */
    #error Internal memory storage model has changed - likely RecoverInfo_t - and no longer matches the define STORAGE_MAX_SAMPLE_ALON_CACHE_COUNT
#endif

#pragma GCC diagnostic pop

/* ------------------------------------------------------------------------- */

/**
 * The module's instance information, where recovered information is copied to, and where all updates are stored.
 * In #Storage_DeInit, this information is copied to more permanent storage.
 */
__attribute__ ((section(".noinit")))
static Storage_Instance_t sInstance;

/**
 * To reduce the call count to @c Chip_PMU_GetRetainedData and @c Chip_PMU_SetRetainedData, data is copied to SRAM in
 * #Storage_Init once, and copied back to GPREG in #Storage_DeInit once.
 */
__attribute__ ((section(".noinit"))) __attribute__((aligned (4)))
static uint8_t sCache[4 * (5 - STORAGE_FIRST_ALON_REGISTER)];

/**
 * Initialized in #Storage_Init to points to the recovery structure in #sCache.
 */
__attribute__ ((section(".noinit")))
static RecoverInfo_t * spRecoverInfo;

/* ------------------------------------------------------------------------- */

/**
 * Set to @c true after writing a new sample in EEPROM, used in #Storage_DeInit to know when to write the marker.
 * Not set when resetting the module; as there is no need for a marker in that case.
 */
static bool sEepromBitCursorChanged = false;

#if STORAGE_WORKAREA_SELF_DEFINED == 1
__attribute__ ((section(".noinit"))) __attribute__((aligned (4)))
uint8_t sStorage_Workarea[STORAGE_WORKAREA_SIZE];
#endif
extern uint8_t STORAGE_WORKAREA[STORAGE_WORKAREA_SIZE];

extern int STORAGE_COMPRESS_CB(int eepromByteOffset, int bitCount, void * pOut);
extern int STORAGE_DECOMPRESS_CB(const uint8_t * pData, int bitCount, void * pOut);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
/**
 * This is a dummy implementation to provoke fallback behavior: a plain copy from EEPROM to FLASH.
 * It is only used when #STORAGE_COMPRESS_CB is not overridden in an application specific @c app_sel.h header file
 * @see pStorage_CompressCb_t
 * @see STORAGE_COMPRESS_CB
 * @param eepromByteOffset unused
 * @param bitCount unused
 * @param pOut unused
 * @return @c 0
 */
int Storage_DummyCompressCb(int eepromByteOffset, int bitCount, void * pOut)
{
    return 0;
}
/**
 * This is a dummy implementation to provoke a failure; it should normally never be called, since the dummy compression
 * implementation will provoke the fallback behavior: a plain copy from EEPROM to FLASH, and vice-versa while reading.
 * It is only used when #STORAGE_COMPRESS_CB is not overridden in an application specific @c app_sel.h header file
 * @see pStorage_DecompressCb_t
 * @see STORAGE_DECOMPRESS_CB
 * @param pData unused
 * @param bitCount unused
 * @param pOut unused
 * @return @c 0
 */
int Storage_DummyDecompressCb(const uint8_t * pData, int bitCount, void * pOut)
{
    return 0;
}
#pragma GCC diagnostic pop

/* ------------------------------------------------------------------------- */

static void ResetInstance(void);
static void ShiftAlignedData(uint8_t * pTo, const uint8_t * pFrom, const int bitAlignment, const int bitCount);
static void ShiftUnalignedData(uint8_t * pTo, const uint8_t * pFrom, const int bitAlignment, const int bitCount);
#if STORAGE_SAMPLE_ALON_CACHE_COUNT > 0
static bool CacheSample(const STORAGE_TYPE * pSample);
static bool GetCachedSample(const int n, void * pData);
#endif
static void WriteToEeprom(const int bitCursor, const void * pData, const int bitCount);
static void ReadFromEeprom(const unsigned int bitCursor, void * pData, const int bitCount);
static unsigned int FindMarker(Marker_t * pMarker);
static int GetEepromCount(void);
static int GetFlashCount(void);
#if STORAGE_FLASH_FIRST_PAGE <= STORAGE_FLASH_LAST_PAGE
static bool WriteToFlash(const int pageCursor, const uint8_t * pData, const int pageCount);
#endif
static int StoreSamplesInEeprom(const STORAGE_TYPE * pSamples, int n);
static bool MoveSamplesFromEepromToFlash(void);
static int ReadAndCacheSamplesFromFlash(int readCursor);
static bool ValidateRecoverInfo(void);
static bool ValidateMarker(const Marker_t * pMarker, int expectedFlashByteCursor);
static bool ValidateHint(const Hint_t * pHint);
static void WriteHint(void);
static void WriteMarker(void);

/* ------------------------------------------------------------------------- */

/**
 * Re-initializes #sInstance with default values.
 * @note No EEPROM reads or writes, no FLASH reads or writes are done.
 * @post sBitCursorChanged is set when samples have become inaccessible.
 */
static void ResetInstance(void)
{
    sInstance.eepromBitCursor = 0;
    sInstance.flashByteCursor = 0;
    sInstance.readLocation = LOCATION_UNKNOWN;
    sInstance.readSequence = -1;
    sInstance.readCursor = -1;
    sInstance.targetSequence = -1;
    sInstance.cachedBlockOffset = -1;
}

/**
 * Copies a number of bits of byte aligned data to a buffer, non-byte aligned.
 * @param pTo The location to copy to. A number of LSBits of the first byte are not touched.
 * @param pFrom The location to copy from. The first bit to copy will become the LSBit of the first byte in this buffer.
 * @param bitAlignment The number of bits to disregard in @c to. Must be less than @c 8.
 * @param bitCount The number of bits to copy.
 * @pre @code STORAGE_IDIVUP(bitCount, 8) @endcode bytes must be available in @c from.
 * @post @code STORAGE_IDIVUP(bitCount + bitAlignment, 8) @endcode bytes will be written to @c to.
 * @note The first @c bitAlignment bits of the first byte written to are not touched.
 * @note For example, use this function to move bits in this way:
 *  - With @c - indicating not touched or don't care,
 *  - @c bitAlignment equal to @c 5, and
 *  - @c bitCount equal to @c 14:
 *  @code 8765 4321   --fe dcba is copied as 321- ----   cba8 7654   0000 0fed @endcode
 */
static void ShiftAlignedData(uint8_t * pTo, const uint8_t * pFrom, const int bitAlignment, const int bitCount)
{
    const uint8_t mask = (uint8_t)(0xFF & ((1 << bitAlignment) - 1)); /* Covering the existing bits in the last byte. */

    ASSERT((bitAlignment >= 0) && (bitAlignment < 8));
    ASSERT(bitCount > 0);

    /* 8 bits at a time, data is moved from @c from to @c to.
     *
     * 8 bits from @c from are copied to two consecutive bytes in @c to: 321- ----   ---8 7654 (-: don't care)
     * - Step 1: write the lsbits of 87654321 such that the ongoing byte is filled
     *   ... xxxxxxxx 321xxxxx -------- ...
     * - Step 2: write the msbits of 87654321 in the lsbit positions of the next byte
     *   ... xxxxxxxx 321xxxxx ---87654 ...
     * (in the above example, @c bitAlignment equals 5)
     */
    int n = 0;
    do {
        /* Step 1: copy x lsbits of the current byte, with x = 8-bitAlignment so left shift bitAlignment places */
        *pTo = (uint8_t)((pFrom[n] << bitAlignment) | ((*pTo) & mask));

        if ((n * 8) + 8 - bitAlignment < bitCount) {
            /* Advance to the next location to copy bits to. */
            pTo++;

            /* Step 2: copy x msbits of the new byte, with x = bitAlignment so right shift 8-bitAlignment places */
            *pTo = (uint8_t)(pFrom[n] >> (8 - bitAlignment));
        }

        n++;
    } while (n * 8 < bitCount);

    /* Clear the bits that were copied in excess. */
    uint8_t finalMask = (uint8_t)~(0xFF << ((bitCount + bitAlignment) % 8));
    if (finalMask != 0) { /* finalMask equals zero when the last bit to retain is the MSBit of the last byte written. */
        *pTo = *pTo & finalMask;
    }
}

/**
 * Copies a number of bits of non-byte aligned data to a buffer, byte aligned.
 * @param pTo The location to copy to. The first bit to copy will become the LSBit of the first byte in this buffer.
 * @param pFrom The location to copy from. A number of LSBits are disregarded.
 * @param bitAlignment The number of bits to disregard in @c from. Must be less than @c 8.
 * @param bitCount The number of bits to copy.
 * @pre @code STORAGE_IDIVUP(bitCount + bitAlignment, 8) @endcode bytes must be available in @c from.
 * @post @code STORAGE_IDIVUP(bitCount, 8) @endcode bytes will be written to @c to.
 * @note The remainder bits in @c to after @c bitCount bits is set to @c 0.
 * @note For example, use this function to move bits in this way:
 *  - With @c - indicating not touched or don't care,
 *  - @c bitAlignment equal to @c 5, and
 *  - @c bitCount equal to @c 14:
 *  @code 321- ----   cba8 7654   ---- -fed is copied as 8765 4321   00fe dcba @endcode
 */
static void ShiftUnalignedData(uint8_t * pTo, const uint8_t * pFrom, const int bitAlignment, const int bitCount)
{
    const uint8_t mask = 0xFF & (uint8_t)((1 << (8 - bitAlignment)) - 1); /* Covering the lsbits to retain in the copied byte. */

    ASSERT((bitAlignment >= 0) && (bitAlignment < 8));
    ASSERT(bitCount > 0);

    /* 8 bits at a time, data is moved from @c from to @c to.
     *
     * 8 bits from two consecutive bytes in @c from: 321- ----   ---8 7654 (-: don't care) are copied to one byte in
     * @c to:
     * - Step 1: read the msbits of 321- ----
     *   byte = 0b00000321
     * - Step 2: read the lsbits of ---8 7654
     *   byte = 0b87654321
     * (in the above example, @c bitAlignment equals 5)
     */
    int n = 0;
    do {
        /* Step 1: copy x msbits of the current byte, with x = 8-bitAlignment so right shift bitAlignment places. */
        *pTo = (uint8_t)((pFrom[n]) >> bitAlignment);

        /* Advance to the next location to copy bits from. */
        n++;

        if (((n - 1) * 8) + 8 - bitAlignment < bitCount) {

            /* Step 2: copy x lsbits of the next byte, with x = bitAlignment so left shift 8-bitAlignment places. */
            *pTo = (uint8_t)((pFrom[n] << (8 - bitAlignment)) | ((*pTo) & mask));

            if (n * 8 < bitCount) {
                pTo++; /* Do not increment @c to unconditionally: @c finalmask still may have to be applied. */
            }
        }
    } while (n * 8 < bitCount);

    /* Clear the bits that were copied in excess. */
    uint8_t finalMask = (uint8_t)~(0xFF << (bitCount % 8));
    if (finalMask != 0) { /* finalMask equals zero when the last bit to retain is the MSBit of the last byte written. */
        *pTo = *pTo & finalMask;
    }
}

/* ------------------------------------------------------------------------- */

#if STORAGE_SAMPLE_ALON_CACHE_COUNT > 0
/**
 * Stores data in #sCache.
 * @param pSample May not be @c NULL. The data to cache.
 * @return @c true when the sample was copied; @c false when it did not fit and no changes were made.
 */
static bool CacheSample(const STORAGE_TYPE * pSample)
{
    bool success = false;

    if (spRecoverInfo->sampleCacheCount < STORAGE_SAMPLE_ALON_CACHE_COUNT) {
        int bitCursor = (32 - FIRST_BITS_OF_CACHE_SIZE) + (spRecoverInfo->sampleCacheCount * STORAGE_BITSIZE);
        int byteOffset = bitCursor / 8;
        int bitAlignment = bitCursor % 8;
        ShiftAlignedData(&sCache[byteOffset], (uint8_t *)pSample, bitAlignment, STORAGE_BITSIZE);
        spRecoverInfo->sampleCacheCount++;
        success = true;
    }

    return success;
}

/**
 * Retrieves a sample cached in an earlier call to #CacheSample.
 * @param n A relative index. Determines the sample to copy. A value of 0 indicates the oldest sample that is present in #sCache.
 * @param pData May not be @c NULL. The sample is copied to the array pointed to.
 * @return @c true if a sample was copied; @c false if less than @c n samples are stored in #sCache
 */
static bool GetCachedSample(const int n, void * pData)
{
    if (n < spRecoverInfo->sampleCacheCount) {
        int bitCursor = (32 - FIRST_BITS_OF_CACHE_SIZE) + (n * STORAGE_BITSIZE);
        int byteOffset = bitCursor / 8;
        int bitAlignment = bitCursor % 8;
        ShiftUnalignedData((uint8_t *)pData, &sCache[byteOffset], bitAlignment, STORAGE_BITSIZE);
    }
    return n < spRecoverInfo->sampleCacheCount;
}
#endif

/* ------------------------------------------------------------------------- */

/**
 * @pre EEPROM is initialized
 * @pre Enough free space must be available in EEPROM starting from @c bitCursor
 * @param bitCursor Must be positive. The first bit where to start writing.
 * @param pData May not be @c NULL.
 * @param bitCount Must be strict positive.
 * @post #sInstance is not touched.
 */
static void WriteToEeprom(const int bitCursor, const void * pData, const int bitCount)
{
    const int bitAlignment = bitCursor % 8; /* The number of lsbits written in the last byte. */
    int byteOffset = EEPROM_ABSOLUTE_FIRST_BYTE_OFFSET + bitCursor / 8; /* The index to the last byte written. */

    ASSERT(bitCursor >= 0);
    ASSERT(pData != NULL);
    ASSERT(bitCount > 0);

    int byteCount = STORAGE_IDIVUP(bitCount + bitAlignment, 8);
    uint8_t bytes[byteCount];

    /* Read the first byte we will write to, to preserve a number of LSBits in that byte which were written
     * previously.
     */
    Chip_EEPROM_Read(NSS_EEPROM, byteOffset, &bytes, 1);
    ShiftAlignedData(bytes, (uint8_t *)pData, bitAlignment, bitCount);
    Chip_EEPROM_Write(NSS_EEPROM, byteOffset, bytes, byteCount);
}

/**
 * @pre EEPROM is initialized
 * @pre Enough bits to read are available in EEPROM starting from @c bitCursor
 * @param bitCursor Must be strict positive. The bit position in EEPROM to start reading: @c 0 means to start reading
 *  from the very first EEPROM bit assigned for bit storage, and so on.
 * @param pData May not be @c NULL. All bits are copied in the array pointed to.
 * @param bitCount Must be strict positive. When not a multiple of 8, the remainder MSBits of the last byte are set to
 *  @c 0.
 * @post #sInstance is not touched.
 */
static void ReadFromEeprom(const unsigned int bitCursor, void * pData, const int bitCount)
{
    const int bitAlignment = bitCursor % 8; /* The number of lsbits written in the last byte. */
    unsigned int byteOffset = EEPROM_ABSOLUTE_FIRST_BYTE_OFFSET + bitCursor / 8; /* The index to the last byte written. */

    ASSERT(pData != NULL);
    ASSERT(bitCount > 0);

    int byteCount = STORAGE_IDIVUP(bitCount + bitAlignment, 8);
    uint8_t bytes[byteCount];

    Chip_EEPROM_Read(NSS_EEPROM, (int)byteOffset, bytes, byteCount);
    ShiftUnalignedData((uint8_t *)pData, bytes, bitAlignment, bitCount);
}

/* ------------------------------------------------------------------------- */

/**
 * Search backwards in the assigned EEPROM region, looking for a valid marker.
 * @param [out] pMarker : Where to copy the found marker data to. If @c 0 is returned, this will contain an invalid
 *  marker.
 * @return The position of the first bit of a valid stored instance of type #Marker_t in EEPROM, relative to
 *  #STORAGE_EEPROM_FIRST_ROW. @c 0 if the marker could not be found.
 */
static unsigned int FindMarker(Marker_t * pMarker)
{
    unsigned int eepromBitCursor;
    bool found = false;

    /* Start a slow search, checking the full EEPROM contents assigned to the storage module. */
    unsigned int byteOffset = EEPROM_ABSOLUTE_LAST_BYTE_OFFSET - (EEPROM_OVERHEAD_IN_BITS / 8); /* Search backwards. */
    do {
        /* The marker consists of a header, a value, and a footer.
         * By reading 16-bit words one at a time (step a) from high offset to low, we must find a value equal to
         * @c 0xFFFF that is part of the FOOTER: see ---------------- below.
         * If found (step b), we can read the word value @c h with relative byte offset @c -8 (step c) that is part
         * of the HEADER: see ++++++++++++++++ below.
         * A validation check is to verify whether @c h+1 is a power of 2 (step d): if so, from the number of bits
         * set in @c h (step e) we know the extra bit offset to subtract to find the complete marker:
         * -7 for Example 1, -13 for Example 2, -1 for Example 3.
         * The full 96 bits can then be read out (step f) and validated (step g): #MARKER_HEADER and #MARKER_FOOTER
         * values must be found, and the size of the value must be acceptable.
         *
         * Header                           Value                            Footer
         * 0x00    0x00    0xFF    0xFF     0x00    0x00    0x??    0x??     0x7F    0xFF    0xFF    0xFF
         * 00000000000000001111111111111111 00000000000000000vvvvvvvvvvvvv00 01111111111111111111111111111111
         *          ++++++++++++++++                                                  ----------------        Example 1
         *    ++++++++++++++++                                                  ----------------              Example 2
         *                ++++++++++++++++                                                  ----------------  Example 3
         * 0         1         2         3          4         5         6          7         8         9
         * 01234567890123456789012345678901 23456789012345678901234567890123 45678901234567890123456789012345
         *
         * Magic values used:
         *  2: (step a) the portion in number of bytes of the FOOTER value to search for.
         *  0xFFFF: (step b) part of the FOOTER value.
         *  8, -8: (while, step c) The distance in bytes between the start of the footer and the start of the header.
         */
        uint16_t word;
        Chip_EEPROM_Read(NSS_EEPROM, (int)byteOffset, &word, 2); /* step a */
        if (word == 0xFFFF) { /* step b */
            Chip_EEPROM_Read(NSS_EEPROM, (int)byteOffset - 8, &word, 2); /* step c */
            if (((word + 1) & word) == 0) { /* step d */
                unsigned int bits;
                for (bits = 0; word; bits++) { /* step e */
                    word &= (uint16_t)(word - 1); /* Counting bits set, Brian Kernighan's way */
                }
                eepromBitCursor = (byteOffset - EEPROM_ABSOLUTE_FIRST_BYTE_OFFSET - 8) * 8 - (16 - bits);
                if ((eepromBitCursor % STORAGE_BITSIZE) == 0) {
                    ReadFromEeprom(eepromBitCursor, pMarker, sizeof(Marker_t) * 8); /* step f */
                    found = ValidateMarker(pMarker, -1);
                }
            }
        }
        byteOffset -= 2;

    } while ((!found) && (byteOffset >= EEPROM_ABSOLUTE_FIRST_BYTE_OFFSET + 8));

    if (!found) {
        eepromBitCursor = 0;
        memset(pMarker, 0, sizeof(Marker_t));
    }
    return eepromBitCursor;
}

/** @return The number of samples stored in EEPROM. */
static int GetEepromCount(void)
{
    ASSERT((sInstance.eepromBitCursor % STORAGE_BITSIZE) == 0); /* The integer division has no remainder. */
    return sInstance.eepromBitCursor / STORAGE_BITSIZE;
}

/** @return The number of samples stored in FLASH. */
static int GetFlashCount(void)
{
    int sequenceCount = 0;
    /* Loop over all the (compressed) data blocks in FLASH - each is storing the same amount of samples. */
    int readCursor = 0;
    while (readCursor < sInstance.flashByteCursor) {
        uint8_t * header = FLASH_CURSOR_TO_BYTE_ADDRESS(readCursor);
        int bitCount = (int)(header[0] | (header[1] << 8));
        sequenceCount += STORAGE_BLOCK_SIZE_IN_SAMPLES;
        readCursor += FLASH_BLOCK_SIZE(bitCount);
    }
    return sequenceCount;
}

/* ------------------------------------------------------------------------- */

#if STORAGE_FLASH_FIRST_PAGE <= STORAGE_FLASH_LAST_PAGE
/**
 * Writes a block of (compressed) samples to FLASH. After the operation, the complete FLASH contents are verified.
 * @param pageCursor Must be strict positive. The absolute page number: a value of @c 0 indicates the first page of
 *  sector 0. Defines the first byte of the page where to start writing.
 * @param pData May not be @c NULL. Must be word (32 bits) aligned.
 * @param pageCount The number of pages to write. @c data must provide this multiple of #FLASH_PAGE_SIZE bytes of data.
 * @warning All interrupts are disabled at the beginning of this function and restored before leaving this function.
 * @warning This function will take 100+ milliseconds to complete.
 * @pre FLASH pages must have been erased beforehand - this is not checked.
 * @post #sInstance is not touched.
 * @return the result of the FLASH program action: @c true for success.
 */
static bool WriteToFlash(const int pageCursor, const uint8_t * pData, const int pageCount)
{
    uint8_t * pDest;
    uint32_t size;
    IAP_STATUS_T status;
    uint32_t sectorStart = (uint32_t)(pageCursor / FLASH_PAGES_PER_SECTOR);
    uint32_t sectorEnd = (uint32_t)((pageCursor + pageCount - 1) / FLASH_PAGES_PER_SECTOR);

    ASSERT(pageCursor > 0);
    ASSERT(pData != NULL);
    ASSERT(pageCount > 0);
    ASSERT(sectorEnd < FLASH_NR_OF_RW_SECTORS);

    pDest = (uint8_t *)FLASH_START + (pageCursor * FLASH_PAGE_SIZE);
    size = (uint32_t)pageCount * FLASH_PAGE_SIZE;

    status = Chip_IAP_Flash_PrepareSector(sectorStart, sectorEnd);
    if (status == IAP_STATUS_CMD_SUCCESS) {
        __disable_irq();
        status = Chip_IAP_Flash_Program(pData, pDest, size, 0);
        __enable_irq();
    }
    if (status == IAP_STATUS_CMD_SUCCESS) {
        /* To compare, only compare the new data, i.e. skip the initial 0xFF values, as they overlap with the last
         * portion of the previously written data ("o" in MoveSamplesFromEepromToFlash).
         */
        uint32_t fillWord = *(uint32_t *)pData;
        while ((fillWord == 0xFFFFFFFF) && (size > 0)) {
            pData += 4;
            pDest += 4;
            size -= 4;
            fillWord = *(uint32_t *)pData;
        }
        __disable_irq();
        status = Chip_IAP_Compare(pData, pDest, size, NULL);
        __enable_irq();
    }
    return status == IAP_STATUS_CMD_SUCCESS;
}
#endif

/**
 * Stores @c n samples in EEPROM. If necessary, the oldest data is moved to FLASH.
 * @param pSamples May not be @c NULL. Pointer to the start of the array where to copy the samples from.
 * @param n The size of the array
 * @return The number of samples that were stored. A value less than @c n indicates insufficient storage capacity.
 */
static int StoreSamplesInEeprom(const STORAGE_TYPE * pSamples, int n)
{
    int count = 0;
    while (count < n) {
        if (GetEepromCount() == STORAGE_BLOCK_SIZE_IN_SAMPLES) {
            /* There are enough samples stored in EEPROM to warrant a compression and a move to FLASH.
             * Clear the EEPROM by moving them to FLASH. When that is done, the EEPROM is fully empty again.
             * We do this just before writing a new sample in EEPROM, to ensure at least one sample is present in
             * EEPROM whenever at least sample has been given to the storage module. There is no code relying on this:
             * it just feels more natural to only move when absolutely necessary.
             */
            (void)MoveSamplesFromEepromToFlash();
            /* Even if it fails, we can still continue and try writing in the rest of the EEPROM.
             * The if test above only tests for equality, so we will not needlessly try again on each added sample.
             */
        }
        if (GetEepromCount() < STORAGE_MAX_BLOCK_SIZE_IN_SAMPLES) {
            WriteToEeprom(sInstance.eepromBitCursor, pSamples + count, STORAGE_BITSIZE);
            sInstance.eepromBitCursor += STORAGE_BITSIZE;
            count++;
        }
        else {
            /* The EEPROM is fully filled with samples, and an earlier call to move data from EEPROM to FLASH failed
             * - if it succeeded, this branch would not be chosen.
             * Storage is full, both EEPROM and FLASH, and no data can be written any more.
             */
            break;
        }
    }

    sEepromBitCursorChanged |= (count > 0);
    return count;
}

/**
 * Clear the assigned EEPROM region, by moving all data to assigned FLASH region. Before moving the data, the
 * application is given the opportunity to compress the data. The (compressed) data block is then appended to the
 * existing data in FLASH.
 * Steps:
 * - Prepare #STORAGE_WORKAREA for FLASH write
 * - Copy data from EEPROM to #STORAGE_WORKAREA
 * - Call #STORAGE_COMPRESS_CB
 * - Flash
 * - Update pointers
 * @return @c true when the compression was successful and the data has been moved to FLASH, @c false when the
 *  compression callback function returned @c false or when the FLASH storage is full: nothing has been changed in
 *  FLASH or EEPROM in that case.
 * @post #sInstance is fully updated when this function returns.
 * @note Uses #STORAGE_WORKAREA. Not in use anymore when this function returns.
 */
static bool MoveSamplesFromEepromToFlash(void)
{
#if STORAGE_FLASH_FIRST_PAGE > STORAGE_FLASH_LAST_PAGE
    /* There is no flash assigned for storage. Cannot move to flash. */
    return false;
#else
    if (STORAGE_FLASH_FIRST_PAGE > STORAGE_FLASH_LAST_PAGE) {
        /* There is no flash available for storage. Cannot move to flash. */
        return false;
    }
    else {
        bool success;
        uint8_t * pOut = STORAGE_WORKAREA;

        sInstance.cachedBlockOffset = -1;

        /* Prepare a number of pages to FLASH:
         * Pages: |----------------|---...-------------|
         * Data:   oooooohhcccccccccccc...ccfffffffffff
         * with:
         * - o: the last portion of the previously written (compressed) data block.
         * - h: the two-byte header indicating the size in bits of the (compressed) data block that follows.
         * - c: the (compressed) data block to write.
         * - f: the yet-unused trailing bytes of the last page where the new (compressed) data block is written to.
         *  By adding 1-bits, we can later write without the need for a costly FLASH page erase cycle.
         */

        /* Determine the first page to flash in this call. It is likely that during the previous call to this
         * function the then-last page flashed was not completely filled; that last page in the previous call gets now
         * completely filled first and becomes the first page to flash in this call.
         */
        int firstFlashPage = FLASH_CURSOR_TO_PAGE(sInstance.flashByteCursor);
        int flashByteOffsetInPage = sInstance.flashByteCursor % FLASH_PAGE_SIZE;

        /* o: Ensure the part of the last page that was already written to in a previous move from EEPROM to FLASH
         * remains untouched.
         */
        memset(pOut, 0xFF, (size_t)flashByteOffsetInPage);
        pOut += flashByteOffsetInPage;

        /* c: The compress algorithm is to store the new (compressed) data block output just after the just copied data.
         * Skip the meta data header for now: that is filled in when the compression completed.
         */
        int bitCount = STORAGE_COMPRESS_CB(EEPROM_ABSOLUTE_FIRST_BYTE_OFFSET, STORAGE_UNCOMPRESSED_BLOCK_SIZE_IN_BITS,
                                           pOut + FLASH_DATA_HEADER_SIZE);
        if ((bitCount <= 0) || (bitCount >= STORAGE_UNCOMPRESSED_BLOCK_SIZE_IN_BITS)) {
            /* Compression failed, or resulted in a larger block size. In this case we still have a fallback:
             * copy the data from EEPROM to FLASH unaltered.
             */
            Chip_EEPROM_Read(NSS_EEPROM, EEPROM_ABSOLUTE_FIRST_BYTE_OFFSET, pOut + FLASH_DATA_HEADER_SIZE,
                             STORAGE_UNCOMPRESSED_BLOCK_SIZE_IN_BYTES);
            bitCount = STORAGE_UNCOMPRESSED_BLOCK_SIZE_IN_BITS;
        }
        int compressedDataSizeInBytes = STORAGE_IDIVUP(bitCount, 8);

        /* h: */
        pOut[0] = (uint8_t)(bitCount & 0xFF);
        pOut[1] = (uint8_t)((bitCount >> 8) & 0xFF);
        pOut += FLASH_DATA_HEADER_SIZE;

        /* c: */
        pOut += compressedDataSizeInBytes;

        /* f: */
        while (((int)(pOut - STORAGE_WORKAREA) % FLASH_PAGE_SIZE) != 0) {
            *pOut = 0xFF;
            pOut++;
        }

        int newFlashByteCursor = sInstance.flashByteCursor + FLASH_BLOCK_SIZE(bitCount);
        ASSERT((newFlashByteCursor & 0x3) == 0); /* Must be 32-bit word-aligned. */
        if (FLASH_CURSOR_TO_BYTE_ADDRESS(newFlashByteCursor) - 1 > FLASH_LAST_BYTE_ADDRESS) {
            /* There is not enough space left in the assigned FLASH region to store the (compressed) data block. */
            success = false;
        }
        else {
            /* Only now we can write the full oooooohhcccccccccccc...ccfffffffffff sequence to FLASH. */
            int pageCountToFlash = (pOut - STORAGE_WORKAREA + FLASH_PAGE_SIZE - 1) / FLASH_PAGE_SIZE;
            ASSERT(pageCountToFlash > 0); /* Must be at least 1. */
            success = WriteToFlash(firstFlashPage, STORAGE_WORKAREA, pageCountToFlash);
        }
        if (success) { /* All that is left now is to do some housekeeping: in EEPROM and in sInstance. */
            int oldEepromBitCursor = sInstance.eepromBitCursor; /* Used after updating the hint information. */

            sInstance.eepromBitCursor = 0;
            /* Update variables used when reading samples. */
            if (sInstance.readLocation == LOCATION_EEPROM) {
                if (sInstance.readCursor < STORAGE_UNCOMPRESSED_BLOCK_SIZE_IN_BITS) {
                    sInstance.readLocation = LOCATION_FLASH;
                    sInstance.readSequence = GetFlashCount() - STORAGE_BLOCK_SIZE_IN_SAMPLES;
                    sInstance.readCursor = sInstance.flashByteCursor;
                }
                else {
                    ASSERT(sInstance.readCursor == STORAGE_UNCOMPRESSED_BLOCK_SIZE_IN_BITS);
                    //sInstance.readLocation remains the same
                    sInstance.readCursor = 0;
                    //sInstance.readSequence remains the same
                }
                //sInstance.targetSequence remains the same
            }
            /* Only update flashByteCursor after updating readCursor & readSequence */
            sInstance.flashByteCursor = newFlashByteCursor;

            /* Now that everything has been copied from EEPROM to FLASH, the new marker is to be written at the
             * beginning of the assigned EEPROM space.
             * If that fails, we still want to have access to the just moved data.
             * Thus: update the hint before changing the marker.
             */
            WriteHint();

            /* Clear the marker in EEPROM - after a power-off we don't want to find this information any more.
             * sBitCursorChanged is set in Storage_Write() - which is the sole caller of this function - and the new
             * correct marker on the new correct location will be written in Storage_DeInit().
             */
            uint8_t zeroMarker[sizeof(Marker_t)] = {0};
            WriteToEeprom(oldEepromBitCursor, zeroMarker, sizeof(Marker_t) * 8);
        }

        return success;
    }
#endif
}

/**
 * Reads a block of compressed data containing #STORAGE_BLOCK_SIZE_IN_SAMPLES samples, decompresses the data
 * and stores the result in the workspace given by the application.
 * @param readCursor The offset in bytes relative to FLASH_FIRST_BYTE_ADDRESS to the header preceding the
 *  (compressed) data block that must be read.
 * @return
 *  - When the samples are available in #STORAGE_WORKAREA (either when the previous decompression was still valid and
 *      the decompression callback was not called; or when decompression was successful as indicated by the returnvalue
 *      of the called decompression callback): the size of the (compressed) data block including the header. This is
 *      equal to the number of bytes to advance the read cursor to the header of the next (compressed) data block.
 *  - @c 0 when the FLASH contents were invalid, or when the decompression callback function returned @c false:
 *      nothing has been changed in that case.
 * @post Only #Storage_Instance_t.cachedBlockOffset is fully updated when this function returns.
 * @note Uses #STORAGE_WORKAREA. Only the first #STORAGE_UNCOMPRESSED_BLOCK_SIZE_IN_BYTES bytes are in use
 *  when this function returns.
 */
static int ReadAndCacheSamplesFromFlash(int readCursor)
{
    uint8_t * pHeader = FLASH_CURSOR_TO_BYTE_ADDRESS(readCursor);
    int bitCount = (int)(pHeader[0] | (pHeader[1] << 8));
    int blockSize;
    /* if header[0:1] == 0xFFFF, the flash was emptied.
     * This indicates a discrepancy between the instance information and the FLASH contents.
     * This may happen during development when re-flashing with the same image and erasing the non-used pages.
     */
    if (bitCount == 0x0000FFFF) {
        blockSize = 0;
    }
    else if (sInstance.cachedBlockOffset == readCursor) {
        /* The output from the previous call to STORAGE_DECOMPRESS_CB is still valid. */
        blockSize = FLASH_BLOCK_SIZE(bitCount);
    }
    else if (bitCount == STORAGE_UNCOMPRESSED_BLOCK_SIZE_IN_BITS) {
        memcpy(STORAGE_WORKAREA, pHeader + FLASH_DATA_HEADER_SIZE, (size_t)STORAGE_UNCOMPRESSED_BLOCK_SIZE_IN_BYTES);
        blockSize = FLASH_BLOCK_SIZE(STORAGE_UNCOMPRESSED_BLOCK_SIZE_IN_BITS);
    } else {
        int decompressedBitCount = STORAGE_DECOMPRESS_CB(pHeader + FLASH_DATA_HEADER_SIZE, bitCount, STORAGE_WORKAREA);
        if (decompressedBitCount == STORAGE_UNCOMPRESSED_BLOCK_SIZE_IN_BITS) {
            sInstance.cachedBlockOffset = readCursor;
            blockSize = FLASH_BLOCK_SIZE(bitCount);
        }
        else {
            blockSize = 0;
        }
    }
    return blockSize;
}

/**
 * Checks whether #spRecoverInfo contains possible correct information.
 * @return @c true when the information contained in the structure #spRecoverInfo points to can be used.
 */
static bool ValidateRecoverInfo(void)
{
    return ((spRecoverInfo->eepromBitCursor > 0) || (spRecoverInfo->sampleCacheCount > 0)) /* Situation after Storage_Reset or power-off: 0 */
            && (spRecoverInfo->sampleCacheCount <= STORAGE_SAMPLE_ALON_CACHE_COUNT) /* Validity check */
            && ((spRecoverInfo->eepromBitCursor % STORAGE_BITSIZE) == 0) /* Validity check */
            && (spRecoverInfo->eepromBitCursor <= STORAGE_MAX_UNCOMPRESSED_BLOCK_SIZE_IN_BITS); /* Validity check */
}

/**
 * Checks whether the given structure contains possible correct information.
 * @param pMarker : May not be NULL. Points to the structure to check.
 * @param expectedFlashByteCursor : Ignored when negative. When zero or positive, provides an additional constraint on
 *  the marker structure.
 * @return @c true when the information contained in the structure can be used.
 */
static bool ValidateMarker(const Marker_t * pMarker, int expectedFlashByteCursor)
{
    bool ok = (pMarker->header == MARKER_HEADER)
            && (pMarker->footer == MARKER_FOOTER)
            && (((unsigned int)pMarker->flashByteCursor & MARKER_CURSOR_ZERO_MASK) == 0)
            && ((pMarker->flashByteCursor == 0) /* if no flash memory is available, FLASH_CURSOR_TO_BYTE_ADDRESS gives 0x7800 */
                || (FLASH_CURSOR_TO_BYTE_ADDRESS(pMarker->flashByteCursor) <= FLASH_LAST_BYTE_ADDRESS));
    if (ok && (expectedFlashByteCursor >= 0)) {
        ok = pMarker->flashByteCursor == expectedFlashByteCursor;
    }
    return ok;
}

/**
 * Checks whether the given structure contains possible correct information.
 * @param pHint : May not be NULL. Points to the structure to check.
 * @return @c true when the information contained in the structure can be used.
 */
static bool ValidateHint(const Hint_t * pHint)
{
    bool ok = ((pHint->eepromBitCursor % STORAGE_BITSIZE) == 0) /* Validity check */
                && (pHint->eepromBitCursor <= STORAGE_MAX_UNCOMPRESSED_BLOCK_SIZE_IN_BITS); /* Validity check */
    if (ok) {
        Hint_t inverseHint;
        Chip_EEPROM_Read(NSS_EEPROM, INVERSE_HINT_ABSOLUTE_BYTE_OFFSET, &inverseHint, sizeof(Hint_t));
        inverseHint.eepromBitCursor = (uint16_t)~inverseHint.eepromBitCursor;
        inverseHint.flashByteCursor = (uint16_t)~inverseHint.flashByteCursor;
        /* Normal situation: */
        ok = (inverseHint.eepromBitCursor == pHint->eepromBitCursor)
                && (inverseHint.flashByteCursor == pHint->flashByteCursor);
        if (!ok) {
            /* Initial situation: no data has been written, all EEPROM is in blank state */
            uint32_t data;
            Chip_EEPROM_Read(NSS_EEPROM, EEPROM_ABSOLUTE_FIRST_BYTE_OFFSET, &data, sizeof(data));
            ok = (data == 0)
                    && (inverseHint.eepromBitCursor == 0xFFFF) /* The inverse from the memory content was taken above. */
                    && (inverseHint.flashByteCursor == 0xFFFF) /* The inverse from the memory content was taken above. */
                    && (pHint->eepromBitCursor == 0)
                    && (pHint->flashByteCursor == 0);
        }
    }
    /* if !ok -> data corruption */
    return ok;
}

/** Uses the information of sInstance to create a #Hint_t structure, and writes it to EEPROM. */
static void WriteHint(void)
{
    Hint_t hint = {.eepromBitCursor = (uint16_t)sInstance.eepromBitCursor,
                   .flashByteCursor = (uint16_t)sInstance.flashByteCursor};
    Chip_EEPROM_Write(NSS_EEPROM, HINT_ABSOLUTE_BYTE_OFFSET, &hint, sizeof(Hint_t));

    Hint_t inverseHint = {.eepromBitCursor = (uint16_t)~hint.eepromBitCursor,
                          .flashByteCursor = (uint16_t)~hint.flashByteCursor};
    Chip_EEPROM_Write(NSS_EEPROM, INVERSE_HINT_ABSOLUTE_BYTE_OFFSET, &inverseHint, sizeof(Hint_t));

    /* Store extra recovery information together with the hint by copying as many of the last bytes of the page where
     * the marker starts to the duplicate data area.
     */
    int byteCursor = ((sInstance.eepromBitCursor / 8) / EEPROM_ROW_SIZE) * EEPROM_ROW_SIZE;
    uint8_t duplicate[SIZE_OF_DUPLICATE_DATA];
    Chip_EEPROM_Read(NSS_EEPROM, EEPROM_ABSOLUTE_FIRST_BYTE_OFFSET + byteCursor, duplicate, SIZE_OF_DUPLICATE_DATA);
    Chip_EEPROM_Write(NSS_EEPROM, DUPLICATE_DATA_ABSOLUTE_BYTE_OFFSET, duplicate, SIZE_OF_DUPLICATE_DATA);
}

/** Uses the information of sInstance to create a #Marker_t structure, and writes it to EEPROM. */
static void WriteMarker(void)
{
    Marker_t marker = {.header = MARKER_HEADER, .flashByteCursor = sInstance.flashByteCursor, .footer = MARKER_FOOTER};
    WriteToEeprom(sInstance.eepromBitCursor, &marker, sizeof(marker) * 8);
}

/* ------------------------------------------------------------------------- */

void Storage_Init(void)
{
#if !STORAGE_FLASH_FIRST_PAGE
    sStorageFlashFirstPage = ((int)&_etext + (int)&_edata - (int)&_data + FLASH_PAGE_SIZE - 1) / FLASH_PAGE_SIZE;
#endif
    ResetInstance();

    /** Be sure to check out the @ref storage_initializing_par "initialization flowchart" */

    Chip_PMU_GetRetainedData((uint32_t *)sCache, STORAGE_FIRST_ALON_REGISTER, 5 - STORAGE_FIRST_ALON_REGISTER);
    spRecoverInfo = (RecoverInfo_t *)sCache;
    bool recoverInfoIsValid = ValidateRecoverInfo();

    Hint_t hint;
    Chip_EEPROM_Read(NSS_EEPROM, HINT_ABSOLUTE_BYTE_OFFSET, &hint, sizeof(Hint_t));
    bool hintIsValid = ValidateHint(&hint);

    Marker_t marker;
    bool markerIsValid = false;

    if (recoverInfoIsValid) {
        ReadFromEeprom(spRecoverInfo->eepromBitCursor, &marker, sizeof(marker) * 8);
        markerIsValid = ValidateMarker(&marker, hintIsValid ? hint.flashByteCursor : -1);
    }

    if (!markerIsValid) {
        if (hintIsValid) {
            spRecoverInfo->eepromBitCursor = (unsigned int)hint.eepromBitCursor & 0x7FFF;
            ReadFromEeprom(spRecoverInfo->eepromBitCursor, &marker, sizeof(marker) * 8);
            markerIsValid = ValidateMarker(&marker, hint.flashByteCursor);
        }
    }

    if (!markerIsValid) {
        spRecoverInfo->eepromBitCursor = (unsigned int)FindMarker(&marker) & 0x7FFF;
        //markerIsValid = ValidateMarker(&marker, hint.flashByteCursor);
        markerIsValid = spRecoverInfo->eepromBitCursor != 0; /* Equivalent to commented out line above. */
    }

    if (markerIsValid) {
        sInstance.eepromBitCursor = spRecoverInfo->eepromBitCursor;
        sInstance.flashByteCursor = marker.flashByteCursor;
    }
    else if (hintIsValid) {
        sInstance.eepromBitCursor = hint.eepromBitCursor;
        /* The bytes in the last written page - where the marker should start if there was no corruption - can not be
         * trusted. Recover most of them by using the duplicate data area.
         */
        int byteCursor = ((sInstance.eepromBitCursor / 8) / EEPROM_ROW_SIZE) * EEPROM_ROW_SIZE;
        uint8_t duplicate[SIZE_OF_DUPLICATE_DATA];
        Chip_EEPROM_Read(NSS_EEPROM, DUPLICATE_DATA_ABSOLUTE_BYTE_OFFSET, duplicate, SIZE_OF_DUPLICATE_DATA);
        Chip_EEPROM_Write(NSS_EEPROM, EEPROM_ABSOLUTE_FIRST_BYTE_OFFSET + byteCursor, duplicate, SIZE_OF_DUPLICATE_DATA);
        /* Remove the bits we couldn't recover. */
        int lastRecoveredBit = (byteCursor + SIZE_OF_DUPLICATE_DATA) * 8;
        while (sInstance.eepromBitCursor > lastRecoveredBit) {
            sInstance.eepromBitCursor -= STORAGE_BITSIZE;
        }

        sInstance.flashByteCursor = hint.flashByteCursor;
        spRecoverInfo->sampleCacheCount = 0;
    }
    else {
        sInstance.eepromBitCursor = 0;
        sInstance.flashByteCursor = 0;
        spRecoverInfo->sampleCacheCount = 0;
    }

    if (!markerIsValid) {
        WriteMarker();
    }
    if (!hintIsValid) {
        WriteHint();
    }
    Chip_EEPROM_Flush(NSS_EEPROM, true);
    /* After this point and before adding new samples, a new initialization of the storage module after a power failure
     * will always have a fast initialization, avoiding a slow search by calling FindMarker.
     */
}

void Storage_DeInit(void)
{
    /* Write the marker, but only when samples have been added or when the module has been reset - to avoid hitting
     * the max write cycles of the EEPROM.
     */

    Marker_t marker;
    ReadFromEeprom(spRecoverInfo->eepromBitCursor, &marker, sizeof(marker) * 8);
    bool markerIsValid = ValidateMarker(&marker, -1);

    if (sEepromBitCursorChanged || (!markerIsValid)) {
        WriteMarker();
    }

    /* Write the hint every X samples. */
    Hint_t hint;
    Chip_EEPROM_Read(NSS_EEPROM, HINT_ABSOLUTE_BYTE_OFFSET, &hint, sizeof(Hint_t));
    bool hintIsValid = ValidateHint(&hint);
    int difference = sInstance.eepromBitCursor - hint.eepromBitCursor;
    if ((!hintIsValid) || (difference < 0)
        || (difference >= STORAGE_WRITE_RECOVERY_EVERY_X_SAMPLES * STORAGE_BITSIZE)) {
        WriteHint();
    }

    Chip_EEPROM_Flush(NSS_EEPROM, true);

    spRecoverInfo->eepromBitCursor = (unsigned int)sInstance.eepromBitCursor & 0x7FFF;
    Chip_PMU_SetRetainedData((uint32_t *)sCache, STORAGE_FIRST_ALON_REGISTER, 5 - STORAGE_FIRST_ALON_REGISTER);
}

int Storage_GetCount(void)
{
    return spRecoverInfo->sampleCacheCount + GetEepromCount() + GetFlashCount();
}

void Storage_Reset(bool checkFlash)
{
    spRecoverInfo->sampleCacheCount = 0;

    /* The contents in EEPROM do not need to be fully erased, as they can be overwritten. Just ensure the EEPROM is
     * marked as 'empty'.
     * Clear the marker in EEPROM - after a power-off we don't want to find this information any more.
     */
    uint8_t zeroMarker[sizeof(Marker_t)] = {0};
    WriteToEeprom(sInstance.eepromBitCursor, zeroMarker, sizeof(Marker_t) * 8);

    /* Also invalidate the hint information */
    Chip_EEPROM_Memset(NSS_EEPROM, HINT_ABSOLUTE_BYTE_OFFSET, 0, sizeof(Hint_t));

#if STORAGE_FLASH_FIRST_PAGE > STORAGE_FLASH_LAST_PAGE
    (void)checkFlash; /* suppress [-Wunused-parameter]: There is no flash assigned for storage, nothing to check. */
#else
    if ((!checkFlash) || (STORAGE_FLASH_FIRST_PAGE > STORAGE_FLASH_LAST_PAGE)) {
        /* Caller explicitly indicates there is no need to check the flash, or,
         * there is no flash available for storage, so nothing to check.
         */
    }
    else {
        /* The contents in FLASH must be erased, as they can only be overwritten after erasing.
         * It is costly to erase the FLASH: it is taxing the battery and taking a long time. The FLASH is therefore
         * not erased unconditionally.
         * The only fail-safe way - also catching memory corruption; a direct EEPROM erase without a corresponding
         * direct FLASH erase; or a firmware image update - is to check the FLASH memory itself.
         * Check words from first to last, as the first words are more likely to be in use already. This is not done
         * using Chip_IAP_Flash_SectorBlankCheck, as this operates on full sectors only.
         * At 500 kHz, this adds +- 10 ms execution time per kb of assigned FLASH storage (worst case is
         * checking all words to find out an erase is not necessary).
         */
        uint32_t * cursor = FLASH_FIRST_WORD_ADDRESS;
        uint32_t * last = FLASH_LAST_WORD_ADDRESS;
        while ((cursor <= last) && (*cursor == 0xFFFFFFFF)) {
            cursor++;
        }
        if (cursor <= last) { /* If not above the last word to check, erase the FLASH memory. */
            IAP_STATUS_T status;
            const uint32_t sectorStart = (uint32_t)STORAGE_FLASH_FIRST_PAGE / FLASH_PAGES_PER_SECTOR;
            const uint32_t sectorEnd = STORAGE_FLASH_LAST_PAGE / FLASH_PAGES_PER_SECTOR;
            status = Chip_IAP_Flash_PrepareSector(sectorStart, sectorEnd);
            if (status == IAP_STATUS_CMD_SUCCESS) {
                /* - First erase pages up to (not including) the first page @c b of a sector @c s.
                 * - Next erase as many sectors as possible: from @c s till @c t.
                 * - Finally erase pages from the first page of @c sectorEnd till #STORAGE_FLASH_LAST_PAGE.
                 * Since each call involves different sectors, one prepare call (done above) is sufficient - only
                 * involved sectors are locked again in each IAP call below.
                 */
                uint32_t s = (uint32_t)(STORAGE_FLASH_FIRST_PAGE + FLASH_PAGES_PER_SECTOR - 1) / FLASH_PAGES_PER_SECTOR;
                uint32_t b = s * FLASH_PAGES_PER_SECTOR;
                uint32_t t = sectorEnd;
                if ((STORAGE_FLASH_LAST_PAGE % FLASH_PAGES_PER_SECTOR) < FLASH_PAGES_PER_SECTOR - 1) {
                    t--; /* The last sector involved is not fully assigned for sample storage. */
                }
                if ((uint32_t)STORAGE_FLASH_FIRST_PAGE < b) {
                    __disable_irq();
                    status = Chip_IAP_Flash_ErasePage((uint32_t)STORAGE_FLASH_FIRST_PAGE, b - 1, 0);
                    __enable_irq();
                    ASSERT(status == IAP_STATUS_CMD_SUCCESS);
                }
                if (s <= t) {
                    __disable_irq();
                    status = Chip_IAP_Flash_EraseSector(s, t, 0);
                    __enable_irq();
                    ASSERT(status == IAP_STATUS_CMD_SUCCESS);
                }
                if (t < sectorEnd) {
                    __disable_irq();
                    status = Chip_IAP_Flash_ErasePage(sectorEnd * FLASH_PAGES_PER_SECTOR, STORAGE_FLASH_LAST_PAGE, 0);
                    __enable_irq();
                    ASSERT(status == IAP_STATUS_CMD_SUCCESS);
                }
            }
            ASSERT(status == IAP_STATUS_CMD_SUCCESS);
        }
    }
#endif
    ResetInstance();
    /* A new marker will be written in EEPROM in a later call to Storage_DeInit() */
}

#if STORAGE_SAMPLE_ALON_CACHE_COUNT > 0
int Storage_Write(STORAGE_TYPE * samples, int n)
{
    ASSERT(samples != NULL);

    int count = 0;
    while (count < n) {
        if (CacheSample(samples + count)) {
            count++;
        }
        else {
            STORAGE_TYPE cachedSamples[STORAGE_SAMPLE_ALON_CACHE_COUNT + 1];
            int cachedCount = 0;
            while (GetCachedSample(cachedCount, cachedSamples + cachedCount)) {
                cachedCount++;
            }
            ASSERT(cachedCount == STORAGE_SAMPLE_ALON_CACHE_COUNT);
            cachedSamples[cachedCount] = samples[count];
            cachedCount++;

            int stored = StoreSamplesInEeprom(cachedSamples, cachedCount);
            if (stored) {
                count++; /* At least one sample was stored in EEPROM: there is room now in cache if needs be. */
                /* Clear cache, then place back on the cache what could not be moved to EEPROM. */
                spRecoverInfo->sampleCacheCount = 0;
                for (int i = stored; i < cachedCount; i++) {
                    CacheSample(cachedSamples + i);
                }

                /* Update variables used when reading samples. */
                if (sInstance.readLocation == LOCATION_CACHE) {
                    if (sInstance.readCursor >= stored) { /* Possibly true if not all samples could be moved. */
                        // sInstance.readLocation does not change
                        sInstance.readCursor -= stored;
                        // sInstance.readSequence does not change
                    }
                    else {
                        sInstance.readLocation = LOCATION_EEPROM;
                        sInstance.readCursor = sInstance.eepromBitCursor
                                - ((stored - sInstance.readCursor) * STORAGE_BITSIZE);
                        // sInstance.readSequence does not change
                    }
                }

                /* Loop again, cache has been updated. */
            }
            else {
                /* Leave the cache intact. */
                break; /* Stop the while loop: storage is full. */
            }
        }
    }

    return count;
}
#else
int Storage_Write(STORAGE_TYPE * samples, int n)
{
    ASSERT(samples != NULL);
    return StoreSamplesInEeprom(samples, n);
}
#endif

bool Storage_Seek(int n)
{
    sInstance.readLocation = LOCATION_UNKNOWN;
    sInstance.readSequence = -1;
    sInstance.readCursor = -1;

    if (n < 0) { return false; }

    int currentSequence = -1;
    int currentCursor = -1;
    int nextSequence = 0;
    int nextCursor = 0;

    /* Step through the (compressed) blocks in FLASH, counting the number of samples stored in there.
     * If the count surpasses n we have found a block where the requested sample is stored in.
     * - The cursor variables below indicate a byte offset in FLASH.
     * - The sequence variables below indicate a sequence count.
     */
    while (nextCursor < sInstance.flashByteCursor) {
        currentSequence = nextSequence;
        currentCursor = nextCursor;
        uint8_t * header = FLASH_CURSOR_TO_BYTE_ADDRESS(nextCursor);

        int bitCount = (int)(header[0] | (header[1] << 8));
        nextSequence += STORAGE_BLOCK_SIZE_IN_SAMPLES;
        nextCursor += FLASH_BLOCK_SIZE(bitCount);
        ASSERT((nextCursor & 0x3) == 0); /* Must be 32-bit word-aligned. */
        if ((currentSequence <= n) && (n < nextSequence)) {
            break;
        }
    }
    if ((currentSequence <= n) && (n < nextSequence)) {
        sInstance.readLocation = LOCATION_FLASH;
        sInstance.readSequence = currentSequence;
        sInstance.readCursor = currentCursor;
    }
    else {
        /* Try to find the sequence number sought for in EEPROM.
         * If the jump location surpasses sInstance.eepromBitCursor then the n-th sample is not yet committed in NVM
         * - The cursor variable below indicates a bit offset in EEPROM.
         */
        nextCursor = (n - nextSequence) * STORAGE_BITSIZE;
        if (nextCursor < sInstance.eepromBitCursor) {
            sInstance.readLocation = LOCATION_EEPROM;
            sInstance.readSequence = n;
            sInstance.readCursor = nextCursor;
        }
        else {
#if STORAGE_SAMPLE_ALON_CACHE_COUNT > 0
            /* Try to find the sequence number sought for in sCache.
             * - The sequence variable below indicates a sequence count.
             * - The cursor variable below indicates a sample offset in sCache.
             */
            nextSequence += GetEepromCount();
            nextCursor = n - nextSequence;
            if (nextCursor < spRecoverInfo->sampleCacheCount) {
                sInstance.readLocation = LOCATION_CACHE;
                sInstance.readSequence = n;
                sInstance.readCursor = nextCursor;
            }
#endif
            /* else: can not comply as there are not yet this many samples stored. */
        }
    }

    sInstance.targetSequence = n;
    return sInstance.readSequence >= 0;
}

int Storage_Read(STORAGE_TYPE * samples, int n)
{
    int count = 0;
    ASSERT(samples != NULL);

    if (sInstance.readSequence < 0) {
        /* A prior successful call to Storage_Seek is required. */
    }
    else {
        if (sInstance.readLocation == LOCATION_FLASH) {
            int blockSize = ReadAndCacheSamplesFromFlash(sInstance.readCursor);
            while (blockSize && (count < n) && (sInstance.readCursor < sInstance.flashByteCursor)) {
                while ((count < n) && (sInstance.readSequence + STORAGE_BLOCK_SIZE_IN_SAMPLES > sInstance.targetSequence)) {
                    /* Determine the offset in bytes and the initial number of LSBits to ignore. */
                    int bitOffset = (sInstance.targetSequence - sInstance.readSequence) * STORAGE_BITSIZE;
                    int byteOffset = bitOffset / 8;
                    int bitAlignment = bitOffset % 8;

                    ShiftUnalignedData((uint8_t *)(samples + count), STORAGE_WORKAREA + byteOffset, bitAlignment,
                                       STORAGE_BITSIZE);
                    bitOffset += STORAGE_BITSIZE;
                    count++;
                    sInstance.targetSequence++;
                }
                if (sInstance.readSequence + STORAGE_BLOCK_SIZE_IN_SAMPLES <= sInstance.targetSequence) {
                    /* A next sample is available in EEPROM or in the next (compressed) block of data in FLASH. */
                    sInstance.readCursor += blockSize;
                    ASSERT((sInstance.readCursor & 0x3) == 0); /* Must be 32-bit word-aligned. */
                    sInstance.readSequence += STORAGE_BLOCK_SIZE_IN_SAMPLES;
                }
                blockSize = ReadAndCacheSamplesFromFlash(sInstance.readCursor);
            }

            if (sInstance.readCursor >= sInstance.flashByteCursor) {
                /* All is read from FLASH, ensure the next read will pick the samples from EEPROM. */
                sInstance.readLocation = LOCATION_EEPROM;
                sInstance.readCursor = 0;
            }
        }

        if (sInstance.readLocation == LOCATION_EEPROM) {
            while ((count < n) && (sInstance.readCursor < sInstance.eepromBitCursor)) {
                ReadFromEeprom((unsigned int)sInstance.readCursor, samples + count, STORAGE_BITSIZE);
                count++;
                sInstance.readSequence++;
                sInstance.readCursor += STORAGE_BITSIZE;
                sInstance.targetSequence++;
            };

#if STORAGE_SAMPLE_ALON_CACHE_COUNT > 0
            if (sInstance.readCursor >= sInstance.eepromBitCursor) {
                /* All is read from EEPROM, ensure the next read will pick the samples from cache. */
                sInstance.readLocation = LOCATION_CACHE;
                sInstance.readCursor = 0;
            }
#endif
        }

#if STORAGE_SAMPLE_ALON_CACHE_COUNT > 0
        if (sInstance.readLocation == LOCATION_CACHE) {
            while ((count < n) && GetCachedSample(sInstance.readCursor, samples + count)) {
                count++;
                sInstance.readSequence++;
                sInstance.readCursor++;
                sInstance.targetSequence++;
            }
        }
#endif
    }

#if STORAGE_SIGNED
    /* STORAGE_TYPE is signed: propagate the bit at position STORAGE_BITSIZE to the left */
    int msbits = sizeof(STORAGE_TYPE) * 8 - STORAGE_BITSIZE;
    for (int i = 0; i < count; i++) {
        samples[i] = (STORAGE_TYPE)((STORAGE_TYPE)(samples[i] << msbits) >> msbits);
    }
#endif
    return count;
}
