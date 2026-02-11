/*------------------------------------------------------------------------*\
 * QpDecoderW.h
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

#ifndef QP_DECODER_W_H
#define QP_DECODER_W_H

#pragma once

#include "CharBufferW.h"
#include "ByteBuffer.h"

typedef struct QpDecoderW {
    /* options */
    int mOutputCrLf;
    /* state */
    int mError;
    int mState;
    int mTmpPos;
    int mTmpLen;
    unsigned char *mTmpBuffer;
} QpDecoderW;

/*
 * Life cycle
 */
void QpDecoderW_initialize(QpDecoderW *decoder);
void QpDecoderW_finalize(QpDecoderW *decoder);
/*
 * Options
 */
void QpDecoderW_setOutputCrLf(QpDecoderW *decoder, int b);
int QpDecoderW_getOutputCrLf(QpDecoderW *decoder);
/*
 * Operations
 */
void QpDecoderW_start(QpDecoderW *decoder);
void QpDecoderW_decode(QpDecoderW *decoder, CharBufferW *in, ByteBuffer *out);
void QpDecoderW_finish(QpDecoderW *decoder, ByteBuffer *out);

#endif
