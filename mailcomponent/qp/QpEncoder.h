/*------------------------------------------------------------------------*\
 * QpEncoder.h
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

#ifndef QP_ENCODER_H
#define QP_ENCODER_H

#pragma once

#include "ByteBuffer.h"
#include "CharBuffer.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct QpEncoder {
    /* buffers */
    int mInPos;
    int mInLen;
    unsigned char *mInBuffer;
    int mOutPos;
    int mOutLen;
    char *mOutBuffer;
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
    char *mLineBuffer;
} QpEncoder;

extern unsigned char *QpEncoder_LOW_RISK_MAP;
extern unsigned char *QpEncoder_HIGH_RISK_MAP;

/*
 * Life cycle
 */
void QpEncoder_initialize(QpEncoder *encoder);
void QpEncoder_finalize(QpEncoder *encoder);
/*
 * Options
 */
void QpEncoder_setMaxLineLen(QpEncoder *encoder, int len);
int QpEncoder_getMaxLineLen(QpEncoder *encoder);
void QpEncoder_setOutputCrLf(QpEncoder *encoder, int b);
int QpEncoder_getOutputCrLf(QpEncoder *encoder);
void QpEncoder_setSuppressFinalNewline(QpEncoder *encoder, int b);
int QpEncoder_getSuppressFinalNewline(QpEncoder *encoder);
void QpEncoder_setProtectFrom(QpEncoder *encoder, int b);
int QpEncoder_getProtectFrom(QpEncoder *encoder);
void QpEncoder_setProtectDot(QpEncoder *encoder, int b);
int QpEncoder_getProtectDot(QpEncoder *encoder);
void QpEncoder_setEncodeMap(QpEncoder *encoder, unsigned char *emap);
/*
 * Operations
 */
void QpEncoder_start(QpEncoder *encoder);
void QpEncoder_encode(QpEncoder *encoder, ByteBuffer *in, CharBuffer *out);
void QpEncoder_finish(QpEncoder *encoder, CharBuffer *out);

#ifdef __cplusplus
}
#endif

#endif
