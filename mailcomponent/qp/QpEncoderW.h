/*------------------------------------------------------------------------*\
 * QpEncoderW.h
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

#ifndef QP_ENCODER_W_H
#define QP_ENCODER_W_H

#pragma once

#include "ByteBuffer.h"
#include "CharBufferW.h"

typedef struct QpEncoderW {
    /* buffers */
    int mInPos;
    int mInLen;
    unsigned char *mInBuffer;
    int mOutPos;
    int mOutLen;
    unsigned short *mOutBuffer;
    /* options */
    int mMaxLineLen;
    int mOutputCrLf;
    int mSuppressFinalNewline;
    int mProtectFrom;
    int mProtectDot;
    unsigned char *mEncodeMap;
    /* state */
    int mState;
    int mLinePos;
    int mLineLen;
    int mLineBreakType;
    int mLineBufferLen;
    unsigned short *mLineBuffer;
} QpEncoderW;

extern unsigned char *QpEncoderW_LOW_RISK_MAP;
extern unsigned char *QpEncoderW_HIGH_RISK_MAP;

/*
 * Life cycle
 */
void QpEncoderW_initialize(QpEncoderW *encoder);
void QpEncoder_finalize(QpEncoderW *encoder);
/*
 * Options
 */
void QpEncoderW_setMaxLineLen(QpEncoderW *encoder, int len);
int QpEncoderW_getMaxLineLen(QpEncoderW *encoder);
void QpEncoderW_setOutputCrLf(QpEncoderW *encoder, int b);
int QpEncoderW_getOutputCrLf(QpEncoderW *encoder);
void QpEncoderW_setSuppressFinalNewline(QpEncoderW *encoder, int b);
int QpEncoderW_getSuppressFinalNewline(QpEncoderW *encoder);
void QpEncoderW_setProtectFrom(QpEncoderW *encoder, int b);
int QpEncoderW_getProtectFrom(QpEncoderW *encoder);
void QpEncoderW_setProtectDot(QpEncoderW *encoder, int b);
int QpEncoderW_getProtectDot(QpEncoderW *encoder);
void QpEncoderW_setEncodeMap(QpEncoderW *encoder, unsigned char *map);
/*
 * Operations
 */
void QpEncoderW_start(QpEncoderW *encoder);
void QpEncoderW_encode(QpEncoderW *encoder, ByteBuffer *in, CharBufferW *out);
void QpEncoderW_finish(QpEncoderW *encoder, CharBufferW *out);

#endif
