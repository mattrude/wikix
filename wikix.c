#include "platform.h"
 
#ifdef WINDOWS
 
#define strncasecmp strnicmp
 
#include "windows.h"
#include "winioctl.h"
#include "winuser.h"
#include "stdarg.h"
typedef UCHAR BYTE;
typedef USHORT WORD;
#include "stdio.h"
#include "stdlib.h"
#include "ctype.h"
#include "conio.h"
 
#endif
 
#ifdef LINUX
 
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ctype.h>
#include <string.h>
//#include <ncurses.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sched.h>
#include <ctype.h>
#include <openssl/md5.h>
 
#endif
 
#define NAME_HASH_SIZE 8192
 
typedef struct _hash
{
   struct _hash *next;
   struct _hash *prior;
   unsigned long len;
   char *text;
} hash;
 
typedef struct _hash_list {
   hash *head;
   hash *tail;
} hash_list;
 
unsigned char buffer[8192 * 4];
unsigned char ImagePath[512];
unsigned char OutputPath[512];
unsigned char iPath[512];
unsigned char cPath[512];
unsigned char md5_out[1024];
unsigned char md5_ulout[1024];
unsigned char wk[8192];
unsigned char final1[4096];
unsigned char final2[4096];
unsigned char ulwk[4096];
unsigned char fwk[4096];
unsigned char expand[4096];
unsigned char html[4096];
FILE *fpl[16];
int pmode = 0, tree = 0;
hash_list *learn_list_head = NULL;
hash_list *name_list_head = NULL;
int lobj = 0;
FILE *imagelog = NULL, *imagereject = NULL, *fragmentlog = NULL;
 
 
unsigned long shash(char *v, unsigned long len, unsigned long M)
{
   register unsigned long h = 0, a = 127, i;
 
   for (i = 0; i < len && *v; v++, i++)
      h = ((a * h) + tolower(*v)) % M;
 
   return h;
}
 
unsigned long add_to_hash(hash_list *top, hash *name)
{
    register unsigned long Value;
    register hash_list *HashTable;
 
    Value = shash(name->text, name->len, NAME_HASH_SIZE);
    if (Value == (unsigned long) -1)
       return -1;
 
    HashTable = (hash_list *) top;
    if (HashTable)
    {
       if (!HashTable[Value].head)
       {
          HashTable[Value].head = name;
          HashTable[Value].tail = name;
          name->next = name->prior = 0;
       }
       else
       {
          HashTable[Value].tail->next = name;
          name->next = 0;
          name->prior = HashTable[Value].tail;
          HashTable[Value].tail = name;
       }
       return 0;
    }
    return -1;
}
 
unsigned long remove_from_hash(hash_list *top, hash *name)
{
    register unsigned long Value;
    register hash_list *HashTable;
 
    Value = shash(name->text, name->len, NAME_HASH_SIZE);
    if (Value == (unsigned long) -1)
       return -1;
 
    HashTable = (hash_list *) top;
    if (HashTable)
    {
       if (HashTable[Value].head == name)
       {
          HashTable[Value].head = name->next;
          if (HashTable[Value].head)
             HashTable[Value].head->prior = NULL;
          else
             HashTable[Value].tail = NULL;
       }
       else
       {
          name->prior->next = name->next;
          if (name != HashTable[Value].tail)
             name->next->prior = name->prior;
          else
             HashTable[Value].tail = name->prior;
       }
       if (lobj)
          lobj--;
       return 0;
    }
    return -1;
}
 
void free_hash(void)
{
    register int i;
    register hash_list *HashTable;
    register hash *tmp, *name;
 
    if (learn_list_head)
    {
       HashTable = (hash_list *) learn_list_head;
       for (i=0; i < NAME_HASH_SIZE; i++)
       {
          name = HashTable[i].head;
          HashTable[i].head = HashTable[i].tail = 0;
          while (name)
          {
             tmp = name;
             name = name->next;
             free((void *)tmp);
          }
       }
       free(learn_list_head);
       learn_list_head = NULL;
    }
 
    if (name_list_head)
    {
       HashTable = (hash_list *) name_list_head;
       for (i=0; i < NAME_HASH_SIZE; i++)
       {
          name = HashTable[i].head;
          HashTable[i].head = HashTable[i].tail = 0;
          while (name)
          {
             tmp = name;
             name = name->next;
             free((void *)tmp);
          }
       }
       free(name_list_head);
       name_list_head = NULL;
    }
 
}
 
hash_list *init_hash_list(void)
{
    learn_list_head = (hash_list *) malloc(sizeof(hash_list) * NAME_HASH_SIZE);
    if (!learn_list_head)
       return NULL;
 
    memset(learn_list_head, 0, sizeof(hash_list) * NAME_HASH_SIZE);
 
    name_list_head = (hash_list *) malloc(sizeof(hash_list) * NAME_HASH_SIZE);
    if (!name_list_head)
       return NULL;
 
    memset(name_list_head, 0, sizeof(hash_list) * NAME_HASH_SIZE);
 
    return learn_list_head;
}
 
hash *search_name_hash(hash_list *top, char *text, unsigned long len)
{
    register unsigned long Value;
    register hash *name;
    register hash_list *HashTable;
 
    Value = shash(text, len, NAME_HASH_SIZE);
 
    HashTable = (hash_list *) top;
    name = (hash *) HashTable[Value].head;
    while (name)
    {
       if (len == name->len) 
       {
          if (!strncasecmp(name->text, text, len))
             return (hash *) name;
       }
       name = name->next;
    }
    return NULL;
 
}
 
int learn(char *s, int len)
{
   register hash *name;
 
   name = search_name_hash(learn_list_head, s, len);
   if (name)
      return 1;
 
   name = malloc(sizeof(hash) + len + 2);
   if (!name)
      return 1;
 
   memset(name, 0, sizeof(hash) + len);
   name->text = (char *)((unsigned long)name + sizeof(hash));
   name->len = len;
   strncpy(name->text, s, len);
 
   if (add_to_hash(learn_list_head, name) == -1)
   {
      free(name);
      return 1;
   }
   lobj++;
   return 0;
 
}
 
int imagename(char *s, int len)
{
   register hash *name;
 
   name = search_name_hash(name_list_head, s, len);
   if (name)
      return 1;
 
   name = malloc(sizeof(hash) + len + 2);
   if (!name)
      return 1;
 
   memset(name, 0, sizeof(hash) + len);
   name->text = (char *)((unsigned long)name + sizeof(hash));
   name->len = len;
   strncpy(name->text, s, len);
 
   if (add_to_hash(name_list_head, name) == -1)
   {
      free(name);
      return 1;
   }
   lobj++;
   return 0;
 
}
 
unsigned char *nprintf(char *s, int len, FILE *fp)
{
    register int i;
 
    if (!s || !*s)
       return s;
 
    for (i=0; *s && (i < len); i++)
       putc(*s++, fp);
 
    return s;
}
 
unsigned char *str8rchr(const char * s, int c1, int c2, int c3, int c4, 
                        int c5, int c6, int c7, int c8)
{
       const char *p = s + strlen(s);
       do {
           if ((*p == (char)c1) || (*p == (char)c2) || (*p == (char)c3) ||
               (*p == (char)c4) || (*p == (char)c5) || (*p == (char)c6) || 
               (*p == (char)c7) || (*p == (char)c8))
               return (char *)p;
       } while (--p >= s);
       return NULL;
}
 
unsigned char *str5rchr(const char * s, int c1, int c2, int c3, int c4, 
                        int c5)
{
       const char *p = s + strlen(s);
       do {
           if ((*p == (char)c1) || (*p == (char)c2) || (*p == (char)c3) ||
               (*p == (char)c4) || (*p == (char)c5))
               return (char *)p;
       } while (--p >= s);
       return NULL;
}
 
char *strnstr(const char * s1,const char * s2)
{
        int l1, l2;
 
        l2 = strlen(s2);
        if (!l2)
                return (char *) s1;
        l1 = strlen(s1);
        while (l1 >= l2) {
                l1--;
                if (!strncasecmp(s1,s2,l2))
                        return (char *) s1;
                s1++;
        }
        return NULL;
}
 
unsigned char *imagetypes[]=
{
    // 7
    ".svg+xml",
    ".xcf.bz2", 
 
    // 6
    ".bitmap", 
    ".xcfbz2", 
 
    // 5
    ".xcfgz", 
    ".alpha", 
    ".dicom", 
    ".matte", 
    ".xjtgz", 
 
    // 4
    ".mask", 
    ".aifc",
    ".aiff",
    ".fits", 
    ".icon", 
    ".im24",
    ".im32", 
    ".jpeg", 
    ".midi", 
    ".mpeg", 
    ".xwav", 
    ".mpga", 
    ".tiff", 
    ".djvu",
 
    // 3
    ".aif",
    ".als", 
    ".apm", 
    ".bmp", 
    ".bz2", 
    ".cel", 
    ".dcm", 
    ".eps", 
    ".fit",  
    ".flc", 
    ".fli", 
    ".gbr",
    ".gif", 
    ".gih", 
    ".gpb", 
    ".ico", 
    ".im1", 
    ".im8", 
    ".jpe", 
    ".jpg", 
    ".kar",
    ".mid",
    ".mov", 
    ".mp2",
    ".mp3", 
    ".mp4", 
    ".mpa",
    ".mpg", 
    ".ogg", 
    ".ogm", 
    ".pcc", 
    ".pcx", 
    ".pdf", 
    ".pdm", 
    ".pgm", 
    ".pix", 
    ".png", 
    ".pnm", 
    ".ppm",
    ".psd", 
    ".psp", 
    ".ras", 
    ".rgb",
    ".sgi", 
    ".svg", 
    ".swf",
    ".tga", 
    ".tif",
    ".tub", 
    ".wav", 
    ".wmf", 
    ".xbm", 
    ".xcf", 
    ".xjt", 
    ".xpm", 
    ".xwd", 
    ".pov", 
    ".wma", 
    ".dia", 
    ".fig", 
    ".jif", 
    ".pgn", 
    ".art", 
    ".djv",
 
    // 2
    ".bw", 
    ".ps", 
    ".g3", 
    ".js",
    ".rs", 
};
 
unsigned char *strip_image_info(unsigned char *s, char *title)
{
    register int i;
    unsigned char *p, *j;
    FILE *fp = stdout;
    unsigned char ch = '\0';
 
    while (*s && (isspace(*s))) s++;
 
    if (!strncasecmp(s, "no image", 8))
       return s;
 
    p = s;
    while (*s)
    {
       if ((!strncasecmp(s, "image", 5) || 
           !strncasecmp(s, "map", 3)) && !isalnum(ch))
       {
          unsigned char *fragment, *end;
 
          fragment = s;
          if (!strncasecmp(s, "image", 5))
             s += 5;
          else
          if (!strncasecmp(s, "map", 3)) 
             s += 3;
 
          if (*s)
          {
             while (*s && isalnum(*s)) s++;
 
             end = s;
             while (*s && isspace(*s)) s++;
 
             if (*s && *s == '=' || *s == ':')
             {
                memset(&fwk[0], 0, 256);      
                memmove(&fwk[0], fragment, (end - fragment));
                if (!learn(&fwk[0], end - fragment))
                {
                   if (*title)
                      fprintf(fragmentlog, "[%s] %s\n", title, &fwk[0]);
                   else
                      fprintf(fragmentlog, "%s\n", &fwk[0]);
                   fflush(fragmentlog);
                }
 
                s++;
                s = strip_image_info(s, title);
                ch = '\0';
             }
          } 
          continue;
       }
 
       if ((*s == '|') || (*s == ']') || (*s == '\n'))
       {
          register int y;
          unsigned char ch = '\x22', *l;
          unsigned char dir1[32], dir2[32]; 
          unsigned char *lp, *lw, *lo, *delim, *blp;
          unsigned char *ulp, *fname, *bulp;
          register int cnvt = 0, bcnvt = 0, unicnvt = 0, invl = 0;
 
          lp = &wk[0];
          j = lp;
          while (*p && (p < s))
          {
             // skip self referencing images
             if (!strncasecmp(p, "{{", 2))
                return s;                
 
             if (!memcmp(p, "&quot;", 6))
             {
                p += 6;
                *j++ = '\x22';
             }
             if (!memcmp(p, "&amp;", 5))
             {
                p += 5;
                *j++ = '&';
             }
             if (!memcmp(p, "&lt;", 4))
             {
                p += 4;
                while (*p)
                {
                   if (!memcmp(p, "&gt;", 4))
                   {
                      p += 4;
                      break;                     
                   }
                   p++;
                }   
             }
 
             if (*p == '\n')
                p++;
 
             if (!memcmp(p, "[[", 2))
                break;
 
             *j++ = *p++;
          }
          *j = '\0';
          s++;
 
 
          for (j=NULL, y=0; y < (sizeof(imagetypes) / sizeof (char *)); y++)
          {
             j = strnstr(lp, imagetypes[y]);
             if (j)
             {
                register int ilen = strlen(imagetypes[y]);
 
                j += ilen;
                *j = '\0';
                break;
             }
          }
 
          if (!j)
          {
             if (*lp && isalpha(*lp))
             {
                unsigned char *sp = strchr(lp, '.');
                unsigned char *sj, *slp = lp;
 
                if (sp)
                {
                   unsigned char *sllp = sp, *meter;
 
                   sllp++;
                   if ((*sllp != ' ') && (isalpha(*sllp)))
                   {
                      sj = str8rchr(slp, ':', '/', '\\', '{', '\n', '&', 
                                    '=', '>');
                      if (sj)
                         slp = ++sj;
 
                      meter = sllp;
                      while (*sllp)
                      {
                         if (!isalpha(*sllp))
                         {
                            *sllp = '\0';
                            break;
                         }
                         sllp++;
                      }
 
                      if (*slp && 
                         (((sllp - meter) >= 3) && ((sllp - meter) <= 5)))
                      {
                         if (*title)
                            fprintf(imagereject, "[%s] %s\n", title, slp);
                         else
                            fprintf(imagereject, "%s\n", slp);
                         fflush(imagereject);
                      }
                   }
                }
             }
             return s;
          }
 
          j = str5rchr(lp, ':', '/', '\\', '{', '\n');
          if (j)
             lp = ++j;
 
          if (!*lp)
             return s;
 
#ifdef UNICODE_EXPANSION
          // filename string extracted.  convert xml control character tags
          l = &expand[0];
          ulp = lp;
          while (*ulp)
          {
             if (!strncasecmp(ulp, "&amp;", 5))
             {
                ulp += 5;
                *l++ = '&';
                continue;
             }
             if (!strncasecmp(ulp, "&lt;", 4))
             {
                ulp += 4;
                *l++ = '<';
                continue;
             }
             if (!strncasecmp(ulp, "&gt;", 4))
             {
                ulp += 4;
                *l++ = '>';
                continue;
             }
             if (!strncasecmp(ulp, "&quot;", 6))
             {
                ulp += 6;
                *l++ = '\"';
                continue;
             }
             if (!strncasecmp(ulp, "&apos;", 6))
             {
                ulp += 6;
                *l++ = '\'';
                continue;
             }
             if (!strncasecmp(ulp, "&nbsp;", 6))
             {
                ulp += 6;
                *l++ = ' ';
                continue;
             }
             if (!strncasecmp(ulp, "&ndash;", 6))
             {
                ulp += 6;
                *l++ = '-';
                continue;
             }
             if ((ulp[0] == '&') && (ulp[1] != '&'))
             {
                unsigned char *sc = strchr(ulp, ';'), *slp;
                unsigned char unicode[32];
                unsigned char unidest[32];
                unsigned short uni;
 
                if (sc)
                {
                   slp = ulp;
                   slp++;
                   while (*slp != ';')
                   {
                      if ((*slp == '#') || (*slp == '-') ||
                          (*slp == 'x') || (*slp == 'X') ||
                          isxdigit(*slp))
                         slp++;
                      else
                      {
                         invl = 1;
                         break;
                      }
                   }
 
                   if (!invl)
                   {
                      int unilen = sc - ulp;
                      int slen = sc - ulp;
 
                      slp = ulp;
                      slp++;
                      unilen--;
                      if (*slp == '#')
                      {
                         unilen--;
                         slp++;
                      }
 
                      if (unilen < 31)
                      {
                         memset(unicode, 0, 32);
                         strncpy(unicode, slp, unilen);
                         uni = atoi(unicode);
 
                         fprintf(imagelog, "UNI1: %s (#%d) %s \n", 
                                 unicode, (int)uni,
                                 lp);
 
                         unicode[0] = '\0';
                         sprintf(unicode, "\\u%04X", uni);
 
                         unilen = u8_unescape(l, 32, unicode);
 
                         fprintf(imagelog, "UNI2: %s unilen %d slen  %d\n", 
                                 unicode, (int)unilen, (int)slen);
 
                         ulp += slen;
                         l += unilen;
                         ulp++;
 
                         unicnvt = 1;
                         continue;
                      }
                   }
                }
             }
             *l++ = *ulp++;
          }
          *l = '\0'; 
          lp = &expand[0];
#endif
 
          // convert spaces to underline characters in image names 
          ulp = &ulwk[0];
          memmove(ulp, lp, strlen(lp) + 1);
          ulp[0] = toupper(ulp[0]);          
          {
             l = ulp;
             while (*l)
             {
                if (*l == ' ')
                {
                   *l = '_';
                   cnvt = 1;
                }
                l++;
             }
          }
 
          if (learn(lp, strlen(lp)))
             return s;
 
          if (cnvt && learn(ulp, strlen(ulp)))
             return s;
 
          memset(md5_out, 0, 16);
          lp[0] = toupper(lp[0]);          
 
#ifdef UNICODE_EXPANSION
          if (unicnvt || invl)
          {
             if (invl)
                fprintf(imagelog, "INVL: %s -> %s\n", wk, lp);
             else
                fprintf(imagelog, "%s -> %s\n", wk, lp);
             fflush(imagelog);
             if (invl)
                return s;
          }
          else
             return s;
#else
          fprintf(imagelog, "%s\n", lp);
          fflush(imagelog);
#endif
 
          MD5(lp, strlen(lp), md5_out);
          dir1[0] = '\0';              
          sprintf(dir1, "%x/%02x/", (md5_out[0] >> 4), md5_out[0]);
 
          if (cnvt)
          {
             memset(md5_ulout, 0, 16);
             ulp[0] = toupper(ulp[0]);          
             MD5(ulp, strlen(ulp), md5_ulout);
             dir2[0] = '\0';              
             sprintf(dir2, "%x/%02x/", (md5_ulout[0] >> 4), md5_ulout[0]);
          }
 
          // add trailing \\ characters to bash control chars
          fname = &final1[0];
          blp = lp;
          while (*blp)
          {
             if ((*blp == '\"') || (*blp == '\'') || (*blp == '`'))
             {
                bcnvt = 1;                
                *fname++ = '\\';
             }
             else
             if ((*blp == ' ') || (*blp == '(') || (*blp == ')') ||
                 (*blp == '{') || (*blp == '}') || (*blp == '[') || 
                 (*blp == ']') || (*blp == '&') || (*blp == '-') ||
                 (*blp == ';'))
                *fname++ = '\\';
 
             *fname++ = *blp++;
          }
          *fname = '\0';
          blp = &final1[0];
 
          // add trailing \\ characters to bash control chars
          fname = &final2[0];
          bulp = ulp;
          while (*bulp)
          {
             if ((*bulp == '\"') || (*bulp == '\'') || (*bulp == '`'))
             {
                bcnvt = 1;                
                *fname++ = '\\';
             }
             else
             if ((*bulp == ' ') || (*bulp == '(') || (*bulp == ')') ||
                 (*bulp == '{') || (*bulp == '}') || (*bulp == '[') || 
                 (*bulp == ']') || (*bulp == '&') || (*bulp == '-') ||
                 (*bulp == ';'))
                *fname++ = '\\';
 
             *fname++ = *bulp++;
          }
          *fname = '\0';
          bulp = &final2[0];
 
          // debug of control characters
//          if (!bcnvt)
//             return s;
 
          if (tree)
          {
             if (pmode)
                fp = fpl[(md5_out[0] >> 4) % 16];
 
             fprintf(fp, "if [ -a $IMAGE./%s%s ]; then\n", 
                     dir1, blp);
 
             fprintf(fp, "\t/bin/mkdir -p $OUTPUT./%s\n", dir1);
             fprintf(fp, "\tcp -f $IMAGE./%s%s $OUTPUT./%s%s\n", 
                     dir1, blp, dir1, blp);
             fprintf(fp, "\techo ./%s%s copied to $OUTPUT./%s%s >> "
                     "copied.log\n",  dir1, blp, dir1, blp);
 
             if (cnvt)          
             {
                fprintf(fp, "elif [ -a $IMAGE./%s%s ]; then\n", 
                     dir2, bulp);
                fprintf(fp, "\t/bin/mkdir -p $OUTPUT./%s\n", dir2);
                fprintf(fp, "\tcp -f $IMAGE./%s%s $OUTPUT./%s%s\n", 
                     dir2, bulp, dir2, bulp);
                fprintf(fp, "\techo ./%s%s copied to $OUTPUT./%s%s >> "
                     "copied.log\n", dir2, bulp, dir2, bulp);
             }
             fprintf(fp, "else\n");
             fprintf(fp, 
                 "\techo ./%s%s file not found >> failed.log\n", dir1, 
                 blp);
             fprintf(fp, "fi\n\n");
          }
          else
          {
             if (pmode)
                fp = fpl[(md5_out[0] >> 4) % 16];
 
             fprintf(fp, "if [ -a $IMAGE./%s%s ]; then\n", 
                     dir1, blp);
             fprintf(fp, "\techo %s%s already exists >> exists.log\n", 
                  dir1, blp);
 
             if (cnvt)          
             {
                fprintf(fp, "elif [ -a $IMAGE./%s%s ]; then\n", 
                     dir2, bulp);
                fprintf(fp, "\techo %s%s already exists >> exists.log\n", 
                     dir2, bulp);
             }
             fprintf(fp, "else\n");
 
             fprintf(fp, "\tcurl --retry 7 -f -O $IMAGEPATH./%s%s\n",
                  dir1, blp);
             fprintf(fp, "\tif [ -a $IMAGE./%s ]; then\n", blp);
             fprintf(fp, "\t\t/bin/mkdir -p $OUTPUT./%s\n", dir1);
             fprintf(fp, "\t\t/bin/mv ./%s $OUTPUT./%s\n", 
                   blp, dir1);
             fprintf(fp, "\t\techo ./%s%s downloaded >> download.log\n", 
                   dir1, blp);
 
             fprintf(fp, "\telse\n");
             fprintf(fp, "\t\tcurl --retry 7 -f -O $COMMONSPATH./%s%s\n",
                  dir1, blp);
 
             fprintf(fp, "\t\tif [ -a $IMAGE./%s ]; then\n", 
                  blp);
             fprintf(fp, "\t\t\t/bin/mkdir -p $OUTPUT./%s\n", dir1);
             fprintf(fp, "\t\t\t/bin/mv ./%s $OUTPUT./%s\n", 
                     blp, dir1);
             fprintf(fp, "\t\t\techo ./%s%s downloaded >> download.log\n", 
                   dir1, blp);
             fprintf(fp, "\t\telse\n");
 
             if (cnvt)
             {
 
                fprintf(fp, "\t\t\tcurl --retry 7 -f -O $IMAGEPATH./%s%s\n",
                  dir2, bulp);
                fprintf(fp, "\t\t\tif [ -a $IMAGE./%s ]; then\n", 
                  bulp);
                fprintf(fp, "\t\t\t\t/bin/mkdir -p $OUTPUT./%s\n", 
                 dir2);
                fprintf(fp, "\t\t\t\t/bin/mv ./%s $OUTPUT./%s\n", 
                  bulp, dir2);
                fprintf(fp, "\t\t\t\techo ./%s%s downloaded >> "
                    "download.log\n", dir2, bulp);
 
                fprintf(fp, "\t\t\telse\n");
                fprintf(fp, "\t\t\t\tcurl --retry 7 -f -O $COMMONSPATH./%s%s\n",
                        dir2, bulp);
 
                fprintf(fp, "\t\t\t\tif [ -a $IMAGE./%s ]; then\n", 
                  bulp);
                fprintf(fp, "\t\t\t\t\t/bin/mkdir -p $OUTPUT./%s\n", 
                  dir2);
                fprintf(fp, "\t\t\t\t\t/bin/mv ./%s $OUTPUT./%s\n", 
                  bulp, dir2);
                fprintf(fp, "\t\t\t\t\techo ./%s%s downloaded >> "
                  "download.log\n", dir2, bulp);
                fprintf(fp, "\t\t\t\telse\n");
                fprintf(fp, "\t\t\t\t\techo ./%s%s failed >> failed.log\n", 
                  dir1, blp);
                fprintf(fp, "\t\t\t\t\techo ./%s%s failed >> failed.log\n", 
                  dir2, bulp);
                fprintf(fp, "\t\t\t\tfi\n");
                fprintf(fp, "\t\t\tfi\n");
             }
             else
             {
                fprintf(fp, 
                 "\t\t\techo ./%s%s failed >> failed.log\n", dir1, 
                 blp);
             }
             fprintf(fp, "\t\tfi\n");
             fprintf(fp, "\tfi\n");
             fprintf(fp, "fi\n\n");
          }
 
          return s;
       }
       ch = *s;
       s++;
    }
    return s;
}
 
int main(int argc, char *argv[])
{
    register int i, r, inpage = 0;
    unsigned char *s, *j, fname[32], *buffer, *title, *title_p;
    FILE *fl;
 
    ImagePath[0] = '\0';
    OutputPath[0] = '\0';
 
    // http://upload.wikimedia.org/wikipedia/en/
    // http://upload.wikimedia.org/wikipedia/commons/
    iPath[0] = '\0';
    cPath[0] = '\0';
    strcpy(iPath, "http://upload.wikimedia.org/wikipedia/en/");
    strcpy(cPath, "http://upload.wikimedia.org/wikipedia/commons/");
 
    for (i=0; i < argc; i++)
    {
       // remote path
       if (!memcmp(argv[i], "-h", 2))
       {
          printf("USAGE:  wikix -htrciop < file.xml [ > script.out ]\n");
          printf("              -h   this help screen\n");
          printf("              -t   use xml dump to strip from tree\n");
          printf("              -r   wikipedia path\n");
          printf("              -c   commons path\n");
          printf("              -i   image path\n");
          printf("              -o   output path\n");
          printf("              -p   parallel (16 process) mode\n");
          exit(1);
       }
 
       // remote path
       if (!memcmp(argv[i], "-t", 2))
       {
          tree = 1;
       }
 
       // remote path
       if (!memcmp(argv[i], "-r", 2))
       {
          i++;
          if (argv[i])
             strncpy(iPath, argv[i], 256);
       }
 
       // commons
       if (!memcmp(argv[i], "-c", 2))
       {
          i++;
          if (argv[i])
             strncpy(cPath, argv[i], 256);
       }
 
       // image tree
       if (!memcmp(argv[i], "-i", 2))
       {
          i++;
          if (argv[i])
             strncpy(ImagePath, argv[i], 256);
       }
 
       // output image tree
       if (!memcmp(argv[i], "-o", 2))
       {
          i++;
          if (argv[i])
             strncpy(OutputPath, argv[i], 256);
       }
 
       //parallel thread mode (16 processes)
       if (!memcmp(argv[i], "-p", 2))
       {
          pmode = 1;
       }
    }
 
    memset(&fwk[0], 0xFF, 256);      
 
    if (!init_hash_list())
    {
        printf("wikix:  could not allocate workspace\n");
        exit(1);
    }
 
    buffer = malloc(0x10000);
    if (!buffer)
    {
       printf("gfdl-wikititle:  could not allocate buffer workspace\n");
       exit(1);
    }
    buffer[0] = '\0';
 
    title = malloc(0x10000);
    if (!title)
    {
       printf("gfdl-wikititle:  could not allocate namespace\n");
       exit(1);
    }
    title[0] = '\0';
 
    if (!pmode)
    {
       printf("#!/bin/sh\n\n");
 
       printf("IMAGE=%s\n", ImagePath);
       printf("OUTPUT=%s\n", OutputPath);
       printf("IMAGEPATH=%s\n", iPath);
       printf("COMMONSPATH=%s\n\n", cPath);
 
       printf("/bin/mkdir -p $OUTPUT./thumb\n");
       printf("/bin/chmod 777 $OUTPUT./thumb\n");
       printf("/bin/mkdir -p $OUTPUT./temp\n");
       printf("/bin/chmod 777 $OUTPUT./temp\n");
       printf("/bin/mkdir -p $OUTPUT./tmp\n");
       printf("/bin/chmod 777 $OUTPUT./tmp\n\n");
 
    }
    else
    {
       fl = fopen("image_sh", "w");
       if (!fl)
       {
          printf("FILE error could not create image_sh\n");
          exit(1);
       }
 
       chmod("image_sh", 0755);
 
       fprintf(fl, "#!/bin/sh\n\n");
 
       fprintf(fl, "IMAGE=%s\n", ImagePath);
       fprintf(fl, "OUTPUT=%s\n", OutputPath);
       fprintf(fl, "IMAGEPATH=%s\n", iPath);
       fprintf(fl, "COMMONSPATH=%s\n\n", cPath);
 
       fprintf(fl, "/bin/mkdir -p $OUTPUT./thumb\n");
       fprintf(fl, "/bin/chmod 777 $OUTPUT./thumb\n");
       fprintf(fl, "/bin/mkdir -p $OUTPUT./temp\n");
       fprintf(fl, "/bin/chmod 777 $OUTPUT./temp\n");
       fprintf(fl, "/bin/mkdir -p $OUTPUT./tmp\n");
       fprintf(fl, "/bin/chmod 777 $OUTPUT./tmp\n\n");
 
       for (r=0; r < 16; r++)
       {
          fname[0] = '\0';
          sprintf(fname, "image%02d", r);
          fpl[r] = fopen(fname, "w");
          if (!fpl[r])
          {
             printf("FILE error could not create [%s]\n", fname);
             exit(1);
          }
 
          chmod(fname, 0755);
 
          fprintf(fpl[r], "#!/bin/sh\n\n");
          fprintf(fpl[r], "\nIMAGE=%s\n", ImagePath);
          fprintf(fpl[r], "OUTPUT=%s\n", OutputPath);
          fprintf(fpl[r], "IMAGEPATH=%s\n", iPath);
          fprintf(fpl[r], "COMMONSPATH=%s\n\n", cPath);
 
          fprintf(fl, "./%s >& imagelog.%02d &\n", fname, r);
       }
       fclose(fl);
    }
 
    imagelog = fopen("image.log", "wb");
    if (!imagelog)
    {
       printf("FILE error could not create image log\n");
    }
 
    imagereject = fopen("reject.log", "wb");
    if (!imagereject)
    {
       printf("FILE error could not create reject log\n");
    }
 
    fragmentlog = fopen("fragment.log", "wb");
    if (!fragmentlog)
    {
       printf("FILE error could not create image name fragment log\n");
    }
 
    while (s = fgets(buffer, 8192 * 4, stdin))
    {
       unsigned char ch = '\0';
 
       if (strstr(s, "<page>"))
       {
          inpage++;
          if (*title)
             *title = '\0';
          continue;
       }
 
       if (strstr(s, "</page>"))
       {
          if (inpage)
             inpage--;
          if (*title)
             *title = '\0';
          continue; 
       }
 
       title_p = strstr(s, "<title>");
       if (inpage && title_p)
       {
          register char *ts, *tp;
 
          ts = title_p;
          ts += 7;
          tp = strstr(ts, "</title>");
          if (tp)
          {
             if (tp - ts)
             {
                strncpy(title, ts, tp - ts);
                title[tp - ts] = '\0';
             }
          }
       }
 
       while (*s)
       {
          if (inpage && !strncasecmp(s, "<title>", 7))
          {
             register char *ts, *tp;
 
             s += 7;
             ts = s;
             tp = strstr(ts, "</title>");
             if (tp)
             {
                if (tp - ts)
                {
                   strncpy(title, ts, tp - ts);
                   title[tp - ts] = '\0';
                }
             }
          }
 
          if ((!strncasecmp(s, "image", 5) || 
                !strncasecmp(s, "map", 3)) && 
                !isalnum(ch))
          {
             unsigned char *fragment, *end;
 
             fragment = s;
             if (!strncasecmp(s, "image", 5))
                s += 5;
             else 
             if (!strncasecmp(s, "map", 3)) 
                s += 3;
 
             if (*s)
             {
                while (*s && isalnum(*s)) s++;
 
                end = s;
                while (*s && isspace(*s)) s++;
 
                if (*s && (*s == '=' || *s == ':'))
                {
                   memset(&fwk[0], 0, 256);      
                   memmove(&fwk[0], fragment, (end - fragment));
                   if (!imagename(&fwk[0], end - fragment))
                   {
                      if (*title)
                         fprintf(fragmentlog, "[%s] %s\n", title, &fwk[0]);
                      else
                         fprintf(fragmentlog, "%s\n", &fwk[0]);
                      fflush(fragmentlog);
                   }
 
                   s++;
                   s = strip_image_info(s, title);
                   ch = '\0';
                }
             } 
             continue;
          }
          ch = *s;
          s++;
       }
    }
 
    if (pmode)
    {
       for (r=0; r < 16; r++)
       {
          if (!fpl[r])
             fclose(fpl[r]);
          fpl[r] = NULL;
       }
    }
    fclose(fragmentlog);
    fclose(imagelog);
    fclose(imagereject);
    free(title);
    free(buffer);
    free_hash();
    return 0;
 
}
