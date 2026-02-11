/*------------------------------------------------------------------------*\
 * Base64Decoder.c
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

#include "Base64Decoder.h"
#include "CharBuffer.h"
#include "ByteBuffer.h"
#include <assert.h>

typedef char CHAR;
typedef unsigned char BYTE;

#define SKIP 0177
#define BAD  0277
#define END  0100

static BYTE BASE64_MAP[] = {
  /*  00     01     02     03     04     05     06     07 */
     BAD,   BAD,   BAD,   BAD,   BAD,   BAD,   BAD,   BAD,
  /*  08     09     0A     0B     0C     0D     0E     0F */
     BAD,  SKIP,  SKIP,   BAD,  SKIP,  SKIP,   BAD,   BAD,
  /*  10     11     12     13     14     15     16     17 */
     BAD,   BAD,   BAD,   BAD,   BAD,   BAD,   BAD,   BAD,
  /*  18     19     1A     1B     1C     1D     1E     1F */
     BAD,   BAD,   BAD,   BAD,   BAD,   BAD,   BAD,   BAD,
  /*  SP     !      "      #      $      %      &      '  */
    SKIP,   BAD,   BAD,   BAD,   BAD,   BAD,   BAD,   BAD,
  /*  (      )      *      +      ,      -      .      /  */
     BAD,   BAD,   BAD,    62,   BAD,   BAD,   BAD,    63,
  /*  0      1      2      3      4      5      6      7 */
      52,    53,    54,    55,    56,    57,    58,    59,
  /*  8      9      :      ;      <      =      >      ?  */
      60,    61,   BAD,   BAD,   BAD,   END,   BAD,   BAD,
  /*  @      A      B      C      D      E      F      G  */
     BAD,    0 ,    1 ,    2 ,    3 ,    4 ,    5 ,    6 ,
  /*  H      I      J      K      L      M      N      O  */
      7 ,    8 ,    9 ,    10,    11,    12,    13,    14,
  /*  P      Q      R      S      T      U      V      W  */
      15,    16,    17,    18,    19,    20,    21,    22,
  /*  X      Y      Z      [      \      ]      ^      _  */
      23,    24,    25,   BAD,   BAD,   BAD,   BAD,   BAD,
  /*  `      a      b      c      d      e      f      g  */
     BAD,    26,    27,    28,    29,    30,    31,    32,
  /*  h      i      j      k      l      m      n      o  */
      33,    34,    35,    36,    37,    38,    39,    40,
  /*  p      q      r      s      t      u      v      w  */
      41,    42,    43,    44,    45,    46,    47,    48,
  /*  x      y      z      {      |      }      ~     DEL */
      49,    50,    51,   BAD,   BAD,   BAD,   BAD,   BAD,
     BAD,   BAD,   BAD,   BAD,   BAD,   BAD,   BAD,   BAD,
     BAD,   BAD,   BAD,   BAD,   BAD,   BAD,   BAD,   BAD,
     BAD,   BAD,   BAD,   BAD,   BAD,   BAD,   BAD,   BAD,
     BAD,   BAD,   BAD,   BAD,   BAD,   BAD,   BAD,   BAD,
     BAD,   BAD,   BAD,   BAD,   BAD,   BAD,   BAD,   BAD,
     BAD,   BAD,   BAD,   BAD,   BAD,   BAD,   BAD,   BAD,
     BAD,   BAD,   BAD,   BAD,   BAD,   BAD,   BAD,   BAD,
     BAD,   BAD,   BAD,   BAD,   BAD,   BAD,   BAD,   BAD,
     BAD,   BAD,   BAD,   BAD,   BAD,   BAD,   BAD,   BAD,
     BAD,   BAD,   BAD,   BAD,   BAD,   BAD,   BAD,   BAD,
     BAD,   BAD,   BAD,   BAD,   BAD,   BAD,   BAD,   BAD,
     BAD,   BAD,   BAD,   BAD,   BAD,   BAD,   BAD,   BAD,
     BAD,   BAD,   BAD,   BAD,   BAD,   BAD,   BAD,   BAD,
     BAD,   BAD,   BAD,   BAD,   BAD,   BAD,   BAD,   BAD,
     BAD,   BAD,   BAD,   BAD,   BAD,   BAD,   BAD,   BAD,
     BAD,   BAD,   BAD,   BAD,   BAD,   BAD,   BAD,   BAD
};

#define STATE_0             0
#define STATE_1             1
#define STATE_2             3
#define STATE_3             4
#define STATE_OUTPUT_CRLF   5
#define STATE_OUTPUT_LF     6
#define STATE_OUTPUT_EQ_EQ  7
#define STATE_OUTPUT_EQ     8
#define STATE_DONE          9

static void Base64Decoder_decode4(Base64Decoder*, CharBuffer*, ByteBuffer*);


void Base64Decoder_start(
    Base64Decoder *self)
{
    self->mState = STATE_0;
    self->mError = 0;
    self->mLastSextet = 0;
}


int Base64Decoder_decode(
    Base64Decoder *self,
    CharBuffer *in,
    ByteBuffer *out)
{
    int char_, byte_, sextet;
    int inPos, inEndPos, outPos, outEndPos;
    CHAR *inBuffer;
    BYTE *outBuffer;

    if (self->mState == STATE_DONE) {
        return 1;
    }

    inBuffer = in->chars;
    inPos = in->pos;
    inEndPos = in->endPos;
    outBuffer = (unsigned char*)out->bytes;
    outPos = out->pos;
    outEndPos = out->endPos;

    while (inPos < inEndPos && outPos < outEndPos) {
        char_ = inBuffer[inPos++];
        sextet = BASE64_MAP[char_ & 0xff];
        if ((sextet & 0xc0) == 0) {
            switch (self->mState) {
            case STATE_0:
                self->mState = STATE_1;
                break;
            case STATE_1:
                self->mState = STATE_2;
                byte_ = self->mLastSextet << 2;
                byte_ += sextet >> 4;
                outBuffer[outPos++] = byte_;
                break;
            case STATE_2:
                self->mState = STATE_3;
                byte_ = self->mLastSextet << 4;
                byte_ += sextet >> 2;
                outBuffer[outPos++] = byte_;
                break;
            case STATE_3:
                self->mState = STATE_0;
                byte_ = self->mLastSextet << 6;
                byte_ += sextet;
                outBuffer[outPos++] = byte_;
                break;
            }
            self->mLastSextet = sextet;
        }
        else if (sextet == SKIP) {
        }
        else if (sextet == END) {
            self->mState = STATE_DONE;
            goto LOOP_EXIT;
        }
        else if (sextet == BAD) {
            self->mError = 1;
        }
    }
LOOP_EXIT:
    in->pos = inPos;
    out->pos = outPos;
    if (self->mState == STATE_DONE) {
        return 1;
    }
    else {
        return 0;
    }
}


int Base64Decoder_decodeFast(
    Base64Decoder *self,
    CharBuffer *in,
    ByteBuffer *out)
{
    int i, n, m, sextet, sextet1, sextet2, inLimit, saveInPos;
    int inPos, inEndPos, outPos, outEndPos;
    CHAR *inBuffer;
    BYTE *outBuffer;

    if (self->mState == STATE_DONE) {
        return 1;
    }
    /*
     * First, get in sync.  That is, make sure we are at the beginning
     * of an input quartet of base64 chars.
     */
    Base64Decoder_decode4(self, in, out);
    if (self->mState == STATE_DONE) {
        return 1;
    }
    inBuffer = in->chars;
    inPos = in->pos;
    inEndPos = in->endPos;
    outBuffer = (unsigned char*)out->bytes;
    outPos = out->pos;
    outEndPos = out->endPos;
    /*
     * Now, decode input quartets as fast as possible
     */
    while (inPos + 4 <= inEndPos && outPos + 3 <= outEndPos) {
        /*
         * Calculate a conservative max length of the input for the given
         * available space in the output buffer.  After this calculation,
         * we only need to compare the input pos to the input limit;
         * we don't have to compare the output pos to the output limit.
         */
        inLimit = inPos + (outEndPos - outPos) / 3 * 4;
        if (inLimit > inEndPos) {
            inLimit = inEndPos;
        }
        inLimit -= 4;
        while (inPos <= inLimit) {
            saveInPos = inPos;
            sextet1 = BASE64_MAP[inBuffer[inPos++] & 0xff];
            m = sextet1;
            n = sextet1;
            n <<= 6;
            sextet2 = BASE64_MAP[inBuffer[inPos++] & 0xff];
            m |= sextet2;
            n |= sextet2;
            n <<= 6;
            sextet = BASE64_MAP[inBuffer[inPos++] & 0xff];
            m |= sextet;
            n |= sextet;
            n <<= 6;
            sextet = BASE64_MAP[inBuffer[inPos++] & 0xff];
            m |= sextet;
            n |= sextet;
            /*
             * If all four input chars are "normal" base64 chars (that is,
             * not CR, LF, or other characters) then decode them.
             */
            if (! (0xc0 & m)) {
                outPos += 3;
                i = outPos;
                outBuffer[--i] = (BYTE) n;
                n >>= 8;
                outBuffer[--i] = (BYTE) n;
                n >>= 8;
                outBuffer[--i] = (BYTE) n;
            }
            /*
             * Otherwise, handle this case by advancing by one input quartet,
             * skipping characters as necessary
             */
            else {
                inPos = saveInPos;
                /* Optimization: in most cases, we just skip LF or CR LF */
                if (sextet1 == SKIP) {
                    ++inPos;
                    if (sextet2 == SKIP) {
                        ++inPos;
                    }
                }
                else {
                    in->pos = inPos;
                    out->pos = outPos;
                    Base64Decoder_decode4(self, in, out);
                    if (self->mState == STATE_DONE) {
                        return 1;
                    }
                    inPos = in->pos;
                    outPos = out->pos;
                }
            }
        }
    }
    /*
     * Now, finish up any remaining partial quartet
     */
    in->pos = inPos;
    out->pos = outPos;
    return Base64Decoder_decode(self, in, out);
}


static void Base64Decoder_decode4(
    Base64Decoder *self,
    CharBuffer *in,
    ByteBuffer *out)
{
    int char_, byte_, sextet, inPos, inEndPos, outPos, outEndPos;
    CHAR *inBuffer;
    BYTE *outBuffer;

    inBuffer = in->chars;
    inPos = in->pos;
    inEndPos = in->endPos;
    outBuffer = (unsigned char*)out->bytes;
    outPos = out->pos;
    outEndPos = out->endPos;

    while (inPos < inEndPos && outPos < outEndPos) {
        char_ = inBuffer[inPos++];
        sextet = BASE64_MAP[char_ & 0xff];
        if (! (sextet & 0xc0)) {
            switch (self->mState) {
            case STATE_0:
                self->mState = STATE_1;
                break;
            case STATE_1:
                self->mState = STATE_2;
                byte_ = self->mLastSextet << 2;
                byte_ += sextet >> 4;
                outBuffer[outPos++] = byte_;
                break;
            case STATE_2:
                self->mState = STATE_3;
                byte_ = self->mLastSextet << 4;
                byte_ += sextet >> 2;
                outBuffer[outPos++] = byte_;
                break;
            case STATE_3:
                self->mState = STATE_0;
                byte_ = self->mLastSextet << 6;
                byte_ += sextet;
                outBuffer[outPos++] = byte_;
                goto LOOP_EXIT;
                break;
            }
            self->mLastSextet = sextet;
        }
        else if (sextet == SKIP) {
        }
        else if (sextet == END) {
            self->mState = STATE_DONE;
            goto LOOP_EXIT;
        }
        else if (sextet == BAD) {
            self->mError = 1;
        }
    }
LOOP_EXIT:
    in->pos = inPos;
    out->pos = outPos;
}
