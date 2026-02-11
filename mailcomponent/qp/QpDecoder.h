/*------------------------------------------------------------------------*\
 * QpDecoder.h
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

#ifndef QP_DECODER_H
#define QP_DECODER_H

#pragma once

#include "CharBuffer.h"
#include "ByteBuffer.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct QpDecoder {
    /* options */
    int mOutputCrLf;
    /* state */
    int mError;
    int mState;
    int mTmpPos;
    int mTmpLen;
    unsigned char *mTmpBuffer;
} QpDecoder;

/*
 * Life cycle
 */
void QpDecoder_initialize(QpDecoder *decoder);
void QpDecoder_finalize(QpDecoder *decoder);
/*
 * Options
 */
void QpDecoder_setOutputCrLf(QpDecoder *decoder, int b);
int QpDecoder_getOutputCrLf(QpDecoder *decoder);
/*
 * Operations
 */
void QpDecoder_start(QpDecoder *decoder);
void QpDecoder_decode(QpDecoder *decoder, CharBuffer *in, ByteBuffer *out);
void QpDecoder_finish(QpDecoder *decoder, ByteBuffer *out);

#ifdef __cplusplus
}
#endif

#endif
