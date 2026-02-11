#include	<stdio.h>
#include	<ctype.h>
#include	<string.h>
#include	<stdlib.h>

#if defined(DEBUG) && defined(DMALLOC)
#include <dmalloc.h>
#endif				/* DEBUG */

static const char rcsid[] = "$Id: decode2047.c,v 1.1 2005/05/09 13:53:41 markm Exp $";
static const char xdigit[] = "0123456789ABCDEF";

static char* rfc2047_search_quote(const char **ptr)
{
	const char     *p = *ptr;
	char           *s;

	while (**ptr && **ptr != '?')
		++* ptr;
	if ((s = (char*)malloc(*ptr - p + 1)) == 0)
		return (0);
	memcpy(s, p, *ptr - p);
	s[*ptr - p] = 0;
	return (s);
}

static int nyb(int c)
{
	const char     *p;

	c = toupper((int)(unsigned char)c);
	p = strchr(xdigit, c);
	return (p ? p - xdigit : 0);
}

static unsigned char decode64tab[256];
static int      decode64tab_init = 0;

static size_t decodebase64(char *ptr, size_t cnt)
{
	size_t          i, j;
	char            a, b, c;
	size_t          k;

	if (!decode64tab_init) {
		for (i = 0; i < 256; i++)
			decode64tab[i] = 0;
		for (i = 0; i < 64; i++)
			decode64tab[(int)
			("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[i])] = i;
		decode64tab[(int)'='] = 99;
	}

	i = cnt / 4;
	i = i * 4;
	k = 0;
	for (j = 0; j < i; j += 4) {
		int             w = decode64tab[(int)(unsigned char)ptr[j]];
		int             x = decode64tab[(int)(unsigned char)ptr[j + 1]];
		int             y = decode64tab[(int)(unsigned char)ptr[j + 2]];
		int             z = decode64tab[(int)(unsigned char)ptr[j + 3]];

		a = (w << 2) | (x >> 4);
		b = (x << 4) | (y >> 2);
		c = (y << 6) | z;
		ptr[k++] = a;
		if (ptr[j + 2] != '=')
			ptr[k++] = b;
		if (ptr[j + 3] != '=')
			ptr[k++] = c;
	}
	return (k);
}

/*
**	This is the main rfc2047 decoding function.  It receives rfc2047-encoded
**	text, and a callback function.  The callback function is repeatedly
**	called, each time receiving a piece of decoded text.  The decoded
**	info includes a text fragment - string, string length arg - followed
**	by the character set, followed by a context pointer that is received
**	from the caller.  If the callback function returns non-zero, rfc2047
**	decoding terminates, returning the result code.  Otherwise,
**	rfc2047_decode returns 0 after a successfull decoding (-1 if malloc
**	failed).
*/

int rfc2047_decode(const char *text, 
				   int(*func)(const char *, int, const char *, void *),
				   void *arg)
{
	int             rc;
	int             had_last_word = 0;
	const char     *p;
	char           *chset;
	char           *encoding;
	char           *enctext;

	while (text && *text) {
		if (text[0] != '=' || text[1] != '?') {
			p = text;
			while (*text) {
				if (text[0] == '=' && text[1] == '?')
					break;
				if (!isspace((int)(unsigned char)*text))
					had_last_word = 0;
				++text;
			}
			if (text > p && !had_last_word) {
				rc = (*func) (p, text - p, 0, arg);
				if (rc)
					return (rc);
			}
			continue;
		}

		text += 2;
		if ((chset = rfc2047_search_quote(&text)) == 0)
			return (-1);
		if (*text)
			++text;
		if ((encoding = rfc2047_search_quote(&text)) == 0) {
			free(chset);
			return (-1);
		}
		if (*text)
			++text;
		if ((enctext = rfc2047_search_quote(&text)) == 0) {
			free(encoding);
			free(chset);
			return (-1);
		}
		if (*text == '?' && text[1] == '=')
			text += 2;
		if (strcmp(encoding, "Q") == 0 || strcmp(encoding, "q") == 0) {
			char           *q, *r;

			for (q = r = enctext; *q;) {
				int             c;

				if (*q == '=' && q[1] && q[2]) {
					*r++ = (char)(
						nyb(q[1]) * 16 + nyb(q[2]));
					q += 3;
					continue;
				}

				c = *q++;
				if (c == '_')
					c = ' ';
				*r++ = c;
			}
			*r = 0;
		}
		else if (strcmp(encoding, "B") == 0 || strcmp(encoding, "b") == 0) {
			enctext[decodebase64(enctext, strlen(enctext))] = 0;
		}
		rc = (*func) (enctext, strlen(enctext), chset, arg);
		free(enctext);
		free(chset);
		free(encoding);
		if (rc)
			return (rc);

		had_last_word = 1;	/* Ignore blanks between enc words */
	}
	return (0);
}

/*
** rfc2047_decode_simple just strips out the rfc2047 decoding, throwing away
** the character set.  This is done by calling rfc2047_decode twice, once
** to count the number of characters in the decoded text, the second time to
** actually do it.
*/

struct simple_info {
	char           *string;
	int             index;
	const char *mychset;
};

static int count_simple(const char *txt, int len, const char *chset,
			 void *arg)
{
	struct simple_info *iarg = (struct simple_info *) arg;

	iarg->index += len;

	return (0);
}

static int save_simple(const char *txt, int len, const char *chset,
			void *arg)
{
	struct simple_info *iarg = (struct simple_info *) arg;

	memcpy(iarg->string + iarg->index, txt, len);
	iarg->index += len;
	return (0);
}

char* rfc2047_decode_simple(const char *text)
{
	struct simple_info info;

	info.index = 1;
	if (rfc2047_decode(text, &count_simple, &info))
		return (0);

	if ((info.string = (char*)malloc(info.index)) == 0)
		return (0);
	info.index = 0;
	if (rfc2047_decode(text, &save_simple, &info)) {
		free(info.string);
		return (0);
	}
	info.string[info.index] = 0;
	return (info.string);
}
