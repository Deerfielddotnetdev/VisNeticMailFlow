/*------------------------------------------------------------------------*\
 * QpEncoder.c
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

#include "QpEncoder.h"
#include "CharBuffer.h"
#include "ByteBuffer.h"
#include <stdlib.h>
#include <string.h>

typedef char CHAR;
typedef unsigned char BYTE;

/* States */
#define STATE_FILL_LINE_BUFFER   1
#define STATE_WRITE_LINE_CHARS   2
#define STATE_WRITE_EQUALS       3
#define STATE_WRITE_CR           4
#define STATE_WRITE_LF           5

/* Line break types */
#define NO_LINE_BREAK     0
#define SOFT_LINE_BREAK   1
#define HARD_LINE_BREAK   2

#define HT       9
#define LF      10
#define CR      13
#define SPACE   32
#define PERIOD  46
#define EQUALS  61

static BYTE *HEX_CHARS = (BYTE *) "0123456789ABCDEF";

#define SAFE     1
#define UNSAFE   2
#define SPECIAL  3

/*
 * The following encode map encodes all of the characters listed in
 * RFC 2045 as risky.  The TAB character (9) is encoded.
 */
BYTE ENCODE_MAP_LOW_RISK[] = {
  /* NUL    SOH    STX    ETX    EOT    ENQ    ACK    BEL */
      2,     2,     2,     2,     2,     2,     2,     2,
  /*  BS     HT     LF     VT     FF     CR     SO     SI */
      2,     2,     3,     2,     2,     3,     2,     2,
  /* DLE    DC1    DC2    DC3    DC4    NAK    SYN    ETB */
      2,     2,     2,     2,     2,     2,     2,     2,
  /* CAN     EM    SUB    ESC     FS     GS     RS     US */
      2,     2,     2,     2,     2,     2,     2,     2,
  /*  SP     !      "      #      $      %      &      ' */
      1,     2,     2,     2,     2,     1,     1,     1,
  /*  (      )      *      +      ,      -      .      / */
      1,     1,     1,     1,     1,     1,     1,     1,
  /*  0      1      2      3      4      5      6      7 */
      1,     1,     1,     1,     1,     1,     1,     1,
  /*  8      9      :      ;      <      =      >      ? */
      1,     1,     1,     1,     1,     2,     1,     1,
  /*  @      A      B      C      D      E      F      G */
      2,     1,     1,     1,     1,     1,     1,     1,
  /*  H      I      J      K      L      M      N      O */
      1,     1,     1,     1,     1,     1,     1,     1,
  /*  P      Q      R      S      T      U      V      W */
      1,     1,     1,     1,     1,     1,     1,     1,
  /*  X      Y      Z      [      \      ]      ^      _ */
      1,     1,     1,     2,     2,     2,     2,     1,
  /*  `      a      b      c      d      e      f      g */
      2,     1,     1,     1,     1,     1,     1,     1,
  /*  h      i      j      k      l      m      n      o */
      1,     1,     1,     1,     1,     1,     1,     1,
  /*  p      q      r      s      t      u      v      w  */
      1,     1,     1,     1,     1,     1,     1,     1,
  /*  x      y      z      {      |      }      ~     DEL */
      1,     1,     1,     2,     2,     2,     2,     2,
      2,     2,     2,     2,     2,     2,     2,     2,
      2,     2,     2,     2,     2,     2,     2,     2,
      2,     2,     2,     2,     2,     2,     2,     2,
      2,     2,     2,     2,     2,     2,     2,     2,
      2,     2,     2,     2,     2,     2,     2,     2,
      2,     2,     2,     2,     2,     2,     2,     2,
      2,     2,     2,     2,     2,     2,     2,     2,
      2,     2,     2,     2,     2,     2,     2,     2,
      2,     2,     2,     2,     2,     2,     2,     2,
      2,     2,     2,     2,     2,     2,     2,     2,
      2,     2,     2,     2,     2,     2,     2,     2,
      2,     2,     2,     2,     2,     2,     2,     2,
      2,     2,     2,     2,     2,     2,     2,     2,
      2,     2,     2,     2,     2,     2,     2,     2,
      2,     2,     2,     2,     2,     2,     2,     2,
      2,     2,     2,     2,     2,     2,     2,     2
};


/*
 * The following encode map does not encode any of the characters listed in
 * RFC 2045 as risky.  The TAB character (9) is not encoded.
 */

BYTE ENCODE_MAP_HIGH_RISK[] = {
  /* NUL    SOH    STX    ETX    EOT    ENQ    ACK    BEL */
      2,     2,     2,     2,     2,     2,     2,     2,
  /*  BS     HT     LF     VT     FF     CR     SO     SI */
      2,     1,     3,     2,     2,     3,     2,     2,
  /* DLE    DC1    DC2    DC3    DC4    NAK    SYN    ETB */
      2,     2,     2,     2,     2,     2,     2,     2,
  /* CAN     EM    SUB    ESC     FS     GS     RS     US */
      2,     2,     2,     2,     2,     2,     2,     2,
  /*  SP     !      "      #      $      %      &      '  */
      1,     1,     1,     1,     1,     1,     1,     1,
  /*  (      )      *      +      ,      -      .      /  */
      1,     1,     1,     1,     1,     1,     1,     1,
  /*  0      1      2      3      4      5      6      7 */
      1,     1,     1,     1,     1,     1,     1,     1,
  /*  8      9      :      ;      <      =      >      ?  */
      1,     1,     1,     1,     1,     2,     1,     1,
  /*  @      A      B      C      D      E      F      G  */
      1,     1,     1,     1,     1,     1,     1,     1,
  /*  H      I      J      K      L      M      N      O  */
      1,     1,     1,     1,     1,     1,     1,     1,
  /*  P      Q      R      S      T      U      V      W  */
      1,     1,     1,     1,     1,     1,     1,     1,
  /*  X      Y      Z      [      \      ]      ^      _  */
      1,     1,     1,     1,     1,     1,     1,     1,
  /*  `      a      b      c      d      e      f      g  */
      1,     1,     1,     1,     1,     1,     1,     1,
  /*  h      i      j      k      l      m      n      o  */
      1,     1,     1,     1,     1,     1,     1,     1,
  /*  p      q      r      s      t      u      v      w  */
      1,     1,     1,     1,     1,     1,     1,     1,
  /*  x      y      z      {      |      }      ~     DEL */
      1,     1,     1,     1,     1,     1,     1,     2,
      2,     2,     2,     2,     2,     2,     2,     2,
      2,     2,     2,     2,     2,     2,     2,     2,
      2,     2,     2,     2,     2,     2,     2,     2,
      2,     2,     2,     2,     2,     2,     2,     2,
      2,     2,     2,     2,     2,     2,     2,     2,
      2,     2,     2,     2,     2,     2,     2,     2,
      2,     2,     2,     2,     2,     2,     2,     2,
      2,     2,     2,     2,     2,     2,     2,     2,
      2,     2,     2,     2,     2,     2,     2,     2,
      2,     2,     2,     2,     2,     2,     2,     2,
      2,     2,     2,     2,     2,     2,     2,     2,
      2,     2,     2,     2,     2,     2,     2,     2,
      2,     2,     2,     2,     2,     2,     2,     2,
      2,     2,     2,     2,     2,     2,     2,     2,
      2,     2,     2,     2,     2,     2,     2,     2,
      2,     2,     2,     2,     2,     2,     2,     2
};

unsigned char *QpEncoder_LOW_RISK_MAP = ENCODE_MAP_LOW_RISK;
unsigned char *QpEncoder_HIGH_RISK_MAP = ENCODE_MAP_HIGH_RISK;

static void QpEncoder_fillLineBuffer(QpEncoder *encoder);
static void QpEncoder_prepareLine(QpEncoder *encoder);
static void QpEncoder_writeLineChars(QpEncoder *encoder);

void QpEncoder_initialize(
    QpEncoder *self)
{
    /* options */
    self->mMaxLineLen = 76;
    self->mOutputCrLf = 1;
    self->mSuppressFinalNewline = 0;
    self->mProtectFrom = 1;
    self->mProtectDot = 1;
    self->mEncodeMap = ENCODE_MAP_LOW_RISK;
    /* state */
    self->mState = STATE_FILL_LINE_BUFFER;
    self->mLinePos = 0;
    self->mLineLen = 0;
    self->mLineBreakType = NO_LINE_BREAK;
    self->mLineBufferLen = 0;
    self->mLineBuffer = (CHAR *) malloc(100*sizeof(CHAR));
}

void QpEncoder_finalize(
    QpEncoder *self)
{
    free(self->mLineBuffer);
    self->mLineBuffer = NULL;
}

void QpEncoder_setMaxLineLen(
    QpEncoder *self,
    int len)
{
    if (len > 76) {
        len = 76;
    }
    if (len < 10) {
        len = 10;
    }
    self->mMaxLineLen = len;
}

int QpEncoder_getMaxLineLen(
    QpEncoder *self)
{
    return self->mMaxLineLen;
}

void QpEncoder_setOutputCrLf(
    QpEncoder *self,
    int b)
{
    self->mOutputCrLf = b;
}

int QpEncoder_getOutputCrLf(
    QpEncoder *self)
{
    return self->mOutputCrLf;
}

void QpEncoder_setSuppressFinalNewline(
    QpEncoder *self,
    int b)
{
    self->mSuppressFinalNewline = b;
}

int QpEncoder_getSuppressFinalNewline(
    QpEncoder *self)
{
    return self->mSuppressFinalNewline;
}

void QpEncoder_setProtectFrom(
    QpEncoder *self,
    int b)
{
    self->mProtectFrom = b;
}

int QpEncoder_getProtectFrom(
    QpEncoder *self)
{
    return self->mProtectFrom;
}

void QpEncoder_setProtectDot(
    QpEncoder *self,
    int b)
{
    self->mProtectDot = b;
}

int QpEncoder_getProtectDot(
    QpEncoder *self)
{
    return self->mProtectDot;
}

void QpEncoder_setEncodeMap(
    QpEncoder *self,
    unsigned char *emap)
{
    self->mEncodeMap = emap;
}

void QpEncoder_start(
    QpEncoder *self)
{
    /*
     * Initialize dynamic state
     */
    self->mState = STATE_FILL_LINE_BUFFER;
    self->mLinePos = 0;
    self->mLineLen = 0;
    self->mLineBreakType = NO_LINE_BREAK;
    self->mLineBufferLen = 0;
}

void QpEncoder_encode(
    QpEncoder *self,
    ByteBuffer *in,
    CharBuffer *out)
{
    self->mInBuffer = (unsigned char*)in->bytes;
    self->mInPos = in->pos;
    self->mInLen = in->endPos;
    self->mOutBuffer = out->chars;
    self->mOutPos = out->pos;
    self->mOutLen = out->endPos;
    /*
     * Continue until input buffer is empty or output buffer is full
     */
    while (1) {
        if (self->mState == STATE_FILL_LINE_BUFFER) {
            if (self->mInPos >= self->mInLen) {
                break;
            }
            QpEncoder_fillLineBuffer(self);
        }
        if (self->mState == STATE_WRITE_LINE_CHARS) {
            if (self->mOutPos >= self->mOutLen) {
                break;
            }
            QpEncoder_writeLineChars(self);
        }
        if (self->mState == STATE_WRITE_EQUALS) {
            if (self->mOutPos >= self->mOutLen) {
                break;
            }
            self->mOutBuffer[self->mOutPos++] = EQUALS;
            if (self->mOutputCrLf) {
                self->mState = STATE_WRITE_CR;
            }
            else /* if (! self->mOutputCrLf) */ {
                self->mState = STATE_WRITE_LF;
            }
        }
        if (self->mState == STATE_WRITE_CR) {
            if (self->mOutPos >= self->mOutLen) {
                break;
            }
            self->mOutBuffer[self->mOutPos++] = CR;
            self->mState = STATE_WRITE_LF;
        }
        if (self->mState == STATE_WRITE_LF) {
            if (self->mOutPos >= self->mOutLen) {
                break;
            }
            self->mOutBuffer[self->mOutPos++] = LF;
            self->mState = STATE_FILL_LINE_BUFFER;
            self->mLineBreakType = NO_LINE_BREAK;
        }
    }
    in->pos = self->mInPos;
    out->pos = self->mOutPos;
}

static void QpEncoder_fillLineBuffer(
    QpEncoder *self)
{
#define LINE_BUFFER_FULL  1
#define END_OF_LINE       2
#define NO_MORE_INPUT     3

    int breakCondition, by, category;

    /*
     * Encode chars until we exhaust input buffer, get a hard line break,
     * or exceed maximum line length
     */
    breakCondition = 0;
    while (1) {
        if (self->mInPos >= self->mInLen) {
            breakCondition = NO_MORE_INPUT;
            break;
        }
        by = self->mInBuffer[self->mInPos++] & 0xff;
        category = self->mEncodeMap[by];
        if (category == SAFE) {
            self->mLineBuffer[self->mLineBufferLen++] = (BYTE) by;
        }
        else if (category == UNSAFE) {
            self->mLineBuffer[self->mLineBufferLen++] = EQUALS;
            self->mLineBuffer[self->mLineBufferLen++] = HEX_CHARS[by >> 4];
            self->mLineBuffer[self->mLineBufferLen++] = HEX_CHARS[by & 0x0f];
        }
        else if (category == SPECIAL) {
            if (by == LF) {
                breakCondition = END_OF_LINE;
                break;
            }
            else if (by == CR) {
                /* Ignore CR (assume it's part of a CR LF sequence) */
            }
            else {
                /* <error> */
            }
        }
        if (self->mLineBufferLen > self->mMaxLineLen) {
            breakCondition = LINE_BUFFER_FULL;
            break;
        }
    }

    /* Output line with hard line break */

    if (breakCondition == END_OF_LINE) {
        /* If last char on line was ' ' or '\t', then encode it */
        if (self->mLineBufferLen > 0) {
            by = self->mLineBuffer[self->mLineBufferLen-1] & 0xff;
            if (by == SPACE || by == HT) {
                --self->mLineBufferLen;
                self->mLineBuffer[self->mLineBufferLen++] = EQUALS;
                self->mLineBuffer[self->mLineBufferLen++] = HEX_CHARS[by >> 4];
                self->mLineBuffer[self->mLineBufferLen++] = HEX_CHARS[by & 0x0f];
            }
        }
        self->mLineBreakType = HARD_LINE_BREAK;
        QpEncoder_prepareLine(self);
        if (self->mLineBreakType == SOFT_LINE_BREAK) {
            /* Line was wrapped, put back LF */
            --self->mInPos;
        }
        self->mState = STATE_WRITE_LINE_CHARS;
    }

    /* Output line with soft line break */

    else if (breakCondition == LINE_BUFFER_FULL) {
        self->mLineBreakType = SOFT_LINE_BREAK;
        QpEncoder_prepareLine(self);
        self->mState = STATE_WRITE_LINE_CHARS;
    }

    /* If the input was exhausted, then return without changing the state */

    else /* if (breakCondition == NO_MORE_INPUT) */ {
    }
}

static void QpEncoder_prepareLine(
    QpEncoder *self)
{
    int pos;

    /*
     * If "From " is at beginning of line, encode it as "From=20" (option)
     */
    if (self->mProtectFrom &&
        self->mLineBufferLen > 4 &&
        self->mLineBuffer[0] == 70 &&  /* 'F' */
        self->mLineBuffer[1] == 114 && /* 'r' */
        self->mLineBuffer[2] == 111 && /* 'o' */
        self->mLineBuffer[3] == 109 && /* 'm' */
        self->mLineBuffer[4] == 32) {  /* ' ' */

        memmove(&self->mLineBuffer[7], &self->mLineBuffer[5],
            (self->mLineBufferLen - 5)*sizeof(CHAR));
        self->mLineBuffer[4] = 61; /* '=' */
        self->mLineBuffer[5] = 50; /* '2' */
        self->mLineBuffer[6] = 48; /* '0' */
        self->mLineBufferLen += 2;
    }

    /*
     * Encode '.' at beginning of line (option)
     */
    if (self->mProtectDot &&
        self->mLineBufferLen > 0 &&
        self->mLineBuffer[0] == PERIOD) {

        memmove(&self->mLineBuffer[3], &self->mLineBuffer[1],
            (self->mLineBufferLen - 1)*sizeof(CHAR));
        self->mLineBuffer[0] = 61; /* '=' */
        self->mLineBuffer[1] = 50; /* '2' */
        self->mLineBuffer[2] = 69; /* 'E' */
        self->mLineBufferLen += 2;
    }
    /*
     * Insert soft line break, if necessary
     */
    if (self->mLineBufferLen > self->mMaxLineLen) {
        self->mLineBreakType = SOFT_LINE_BREAK;
        /*
         * Wrap at word boundary, if possible.
         * Start looking for a space at maxLineLen-2 because we want
         * the line to end with " =".
         */
        pos = self->mMaxLineLen - 2;
        while (pos >= 0 && self->mLineBuffer[pos] != SPACE) {
            --pos;
        }
        ++pos;
        if (pos == 0) {
            /* No word boundary found */
            pos = self->mMaxLineLen - 1;
            if (self->mLineBuffer[pos-2] == EQUALS) {
                pos -= 2;
            }
            else if (self->mLineBuffer[pos-1] == EQUALS) {
                pos -= 1;
            }
        }
        self->mLineLen = pos;
    }
    else /* if (self->mLineBufferLen <= self->mMaxLineLen) */ {
        self->mLineLen = self->mLineBufferLen;
    }
    self->mLinePos = 0;
}

static void QpEncoder_writeLineChars(
    QpEncoder *self)
{
    int n1, n2, n;

    /*
     * Copy chars from line buffer to output buffer
     */
    n1 = self->mLineLen - self->mLinePos;
    n2 = self->mOutLen - self->mOutPos;
    if (n1 <= n2) {
        /* Output entire line ... */
        memcpy(&self->mOutBuffer[self->mOutPos],
            &self->mLineBuffer[self->mLinePos], n1*sizeof(CHAR));
        self->mOutPos += n1;
        /* ... and begin next line */
        n = self->mLineBufferLen - self->mLineLen;
        memmove(&self->mLineBuffer[0], &self->mLineBuffer[self->mLineLen],
            n*sizeof(CHAR));
        self->mLineBufferLen = n;
        self->mLineLen = 0;
        self->mLinePos = 0;
        if (self->mLineBreakType == SOFT_LINE_BREAK) {
            self->mState = STATE_WRITE_EQUALS;
        }
        else if (self->mLineBreakType == HARD_LINE_BREAK) {
            if (self->mOutputCrLf) {
                self->mState = STATE_WRITE_CR;
            }
            else /* if (! self->mOutputCrLf) */ {
                self->mState = STATE_WRITE_LF;
            }
        }
        else /* if (self->mLineBreakType == NO_LINE_BREAK) */ {
            self->mState = STATE_FILL_LINE_BUFFER;
        }
    }
    else /* if (n1 > n2) */ {
        /* Output part of line -- as much as we can get into output buffer */
        memcpy(&self->mOutBuffer[self->mOutPos],
            &self->mLineBuffer[self->mLinePos], n2*sizeof(CHAR));
        self->mOutPos += n2;
        self->mLinePos += n2;
    }
}

void QpEncoder_finish(
    QpEncoder *self,
    CharBuffer *out)
{
    self->mOutBuffer = out->chars;
    self->mOutPos = out->pos;
    self->mOutLen = out->endPos;

    /*
     * Continue until line buffer is empty or output buffer is full
     */
    while (1) {
        if (self->mState == STATE_FILL_LINE_BUFFER) {
            /*
             * Note: We should only get here if the input does not end
             * with a newline
             */
            if (self->mLineBufferLen == 0) {
                /* No more characters remaining */
                break;
            }
            else if (! self->mSuppressFinalNewline) {
                /*
                 * At this point, we have reached the end of the input,
                 * which has no final newline.  We want to force a soft
                 * line break at the end, so that the output will end
                 * with a newline.
                 */

                /* Append extra char to force a possible line break */
                self->mLineBuffer[self->mLineBufferLen++] = 65;
                QpEncoder_prepareLine(self);
                /* Remove extra char */
                --self->mLineBufferLen;
                /*
                 * If we haven't forced a line break, then this is the
                 * last output line.  We set the state to Soft Line
                 * Break, so that the output will end with "=\r\n".
                 */
                if (self->mLineBreakType == NO_LINE_BREAK) {
                    self->mLineLen = self->mLineBufferLen;
                    self->mLineBreakType = SOFT_LINE_BREAK;
                }
            }
            else /* if (self->mSuppressFinalNewline) */ {
                /* If last char on line is ' ' or '\t', encode it */
                if (self->mLineBufferLen > 0) {
                    int by = self->mLineBuffer[self->mLineBufferLen-1] & 0xff;
                    if (by == SPACE || by == HT) {
                        --self->mLineBufferLen;
                        self->mLineBuffer[self->mLineBufferLen++] = EQUALS;
                        self->mLineBuffer[self->mLineBufferLen++] = HEX_CHARS[by>>4];
                        self->mLineBuffer[self->mLineBufferLen++] = HEX_CHARS[by&0x0f];
                    }
                }
                QpEncoder_prepareLine(self);
            }
            self->mState = STATE_WRITE_LINE_CHARS;
        }
        if (self->mState == STATE_WRITE_LINE_CHARS) {
            if (self->mOutPos >= self->mOutLen) {
                break;
            }
            QpEncoder_writeLineChars(self);
        }
        if (self->mState == STATE_WRITE_EQUALS) {
            if (self->mOutPos >= self->mOutLen) {
                break;
            }
            self->mOutBuffer[self->mOutPos++] = EQUALS;
            if (self->mOutputCrLf) {
                self->mState = STATE_WRITE_CR;
            }
            else /* if (! self->mOutputCrLf) */ {
                self->mState = STATE_WRITE_LF;
            }
        }
        if (self->mState == STATE_WRITE_CR) {
            if (self->mOutPos >= self->mOutLen) {
                break;
            }
            self->mOutBuffer[self->mOutPos++] = CR;
            self->mState = STATE_WRITE_LF;
        }
        if (self->mState == STATE_WRITE_LF) {
            if (self->mOutPos >= self->mOutLen) {
                break;
            }
            self->mOutBuffer[self->mOutPos++] = LF;
            self->mState = STATE_FILL_LINE_BUFFER;
            self->mLineBreakType = NO_LINE_BREAK;
        }
    }
    out->pos = self->mOutPos;
}
