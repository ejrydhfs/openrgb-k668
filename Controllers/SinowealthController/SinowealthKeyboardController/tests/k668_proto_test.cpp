/*-------------------------------------------------------------------------------------------------*\
|  k668_proto_test.cpp                                                                               |
|                                                                                                   |
|  Static ("offline") verification that the OpenRGB K668WBO-RGB implementation builds exactly the    |
|  same feature reports as the vendor driver KeyboardDrv.exe V1.6.6.                                 |
|                                                                                                   |
|  Every expected value below was recovered from the vendor binary (addresses cited per check) and   |
|  is asserted against K668Protocol.h -- the same header the controller ships.  No hardware needed.  |
|                                                                                                   |
|  Build:  g++ -std=gnu++17 -I.. k668_proto_test.cpp -o k668_proto_test                              |
|  Run:    ./k668_proto_test [path/to/Cfg.ini]                                                       |
\*-------------------------------------------------------------------------------------------------*/

#include "../K668Protocol.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

static int failures = 0;
static int checks   = 0;

static void check(bool condition, const std::string& what)
{
    checks++;
    if(condition)
    {
        printf("  ok   %s\n", what.c_str());
    }
    else
    {
        failures++;
        printf("  FAIL %s\n", what.c_str());
    }
}

static bool buffer_is(const unsigned char* buf, const std::vector<unsigned char>& expected)
{
    return memcmp(buf, expected.data(), expected.size()) == 0;
}

/*-------------------------------------------------------------------------------------------------*\
| Vendor Cfg.ini [KEY] "LED index" column, transcribed independently from                              |
| Redragon_K668WBO-RGB_Setup_V1.6.6_20230612.exe (app/Cfg.ini, K1..K108, 8th field).                  |
\*-------------------------------------------------------------------------------------------------*/
static const unsigned int cfg_ini_led_index[108] =
{     0,  12,  18,  24,  30,  36,  42,  48,  54,  60,  66,  72,  78,  84,  90,  96,   1,   7,
     13,  19,  25,  31,  37,  43,  49,  55,  61,  67,  73,  79,  85,  91,  97, 103, 109, 115,
    121,   2,   8,  14,  20,  26,  32,  38,  44,  50,  56,  62,  68,  74,  80,  86,  92,  98,
    104, 110, 116, 122,   3,   9,  15,  21,  27,  33,  39,  45,  51,  57,  63,  69,  81, 105,
    111, 117,   4,  10,  16,  22,  28,  34,  40,  46,  52,  58,  64,  82,  94, 106, 112, 118,
      5,  11,  17,  35,  53,  59,  65,  83,  89,  95, 101, 107, 119, 124, 102, 108, 114, 120 };

static void test_get_requests()
{
    printf("[request reports]\n");

    unsigned char buf[6];

    K668Protocol::BuildGetPsdRequest(buf);
    check(buffer_is(buf, {0x05, 0x81, 0x00, 0x00, 0x00, 0x00}), "GetPsd          = 05 81 00 00 00 00 (vendor FUN_00445690)");

    K668Protocol::BuildGetProfileRequest(buf);
    check(buffer_is(buf, {0x05, 0x83, 0xB6, 0x00, 0x00, 0x00}), "GetProfile      = 05 83 B6 00 00 00 (vendor FUN_004458c0)");

    K668Protocol::BuildGetKeyMatrixRequest(buf);
    check(buffer_is(buf, {0x05, 0x84, 0xD4, 0x00, 0x00, 0x00}), "GetKeyMatrix    = 05 84 D4 00 00 00 (vendor FUN_00445bc0, read-only)");
}

static void test_set_profile()
{
    printf("[SetProfile - report 0x06 opcode 0x03 addr 0xB6]\n");

    unsigned char base[K668Protocol::PROFILE_SIZE];
    memset(base, 0xAA, sizeof(base));

    unsigned char out[K668Protocol::PROFILE_SIZE];

    K668Protocol::BuildSetProfile(out, base, 6, true, 2, 3);

    check(out[0] == 0x06, "report id [0] = 0x06");
    check(out[1] == 0x03, "opcode    [1] = 0x03");
    check(out[2] == 0xB6, "addr      [2] = 0xB6");
    check(out[K668Protocol::PROFILE_MODE_OFFSET] == 6, "mode byte [0x15] = effect id 6");
    check(out[0x30] == 0x00, "colour    [0x24+2*6] = 0x00 fixed (vendor LEDParam semantics)");
    check(out[0x31] == 0x23, "speed/brt [0x25+2*6] = 0x23 (speed 2 << 4 | brightness 3)");
    check(out[0x10] == 0xAA, "unrelated profile byte preserved (0xAA)");

    K668Protocol::BuildSetProfile(out, base, 6, false, 4, 4);
    check(out[0x30] == 0x07, "random colour [0x24+2*6] = 0x07 (vendor LEDParam default)");
    check(out[0x31] == 0x44, "speed/brt [0x25+2*6] = 0x44 (speed 4 | brightness 4)");

    unsigned char mode_only[K668Protocol::PROFILE_SIZE];
    K668Protocol::BuildSetProfileMode(mode_only, base, 0x15);
    check(mode_only[0] == 0x06 && mode_only[1] == 0x03 && mode_only[2] == 0xB6, "per-key profile header correct");
    check(mode_only[K668Protocol::PROFILE_MODE_OFFSET] == 0x15, "per-key light-mode byte = 0x15 (vendor MODE_PER_KEY)");
    check(mode_only[0x30] == 0xAA, "per-key profile leaves the effect table untouched");
}

static void test_rgbtab()
{
    printf("[SetLedRgbTab - report 0x06 opcode 0x08 addr 0xB8]\n");

    unsigned char palette[K668Protocol::RGBTAB_SIZE];
    memset(palette, 0x00, sizeof(palette));

    K668Protocol::SetPaletteColor(palette, 6 - 1, 0x00, 0xFF, 0x00);

    unsigned char out[K668Protocol::PROFILE_SIZE];
    K668Protocol::BuildRgbTab(out, palette);

    check(out[0] == 0x06, "report id [0] = 0x06");
    check(out[1] == 0x08, "opcode    [1] = 0x08");
    check(out[2] == 0xB8, "addr      [2] = 0xB8");
    check(out[3] == 0x00, "byte      [3] = 0x00");
    check(out[4] == 0x40, "byte      [4] = 0x40");
    check(K668Protocol::RGBTAB_PAYLOAD_OFFSET == 0x1D, "palette payload offset = 0x1D (vendor FUN_00445ce0)");
    check(K668Protocol::RGBTAB_EFFECT_STRIDE == 21, "effect stride = 21 = 7 colours * 3 (vendor FUN_0040d4e0)");

    unsigned int effect5 = 5 * K668Protocol::RGBTAB_EFFECT_STRIDE;
    bool effect_ok = true;
    for(unsigned int slot = 0; slot < 7; slot++)
    {
        unsigned int o = K668Protocol::RGBTAB_PAYLOAD_OFFSET + effect5 + (slot * 3);
        if(out[o + 0] != 0x00 || out[o + 1] != 0xFF || out[o + 2] != 0x00)
        {
            effect_ok = false;
        }
    }
    check(effect_ok, "effect 6 all 7 slots = R,G,B interleaved (0x00,0xFF,0x00)");

    check(out[K668Protocol::RGBTAB_PAYLOAD_OFFSET + (4 * 21)] == 0x00, "neighbouring effect palette untouched");
}

static void test_led_frame()
{
    printf("[per-key LED frame - report 0x08 / 0x0A 0x7A 0x01]\n");

    unsigned char slots[K668Protocol::LED_SLOTS * 3];
    memset(slots, 0x00, sizeof(slots));

    K668Protocol::SetLedSlot(slots, K668Protocol::LED_INDEX[0], 0x11, 0x22, 0x33);
    K668Protocol::SetLedSlot(slots, K668Protocol::LED_INDEX[1], 0x44, 0x55, 0x66);

    unsigned char frame[K668Protocol::LED_FRAME_SIZE];
    K668Protocol::BuildLedFrame(frame, slots);

    check(K668Protocol::LED_FRAME_SIZE == 382, "frame length = 382 = 4 + 126*3 (vendor FUN_004455a0)");
    check(frame[0] == 0x08, "report id [0] = 0x08");
    check(frame[1] == 0x0A, "byte      [1] = 0x0A");
    check(frame[2] == 0x7A, "addr      [2] = 0x7A");
    check(frame[3] == 0x01, "byte      [3] = 0x01");

    check(frame[4 + (0 * 3) + 0] == 0x11 && frame[4 + (0 * 3) + 1] == 0x22 && frame[4 + (0 * 3) + 2] == 0x33,
          "key 0 (LED slot 0) = R,G,B at 4+slot*3");
    check(frame[4 + (12 * 3) + 0] == 0x44 && frame[4 + (12 * 3) + 1] == 0x55 && frame[4 + (12 * 3) + 2] == 0x66,
          "key 1 (LED slot 12) = R,G,B at 4+slot*3");
}

static void test_led_matrix()
{
    printf("[SetLedMatrix - report 0x06 opcode 0x09 addr 0xBC/0xC0]\n");

    unsigned char matrix[K668Protocol::LED_MATRIX_SIZE];
    memset(matrix, 0x00, sizeof(matrix));

    K668Protocol::SetLedMatrixSlot(matrix, 5, 0xAA, 0xBB, 0xCC);

    unsigned char out[K668Protocol::PROFILE_SIZE];
    K668Protocol::BuildLedMatrix(out, K668Protocol::ADDR_MATRIX_A, matrix);

    check(out[0] == 0x06, "report id [0] = 0x06");
    check(out[1] == 0x09, "opcode    [1] = 0x09");
    check(out[2] == 0xBC, "addr      [2] = 0xBC (vendor DAT_005e0278)");
    check(out[3] == 0x00 && out[4] == 0x40, "bytes [3],[4] = 0x00,0x40");
    check(K668Protocol::LED_MATRIX_OFFSET == 8, "matrix data offset = 8 (vendor FUN_004459f0)");
    check(out[8 + 5] == 0xAA && out[8 + 126 + 5] == 0xBB && out[8 + 252 + 5] == 0xCC,
          "slot 5 written to the three 126-byte R/G/B planes");

    K668Protocol::BuildLedMatrix(out, K668Protocol::ADDR_MATRIX_B, matrix);
    check(out[2] == 0xC0, "second half addr = 0xC0 (vendor calls SetLedMatrix twice, +4)");
}

static bool parse_cfg_ini(const std::string& path, std::vector<unsigned int>& out)
{
    std::ifstream file(path.c_str());

    if(!file.is_open())
    {
        return false;
    }

    std::string line;

    while(std::getline(file, line))
    {
        if(line.size() < 2 || line[0] != 'K')
        {
            continue;
        }

        std::vector<std::string> fields;
        std::stringstream ss(line.substr(line.find('=') + 1));
        std::string field;

        while(std::getline(ss, field, ','))
        {
            fields.push_back(field);
        }

        if(fields.size() >= 9)
        {
            out.push_back((unsigned int)strtoul(fields[7].c_str(), nullptr, 0));
        }
    }

    return out.size() == K668Protocol::LED_INDEX_COUNT;
}

static void test_key_index(const char* cfg_path)
{
    printf("[key -> LED slot map vs Cfg.ini]\n");

    check(K668Protocol::LED_INDEX_COUNT == 108, "108 keys mapped");

    bool same = true;
    for(size_t i = 0; i < K668Protocol::LED_INDEX_COUNT; i++)
    {
        if(K668Protocol::LED_INDEX[i] != cfg_ini_led_index[i])
        {
            same = false;
            printf("        mismatch at key %zu: ship=%u cfg=%u\n", i, K668Protocol::LED_INDEX[i], cfg_ini_led_index[i]);
        }
    }
    check(same, "K668Protocol::LED_INDEX == transcribed Cfg.ini LED-index column (108/108)");

    if(cfg_path != nullptr)
    {
        std::vector<unsigned int> parsed;

        if(parse_cfg_ini(cfg_path, parsed))
        {
            same = true;
            for(size_t i = 0; i < parsed.size(); i++)
            {
                if(K668Protocol::LED_INDEX[i] != parsed[i])
                {
                    same = false;
                }
            }
            check(same, std::string("K668Protocol::LED_INDEX == live ") + cfg_path + " (108/108)");
        }
        else
        {
            printf("  skip  could not parse %s\n", cfg_path);
        }
    }
}

int main(int argc, char** argv)
{
    const char* cfg_path = (argc > 1) ? argv[1] : nullptr;

    printf("K668WBO-RGB protocol static verification vs vendor KeyboardDrv.exe V1.6.6\n");
    printf("=========================================================================\n");

    test_get_requests();
    test_set_profile();
    test_rgbtab();
    test_led_frame();
    test_led_matrix();
    test_key_index(cfg_path);

    printf("=========================================================================\n");
    printf("%d checks, %d failures\n", checks, failures);

    return failures == 0 ? 0 : 1;
}
