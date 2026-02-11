/*------------------------------------------------------------------------*\
 * QpDecoder.c
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

#include "QpDecoder.h"
#include "CharBuffer.h"
#include "ByteBuffer.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

typedef char CHAR;
typedef unsigned char BYTE;

#define INTERNAL_BUFFER_SIZE  100

#define STATE_NORMAL   0
#define STATE_BUFFER   1
#define STATE_SKIP     2
#define STATE_FLUSH    3
#define STATE_ESCAPE   4
#define STATE_HEX      5

#define HT       9
#define LF      10
#define CR      13
#define SP      32
#define EQUALS  61

#define BAD     255
#define LITERAL   1
#define SPACE     2
#define SPECIAL   3

#define LIT  LITERAL
#define WSP  SPACE
#define SPE  SPECIAL

static BYTE HEX_MAP[] = {
  /*  00     01     02     03     04     05     06     07 */
     BAD,   BAD,   BAD,   BAD,   BAD,   BAD,   BAD,   BAD,
  /*  08     09     0A     0B     0C     0D     0E     0F */
     BAD,   BAD,   BAD,   BAD,   BAD,   BAD,   BAD,   BAD,
  /*  10     11     12     13     14     15     16     17 */
     BAD,   BAD,   BAD,   BAD,   BAD,   BAD,   BAD,   BAD,
  /*  18     19     1A     1B     1C     1D     1E     1F */
     BAD,   BAD,   BAD,   BAD,   BAD,   BAD,   BAD,   BAD,
  /*  SP     !      "      #      $      %      &      '  */
     BAD,   BAD,   BAD,   BAD,   BAD,   BAD,   BAD,   BAD,
  /*  (      )      *      +      ,      -      .      /  */
     BAD,   BAD,   BAD,   BAD,   BAD,   BAD,   BAD,   BAD,
  /*  0      1      2      3      4      5      6      7 */
      0 ,    1 ,    2 ,    3 ,    4 ,    5 ,    6 ,    7 ,
  /*  8      9      :      ;      <      =      >      ?  */
      8 ,    9 ,   BAD,   BAD,   BAD,   BAD,   BAD,   BAD,
  /*  @      A      B      C      D      E      F      G  */
     BAD,    10,    11,    12,    13,    14,    15,   BAD,
  /*  H      I      J      K      L      M      N      O  */
     BAD,   BAD,   BAD,   BAD,   BAD,   BAD,   BAD,   BAD,
  /*  P      Q      R      S      T      U      V      W  */
     BAD,   BAD,   BAD,   BAD,   BAD,   BAD,   BAD,   BAD,
  /*  X      Y      Z      [      \      ]      ^      _  */
     BAD,   BAD,   BAD,   BAD,   BAD,   BAD,   BAD,   BAD,
  /*  `      a      b      c      d      e      f      g  */
     BAD,    10,    11,    12,    13,    14,    15,   BAD,
  /*  h      i      j      k      l      m      n      o  */
     BAD,   BAD,   BAD,   BAD,   BAD,   BAD,   BAD,   BAD,
  /*  p      q      r      s      t      u      v      w  */
     BAD,   BAD,   BAD,   BAD,   BAD,   BAD,   BAD,   BAD,
  /*  x      y      z      {      |      }      ~     DEL */
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
     BAD,   BAD,   BAD,   BAD,   BAD,   BAD,   BAD,   BAD,
     BAD,   BAD,   BAD,   BAD,   BAD,   BAD,   BAD,   BAD
};

static BYTE DECODE_MAP[] = {
  /* NUL    SOH    STX    ETX    EOT    ENQ    ACK    BEL */
     BAD,   BAD,   BAD,   BAD,   BAD,   BAD,   BAD,   BAD,
  /*  BS     HT     LF     VT     FF     CR     SO     SI */
     BAD,   WSP,   SPE,   BAD,   BAD,   SPE,   BAD,   BAD,
  /* DLE    DC1    DC2    DC3    DC4    NAK    SYN    ETB */
     BAD,   BAD,   BAD,   BAD,   BAD,   BAD,   BAD,   BAD,
  /* CAN     EM    SUB    ESC     FS     GS     RS     US */
     BAD,   BAD,   BAD,   BAD,   BAD,   BAD,   BAD,   BAD,
  /*  SP     !      "      #      $      %      &      '  */
     WSP,   LIT,   LIT,   LIT,   LIT,   LIT,   LIT,   LIT,
  /*  (      )      *      +      ,      -      .      /  */
     LIT,   LIT,   LIT,   LIT,   LIT,   LIT,   LIT,   LIT,
  /*  0      1      2      3      4      5      6      7 */
     LIT,   LIT,   LIT,   LIT,   LIT,   LIT,   LIT,   LIT,
  /*  8      9      :      ;      <      =      >      ?  */
     LIT,   LIT,   LIT,   LIT,   LIT,   SPE,   LIT,   LIT,
  /*  @      A      B      C      D      E      F      G  */
     LIT,   LIT,   LIT,   LIT,   LIT,   LIT,   LIT,   LIT,
  /*  H      I      J      K      L      M      N      O  */
     LIT,   LIT,   LIT,   LIT,   LIT,   LIT,   LIT,   LIT,
  /*  P      Q      R      S      T      U      V      W  */
     LIT,   LIT,   LIT,   LIT,   LIT,   LIT,   LIT,   LIT,
  /*  X      Y      Z      [      \      ]      ^      _  */
     LIT,   LIT,   LIT,   LIT,   LIT,   LIT,   LIT,   LIT,
  /*  `      a      b      c      d      e      f      g  */
     LIT,   LIT,   LIT,   LIT,   LIT,   LIT,   LIT,   LIT,
  /*  h      i      j      k      l      m      n      o  */
     LIT,   LIT,   LIT,   LIT,   LIT,   LIT,   LIT,   LIT,
  /*  p      q      r      s      t      u      v      w  */
     LIT,   LIT,   LIT,   LIT,   LIT,   LIT,   LIT,   LIT,
  /*  x      y      z      {      |      }      ~     DEL */
     LIT,   LIT,   LIT,   LIT,   LIT,   LIT,   LIT,   BAD,
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

void QpDecoder_initialize(
    QpDecoder *self)
{
    /* options */
    self->mOutputCrLf = 0;
    /* state */
    self->mError = 0;
    self->mState = STATE_NORMAL;
    self->mTmpPos = 0;
    self->mTmpLen = 0;
    self->mTmpBuffer = (BYTE*) malloc(INTERNAL_BUFFER_SIZE*sizeof(BYTE));
    self->mTmpBuffer[0] = 0;
}

void QpDecoder_finalize(
    QpDecoder *self)
{
    free(self->mTmpBuffer);
    self->mTmpBuffer = NULL;
}

void QpDecoder_setOutputCrLf(
    QpDecoder *self,
    int b)
{
    self->mOutputCrLf = b;
}

int QpDecoder_getOutputCrLf(
    QpDecoder *self)
{
    return self->mOutputCrLf;
}

void QpDecoder_start(
    QpDecoder *self)
{
    self->mError = 0;
    self->mState = STATE_NORMAL;
    self->mTmpPos = 0;
    self->mTmpLen = 0;
    self->mTmpBuffer[0] = 0;
}

void QpDecoder_decode(
    QpDecoder *self,
    CharBuffer *in,
    ByteBuffer *out)
{
    int inPos, inEndPos, outPos, outEndPos;
    int ch, n, n1, n2, category, loNibble, hiNibble;
    CHAR *inBuffer;
    BYTE *outBuffer;

    inBuffer = in->chars;
    inPos = in->pos;
    inEndPos = in->endPos;
    outBuffer = (unsigned char*)out->bytes;
    outPos = out->pos;
    outEndPos = out->endPos;
    while (inPos < inEndPos && outPos < outEndPos) {
        /*
         * Normal input processing state
         */
        if (self->mState == STATE_NORMAL) {
            assert(self->mTmpPos == 0);
            assert(self->mTmpLen == 0);
            ch = inBuffer[inPos++] & 0xff;
            category = DECODE_MAP[ch];
            /* Normal printable chars are immediately sent to output */
            if (category == LITERAL) {
                outBuffer[outPos++] = (BYTE) ch;
            }
            /* SPACE and TAB must be buffered */
            else if (category == SPACE) {
                self->mState = STATE_BUFFER;
                self->mTmpPos = 0;
                self->mTmpLen = 0;
                self->mTmpBuffer[self->mTmpLen++] = (BYTE) ch;
            }
            /* '=' is escape char */
            else if (ch == EQUALS) {
                self->mState = STATE_ESCAPE;
                self->mTmpPos = 0;
                self->mTmpLen = 0;
                self->mTmpBuffer[self->mTmpLen++] = (BYTE) ch;
            }
            /* LF is line break */
            else if (ch == LF) {
                self->mState = STATE_FLUSH;
                self->mTmpPos = 0;
                self->mTmpLen = 0;
                if (self->mOutputCrLf) {
                    self->mTmpBuffer[self->mTmpLen++] = CR;
                }
                self->mTmpBuffer[self->mTmpLen++] = (BYTE) ch;
            }
            /* CR is ignored */
            else if (ch == CR) {
            }
            /* Any other character is an error */
            else {
                self->mError = 1;
                /* If character is >= 128 it's probably printable */
                if (ch >= 128) {
                    outBuffer[outPos++] = (BYTE) ch;
                }
                else {
                }
            }
        }
        /*
         * Buffering space characters
         */
        else if (self->mState == STATE_BUFFER) {
            ch = inBuffer[inPos] & 0xff;
            category = DECODE_MAP[ch];
            /* Printable character */
            if (category == LITERAL) {
                self->mState = STATE_FLUSH;
            }
            /* SPACE or TAB is appended to buffer */
            else if (category == SPACE) {
                ++inPos;
                if (self->mTmpLen < INTERNAL_BUFFER_SIZE) {
                    self->mTmpBuffer[self->mTmpLen++] = (BYTE) ch;
                }
                else {
                    /*
                     * The buffer is full.  This shouldn't happen.  It
                     * can happen if the input has lines longer than
                     * the standard allows (76 chars per line).  To
                     * deal with the problem, just ignore the input
                     * char, which means some space chars might be
                     * dropped during the decoding.
                     * <error>
                     */
                }
            }
            /* Spaces at end of line are discarded */
            else if (ch == LF) {
                self->mState = STATE_NORMAL;
                self->mTmpLen = 0;
            }
            /* CR is ignored */
            else if (ch == CR) {
                ++inPos;
            }
            /* Other character */
            else {
                self->mState = STATE_FLUSH;
            }
        }
        /*
         * Getting character following escape character ('=')
         */
        else if (self->mState == STATE_ESCAPE) {
            assert(self->mTmpPos == 0);
            assert(self->mTmpLen == 1);
            ch = inBuffer[inPos] & 0xff;
            /* Hex "digit" is high nibble of hex byte */
            hiNibble = HEX_MAP[ch];
            if (hiNibble != BAD) {
                self->mState = STATE_HEX;
                ++inPos;
                self->mTmpBuffer[self->mTmpLen++] = (BYTE) ch;
            }
            /* Soft line break */
            else if (ch == LF) {
                self->mState = STATE_NORMAL;
                ++inPos;
                self->mTmpLen = 0;
            }
            /* CR is ignored */
            else if (ch == CR) {
                ++inPos;
            }
            /* Possibly a soft line break */
            else if (ch == SP || ch == HT) {
                self->mState = STATE_SKIP;
                ++inPos;
                self->mTmpBuffer[self->mTmpLen++] = (BYTE) ch;
            }
            /* Any other character is an error */
            else {
                self->mState = STATE_FLUSH;
                self->mError = 1;
            }
        }
        /*
         * Getting second nibble of hex byte
         */
        else if (self->mState == STATE_HEX) {
            ch = inBuffer[inPos] & 0xff;
            /* Hex "digit" is low nibble of hex byte */
            loNibble = HEX_MAP[ch];
            if (loNibble != BAD) {
                self->mState = STATE_NORMAL;
                ++inPos;
                hiNibble = HEX_MAP[(int) self->mTmpBuffer[1]];
                n = (hiNibble << 4) + loNibble;
                outBuffer[outPos++] = (BYTE) n;
                self->mTmpLen = 0;
            }
            /* Any other character is an error */
            else {
                self->mState = STATE_FLUSH;
                self->mError = 1;
            }
        }
        /*
         * Buffering characters that follow escape character
         */
        else if (self->mState == STATE_SKIP) {
            ch = inBuffer[inPos] & 0xff;
            /* Append SP and TAB to buffer */
            if (ch == SP || ch == HT) {
                ++inPos;
                if (self->mTmpLen < INTERNAL_BUFFER_SIZE) {
                    self->mTmpBuffer[self->mTmpLen++] = (BYTE) ch;
                }
                else {
                    /* <error> */
                }
            }
            /* Discard buffer at end of line */
            else if (ch == LF) {
                self->mState = STATE_NORMAL;
                ++inPos;
                self->mTmpLen = 0;
            }
            /* CR is ignored */
            else if (ch == CR) {
                ++inPos;
            }
            /* Any other character is an error */
            else {
                self->mState = STATE_FLUSH;
                self->mError = 1;
            }
        }
        /*
         * Sending buffered characters to output
         */
        else if (self->mState == STATE_FLUSH) {
            n1 = self->mTmpLen - self->mTmpPos;
            n2 = outEndPos - outPos;
            if (n1 <= n2) {
                memcpy(&outBuffer[outPos], &self->mTmpBuffer[self->mTmpPos],
                    n1);
                outPos += n1;
                self->mState = STATE_NORMAL;
                self->mTmpLen = 0;
                self->mTmpPos = 0;
            }
            else /* if (n1 > n2) */ {
                memcpy(&outBuffer[outPos], &self->mTmpBuffer[self->mTmpPos],
                    n2);
                outPos += n2;
                self->mTmpPos += n2;
            }
        }
    }
    in->pos = inPos;
    out->pos = outPos;
}

void QpDecoder_finish(
    QpDecoder *self,
    ByteBuffer *out)
{
    int outPos, outEndPos, n1, n2;
    BYTE *outBuffer;

    outBuffer = (unsigned char*)out->bytes;
    outPos = out->pos;
    outEndPos = out->endPos;
    if (self->mState == STATE_FLUSH) {
        n1 = self->mTmpLen - self->mTmpPos;
        n2 = outEndPos - outPos;
        if (n1 <= n2) {
            memcpy(&outBuffer[outPos], &self->mTmpBuffer[self->mTmpPos], n1);
            outPos += n1;
            self->mState = STATE_NORMAL;
            self->mTmpLen = 0;
            self->mTmpPos = 0;
        }
        else /* if (n1 > n2) */ {
            memcpy(&outBuffer[outPos], &self->mTmpBuffer[self->mTmpPos], n2);
            outPos += n2;
            self->mTmpPos += n2;
        }
    }
    out->pos = outPos;
}
