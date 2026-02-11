/*------------------------------------------------------------------------*\
 * Base64Encoder.h
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

#ifndef BASE64_ENCODER_H
#define BASE64_ENCODER_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Base64Encoder {
    /* options */
    int mMaxLineLen;      /* maximum number of chars on output line
                           * (not including end of line chars */
    int mOutputCrLf;      /* TRUE  ==> write CRLF at end of line
                           * FALSE ==> write LF at end of line */
    int mSuppressFinalNewline;/* TRUE ==> don't write LF or CRLF at end */
    /* state */
    int mState;
    int mLastByte;        /* left over byte from previous input buffer */
    int mLineLen;         /* current output line length */
} Base64Encoder;

/*
 * Attributes
 */
void Base64Encoder_setMaxLineLen(Base64Encoder *encoder, int n);
int Base64Encoder_getMaxLineLen(Base64Encoder *encoder);
void Base64Encoder_setOutputCrLf(Base64Encoder *encoder, int b);
int Base64Encoder_getOutputCrLf(Base64Encoder *encoder);
void Base64Encoder_setSuppressFinalNewline(Base64Encoder *encoder, int b);
int Base64Encoder_getSuppressFinalNewline(Base64Encoder *encoder);

/*
 * Operations
 */
void Base64Encoder_setDefaults(Base64Encoder *encoder);

void Base64Encoder_start(Base64Encoder *encoder);
void Base64Encoder_encode(Base64Encoder *encoder,
    struct ByteBuffer *in, struct CharBuffer *out);
void Base64Encoder_encodeFast(Base64Encoder *encoder,
    struct ByteBuffer *in, struct CharBuffer *out);
void Base64Encoder_finish(Base64Encoder *encoder,
    struct CharBuffer *out);

#ifdef __cplusplus
}
#endif

#endif
