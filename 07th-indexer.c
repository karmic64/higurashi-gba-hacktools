
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>

#include <iconv.h>


#define DOUBLEBYTE(c) (((uint8_t)(c) >= 0x80 && (uint8_t)(c) < 0xa1) || ((uint8_t)(c) >= 0xe0))


void fputvnum(unsigned int v, FILE *f)
{
    while (v >= 0x80)
    {
        fputc((v & 0x7f) | 0x80, f);
        v >>= 7;
    }
    fputc(v, f);
}



unsigned int *jpstrings = NULL;
unsigned int *enstrings = NULL;
size_t stringlistsize = 0;
size_t stringlistmax = 0;

char *stringbuf = NULL;
size_t stringbufsize = 0;
size_t stringbufmax = 0;


void addchar(char c)
{
    if (stringbufsize == stringbufmax)
    {
        stringbuf = realloc(stringbuf, stringbufmax *= 2);
    }
    stringbuf[stringbufsize++] = c;
}

char *addstr(char *s)
{
    while (1)
    {
        char c = *(s++);
        char next = *s;
        if (!c || c == '\"') break;
        else if (DOUBLEBYTE(c))
        {
            addchar(c);
            addchar(next);
            s++;
        }
        else if (c == '\\')
        {
            addchar(next);
            s++;
        }
        else
        {
            addchar(c);
        }
    }
    
    return s;
}



void scandir(char *dirname)
{
    char *oldcwd = getcwd(NULL, 0);
    
    if (chdir(dirname) < 0)
    {
        printf("Couldn't enter %s: %s\n", dirname, strerror(errno));
        free(oldcwd);
        return;
    }
    
    printf("Entering %s...\n", dirname);
    DIR *dir = opendir(".");
    struct dirent *de = NULL;
    struct stat st;
    while ((de = readdir(dir)))
    {
        char *fnam = de->d_name;
        if (!strcmp(fnam, ".")) continue;
        if (!strcmp(fnam, "..")) continue;
        
        if (access(fnam, R_OK) < 0) continue;
        if (stat(fnam, &st) < 0)
        {
            printf("Couldn't get information on %s: %s\n", fnam, strerror(errno));
            continue;
        }
        if (S_ISDIR(st.st_mode))
        {
            scandir(fnam);
        }
        else if (S_ISREG(st.st_mode))
        {
            char *srcbuf = malloc(st.st_size);
            char *destbuf = malloc(st.st_size);
            if (!srcbuf || !destbuf)
            {
                printf("Not enough memory for %s\n", fnam);
                goto fail;
            }
            
            FILE *f = fopen(fnam, "rb");
            if (!f)
            {
                printf("Couldn't open %s: %s\n", fnam, strerror(errno));
                continue;
            }
            
            printf("Reading %s...\n", fnam);
            fread(srcbuf, 1, st.st_size, f);
            fclose(f);
            
            iconv_t ic = iconv_open("Shift_JISX0213//TRANSLIT", "UTF-8");
            if (ic == (iconv_t)-1)
            {
                if (errno == EINVAL)
                {
                    puts("Your system does not support UTF-8 -> Shift-JIS conversion.");
                    exit(EXIT_FAILURE);
                }
                else
                {
                    printf("Couldn't open iconv descriptor: %s\n", strerror(errno));
                    exit(EXIT_FAILURE);
                }
            }
            
            char *inptr = srcbuf;
            char *outptr = destbuf;
            size_t inleft = st.st_size;
            size_t outleft = st.st_size;
            
            size_t status;
            while (1)
            {
                status = iconv(ic, &inptr, &inleft, &outptr, &outleft);
                if (status != -1) break;
                /* iconv errors when encountering a modified ascii char */
                if (*inptr != '\\' && *inptr != '~') break;
                *(outptr++) = *(inptr++);
                inleft--;
                outleft--;
            }
            
            if (status == -1)
            {
                printf("Conversion failure at 0x%llX: %s\n", inptr-srcbuf, strerror(errno));
            }
            else
            {
                /* tack a null at the end to make sure strstr works */
                *outptr = 0;
                
                free(srcbuf);
                srcbuf = NULL;
                unsigned int thisstrings = 0;
                
                inptr = destbuf;
                
                size_t actualstringbufsize = stringbufsize;
                while ((inptr = strstr(inptr, "OutputLine")))
                {
                    /* ignore OutputLineAll */
                    if (memcmp(inptr+10, "All", 3))
                    {
                        inptr = strchr(inptr+10, '(');
                        if (!inptr) break;
                        inptr = strchr(inptr, '\"');
                        if (!inptr) break;
                        inptr++;
                        
                        /* we now have located the jp string */
                        /* ignore character name tags */
                        if (!memcmp(inptr, "<color=#", sizeof("<color=#")-1)) continue;
                        size_t jpoffs = stringbufsize;
                        inptr = addstr(inptr);
                        unsigned int jpsize = stringbufsize - jpoffs;
                        
                        inptr = strchr(inptr, '\"');
                        if (!inptr) break;
                        inptr++;
                        
                        size_t enoffs = stringbufsize;
                        inptr = addstr(inptr);
                        unsigned int ensize = stringbufsize - enoffs;
                        
                        /* add strings to list */
                        if (stringlistsize == stringlistmax)
                        {
                            jpstrings = realloc(jpstrings, (stringlistmax *= 2) * sizeof(*jpstrings));
                            enstrings = realloc(enstrings, (stringlistmax) * sizeof(*enstrings));
                        }
                        jpstrings[stringlistsize] = jpsize;
                        enstrings[stringlistsize++] = ensize;
                        thisstrings++;
                        
                        /* this is to prevent "incomplete" strings being written */
                        actualstringbufsize = stringbufsize;
                    }
                    else
                    {
                        inptr += 13;
                    }
                }
                
                printf("%u strings found.\n", thisstrings);
                stringbufsize = actualstringbufsize;
            }
            
            iconv_close(ic);
            
fail:       free(srcbuf);
            free(destbuf);
        }
    }
    closedir(dir);
    printf("Leaving %s...\n", dirname);
    
    chdir(oldcwd);
    free(oldcwd);
}



int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        puts(   
                "Usage: 07th-indexer outname scriptdir...\n"
                "\n"
                "This tool reads 07th-Mod Higurashi scripts, converts them to Shift-JIS, and\n"
                "builds an index file that the script transformer uses to replace the text of\n"
                "the original mint script with the English text.\n"
                "\n"
                "\"outname\" is the name of the output index file, and \"scriptdir\" is a\n"
                "directory containing the script(s). The tool WILL recurse through\n"
                "subdirectories.");
        return EXIT_FAILURE;
    }
    
    jpstrings = malloc((stringlistmax = 0x1000) * sizeof(*jpstrings));
    enstrings = malloc(stringlistmax * sizeof(*enstrings));
    stringbuf = malloc(stringbufmax = 0x10000);
    
    for (int i = 2; i < argc; i++)
    {
        scandir(argv[i]);
        putchar('\n');
    }
    
    unsigned int strings = stringlistsize;
    if (!strings)
    {
        puts("No strings found in scripts. No index file will be written.");
        return EXIT_FAILURE;
    }
    FILE *of = fopen(argv[1], "wb");
    if (!of)
    {
        printf("Couldn't open %s: %s\n", argv[1], strerror(errno));
        return EXIT_FAILURE;
    }
    fputvnum(strings, of);
    for (unsigned int i = 0; i < strings; i++)
    {
        fputvnum(jpstrings[i], of);
        fputvnum(enstrings[i], of);
    }
    fwrite(stringbuf, 1, stringbufsize, of);
    fclose(of);
    printf("%u strings found. %s successfully written.\n", strings, argv[1]);
    return EXIT_SUCCESS;
    
}




