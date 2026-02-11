/*------------------------------------------------------------------------*\
 * Base64DecoderW.h
 *
 * Copyright (c) 1998-2002 Hunny Software, Inc.
 * All rights reserved.
 *
 * WWW: http://hunnysoft.com/
 * Email: Doug Sauder <doug@hunnysoft.com>
 *
 * IN NO EVENT SHALL HUNNY SOFTWARE, INC BE LIABLE TO ANY PARTY FOR DIRECT,
 * INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT OF
 * THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF HUNNY SOFTWARE,
 * INC HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * HUNNY SOFTWARE, INC SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING,
 * BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS ON
 * AN "AS IS" BASIS, AND HUNNY SOFTWARE, INC HAS NO OBLIGATION TO PROVIDE
 * MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
\*------------------------------------------------------------------------*/

#pragma once

#ifndef BASE64_DECODER_W_H
#define BASE64_DECODER_W_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Base64DecoderW {
    /* state */
    int mState;
    int mError;           /* nonzero if bad input seen */
    int mLastSextet;      /* left over sextet from previous input buffer */
} Base64DecoderW;

void Base64DecoderW_start(Base64DecoderW *decoder);
int Base64DecoderW_decode(Base64DecoderW *decoder, struct CharBufferW *in,
    struct ByteBuffer *out);
int Base64DecoderW_decodeFast(Base64DecoderW *decoder, struct CharBufferW *in,
    struct ByteBuffer *out);

#ifdef __cplusplus
}
#endif

#endif
