#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>


#define get16(p) ((uint16_t)((*(uint8_t*)(p)) | ((*(((uint8_t*)(p))+1)) << 8)))


const uint8_t nintendologo[] = {0x24,0xFF,0xAE,0x51,0x69,0x9A,0xA2,0x21,0x3D,0x84,0x82,0x0A,0x84,0xE4,0x09,0xAD,0x11,0x24,0x8B,0x98,0xC0,0x81,0x7F,0x21,0xA3,0x52,0xBE,0x19,0x93,0x09,0xCE,0x20,0x10,0x46,0x4A,0x4A,0xF8,0x27,0x31,0xEC,0x58,0xC7,0xE8,0x33,0x82,0xE3,0xCE,0xBF,0x85,0xF4,0xDF,0x94,0xCE,0x4B,0x09,0xC1,0x94,0x56,0x8A,0xC0,0x13,0x72,0xA7,0xFC,0x9F,0x84,0x4D,0x73,0xA3,0xCA,0x9A,0x61,0x58,0x97,0xA3,0x27,0xFC,0x03,0x98,0x76,0x23,0x1D,0xC7,0x61,0x03,0x04,0xAE,0x56,0xBF,0x38,0x84,0x00,0x40,0xA7,0x0E,0xFD,0xFF,0x52,0xFE,0x03,0x6F,0x95,0x30,0xF1,0x97,0xFB,0xC0,0x85,0x60,0xD6,0x80,0x25,0xA9,0x63,0xBE,0x03,0x01,0x4E,0x38,0xE2,0xF9,0xA2,0x34,0xFF,0xBB,0x3E,0x03,0x44,0x78,0x00,0x90,0xCB,0x88,0x11,0x3A,0x94,0x65,0xC0,0x7C,0x63,0x87,0xF0,0x3C,0xAF,0xD6,0x25,0xE4,0x8B,0x38,0x0A,0xAC,0x72,0x21,0xD4,0xF8,0x07};

const uint8_t initcode[] = {0xd2,0x00,0xa0,0xe3,0x00,0xf0,0x21,0xe1};

const uint8_t killcode[] = {0x00,0x20,0x70,0x47};

/* note: hajimete no orusuban doesn't have any emulator detection */
const uint8_t emucheck[] = {0xF8,0xB5,0x37,0x4A,0x37,0x48,0x96,0x88,0x90,0x80,0x15,0x89,0x11,0x88,0x00,0x20,0x00,0x91,0x10,0x81,0x10,0x80,0x34,0x48,0x00,0x21,0x81,0x63,0x33,0x48,0x41,0x60,0x01,0x61,0xC1,0x61,0x32,0x49,0x00,0x20,0x48,0x80,0xC8,0x80,0x48,0x81,0xC8,0x81,0x48,0x60,0x08,0x60,0x21,0x20,0x80,0x04,0x48,0x60,0xC8,0x03,0x08,0x60,0x03,0x20};



uint8_t higuromcheck(uint8_t *p)
{
    /* push {list} */
    if ((get16(p+0) & 0xff00) != 0xb500) return 0;
    /* ldr rx, [pc,#x] */
    if ((get16(p+2) & 0xf800) != 0x4800) return 0;
    /* ldr rx, [pc,#x] */
    if ((get16(p+4) & 0xf800) != 0x4800) return 0;
    /* ldr rx, [ry,#0] */
    if ((get16(p+6) & 0xffc0) != 0x6800) return 0;
    /* sub rx, ry, #7 */
    if ((get16(p+8) & 0xffc0) != 0x1fc0) return 0;
    /* sub rx, #0xf9 */
    if ((get16(p+10) & 0xf8ff) != 0x38f9) return 0;
    /* sub rx, ry, rz */
    if ((get16(p+12) & 0xfe00) != 0x1a00) return 0;
    /* sub rx, ry, #7 */
    if ((get16(p+14) & 0xffc0) != 0x1fc0) return 0;
    /* sub rx, #0x39 */
    if ((get16(p+16) & 0xf8ff) != 0x3839) return 0;
    /* ldr rx, [ry,#0x3c] */
    if ((get16(p+18) & 0xffc0) != 0x6bc0) return 0;
    
    return 1;
}


uint8_t extdromcheck(uint8_t *p)
{
    /* this is for games which check the initial b instruction too */
    
    /* push {list} */
    if ((get16(p+0) & 0xff00) != 0xb500) return 0;
    /* ldr rx, [pc,#x] */
    if ((get16(p+2) & 0xf800) != 0x4800) return 0;
    /* ldr rx, [pc,#x] */
    if ((get16(p+4) & 0xf800) != 0x4800) return 0;
    /* ldr rx, [ry,#0] */
    if ((get16(p+6) & 0xffc0) != 0x6800) return 0;
    /* cmp rx, ry */
    if ((get16(p+8) & 0xffc0) != 0x4280) return 0;
    /* beq #+3 */
    if ((get16(p+10) & 0xffff) != 0xd003) return 0;
    /* asr rx, ry, #0x1b */
    if ((get16(p+12) & 0xffc0) != 0x16c0) return 0;
    /* pop {list} */
    if ((get16(p+14) & 0xff00) != 0xbc00) return 0;
    /* pop {list} */
    if ((get16(p+16) & 0xff00) != 0xbc00) return 0;
    /* bx rx */
    if ((get16(p+18) & 0xff87) != 0x4700) return 0;
    
    return 1;
}


int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        puts("inside-cap gba rom cracker/header fixer - written by karmic");
        printf("Usage: %s romname\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    FILE *f = fopen(argv[1], "rb");
    if (!f)
    {
        printf("Could not open %s: %s\n", argv[1], strerror(errno));
        return EXIT_FAILURE;
    }
    fseek(f, 0, SEEK_END);
    size_t fsize = ftell(f);
    rewind(f);
    uint8_t *buf = malloc(fsize);
    fread(buf, 1, fsize, f);
    fclose(f);
    
    /* code is at $c4 in to heart 2 */
    if (fsize < 0x100 || (memcmp(initcode, buf+0xc0, sizeof(initcode)) && memcmp(initcode, buf+0xc4, sizeof(initcode))))
    {
        puts("File is not a valid ROM");
        return EXIT_FAILURE;
    }
    
    uint8_t emukilled = 0;
    uint8_t romkilled = 0;
    
    for (uint8_t *p = buf; p < buf+fsize-0x40; p += 4)
    {
        if (!memcmp(p, emucheck, sizeof(emucheck)))
        {
            memcpy(p, killcode, sizeof(killcode));
            emukilled++;
            printf("Found emulator detection routine at $%X\n", (unsigned int)(p-buf));
        }
        else if (higuromcheck(p) || extdromcheck(p))
        {
            memcpy(p, killcode, sizeof(killcode));
            romkilled++;
            printf("Found ROM verification routine at $%X\n", (unsigned int)(p-buf));
        }
        if (emukilled && romkilled) break;
    }
    
    if (!emukilled)
    {
        puts("WARNING: Couldn't find emulator detection routine");
    }
    if (!romkilled)
    {
        puts("WARNING: Couldn't find ROM verification routine");
    }
    
    memcpy(buf+4, nintendologo, sizeof(nintendologo));
    
    uint8_t cpl = 0;
    for (uint8_t *p = buf+0xa0; p < buf+0xbd; p++) cpl -= *p;
    buf[0xbd] = cpl - 0x19;
    
    f = fopen(strcat(argv[1], "-crack.gba"), "wb");
    fwrite(buf, 1, fsize, f);
    fclose(f);
    
    return EXIT_SUCCESS;
}