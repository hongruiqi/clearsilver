/*
 * Neotonic ClearSilver Templating System
 *
 * This code is made available under the terms of the 
 * Neotonic ClearSilver License.
 * http://www.neotonic.com/clearsilver/license.hdf
 *
 * Copyright (C) 2001 by Brandon Long
 */

#include "cs_config.h"

#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <regex.h>
#include "neo_misc.h"
#include "neo_err.h"
#include "neo_str.h"
#include "ulist.h"

#ifndef va_copy
#ifdef __va_copy
# define va_copy(dest,src) __va_copy(dest,src)
#else
# define va_copy(dest,src) ((dest) = (src))
#endif
#endif

char *neos_strip (char *s)
{
  int x;

  x = strlen(s) - 1;
  while (x>=0 && isspace(s[x])) s[x--] = '\0';

  while (*s && isspace(*s)) s++;
  
  return s;
}

char *neos_rstrip (char *s)
{
  int n = strlen (s)-1;

  while (n >= 0 && isspace(s[n]))
  {
    s[n] = '\0';
    n--;
  }

  return s;
}

void neos_lower(char *s)
{
  while(*s != 0) {
    *s = tolower(*s);
    s++;
  }
}


void string_init (STRING *str)
{
  str->buf = NULL;
  str->len = 0;
  str->max = 0;
}

void string_clear (STRING *str)
{
  if (str->buf != NULL)
    free(str->buf);
  string_init(str);
}

static NEOERR* string_check_length (STRING *str, int l)
{
  if (str->buf == NULL)
  {
    if (l * 10 > 256)
      str->max = l * 10;
    else
      str->max = 256;
    str->buf = (char *) malloc (sizeof(char) * str->max);
    if (str->buf == NULL)
      return nerr_raise (NERR_NOMEM, "Unable to allocate render buf of size %d",
	  str->max);
   /*  ne_warn("Creating string %x at %d (%5.2fK)", str, str->max, (str->max / 1024.0)); */
  }
  else if (str->len + l >= str->max)
  {
    do
    {
      str->max *= 2;
    } while (str->len + l >= str->max);
    str->buf = (char *) realloc (str->buf, sizeof(char) * str->max);
    if (str->buf == NULL)
      return nerr_raise (NERR_NOMEM, "Unable to allocate STRING buf of size %d",
	  str->max);
    /* ne_warn("Growing string %x to %d (%5.2fK)", str, str->max, (str->max / 1024.0)); */
  }
  return STATUS_OK;
}

NEOERR *string_set (STRING *str, char *buf)
{
  str->len = 0;
  return nerr_pass (string_append (str, buf));
}

NEOERR *string_append (STRING *str, char *buf)
{
  NEOERR *err;
  int l;

  l = strlen(buf);
  err = string_check_length (str, l);
  if (err != STATUS_OK) return nerr_pass (err);
  strcpy(str->buf + str->len, buf);
  str->len += l;

  return STATUS_OK;
}

NEOERR *string_appendn (STRING *str, char *buf, int l)
{
  NEOERR *err;

  err = string_check_length (str, l+1);
  if (err != STATUS_OK) return nerr_pass (err);
  memcpy(str->buf + str->len, buf, l);
  str->len += l;
  str->buf[str->len] = '\0';

  return STATUS_OK;
}

/* currently, this requires a C99 compliant snprintf */
NEOERR *string_appendvf (STRING *str, char *fmt, va_list ap) 
{
  NEOERR *err;
  char buf[4096];
  int bl, size;
  va_list tmp;

  va_copy(tmp, ap);
  /* determine length */
  size = sizeof (buf);
  bl = vsnprintf (buf, size, fmt, ap);
  if (bl > -1 && bl < size)
    return string_appendn (str, buf, bl);

  err = string_check_length (str, bl+1);
  if (err != STATUS_OK) return nerr_pass (err);
  va_copy(ap, tmp);
  vsprintf (str->buf + str->len, fmt, ap);
  str->len += bl;
  str->buf[str->len] = '\0';

  return STATUS_OK;
}

NEOERR *string_appendf (STRING *str, char *fmt, ...)
{
  NEOERR *err;
  va_list ap;

  va_start (ap, fmt);
  err = string_appendvf (str, fmt, ap);
  va_end (ap);
  return nerr_pass(err);
}

NEOERR *string_append_char (STRING *str, char c)
{
  NEOERR *err;
  err = string_check_length (str, 1);
  if (err != STATUS_OK) return nerr_pass (err);
  str->buf[str->len] = c;
  str->buf[str->len + 1] = '\0';
  str->len += 1;

  return STATUS_OK;
}

void string_array_init (STRING_ARRAY *arr)
{
  arr->entries = NULL;
  arr->count = 0;
  arr->max = 0;
}

NEOERR *string_array_split (ULIST **list, char *s, char *sep, int max)
{
  NEOERR *err;
  char *p, *n, *f;
  int sl;
  int x = 0;

  if (sep[0] == '\0') 
    return nerr_raise (NERR_ASSERT, "separator must be at least one character");

  err = uListInit (list, 10, 0);
  if (err) return nerr_pass(err);

  sl = strlen(sep);
  p = (sl == 1) ? strchr (s, sep[0]) : strstr (s, sep);
  f = s;
  while (p != NULL)
  {
    if (x >= max) break;
    *p = '\0';
    n = strdup(f);
    *p = sep[0];
    if (n) err = uListAppend (*list, n);
    else err = nerr_raise(NERR_NOMEM, 
	"Unable to allocate memory to split %s", s);
    if (err) goto split_err;
    f = p+sl;
    p = (sl == 1) ? strchr (f, sep[0]) : strstr (f, sep);
    x++;
  }
  /* Handle remainder */
  n = strdup(f);
  if (n) err = uListAppend (*list, n);
  else err = nerr_raise(NERR_NOMEM, 
      "Unable to allocate memory to split %s", s);
  if (err) goto split_err;
  return STATUS_OK;

split_err:
  uListDestroy(list, ULIST_FREE);
  return err;
}

void string_array_clear (STRING_ARRAY *arr)
{
  int x;

  for (x = 0; x < arr->count; x++)
  {
    if (arr->entries[x] != NULL) free (arr->entries[x]);
    arr->entries[x] = NULL;
  }
  free (arr->entries);
  arr->entries = NULL;
  arr->count = 0;
}

/* This works better with a C99 compliant vsnprintf, but should work ok
 * with versions that return a -1 if it overflows the buffer */
char *vsprintf_alloc (char *fmt, va_list ap)
{
  char buf[4096];
  char *b = NULL;
  int bl, size;
  va_list tmp;

/* PPC doesn't like you re-using a va_list... and it might not be
 * supposed to work at all */
  va_copy(tmp, ap);
  
  size = sizeof (buf);
  bl = vsnprintf (buf, sizeof (buf), fmt, ap);
  if (bl > -1 && bl < size)
    return strdup (buf);

  if (bl > -1)
    size = bl + 1;
  else
    size *= 2;

  b = (char *) malloc (size * sizeof(char));
  if (b == NULL) return NULL;
  while (1)
  {
    va_copy(ap, tmp);
    bl = vsnprintf (b, size, fmt, ap);
    if (bl > -1 && bl < size)
      return b;
    size *= 2;
    b = (char *) realloc (b, size * sizeof(char));
    if (b == NULL) return NULL;
  }
}

char *sprintf_alloc (char *fmt, ...)
{
  va_list ap;
  char *r;

  va_start (ap, fmt);
  r = vsprintf_alloc (fmt, ap);
  va_end (ap);
  return r;
}

BOOL reg_search (char *re, char *str)
{
  regex_t search_re;
  int errcode;
  char buf[256];

  if ((errcode = regcomp (&search_re, re, REG_ICASE | REG_EXTENDED | REG_NOSUB)))
  {
    regerror (errcode, &search_re, buf, sizeof(buf));
    ne_warn ("Unable to compile regex %s: %s", re, buf);
    return FALSE;
  }
  errcode = regexec (&search_re, str, 0, NULL, 0);
  regfree (&search_re);
  if (errcode == 0)
    return TRUE;
  return FALSE;
}

NEOERR *string_readline (STRING *str, FILE *fp)
{
  NEOERR *err;

  /* minimum size for a readline is 256 above current position */
  err = string_check_length (str, str->len + 256);
  if (err != STATUS_OK) return nerr_pass (err);

  while (fgets(str->buf + str->len, str->max - str->len, fp) != NULL)
  {
    str->len = strlen(str->buf);
    if (str->buf[str->len-1] == '\n') break;
    err = string_check_length (str, str->len + 256);
    if (err != STATUS_OK) return nerr_pass (err);
  }
  return STATUS_OK;
}

NEOERR* neos_escape(UINT8 *buf, int buflen, char esc_char, char *escape, char **esc)
{
  int nl = 0;
  int l = 0;
  int x = 0;
  char *s;
  int match = 0;

  while (l < buflen)
  {
    if (buf[l] == esc_char)
    {
      nl += 2;
    } 
    else
    {
      x = 0;
      while (escape[x])
      {
	if (escape[x] == buf[l])
	{
	  nl +=2;
	  break;
	}
	x++;
      }
    }
    nl++;
    l++;
  }

  s = (char *) malloc (sizeof(char) * (nl + 1));
  if (s == NULL) 
    return nerr_raise (NERR_NOMEM, "Unable to allocate memory to escape %s", 
	buf);

  nl = 0; l = 0;
  while (l < buflen)
  {
    match = 0;
    if (buf[l] == esc_char)
    {
      match = 1;
    }
    else
    {
      x = 0;
      while (escape[x])
      {
	if (escape[x] == buf[l])
	{
	  match = 1;
	  break;
	}
	x++;
      }
    }
    if (match)
    {
      s[nl++] = esc_char;
      s[nl++] = "0123456789ABCDEF"[buf[l] / 16];
      s[nl++] = "0123456789ABCDEF"[buf[l] % 16];
      l++;
    }
    else
    {
      s[nl++] = buf[l++];
    }
  }
  s[nl] = '\0';

  *esc = s;
  return STATUS_OK;
}

UINT8 *neos_unescape (UINT8 *s, int buflen, char esc_char)
{
  int i = 0, o = 0;

  if (s == NULL) return s;
  while (i < buflen)
  {
    if (s[i] == esc_char && (i+2 < buflen) && 
	isxdigit(s[i+1]) && isxdigit(s[i+2]))
    {
      UINT8 num;
      num = (s[i+1] >= 'A') ? ((s[i+1] & 0xdf) - 'A') + 10 : (s[i+1] - '0');
      num *= 16;
      num += (s[i+2] >= 'A') ? ((s[i+2] & 0xdf) - 'A') + 10 : (s[i+2] - '0');
      s[o++] = num;
      i+=3;
    }
    else {
      s[o++] = s[i++];
    }
  }
  if (i && o) s[o] = '\0';
  return s;
}

char *repr_string_alloc (char *s)
{
  int l,x,i;
  int nl = 0;
  char *rs;

  if (s == NULL)
  {
    return strdup("NULL");
  }

  l = strlen(s);
  for (x = 0; x < l; x++)
  {
    if (isprint(s[x]) && s[x] != '"' && s[x] != '\\')
    {
      nl++;
    }
    else
    {
      if (s[x] == '\n' || s[x] == '\t' || s[x] == '\r' || s[x] == '"' ||
	  s[x] == '\\')
      {
	nl += 2;
      }
      else nl += 4;
    }
  }

  rs = (char *) malloc ((nl+3) * sizeof(char));
  if (rs == NULL)
    return NULL;

  i = 0;
  rs[i++] = '"';
  for (x = 0; x < l; x++)
  {
    if (isprint(s[x]) && s[x] != '"' && s[x] != '\\')
    {
      rs[i++] = s[x];
    }
    else
    {
      rs[i++] = '\\';
      switch (s[x])
      {
	case '\n':
	  rs[i++] = 'n';
	  break;
	case '\t':
	  rs[i++] = 't';
	  break;
	case '\r':
	  rs[i++] = 'r';
	  break;
	case '"':
	  rs[i++] = '"';
	  break;
	case '\\':
	  rs[i++] = '\\';
	  break;
	default:
	  sprintf(&(rs[i]), "%03o", (s[x] & 0377));
	  i += 3;
	  break;
      }
    }
  }
  rs[i++] = '"';
  rs[i] = '\0';
  return rs;
}