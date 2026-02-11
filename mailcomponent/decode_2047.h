#if !defined(rfc2047_h)
static const char rfc2047_h_rcsid[] = "$Id: decode_2047.h,v 1.1 2005/05/09 13:53:41 markm Exp $";

extern int
rfc2047_decode(const char *text,
			   int (*func) (const char *, int, const char *, void *),
			   void *arg);

extern char    *
rfc2047_decode_simple(const char *text);

#endif				
/* rfc2047_h */