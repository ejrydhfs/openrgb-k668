/*------------------------------------------*\
|  K668Protocol.h                            |
|                                            |
|  Wire-format definitions and pure packet   |
|  builders for the Redragon K668WBO-RGB     |
|  (Sinowealth 258A:0049) lighting protocol. |
|                                            |
|  Every constant here was recovered from the |
|  vendor driver KeyboardDrv.exe V1.6.6 and   |
|  cross-checked against the live device (see |
|  REPROTOCOL.md).  The builders are pure so  |
|  they can be unit tested byte-for-byte      |
|  against the vendor layout.                 |
\*------------------------------------------*/

#pragma once

#include <cstddef>
#include <cstring>

namespace K668Protocol
{
    /*--------------------------------------------------------------*\
    | Feature report ids                                              |
    \*--------------------------------------------------------------*/
    static const unsigned char REPORT_CMD      = 0x05;  /* command request (GetXxx)  */
    static const unsigned char REPORT_DATA     = 0x06;  /* profile / palette / matrix */
    static const unsigned char REPORT_LED      = 0x08;  /* per-key LED frame          */

    /*--------------------------------------------------------------*\
    | Opcodes (report byte [1])                                       |
    \*--------------------------------------------------------------*/
    static const unsigned char OP_GET_PSD       = 0x81;
    static const unsigned char OP_GET_PROFILE   = 0x83;
    static const unsigned char OP_SET_PROFILE   = 0x03;
    static const unsigned char OP_GET_KEYMATRIX = 0x84;   /* read-only diagnostics    */
    static const unsigned char OP_SET_MACRO     = 0x05;
    static const unsigned char OP_SET_RGBTAB    = 0x08;
    static const unsigned char OP_SET_LEDMATRIX = 0x09;
    static const unsigned char OP_MUSIC         = 0x0A;

    /*--------------------------------------------------------------*\
    | SetKeyMatrix (report 0x06 opcode 0x04 addr 0xD4) is deliberately |
    | NOT defined or implemented: it stores the physical key-code map,  |
    | and a malformed write wipes the layout so the board stops typing. |
    | Only the read (opcode 0x84) is provided, for diagnostics/testing. |
    \*--------------------------------------------------------------*/

    /*--------------------------------------------------------------*\
    | Addresses (report byte [2]); vendor Cfg.ini [OPT] addresses     |
    \*--------------------------------------------------------------*/
    static const unsigned char ADDR_PROFILE   = 0xB6;
    static const unsigned char ADDR_RGBTAB    = 0xB8;
    static const unsigned char ADDR_MATRIX_A  = 0xBC;   /* SetLedMatrix first half  */
    static const unsigned char ADDR_MATRIX_B  = 0xC0;   /* SetLedMatrix second half */
    static const unsigned char ADDR_KEYMATRIX = 0xD4;   /* read-only diagnostics    */
    static const unsigned char ADDR_MUSIC     = 0x7A;

    /*--------------------------------------------------------------*\
    | Sizes and offsets                                               |
    \*--------------------------------------------------------------*/
    static const size_t PROFILE_SIZE          = 1032;   /* full feature report       */
    static const size_t PROFILE_BLOCK_SIZE    = 0x88;   /* profile payload           */
    static const size_t PROFILE_MODE_OFFSET   = 0x15;   /* effect id (LedOpt row N)  */
    static const size_t PROFILE_TABLE_BASE    = 0x24;   /* per-effect table base     */
    static const size_t PROFILE_MAX_MODE      = 0x15;   /* MODE_PER_KEY              */
    static const size_t PROFILE_HEADER        = 3;      /* report id + opcode + addr */

    static const size_t RGBTAB_PAYLOAD_OFFSET = 0x1D;   /* palette in the report     */
    static const size_t RGBTAB_EFFECTS        = 20;     /* LedOpt rows 1..20         */
    static const size_t RGBTAB_COLORS         = 7;      /* colours per effect        */
    static const size_t RGBTAB_EFFECT_STRIDE  = RGBTAB_COLORS * 3; /* 21 bytes      */
    static const size_t RGBTAB_SIZE           = RGBTAB_EFFECTS * RGBTAB_EFFECT_STRIDE;

    static const size_t LED_SLOTS             = 126;    /* physical LED frame slots  */
    static const size_t LED_FRAME_SIZE        = 4 + (LED_SLOTS * 3); /* 382          */
    static const size_t LED_MATRIX_SIZE       = 0x400;  /* SetLedMatrix payload      */
    static const size_t LED_MATRIX_OFFSET     = 8;      /* data start in the report  */

    /*--------------------------------------------------------------*\
    | Key -> physical LED slot map for the 108-key K668WBO-RGB.       |
    |                                                                 |
    | Recovered 1:1 from the Cfg.ini [KEY] "LED index" column of the  |
    | vendor installer (all 108 entries verified equal).  The vendor  |
    | driver indexes its LED frames with exactly this column           |
    | (DAT_005dcd94 in KeyboardDrv.exe).                              |
    \*--------------------------------------------------------------*/
    inline const unsigned int LED_INDEX[] =
    {     0,  12,  18,  24,  30,  36,  42,  48,  54,  60,  66,  72,  78,  84,  90,  96,   1,   7,
         13,  19,  25,  31,  37,  43,  49,  55,  61,  67,  73,  79,  85,  91,  97, 103, 109, 115,
        121,   2,   8,  14,  20,  26,  32,  38,  44,  50,  56,  62,  68,  74,  80,  86,  92,  98,
        104, 110, 116, 122,   3,   9,  15,  21,  27,  33,  39,  45,  51,  57,  63,  69,  81, 105,
        111, 117,   4,  10,  16,  22,  28,  34,  40,  46,  52,  58,  64,  82,  94, 106, 112, 118,
          5,  11,  17,  35,  53,  59,  65,  83,  89,  95, 101, 107, 119, 124, 102, 108, 114, 120 };

    inline constexpr size_t LED_INDEX_COUNT = sizeof(LED_INDEX) / sizeof(LED_INDEX[0]);

    /*--------------------------------------------------------------*\
    | Per-effect colour-mode byte values (PROFILE_TABLE_BASE + 2*N)   |
    \*--------------------------------------------------------------*/
    static const unsigned char COLOR_RANDOM   = 0x07;   /* vendor LEDParam default */
    static const unsigned char COLOR_FIXED    = 0x00;

    /*--------------------------------------------------------------*\
    | GetPsd - report 0x05 / opcode 0x81 / addr 0x00                  |
    \*--------------------------------------------------------------*/
    inline void BuildGetPsdRequest(unsigned char* out)
    {
        out[0] = REPORT_CMD;
        out[1] = OP_GET_PSD;
        out[2] = 0x00;
        out[3] = 0x00;
        out[4] = 0x00;
        out[5] = 0x00;
    }

    /*--------------------------------------------------------------*\
    | GetProfile - report 0x05 / opcode 0x83 / addr 0xB6              |
    \*--------------------------------------------------------------*/
    inline void BuildGetProfileRequest(unsigned char* out)
    {
        out[0] = REPORT_CMD;
        out[1] = OP_GET_PROFILE;
        out[2] = ADDR_PROFILE;
        out[3] = 0x00;
        out[4] = 0x00;
        out[5] = 0x00;
    }

    /*--------------------------------------------------------------*\
    | GetKeyMatrix - report 0x05 / opcode 0x84 / addr 0xD4 (read only).|
    |                                                                 |
    | Diagnostics/testing only; the matching write (opcode 0x04) is    |
    | deliberately not defined.                                       |
    \*--------------------------------------------------------------*/
    inline void BuildGetKeyMatrixRequest(unsigned char* out)
    {
        out[0] = REPORT_CMD;
        out[1] = OP_GET_KEYMATRIX;
        out[2] = ADDR_KEYMATRIX;
        out[3] = 0x00;
        out[4] = 0x00;
        out[5] = 0x00;
    }

    /*--------------------------------------------------------------*\
    | Patch only the light-mode byte, preserving every other setting. |
    \*--------------------------------------------------------------*/
    inline void BuildSetProfileMode(unsigned char* out,
                                    const unsigned char* base,
                                    unsigned char mode)
    {
        std::memcpy(out, base, PROFILE_SIZE);

        out[0] = REPORT_DATA;
        out[1] = OP_SET_PROFILE;
        out[2] = ADDR_PROFILE;
        out[PROFILE_MODE_OFFSET] = mode;
    }

    /*--------------------------------------------------------------*\
    | SetProfile - report 0x06 / opcode 0x03 / addr 0xB6.             |
    |                                                                 |
    | Copies the 0x88-byte profile block out of `base` (a full        |
    | GetProfile report) and patches the effect id plus that effect's |
    | colour-mode / speed+brightness bytes, exactly like the vendor   |
    | apply path (KeyboardDrv.exe FUN_0040d370 + FUN_004457c0).       |
    |                                                                 |
    |   mode      = LedOpt row index N (1..20 effects, 0x15 per-key)  |
    |   fixed     = true -> fixed colour (0x00), false -> random(0x07)|
    |   speed     = 1..4   (profile nibble high)                      |
    |   brightness= 0..4   (profile nibble low)                       |
    \*--------------------------------------------------------------*/
    inline void BuildSetProfile(unsigned char* out,
                                const unsigned char* base,
                                unsigned char mode,
                                bool fixed,
                                unsigned char speed,
                                unsigned char brightness)
    {
        BuildSetProfileMode(out, base, mode);

        unsigned int color_index = PROFILE_TABLE_BASE + ((unsigned int)mode * 2u);

        out[color_index]     = fixed ? COLOR_FIXED : COLOR_RANDOM;
        out[color_index + 1] = (unsigned char)((speed << 4) | (brightness & 0x0F));
    }

    /*--------------------------------------------------------------*\
    | SetLedRgbTab - report 0x06 / opcode 0x08 / addr 0xB8.           |
    |                                                                 |
    | Carries every effect's 7-colour palette, interleaved R,G,B      |
    | triples with a 21-byte stride per effect (verified against the  |
    | vendor FUN_0040d4e0 palette build).                             |
    \*--------------------------------------------------------------*/
    inline void BuildRgbTab(unsigned char* out, const unsigned char* palette)
    {
        std::memset(out, 0x00, PROFILE_SIZE);

        out[0] = REPORT_DATA;
        out[1] = OP_SET_RGBTAB;
        out[2] = ADDR_RGBTAB;
        out[3] = 0x00;
        out[4] = 0x40;

        std::memcpy(&out[RGBTAB_PAYLOAD_OFFSET], palette, RGBTAB_SIZE);
    }

    /*--------------------------------------------------------------*\
    | Fill one effect's 7 palette slots with a single colour.         |
    \*--------------------------------------------------------------*/
    inline void SetPaletteColor(unsigned char* palette, unsigned int effect,
                                unsigned char r, unsigned char g, unsigned char b)
    {
        if(effect >= RGBTAB_EFFECTS)
        {
            return;
        }

        unsigned int base = effect * RGBTAB_EFFECT_STRIDE;

        for(unsigned int slot = 0; slot < RGBTAB_COLORS; slot++)
        {
            palette[base + (slot * 3) + 0] = r;
            palette[base + (slot * 3) + 1] = g;
            palette[base + (slot * 3) + 2] = b;
        }
    }

    /*--------------------------------------------------------------*\
    | Per-key LED frame - report 0x08 / [1]=0x0A / [2]=0x7A / [3]=1.  |
    |                                                                 |
    | `slots` is a LED_SLOTS*3 byte R,G,B frame indexed by the        |
    | physical LED slot (the Cfg.ini [KEY] LED index column).         |
    \*--------------------------------------------------------------*/
    inline void BuildLedFrame(unsigned char* out, const unsigned char* slots)
    {
        std::memset(out, 0x00, LED_FRAME_SIZE);

        out[0] = REPORT_LED;
        out[1] = OP_MUSIC;
        out[2] = ADDR_MUSIC;
        out[3] = 0x01;

        std::memcpy(&out[4], slots, LED_SLOTS * 3);
    }

    inline void SetLedSlot(unsigned char* slots, unsigned int slot,
                           unsigned char r, unsigned char g, unsigned char b)
    {
        if(slot >= LED_SLOTS)
        {
            return;
        }

        slots[(slot * 3) + 0] = r;
        slots[(slot * 3) + 1] = g;
        slots[(slot * 3) + 2] = b;
    }

    /*--------------------------------------------------------------*\
    | SetLedMatrix - report 0x06 / opcode 0x09 / addr 0xBC or 0xC0.   |
    |                                                                 |
    | 0x400-byte payload; the vendor lays a single LED frame out as   |
    | three 126-byte planes (R, G, B) indexed by the physical slot.   |
    \*--------------------------------------------------------------*/
    inline void BuildLedMatrix(unsigned char* out, unsigned char addr, const unsigned char* matrix)
    {
        std::memset(out, 0x00, PROFILE_SIZE);

        out[0] = REPORT_DATA;
        out[1] = OP_SET_LEDMATRIX;
        out[2] = addr;
        out[3] = 0x00;
        out[4] = 0x40;

        std::memcpy(&out[LED_MATRIX_OFFSET], matrix, LED_MATRIX_SIZE);
    }

    inline void SetLedMatrixSlot(unsigned char* matrix, unsigned int slot,
                                 unsigned char r, unsigned char g, unsigned char b)
    {
        if(slot >= LED_SLOTS)
        {
            return;
        }

        matrix[(LED_SLOTS * 0) + slot] = r;
        matrix[(LED_SLOTS * 1) + slot] = g;
        matrix[(LED_SLOTS * 2) + slot] = b;
    }
}
