#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX(a,b) ((a) > (b) ? a : b)
#define MIN(a,b) ((a) < (b) ? a : b)

size_t
_strlcpy(char *dst, const char *src, size_t siz)
{
#ifdef HAVE_STRLCPY
    return strlcpy(dst, src, siz);
#else
    char *d = dst;
    const char *s = src;
    size_t n = siz;

    /* Copy as many bytes as will fit */
    if (n != 0)
        while (--n != 0)
        {
            if ((*d++ = *s++) == '\0')
                break;
        }

    /* Not enough room in dst, add NUL and traverse rest of src */
    if (n == 0)
    {
        if (siz != 0)
            *d = '\0';  /* NUL-terminate dst */

        while (*s++)
            ;
    }

    return(s - src - 1); /* count does not include NUL */
#endif
}


static inline char **
_str_split_full_helper(const char *str,
                           const char *delim,
                           int max_tokens,
                           unsigned int *elements)
{
   char *s, *pos, **str_array;
   const char *src;
   size_t len, dlen;
   unsigned int tokens = 0, x;
   const char *idx[256] = {NULL};

   if ((!str) || (!delim))
     {
        if (elements)
          *elements = 0;

        return NULL;
     }
   if (max_tokens < 0) max_tokens = 0;
   if (max_tokens == 1)
     {
        str_array = malloc(sizeof(char *) * 2);
        if (!str_array)
          {
             if (elements)
                *elements = 0;

             return NULL;
          }

        s = strdup(str);
        if (!s)
          {
             free(str_array);
             if (elements)
                *elements = 0;

             return NULL;
          }
        if (elements)
          *elements = 1;
        str_array[0] = s;
        str_array[1] = NULL;
        return str_array;
     }
   dlen = strlen(delim);
   if (dlen == 0)
     {
        if (elements)
           *elements = 0;

        return NULL;
     }

   src = str;
   /* count tokens and check strlen(str) */
   while (*src != '\0')
     {
        const char *d = delim, *d_end = d + dlen;
        const char *tmp = src;
        for (; (d < d_end) && (*tmp != '\0'); d++, tmp++)
          {
             if (*d != *tmp)
                break;
          }
        if (d == d_end)
          {
             src = tmp;
             if (tokens < (sizeof(idx) / sizeof(idx[0])))
               {
                  idx[tokens] = tmp;
                  //printf("token %d='%s'\n", tokens + 1, idx[tokens]);
               }
             tokens++;
             if (tokens && (tokens == (unsigned int)max_tokens)) break;
          }
        else
           src++;
     }
   len = src - str + strlen(src);

   str_array = malloc(sizeof(char *) * (tokens + 2));
   if (!str_array)
     {
        if (elements)
           *elements = 0;

        return NULL;
     }

   if (!tokens)
     {
        s = strdup(str);
        if (!s)
          {
             free(str_array);
             if (elements)
                *elements = 0;

             return NULL;
          }
        str_array[0] = s;
        str_array[1] = NULL;
        if (elements)
          *elements = 1;
        return str_array;
     }

   s = malloc(len + 1);
   if (!s)
     {
        free(str_array);
        if (elements)
           *elements = 0;

        return NULL;
     }

   str_array[0] = s;

   if (len == tokens * dlen)
     {
        /* someone's having a laugh somewhere */
        memset(s, 0, len + 1);
        for (x = 1; x < tokens + 1; x++)
          str_array[x] = s + x;
        str_array[x] = NULL;
        if (elements)
          *elements = x;
        return str_array;
     }
   /* copy tokens and string */
   if (idx[0] - str - dlen > len)
     {
        /* FIXME: don't think this can happen but putting this here just in case */
        abort();
     }
   pos = s;
   for (x = 0; x < MIN(tokens, (sizeof(idx) / sizeof(idx[0]))); x++)
     {
        if (x + 1 < (sizeof(idx) / sizeof(idx[0])))
          {
             /* first one is special */
             if (!x)
               {
                  _strlcpy(pos, str, idx[x] - str - dlen + 1);
                  str_array[x] = pos;
                  //printf("str_array[%d] = '%s'\n", x, str_array[x]);
                  pos += idx[x] - str - dlen + 1;
                  if ((tokens == 1) && (idx[0]))
                    {
                       _strlcpy(pos, idx[x], len + 1 - (pos - s));
                       x++, tokens++;
                       str_array[x] = pos;
                    }
               }
             /* more tokens */
             else if (idx[x + 1])
               {
                  _strlcpy(pos, idx[x - 1], idx[x] - idx[x - 1] - dlen + 1);
                  str_array[x] = pos;
                  //printf("str_array[%d] = '%s'\n", x, str_array[x]);
                  pos += idx[x] - idx[x - 1] - dlen + 1;
               }
             /* last token */
             else
               {
                  if (max_tokens && ((unsigned int)max_tokens < tokens + 1))
                    _strlcpy(pos, idx[x - 1], len + 1 - (pos - s));
                  else
                    {
                       //printf("diff: %d\n", len + 1 - (pos - s));
                       _strlcpy(pos, idx[x - 1], idx[x] - idx[x - 1] - dlen + 1);
                       str_array[x] = pos;
                       //printf("str_array[%d] = '%s'\n", x, str_array[x]);
                       pos += idx[x] - idx[x - 1] - dlen + 1;
                       x++, tokens++;
                       _strlcpy(pos, idx[x - 1], len + 1 - (pos - s));
                    }
                  str_array[x] = pos;
                  //printf("str_array[%d] = '%s'\n", x, str_array[x]);
               }
          }
        /* no more tokens saved after this one */
        else
          {
             _strlcpy(pos, idx[x - 1], idx[x] - idx[x - 1] - dlen + 1);
             str_array[x] = pos;
             //printf("str_array[%d] = '%s'\n", x, str_array[x]);
             pos += idx[x] - idx[x - 1] - dlen + 1;
             src = idx[x];
             x++, tokens++;
             str_array[x] = s = pos;
             break;
          }
     }
   if ((x != tokens) && ((!max_tokens) || (x < tokens)))
     {
        while (*src != '\0')
          {
             const char *d = delim, *d_end = d + dlen;
             const char *tmp = src;
             for (; (d < d_end) && (*tmp != '\0'); d++, tmp++)
               {
                  if (*d != *tmp)
                     break;
               }
             if (((!max_tokens) || (((tokens == (unsigned int)max_tokens) || x < tokens - 2))) && d == d_end)
               {
                  src = tmp;
                  *s = '\0';
                  s++, x++;
                  //printf("str_array[%d] = '%s'\n", x, str_array[x - 1]);
                  str_array[x] = s;
               }
             else
               {
                  *s = *src;
                  s++, src++;
               }
          }
        *s = 0;
     }
   str_array[tokens] = NULL;
   if (elements)
     *elements = tokens;

   return str_array;
}


char ** str_split(const char *str, const char *delim, int max_tokens)
{
    return _str_split_full_helper(str, delim, max_tokens, NULL);
}
