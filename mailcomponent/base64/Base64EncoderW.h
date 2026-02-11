/*------------------------------------------------------------------------*\
 * Base64EncoderW.h
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

#ifndef BASE64_ENCODER_W_H
#define BASE64_ENCODER_W_H

typedef struct Base64EncoderW {
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
} Base64EncoderW;

/*
 * Attributes
 */
void Base64EncoderW_setMaxLineLen(Base64EncoderW *encoder, int n);
int Base64EncoderW_getMaxLineLen(Base64EncoderW *encoder);
void Base64EncoderW_setOutputCrLf(Base64EncoderW *encoder, int b);
int Base64EncoderW_getOutputCrLf(Base64EncoderW *encoder);
void Base64EncoderW_setSuppressFinalNewline(Base64EncoderW *encoder, int b);
int Base64EncoderW_getSuppressFinalNewline(Base64EncoderW *encoder);

/*
 * Operations
 */
void Base64EncoderW_setDefaults(Base64EncoderW *encoder);

void Base64EncoderW_start(Base64EncoderW *encoder);
void Base64EncoderW_encode(Base64EncoderW *encoder,
    struct ByteBuffer *in, struct CharBufferW *out);
void Base64EncoderW_encodeFast(Base64EncoderW *encoder,
    struct ByteBuffer *in, struct CharBufferW *out);
void Base64EncoderW_finish(Base64EncoderW *encoder,
    struct CharBufferW *out);

#endif
