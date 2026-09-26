/*------------------------------------------*\
|  SinowealthKeyboardController.h            |
|                                            |
|  Definitions and types for Sinowealth      |
|  Keyboard, Hopefully generic, this was     |
|  made spefically for FL eSports F11 KB     |
|                                            |
|  Dmitri Kalinichenko (Dima-Kal) 23/06/2021 |
\*------------------------------------------*/

#include "RGBController.h"
#include "K668Protocol.h"
#include <vector>
#include <hidapi.h>

#pragma once

enum
{
    MODE_OFF                    = 0x0,
    MODE_STATIC                 = 0x1,
    MODE_RESPIRE                = 0x2,
    MODE_RAINBOW                = 0x3,
    MODE_FLASH_AWAY             = 0x4,
    MODE_RAINDROPS              = 0x5,
    MODE_RAINBOW_WHEEL          = 0x6,
    MODE_RIPPLES_SHINING        = 0x7,
    MODE_STARS_TWINKLE          = 0x8,
    MODE_SHADOW_DISAPPEAR       = 0x9,
    MODE_RETRO_SNAKE            = 0xA,
    MODE_NEON_STREAM            = 0xB,
    MODE_REACTION               = 0xC,
    MODE_SINE_WAVE              = 0xD,
    MODE_RETINUE_SCANNING       = 0xE,
    MODE_ROTATING_WINDMILL      = 0xF,
    MODE_COLORFUL_WATERFALL     = 0x10,
    MODE_BLOSSOMING             = 0x11,
    MODE_ROTATING_STORM         = 0x12,
    MODE_COLLISION              = 0x13,
    MODE_PERFECT                = 0x14,
    MODE_PER_KEY                = 0x15
};

/*-------------------------------------------------------------------------------------------------*\
| Redragon K668WBO-RGB hardware light-effect ids.                                                    |
|                                                                                                   |
| These are the values the vendor profile writes into the light-mode byte of the 123-byte profile    |
| block (offset 0x15 of the report buffer).  The value is the "LedOpt" ROW INDEX n (1..22) from      |
| Cfg.ini/KB.ini -- it is NOT the row's second column (the app's local effect-preview id, which the  |
| firmware rejects: values >22 draw nothing, which is why an earlier revision was dark).             |
|                                                                                                   |
| The names come from the matching vendor UI string table (Text/en/text.xml "tc_led_modeN").  The    |
| K668WBO-RGB Cfg.ini has LedMask=0xC0000, which hides effect indices 18 and 19 (rows 19 "Collision" |
| and 20 "Perfect") from the vendor UI; those two are therefore not exposed here either.  Rows 4 (Flash|
| Away), 7 (Ripples Shining), 9 (Shadow Disappear) and 12 (Reaction) are the key-press-triggered       |
| transient effects; they are exposed as modes and the firmware drives them on key press.            |
\*-------------------------------------------------------------------------------------------------*/
enum
{
    K668_MODE_RESPIRE           = 0x02,     /* tc_led_mode2  "Respire"            */
    K668_MODE_RAINBOW           = 0x03,     /* tc_led_mode3  "Rainbow"            */
    K668_MODE_FLASH_AWAY        = 0x04,     /* tc_led_mode4  "Flash_away"         */
    K668_MODE_RAIN_DROPS        = 0x05,     /* tc_led_mode5  "Raindrops"          */
    K668_MODE_RAINBOW_WHEEL     = 0x06,     /* tc_led_mode6  "Rainbow_wheel"      */
    K668_MODE_RIPPLES_SHINING   = 0x07,     /* tc_led_mode7  "Ripples_shining"    */
    K668_MODE_STARS_TWINKLE     = 0x08,     /* tc_led_mode8  "Stars_twinkle"      */
    K668_MODE_SHADOW_DISAPPEAR  = 0x09,     /* tc_led_mode9  "Shadow_disappear"   */
    K668_MODE_RETRO_SNAKE       = 0x0A,     /* tc_led_mode10 "Retro_snake"        */
    K668_MODE_NEON_STREAM       = 0x0B,     /* tc_led_mode11 "Neon_stream"        */
    K668_MODE_REACTION          = 0x0C,     /* tc_led_mode12 "Reaction"           */
    K668_MODE_SINE_WAVE         = 0x0D,     /* tc_led_mode13 "Sine_wave"          */
    K668_MODE_RETINUE_SCANNING  = 0x0E,     /* tc_led_mode14 "Retinue scanning"   */
    K668_MODE_ROTATING_WINDMILL = 0x0F,     /* tc_led_mode15 "Rotating windmill"  */
    K668_MODE_COLORFUL_WATERFALL= 0x10,     /* tc_led_mode16 "Colorful waterfall" */
    K668_MODE_BLOSSOMING        = 0x11,     /* tc_led_mode17 "Blossoming"         */
    K668_MODE_ROTATING_STORM    = 0x12,     /* tc_led_mode18 "Rotating storm"     */
    K668_MODE_COLLISION         = 0x13,     /* tc_led_mode19 "Collision"          */
    K668_MODE_PERFECT           = 0x14,     /* tc_led_mode20 "Perfect"            */
};

/*-------------------------------------------------------------------------------------------------*\
| Alias the protocol module's constants under the names this controller historically used.           |
| The wire format itself lives in K668Protocol.h so it can be unit tested byte-for-byte.             |
\*-------------------------------------------------------------------------------------------------*/
#define K668_PROFILE_SIZE           K668Protocol::PROFILE_SIZE
#define K668_PROFILE_MODE_OFFSET    K668Protocol::PROFILE_MODE_OFFSET
#define K668_PROFILE_TABLE_BASE     K668Protocol::PROFILE_TABLE_BASE
#define K668_PROFILE_MAX_MODE       K668Protocol::PROFILE_MAX_MODE

#define K668_RGBTAB_ADDRESS         K668Protocol::ADDR_RGBTAB
#define K668_RGBTAB_PAYLOAD_OFFSET  K668Protocol::RGBTAB_PAYLOAD_OFFSET
#define K668_RGBTAB_MAX_EFFECTS     K668Protocol::RGBTAB_EFFECTS
#define K668_RGBTAB_COLORS_PER_EFFECT K668Protocol::RGBTAB_COLORS
#define K668_RGBTAB_EFFECT_STRIDE   K668Protocol::RGBTAB_EFFECT_STRIDE

enum
{
    SPEED_SLOW                  = 0x12,
    SPEED_NORMAL                = 0x22,
    SPEED_FASTER                = 0x32,
    SPEED_FASTEST               = 0x42,
};

enum
{
    BRIGHTNESS_OFF              = 0x0,
    BRIGHTNESS_QUARTER          = 0x1,
    BRIGHTNESS_HALF             = 0x2,
    BRIGHTNESS_THREE_QUARTERS   = 0x3,
    BRIGHTNESS_FULL             = 0x4
};


class SinowealthKeyboardController
{
public:
    SinowealthKeyboardController(hid_device* dev_cmd_handle, hid_device* dev_data_handle, char *_path, std::string dev_name); //RGB, Command, path
    SinowealthKeyboardController(hid_device* dev_handle, std::string _path, std::string dev_name, bool _k668_layout); // single-handle variant for Redragon K668WBO-RGB
    ~SinowealthKeyboardController();

    bool            GetK668Layout();

    unsigned int    GetLEDCount();
    std::string     GetLocation();
    std::string     GetName();
    std::string     GetSerialString();
    unsigned char   GetCurrentMode();

    void            SetStaticColor(RGBColor* color_buf);
    void            SetMode(unsigned char mode, unsigned char brightness, unsigned char speed, unsigned char color_mode, RGBColor k668_color = 0);
    void            SetLEDsDirect(std::vector<RGBColor> colors);

    /*--------------------------------------------------------------*\
    | Redragon K668WBO-RGB protocol surface (report 0x05/0x06/0x08).  |
    \*--------------------------------------------------------------*/
    bool            GetProfile();               /* read the 0x88-byte profile block  */
    bool            ReadFirmwareInfo();          /* GetPsd handshake + version bytes  */
    bool            GetKeyMatrix();              /* read the 0x400-byte key matrix     */

private:
    bool            SendK668Report(unsigned char* report, size_t length);
    bool            GetK668Report(unsigned char report_id, unsigned char* buffer, size_t length);
    void            CacheK668Profile();
    void            SetK668DirectMode();
    void            SetK668RgbTab(unsigned char mode, RGBColor color);
    hid_device*     dev_cmd;
    hid_device*     dev_data;
    device_type     type;
    unsigned int    led_count;
    unsigned char   current_mode;
    unsigned char   current_speed;
    std::string     location;
    std::string     name;
    bool            k668_layout;
    bool            single_handle;
    bool            k668_direct_mode;
    bool            k668_profile_valid;
    unsigned char   k668_profile[K668_PROFILE_SIZE];
    unsigned char   k668_rgbtab[K668Protocol::RGBTAB_SIZE];
    bool            k668_rgbtab_valid;
    unsigned char   k668_psd[6];
    unsigned char   k668_keymatrix[K668Protocol::LED_MATRIX_SIZE];
};
