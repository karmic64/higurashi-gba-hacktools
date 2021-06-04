#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>



/* note: comment this out when the engine is hacked to support plain ascii */
#define DOUBLEMODE

#ifdef DOUBLEMODE
const uint16_t ascjistbl[] = {
/* 20         !      "      #      $      %      &      '   */
    0x8140,0x8149,0x8168,0x8194,0x8190,0x8193,0x8195,0x8166,
/* 28  (      )      *      +      ,      -      .      /   */
    0x8169,0x816a,0x8196,0x817b,0x8143,0x817c,0x8144,0x815e,
/* 30  0      1      2      3      4      5      6      7   */
    0x824f,0x8250,0x8251,0x8252,0x8253,0x8254,0x8255,0x8256,
/* 38  8      9      :      ;      <      =      >      ?   */
    0x8257,0x8258,0x8146,0x8147,0x8183,0x8181,0x8184,0x8148,
/* 40  @      A      B      C      D      E      F      G   */
    0x8197,0x8260,0x8261,0x8262,0x8263,0x8264,0x8265,0x8266,
/* 48  H      I      J      K      L      M      N      O   */
    0x8267,0x8268,0x8269,0x826a,0x826b,0x826c,0x826d,0x826e,
/* 50  P      Q      R      S      T      U      V      W   */
    0x826f,0x8270,0x8271,0x8272,0x8273,0x8274,0x8275,0x8276,
/* 58  X      Y      Z      [      \      ]      ^      _   */
    0x8277,0x8278,0x8279,0x816d,0x815f,0x816e,0x814f,0x8151,
/* 60  `      a      b      c      d      e      f      g   */
    0x814d,0x8281,0x8282,0x8283,0x8284,0x8285,0x8286,0x8287,
/* 68  h      i      j      k      l      m      n      o   */
    0x8288,0x8289,0x828a,0x828b,0x828c,0x828d,0x828e,0x828f,
/* 70  p      q      r      s      t      u      v      w   */
    0x8290,0x8291,0x8292,0x8293,0x8294,0x8295,0x8296,0x8297,
/* 78  x      y      z      {      |      }      ~   */
    0x8298,0x8299,0x829a,0x816f,0x8162,0x8170,0x8160,
};
#endif


#define DOUBLEBYTE(c) (((uint8_t)(c) >= 0x80 && (uint8_t)(c) < 0xa1) || ((uint8_t)(c) >= 0xe0))



unsigned int fgetvnum(FILE *f)
{
    unsigned int v = 0;
    int s = 0;
    while (1)
    {
        uint8_t c = fgetc(f);
        if (feof(f)) return -1;
        v |= (c & 0x7f) << s;
        if (!(c & 0x80)) break;
        s += 7;
    }
    return v;
}


/*
script text control codes:

$ wait for button
# new line
\ new page

commands start with %
directives start with !
comments start with ;
*/



int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        puts(
            "Usage: script-transformer index inscript outscript\n"
            "\n"
            "This tool uses an index file created by 07th-indexer to change\n"
            "the text of a mint script to new translated text."
            );
        return EXIT_FAILURE;
    }
    
    
    /* --- read index file --- */
    FILE *f = fopen(argv[1], "rb");
    if (!f)
    {
        printf("Couldn't open %s: %s\n", argv[1], strerror(errno));
        return EXIT_FAILURE;
    }
    unsigned int strings = fgetvnum(f);
    printf("Index file contains %u strings\n", strings);
    size_t totalsize = 0;
    size_t *jpstrings = malloc(strings * sizeof(*jpstrings));
    size_t *enstrings = malloc(strings * sizeof(*enstrings));
    unsigned int *stringsizes = malloc(strings * sizeof(*stringsizes) * 2);
    for (unsigned int i = 0; i < strings; i++)
    {
        unsigned int jpsize = fgetvnum(f);
        unsigned int ensize = fgetvnum(f);
        if (feof(f))
        {
            fclose(f);
            puts("Unexpected end-of-file while reading string sizes");
            return EXIT_FAILURE;
        }
        /* +1 because we add nulls (which don't exist in the index file) */
        jpstrings[i] = totalsize;
        totalsize += jpsize + 1;
        enstrings[i] = totalsize;
        totalsize += ensize + 1;
        stringsizes[i*2] = jpsize;
        stringsizes[i*2 + 1] = ensize;
    }
    char *stringbuf = malloc(totalsize);
    size_t i = 0;
    for (unsigned int str = 0; str < strings*2; str++)
    {
        fread(stringbuf+i, 1, stringsizes[str], f);
        if (feof(f))
        {
            fclose(f);
            puts("Unexpected end-of-file while reading strings");
            return EXIT_FAILURE;
        }
        i += stringsizes[str];
        stringbuf[i++] = 0;
    }
    free(stringsizes);
    fclose(f);
    
    
    
    /* --- read and transform script --- */
    size_t linebufmax = 256;
    char *linebuf = malloc(linebufmax);
    
    unsigned int str = 0;
    unsigned int strsleft = strings;
    unsigned int badstrs = 0;
    unsigned int line = 1;
    
    FILE *inf = fopen(argv[2], "rb");
    if (!inf)
    {
        printf("Couldn't open %s: %s\n", argv[2], strerror(errno));
        return EXIT_FAILURE;
    }
    FILE *outf = fopen(argv[3], "wb");
    if (!outf)
    {
        fclose(inf);
        printf("Couldn't open %s: %s\n", argv[3], strerror(errno));
        return EXIT_FAILURE;
    }
    
    while (1)
    {
        size_t len = 0;
        int c;
        while ((c = fgetc(inf)) != EOF)
        {
            if (c == 0x0a) break;
            if (len == linebufmax-1)
                linebuf = realloc(linebuf, linebufmax *= 2);
            linebuf[len++] = c;
        }
        if (ferror(f))
        {
            printf("Error reading script: %s\n", strerror(errno));
            return EXIT_FAILURE;
        }
        if (linebuf[len-1] == 0x0d) len--;
        linebuf[len] = 0;
        
        /* skip blank lines */
        if (len)
        {
            /* print command lines as-is */
            if (linebuf[0] == '%' || linebuf[0] == '!')
            {
                fputs(linebuf, outf);
            }
            /* otherwise try changing the line */
            else
            {
                const char controlcodes[] = "$#\\";
                size_t i = 0;
                
                while (i < len)
                {
                    /* control code? */
                    if (strchr(controlcodes, linebuf[i]))
                    {
                        fputc(linebuf[i++], outf);
                    }
                    /* not a control code, identify and replace the string if possible */
                    else
                    {
                        /* count the length of the string */
                        /* strcspn is not suitable for this due to shiftjis */
                        size_t validlen = 0;
                        while (1)
                        {
                            if (i+validlen == len || strchr(controlcodes, linebuf[i+validlen])) break;
                            uint8_t c = linebuf[i+(validlen++)];
                            if (DOUBLEBYTE(c)) validlen++;
                        }
                        
                        /* now try hunting for the string, in a circular fashion */
                        unsigned int huntstr = str;
                        int foundflag = 0;
                        if (strsleft)
                        {
                            while (!foundflag)
                            {
                                if (jpstrings[huntstr] != -1 && !strncmp(stringbuf+jpstrings[huntstr], linebuf+i, validlen))
                                {
                                    /* found string */
                                    char *enstr = stringbuf+enstrings[huntstr];
                                    for (int i = 0; i < strlen(enstr); i++)
                                    {
                                        char c = enstr[i];
                                        if (DOUBLEBYTE(c))
                                        {
                                            fputc(c, outf);
                                            fputc(enstr[++i], outf);
                                        }
                                        else
                                        {
#ifdef DOUBLEMODE
                                            /* for some reason the mint compiler always dies when encountering "＝" (8181) */
                                            if (c != '=')
                                            {
                                                uint16_t sjc = ascjistbl[c - ' '];
                                                fputc(sjc >> 8, outf);
                                                fputc(sjc & 0xff, outf);
                                            }
#else
                                            fputc(c, outf);
#endif
                                        }
                                    }
                                    
                                    foundflag++;
                                    jpstrings[huntstr] = -1;
                                    strsleft--;
                                }
                                if (++huntstr == strings) huntstr = 0;
                                if (huntstr == str)
                                {
                                    /* not found, output string as-is */
                                    fwrite(linebuf+i, 1, validlen, outf);
                                    badstrs++;
                                    break;
                                }
                            }
                        }
                        str = huntstr;
                        
                        i += validlen;
                    }
                }
            }
            fputc('\n', outf);
        }
        
        if (feof(f)) break;
        line++;
    }
    
    free(linebuf);
    fclose(inf);
    fclose(outf);
    
    printf("%u strings in index file were not used\n", strsleft);
    printf("%u strings in script were not found in index file\n", badstrs);
    
    
    return EXIT_SUCCESS;
}

