/*
 * Copyright 2016 NXP
 * This software is owned or controlled by NXP and may only be used strictly
 * in accordance with the applicable license terms.  By expressly accepting
 * such terms or by downloading, installing, activating and/or otherwise using
 * the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to
 * be bound by the applicable license terms, then you may not retain, install,
 * activate or otherwise use the software.
 */

#include "board.h"
#include "compress/compress.h"
#include "heatshrink/heatshrink_encoder.h"
#include "heatshrink/heatshrink_decoder.h"

int Compress_Encode(const uint8_t * input, int inputLength, uint8_t * output, int outputLength)
{
    heatshrink_encoder encoder;
    bool success = true;
    int compressedSize = 0;

    heatshrink_encoder_reset(&encoder);
    while (success && (inputLength > 0)) {
        /* Add uncompressed data */
        int sunk = 0;
        success &= heatshrink_encoder_sink(&encoder, input, (size_t)inputLength, (size_t *)&sunk) == HSER_SINK_OK;
        input += sunk;
        inputLength -= sunk;
        if (inputLength == 0) {
            success &= heatshrink_encoder_finish(&encoder) == HSER_FINISH_MORE;
        }
        /* Retrieve compressed data */
        HSE_poll_res pollResult;
        int polled;
        do {
            polled = 0;
            pollResult = heatshrink_encoder_poll(&encoder, output, (size_t)outputLength, (size_t *)&polled);
            output += polled;
            outputLength -= polled;
            compressedSize += polled;
        } while ((pollResult == HSER_POLL_MORE) && (polled > 0));
        success &= pollResult == HSER_POLL_EMPTY;
    }
    success &= heatshrink_encoder_finish(&encoder) == HSER_FINISH_DONE;
    return success ? compressedSize : 0;
}

int Compress_Decode(const uint8_t * input, int inputLength, uint8_t * output, int outputLength)
{
    heatshrink_decoder decoder;
    bool success = true;
    int uncompressedSize = 0;

    heatshrink_decoder_reset(&decoder);
    while (success && (inputLength > 0)) {
        /* Add compressed data */
        int sunk = 0;
        success &= heatshrink_decoder_sink(&decoder, input, (size_t)inputLength, (size_t *)&sunk) == HSDR_SINK_OK;
        input += sunk;
        inputLength -= sunk;
        if (inputLength == 0) {
            success &= heatshrink_decoder_finish(&decoder) == HSDR_FINISH_MORE;
        }
        /* Retrieve uncompressed data */
        HSD_poll_res pollResult;
        int polled;
        do {
            polled = 0;
            pollResult = heatshrink_decoder_poll(&decoder, output, (size_t)outputLength, (size_t *)&polled);
            output += polled;
            outputLength -= polled;
            uncompressedSize += polled;
        } while ((pollResult == HSDR_POLL_MORE) && (polled > 0));
        /* If the decoding fully fills the available buffer, polled equaled outputLength when heatshrink_decoder_poll
         * returned the last but one time; and both polled and outputLength are now 0 after heatshrink_decoder_poll a
         * last time. In both cases the returned value is HSDR_POLL_MORE. We can only know for sure the full decoding is
         * done - and the outputLength was chosen exactly big enough - by calling heatshrink_decoder_finish, done below.
         */
        success &= ((pollResult == HSDR_POLL_EMPTY) || (pollResult == HSDR_POLL_MORE));
    }
    success &= heatshrink_decoder_finish(&decoder) == HSDR_FINISH_DONE;

    return success ? uncompressedSize : 0;
}
