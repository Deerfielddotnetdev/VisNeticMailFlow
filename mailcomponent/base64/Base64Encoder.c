/*------------------------------------------------------------------------*\
 * Base64Encoder.c
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

#include "Base64Encoder.h"
#include "CharBuffer.h"
#include "ByteBuffer.h"
#include <assert.h>

typedef unsigned char BYTE;
typedef char CHAR;

static char *BASE64_CHARS = 
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

#define STATE_0              0
#define STATE_1              1
#define STATE_2              2
#define STATE_3              3
#define STATE_OUTPUT_CRLF    4
#define STATE_OUTPUT_LF      5
#define STATE_OUTPUT_EQ_EQ   6
#define STATE_OUTPUT_EQ      7
#define STATE_DONE           8

#define LF      10
#define CR      13
#define EQUALS  61

static void fast_encode(BYTE *inBuffer, int inBufferLen, CHAR *outBuffer);

void Base64Encoder_setDefaults(
    Base64Encoder *self)
{
    self->mMaxLineLen = 72;
    self->mOutputCrLf = 0;
    self->mSuppressFinalNewline = 0;
}

void Base64Encoder_setMaxLineLen(
    Base64Encoder *self,
    int n)
{
    self->mMaxLineLen = n;
}

int Base64Encoder_getMaxLineLen(
    Base64Encoder *self)
{
    return self->mMaxLineLen;
}

void Base64Encoder_setOutputCrLf(
    Base64Encoder *self,
    int b)
{
    self->mOutputCrLf = b;
}

int Base64Encoder_getOutputCrLf(
    Base64Encoder *self)
{
    return self->mOutputCrLf;
}

void Base64Encoder_setSuppressFinalNewline(
    Base64Encoder *self,
    int b)
{
    self->mSuppressFinalNewline = b;
}

int Base64Encoder_getSuppressFinalNewline(
    Base64Encoder *self)
{
    return self->mSuppressFinalNewline;
}

void Base64Encoder_start(
    Base64Encoder *self)
{
    self->mState = STATE_0;
    self->mLastByte = 0;
    self->mLineLen = 0;
}

void Base64Encoder_encode(
    Base64Encoder *self,
    ByteBuffer *aIn,
    CharBuffer *aOut)
{
    int sextet, inPos, inEndPos, outPos, outEndPos, maxLineLen;
    int byte0, byte1;
    BYTE *inBuffer;
    CHAR *outBuffer;

    inBuffer = (unsigned char*)aIn->bytes;
    inPos = aIn->pos;
    inEndPos = aIn->endPos;
    outBuffer = aOut->chars;
    outPos = aOut->pos;
    outEndPos = aOut->endPos;
    maxLineLen = self->mMaxLineLen & 0x7ffffffc;

    byte0 = self->mLastByte;

    while (inPos < inEndPos && outPos < outEndPos) {
        switch (self->mState) {
        case STATE_0:
            self->mState = STATE_1;
            byte0 = inBuffer[inPos++] & 0xff;
            sextet = byte0 >> 2;
            outBuffer[outPos++] = (CHAR) BASE64_CHARS[sextet];
            ++self->mLineLen;
            break;
        case STATE_1:
            self->mState = STATE_2;
            byte1 = inBuffer[inPos++] & 0xff;
            sextet = (byte0 & 0x03) << 4;
            sextet += byte1 >> 4;
            outBuffer[outPos++] = (CHAR) BASE64_CHARS[sextet];
            byte0 = byte1;
            ++self->mLineLen;
            break;
        case STATE_2:
            self->mState = STATE_3;
            byte1 = inBuffer[inPos++] & 0xff;
            sextet = (byte0 & 0x0f) << 2;
            sextet += byte1 >> 6;
            outBuffer[outPos++] = (CHAR) BASE64_CHARS[sextet];
            byte0 = byte1;
            ++self->mLineLen;
            break;
        case STATE_3:
            self->mState = STATE_0;
            sextet = byte0 & 0x3f;
            outBuffer[outPos++] = (CHAR) BASE64_CHARS[sextet];
            ++self->mLineLen;
            if (self->mLineLen >= maxLineLen) {
                self->mLineLen = 0;
                if (self->mOutputCrLf) {
                    self->mState = STATE_OUTPUT_CRLF;
                }
                else /* if (! self->mOutputCrLf) */ {
                    self->mState = STATE_OUTPUT_LF;
                }
            }
            break;
        case STATE_OUTPUT_LF:
            self->mState = STATE_0;
            outBuffer[outPos++] = (CHAR) LF;
            break;
        case STATE_OUTPUT_CRLF:
            self->mState = STATE_OUTPUT_LF;
            outBuffer[outPos++] = (CHAR) CR;
            break;
        }
    }
    while (outPos < outEndPos) {
        switch (self->mState) {
        case STATE_0:
        case STATE_1:
        case STATE_2:
            goto LOOP_EXIT;
            break;
        case STATE_3:
            self->mState = STATE_0;
            sextet = byte0 & 0x3f;
            outBuffer[outPos++] = (CHAR) BASE64_CHARS[sextet];
            ++self->mLineLen;
            if (self->mLineLen >= maxLineLen) {
                self->mLineLen = 0;
                if (self->mOutputCrLf) {
                    self->mState = STATE_OUTPUT_CRLF;
                }
                else /* if (! self->mOutputCrLf) */ {
                    self->mState = STATE_OUTPUT_LF;
                }
            }
            break;
        case STATE_OUTPUT_LF:
            self->mState = STATE_0;
            outBuffer[outPos++] = (CHAR) LF;
            break;
        case STATE_OUTPUT_CRLF:
            self->mState = STATE_OUTPUT_LF;
            outBuffer[outPos++] = (CHAR) CR;
            break;
        }
    }
LOOP_EXIT:
    aIn->pos = inPos;
    aOut->pos = outPos;
    self->mLastByte = byte0;
}

void Base64Encoder_encodeFast(
    Base64Encoder *self,
    ByteBuffer *aIn,
    CharBuffer *aOut)
{
    int byte_, sextet, numEolChars, inSegmentLen, outSegmentLen;
    int inEndPos, outEndPos, inPos, outPos, inLimit, maxLineLen;
    BYTE *inBuffer;
    CHAR *outBuffer;

    inBuffer = (unsigned char*)aIn->bytes;
    inPos = aIn->pos;
    inEndPos = aIn->endPos;
    outBuffer = aOut->chars;
    outPos = aOut->pos;
    outEndPos = aOut->endPos;
    maxLineLen = self->mMaxLineLen & 0x7ffffffc;

    numEolChars = (self->mOutputCrLf) ? 2 : 1;
    /*
     * Finish the current input triplet
     */
    while (inPos < inEndPos && outPos < outEndPos) {
        switch (self->mState) {
        case STATE_0:
            goto LOOP_EXIT;
            break;
        case STATE_1:
            self->mState = STATE_2;
            byte_ = inBuffer[inPos++] & 0xff;
            sextet = (self->mLastByte & 0x03) << 4;
            sextet += byte_ >> 4;
            outBuffer[outPos++] = (CHAR) BASE64_CHARS[sextet];
            self->mLastByte = byte_;
            ++self->mLineLen;
            break;
        case STATE_2:
            self->mState = STATE_3;
            byte_ = inBuffer[inPos++] & 0xff;
            sextet = (self->mLastByte & 0x0f) << 2;
            sextet += byte_ >> 6;
            outBuffer[outPos++] = (CHAR) BASE64_CHARS[sextet];
            self->mLastByte = byte_;
            ++self->mLineLen;
            break;
        case STATE_3:
            self->mState = STATE_0;
            sextet = self->mLastByte & 0x3f;
            outBuffer[outPos++] = (CHAR) BASE64_CHARS[sextet];
            ++self->mLineLen;
            if (self->mLineLen >= maxLineLen) {
                self->mLineLen = 0;
                if (self->mOutputCrLf) {
                    self->mState = STATE_OUTPUT_CRLF;
                }
                else /* if (! self->mOutputCrLf) */ {
                    self->mState = STATE_OUTPUT_LF;
                }
            }
            break;
        case STATE_OUTPUT_LF:
            self->mState = STATE_0;
            outBuffer[outPos++] = (CHAR) LF;
            break;
        case STATE_OUTPUT_CRLF:
            self->mState = STATE_OUTPUT_LF;
            outBuffer[outPos++] = (CHAR) CR;
            break;
        }
    }
LOOP_EXIT:
    /*
     * Finish the current line of output
     */
    outSegmentLen = (maxLineLen - self->mLineLen) & 0x7ffffffc;
    inSegmentLen = outSegmentLen / 4 * 3;
    if (inSegmentLen > 0
        && inPos + inSegmentLen <= inEndPos
        && outPos + outSegmentLen + numEolChars <= outEndPos) {

        assert(self->mState == STATE_0);
        fast_encode(&inBuffer[inPos], inSegmentLen, &outBuffer[outPos]);
        inPos += inSegmentLen;
        outPos += outSegmentLen;
        if (self->mOutputCrLf) {
            outBuffer[outPos++] = (CHAR) CR;
        }
        outBuffer[outPos++] = (CHAR) LF;
        self->mLineLen = 0;
    }
    /*
     * Encode one line at a time, as fast as possible
     */
    outSegmentLen = maxLineLen;
    inSegmentLen = outSegmentLen / 4 * 3;
    inLimit = inPos + (outEndPos - outPos) / (outSegmentLen + numEolChars)
        * inSegmentLen;
    if (inLimit > inEndPos) {
        inLimit = inEndPos;
    }
    inLimit -= inSegmentLen;
    while (inPos <= inLimit) {
        assert(self->mState == STATE_0);
        assert(self->mLineLen == 0);
        fast_encode(&inBuffer[inPos], inSegmentLen, &outBuffer[outPos]);
        inPos += inSegmentLen;
        outPos += outSegmentLen;
        if (self->mOutputCrLf) {
            outBuffer[outPos++] = (CHAR) CR;
        }
        outBuffer[outPos++] = (CHAR) LF;
    }
    /*
     * Encode remaining partial line
     */
    outSegmentLen = (outEndPos - outPos) & 0x7ffffffc;
    inSegmentLen = outSegmentLen / 4 * 3;
    if (inSegmentLen > inEndPos - inPos) {
        inSegmentLen = (inEndPos - inPos) / 3 * 3;
        outSegmentLen = inSegmentLen / 3 * 4;
    }
    if (inSegmentLen > 0) {
        assert(self->mState == STATE_0);
        fast_encode(&inBuffer[inPos], inSegmentLen, &outBuffer[outPos]);
        inPos += inSegmentLen;
        outPos += outSegmentLen;
        self->mLineLen += outSegmentLen;
    }
    /*
     * Encode remaining partial triplet
     */
    aIn->pos = inPos;
    aOut->pos = outPos;
    Base64Encoder_encode(self, aIn, aOut);
}


void Base64Encoder_finish(
    Base64Encoder *self,
    CharBuffer *aOut)
{
    int sextet, outPos, outEndPos, maxLineLen;
    CHAR *outBuffer;

    outBuffer = aOut->chars;
    outPos = aOut->pos;
    outEndPos = aOut->endPos;
    maxLineLen = self->mMaxLineLen & 0x7ffffffc;

    /*
     * Note: this function requires that there be space in the output
     * buffer, even if there will be no output
     */
    while (outPos < outEndPos) {
        switch (self->mState) {
        case STATE_0:
            self->mState = STATE_DONE;
            if (self->mLineLen > 0 && ! self->mSuppressFinalNewline) {
                self->mLineLen = 0;
                if (self->mOutputCrLf) {
                    self->mState = STATE_OUTPUT_LF;
                    outBuffer[outPos++] = (CHAR) CR;
                }
                else /* if (! self->mOutputCrLf) */ {
                    outBuffer[outPos++] = (CHAR) LF;
                }
            }
            break;
        case STATE_1:
            self->mState = STATE_OUTPUT_EQ_EQ;
            sextet = (self->mLastByte & 0x03) << 4;
            outBuffer[outPos++] = (CHAR) BASE64_CHARS[sextet];
            ++self->mLineLen;
            break;
        case STATE_2:
            self->mState = STATE_OUTPUT_EQ;
            sextet = (self->mLastByte & 0x0f) << 2;
            outBuffer[outPos++] = (CHAR) BASE64_CHARS[sextet];
            ++self->mLineLen;
            break;
        case STATE_3:
            self->mState = STATE_DONE;
            sextet = self->mLastByte & 0x3f;
            outBuffer[outPos++] = (CHAR) BASE64_CHARS[sextet];
            ++self->mLineLen;
            if (self->mLineLen >= maxLineLen
                || ! self->mSuppressFinalNewline) {
                self->mLineLen = 0;
                if (self->mOutputCrLf) {
                    self->mState = STATE_OUTPUT_CRLF;
                }
                else /* if (! self->mOutputCrLf) */ {
                    self->mState = STATE_OUTPUT_LF;
                }
            }
            break;
        case STATE_OUTPUT_EQ_EQ:
            self->mState = STATE_OUTPUT_EQ;
            outBuffer[outPos++] = (CHAR) EQUALS;
            break;
        case STATE_OUTPUT_EQ:
            self->mState = STATE_DONE;
            outBuffer[outPos++] = (CHAR) EQUALS;
            if (self->mLineLen >= maxLineLen
                || ! self->mSuppressFinalNewline) {
                self->mLineLen = 0;
                if (self->mOutputCrLf) {
                    self->mState = STATE_OUTPUT_CRLF;
                }
                else /* if (! self->mOutputCrLf) */ {
                    self->mState = STATE_OUTPUT_LF;
                }
            }
            break;
        case STATE_OUTPUT_LF:
            self->mState = STATE_DONE;
            outBuffer[outPos++] = (CHAR) LF;
            break;
        case STATE_OUTPUT_CRLF:
            self->mState = STATE_OUTPUT_LF;
            outBuffer[outPos++] = (CHAR) CR;
            break;
        case STATE_DONE:
            goto LOOP_EXIT;
            break;
        }
    }
LOOP_EXIT:
    aOut->pos = outPos;
}


static void fast_encode(
    BYTE *aInBuffer,
    int aInBufferLen,
    CHAR *aOutBuffer)
{
    int i, byte_, n, inPos, outPos;

    inPos = 0;
    outPos = 0;
    while (inPos < aInBufferLen) {
        byte_ = aInBuffer[inPos++] & 0xff;
        n = byte_;
        n <<= 8;
        byte_ = aInBuffer[inPos++] & 0xff;
        n += byte_;
        n <<= 8;
        byte_ = aInBuffer[inPos++] & 0xff;
        n += byte_;
        outPos += 4;
        i = outPos;
        aOutBuffer[--i] = (CHAR) BASE64_CHARS[n&0x3f];
        n >>= 6;
        aOutBuffer[--i] = (CHAR) BASE64_CHARS[n&0x3f];
        n >>= 6;
        aOutBuffer[--i] = (CHAR) BASE64_CHARS[n&0x3f];
        n >>= 6;
        aOutBuffer[--i] = (CHAR) BASE64_CHARS[n&0x3f];
    }
}
