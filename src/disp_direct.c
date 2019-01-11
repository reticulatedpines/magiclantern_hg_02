

#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "font_direct.h"
#include "disp_direct.h"
#include "compiler.h"
#include "consts.h"
#include "asm.h"

extern uint32_t get_model_id();
extern uint32_t is_digic6();
extern uint32_t is_digic7();
extern uint32_t is_digic8();
extern uint32_t is_vxworks();

#define MEM(x) (*(volatile uint32_t *)(x))
#define UYVY_PACK(u,y1,v,y2) ((u) & 0xFF) | (((y1) & 0xFF) << 8) | (((v) & 0xFF) << 16) | (((y2) & 0xFF) << 24);
 
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define COERCE(x,lo,hi) MAX(MIN((x),(hi)),(lo))

#define ABS(a) ({ __typeof__ (a) _a = (a); _a > 0 ? _a : -_a; })

/* the image buffers will be made uncacheable in display_init */
static uint8_t __attribute__((aligned(4096))) __disp_framebuf_alloc[1024 * 1024];
static uint8_t __attribute__((aligned(4096))) __disp_framebuf_mirror_alloc[1024 * 1024];
static uint8_t __attribute__((aligned(4096))) __disp_yuvbuf_alloc[1024 * 1024];
static uint8_t *disp_framebuf = __disp_framebuf_alloc;
static uint8_t *disp_framebuf_mirror = __disp_framebuf_mirror_alloc;
static uint8_t *disp_yuvbuf = __disp_yuvbuf_alloc;
static uint32_t caching_bit = 0x40000000;

static int disp_yres = 480;     /* buffer Y resolution */
static int disp_yratio = 1;     /* for displays with non-square pixels, e.g. VxWorks models */
static int disp_xres = 720;     /* buffer X resolution, including any padding (this is actually pitch) */
static int disp_xpad = 0;       /* horizontal padding on the right side (to be subtracted from xres) */
static int disp_bpp  = 4;       /* 4 (16-color palette), 8 (256-color palette) or 16 (YUV422) */

uint32_t disp_direct_get_xres()
{
    /* return the usable xres, without the padding */
    return disp_xres - disp_xpad;
}

uint32_t disp_direct_get_yres()
{
    /* return logical yres (i.e. 480 for VxWorks 720x240 models) */
    return disp_yres * disp_yratio;
}

static int disp_current_buf = 0;

/* most cameras use YUV422, but some old models (e.g. 5D2) use YUV411 */
static enum { YUV422, YUV411 } yuv_mode;

static uint32_t BMP_BUF_REG_D6 = 0xD2030108;
static uint32_t BMP_BUF_REG_D7 = 0xD2060048;
static uint32_t PALETTE_REG_D6 = 0xD20139A8;

/* 5D4 is different */
const uint32_t BMP_BUF_REG_5D4 = 0xD2018228;
const uint32_t PALETTE_REG_5D4 = 0xD2018398;

/* M50 is even more different, and no longer palette-based */
const uint32_t BMP_BUF_REG_M50 = 0xD0304230;
static uint32_t * palette_uyvy = NULL;

static void disp_set_palette()
{
    // transparent
    // 1 - red
    // 2 - green
    // 3 - blue
    // 4 - cyan
    // 5 - magenta
    // 6 - yellow
    // 7 - orange
    // 8 - transparent black
    // 9 - black
    // A - gray 1
    // B - gray 2
    // C - gray 3
    // D - gray 4
    // E - gray 5
    // F - white

    uint32_t palette_pb[16] = {0x00fc0000, 0x0346de7f, 0x036dcba1, 0x031a66ea, 0x03a42280, 0x03604377, 0x03cf9a16, 0x0393b94b, 0x00000000, 0x03000000, 0x03190000, 0x033a0000, 0x03750000, 0x039c0000, 0x03c30000, 0x03eb0000};

    if (disp_bpp == 4)
    {
        for(uint32_t i = 0; i < 16; i++)
        {
            MEM(0xC0F14080 + i*4) = palette_pb[i];
        }
        MEM(0xC0F14078) = 1;
    }
    else if (disp_bpp == 8)
    {
        /* DIGIC 6 */
        /* the palette should be in uncacheable memory
         * this trick only makes a difference when running as cacheable
         * e.g. from 0x00800000 / 0x00800120 (AUTOEXEC / FIR) */
        static uint32_t __attribute__((aligned(16))) palette_alloc[16];
        uint32_t * palette = (void *)((uint32_t) palette_alloc | caching_bit);

        for(uint32_t i = 0; i < 16; i++)
        {
            palette[i] = (palette_pb[i] << 8) | 0xFF;
            uint8_t* ovuy = (uint8_t*) &palette[i];
            ovuy[1] += 128; ovuy[2] += 128;
        }

        /* possibly just a DSB SY needed; to be tested */
        sync_caches();

        MEM(PALETTE_REG_D6) = (uint32_t) palette >> 4;
        MEM(PALETTE_REG_D6-8) = 1;
    }
    else if (disp_bpp == 16)
    {
        /* DIGIC 8: some sort of YUV422 */
        /* Will emulate a palette-based half-res display, to keep things simple */

        static uint32_t __attribute__((aligned(16))) palette_alloc[16];
        uint32_t * palette = (void *)((uint32_t) palette_alloc | caching_bit);

        for(uint32_t i = 0; i < 16; i++)
        {
            uint32_t v = (palette_pb[i])       & 0xFF;
            uint32_t u = (palette_pb[i] >> 8)  & 0xFF;
            uint32_t y = (palette_pb[i] >> 16) & 0xFF;
            palette[i] = UYVY_PACK((u + 0x80) & 0xFF, y, (v + 0x80) & 0xFF, y);
        }

        palette_uyvy = palette;
    }
}

static uint32_t rgb2yuv422(int R, int G, int B)
{
    int Y = COERCE(((217) * R + (732) * G + (73) * B) / 1024, 0, 255);
    int U = COERCE(((-117) * R + (-394) * G + (512) * B) / 1024, -128, 127);
    int V = COERCE(((512) * R + (-465) * G + (-46) * B) / 1024, -128, 127);
    return UYVY_PACK(U,Y,V,Y);
}

/* low resolution, only good for smooth gradients */
static uint32_t rgb2yuv411(int R, int G, int B, uint32_t addr)
{
    int Y = COERCE(((217) * R + (732) * G + (73) * B) / 1024, 0, 255);
    int U = COERCE(((-117) * R + (-394) * G + (512) * B) / 1024, -128, 127);
    int V = COERCE(((512) * R + (-465) * G + (-46) * B) / 1024, -128, 127);

    // 4 6  8 A  0 2 
    // uYvY yYuY vYyY
    addr = addr & ~3; // multiple of 4
        
    // multiples of 12, offset 0: vYyY u
    // multiples of 12, offset 4: uYvY
    // multiples of 12, offset 8: yYuY v

    switch ((addr/4) % 3)
    {
        case 0:
            return UYVY_PACK(V,Y,Y,Y);
        case 1:
            return UYVY_PACK(U,Y,V,Y);
        case 2:
            return UYVY_PACK(Y,Y,U,Y);
    }
    
    /* unreachable */
    return 0;
}

uint8_t *disp_get_other_bmp()
{
    return (disp_current_buf ? disp_framebuf : disp_framebuf_mirror);
}

uint8_t *disp_get_active_bmp()
{
    return (disp_current_buf ? disp_framebuf_mirror : disp_framebuf);
}

static void disp_set_pixel_yuv422(uint8_t * buf, uint32_t pixnum, uint32_t color)
{
    uint32_t * buf_uyvy = (uint32_t *) buf;
    uint32_t old_uyvy = buf_uyvy[pixnum/2];
    uint32_t new_uyvy = palette_uyvy[color & 0xF];
    buf_uyvy[pixnum/2] = (pixnum % 2) ?
        (old_uyvy & 0x0000FF00) | (new_uyvy & 0xFFFF00FF) :
        (old_uyvy & 0xFF000000) | (new_uyvy & 0x00FFFFFF) ;
}

void disp_set_pixel_other(uint32_t x, uint32_t y, uint32_t color)
{
    /* some displays have non-square pixels */
    uint32_t pixnum = (y / disp_yratio) * disp_xres + x;
    uint8_t *buf = disp_get_other_bmp();
    
    switch (disp_bpp)
    {
        case 16:
            disp_set_pixel_yuv422(buf, pixnum, color);
            break;

        case 8:
            buf[pixnum] = color;
            break;

        case 4:
            buf[pixnum/2] = (x & 1)
                ? (buf[pixnum/2] & 0x0F) | ((color & 0x0F)<<4)
                : (buf[pixnum/2] & 0xF0) |  (color & 0x0F);
            break;
    }
}

void disp_set_pixel(uint32_t x, uint32_t y, uint32_t color)
{
    /* some displays have non-square pixels */
    uint32_t pixnum = (y / disp_yratio) * disp_xres + x;
    uint8_t *buf = disp_get_active_bmp();

    switch (disp_bpp)
    {
        case 16:
            disp_set_pixel_yuv422(buf, pixnum, color);
            break;

        case 8:
            buf[pixnum] = color;
            break;

        case 4:
            buf[pixnum/2] = (x & 1)
                ? (buf[pixnum/2] & 0x0F) | ((color & 0x0F)<<4)
                : (buf[pixnum/2] & 0xF0) |  (color & 0x0F);
            break;
    }
}

void disp_set_rgb_pixel(uint32_t x, uint32_t y, uint32_t R, uint32_t G, uint32_t B)
{
    /* get linear pixel number */
    uint32_t pixnum = ((y * disp_xres) + x);
    
    /* will update 32 bytes at a time */
    /* not full resolution, but simpler, and enough for smooth gradients */

    if (yuv_mode == YUV411)
    {
        /* 12 bytes per 8 pixels */
        uint32_t *ptr = (uint32_t *)&disp_yuvbuf[pixnum * 12 / 8];  
        *ptr = rgb2yuv411(R, G, B, (uint32_t)ptr);
    }
    else
    {
        /* two bytes per pixel */
        uint32_t *ptr = (uint32_t *)&disp_yuvbuf[pixnum * 2];
        *ptr = rgb2yuv422(R, G, B);
    }
}

void disp_fill_other(uint32_t color)
{
    /* build a 32 bit word */
    uint32_t val = color;
    uint8_t *buf = disp_get_other_bmp();
    
    if (disp_bpp == 4)
    {
        val |= val << 4;
    }
    val |= val << 8;
    val |= val << 16;
    
    for(int ypos = 0; ypos < disp_yres; ypos++)
    {
        /* we are writing 4 or 8 pixels at once with a 32 bit word */
        for(int xpos = 0; xpos < disp_xres; xpos += 32 / disp_bpp)
        {
            /* get linear pixel number */
            uint32_t pixnum = ((ypos * disp_xres) + xpos);
            /* two pixels per byte */
            uint32_t *ptr = (uint32_t *)&buf[pixnum * disp_bpp / 8];
            
            *ptr = val;
        }
    }
}

static uint32_t color_word(uint32_t color)
{
    /* build a 32 bit word */
    uint32_t val = color;

    if (disp_bpp == 16)
    {
        val = palette_uyvy[color & 0xF];
    }

    if (disp_bpp == 4)
    {
        val |= val << 4;
    }

    if (disp_bpp <= 8)
    {
        val |= val << 8;
        val |= val << 16;
    }

    return val;
}

void disp_fill(uint32_t color)
{
    /* build a 32 bit word */
    uint32_t val = color_word(color);
    uint8_t *buf = disp_get_active_bmp();

    for(int ypos = 0; ypos < disp_yres; ypos++)
    {
        /* we are writing 4 or 8 pixels at once with a 32 bit word */
        for(int xpos = 0; xpos < disp_xres; xpos += 32 / disp_bpp)
        {
            /* get linear pixel number */
            uint32_t pixnum = ((ypos * disp_xres) + xpos);
            /* two pixels per byte */
            uint32_t *ptr = (uint32_t *)&buf[pixnum * disp_bpp / 8];
            
            *ptr = val;
        }
    }
}

void disp_fill_yuv_gradient()
{
    /* use signed int here, because uint32_t doesn't like "-" operator */
    for(int ypos = 0; ypos < disp_yres; ypos++)
    {
        /* we are writing 2 pixels at once with a 32 bit word */
        for(int xpos = 0; xpos < disp_xres; xpos += 2)
        {
            /* ok that is making things slow.... */
            /* but we love funny patterns :) */
            disp_set_rgb_pixel(xpos, ypos, xpos/3, ypos/3, ABS(xpos-ypos)/3);
        }
    }
}

void disp_progress(uint32_t progress)
{
    uint32_t width = 480;
    uint32_t height = 20;
    uint32_t paint = progress * width / 255;
    
    for(uint32_t ypos = (disp_yres - height) / 2; ypos < (disp_yres + height) / 2; ypos++)
    {
        for(uint32_t xofs = 0; xofs < width; xofs++)
        {
            uint32_t xpos = (disp_xres - width) / 2 + xofs;
            
            if(xofs >= paint)
            {
                disp_set_pixel(xpos, ypos, COLOR_BLACK);
            }
            else
            {
                disp_set_pixel(xpos, ypos, COLOR_RED);
            }
        }
    }
    
    /* print progress in percent */
    char text[32];
    
    snprintf(text, 32, "%d%%", progress * 100 / 255);
    uint32_t x = disp_xres / 2 - 28;
    uint32_t y = (disp_yres - height) / 2 + 2;
    font_draw(&x, &y, COLOR_WHITE, 2, text);
}

void* disp_init_autodetect()
{
    /* Called right before printing the following strings:
     * "Other models\n"                     (5D2, 5D3, 60D, 500D, 70D, 7D)
     * "File(*.fir) not found\n"            (5D2, 5D3, 60D, 500D, 70D, 400D, 5D)
     * "sum check error or code modify\n"   (5D2, 60D, 500D, 7D)
     * "sum check error\n"                  (5D3, 70D)
     * "CF Read error\n"                    (5D2, 60D, 500D, 7D)
     * "Error File(*.fir)\n"                (400D, 5D)
     * ...
     */

    uint32_t a = 0, b = 0, c = 0, d = 0;
    if (is_digic7() || is_digic8())
    {
        a = find_func_called_before_string_ref_thumb("Other models\n");
        b = find_func_called_before_string_ref_thumb("File(*.fir) not found\n");
        c = find_func_called_before_string_ref_thumb("check sum error\n");
    }
    else
    {
        a = find_func_called_before_string_ref("Other models\n");
        b = find_func_called_before_string_ref("File(*.fir) not found\n");
        c = find_func_called_before_string_ref("sum check error or code modify\n");
        d = find_func_called_before_string_ref("Error File(*.fir)\n");
    }

    /* note: we will do double-checks to avoid jumping to random code */
    if (a && a == b)
    {
        /* this should cover most cameras */
        return (void*) a;
    }

    if (a && a == c)
    {
        /* this will cover 7D (maybe others) */
        return (void*) a;
    }

    if (b && b == d)
    {
        /* this will cover 400D/5D (maybe all VxWorks cameras?) */
        return (void*) b;
    }


    /* no luck */
    return 0;
}

void disp_set_buf(int buf)
{
    disp_current_buf = buf;

    if (disp_bpp == 16)
    {
        MEM(BMP_BUF_REG_M50) = (uint32_t)(buf ? disp_framebuf_mirror : disp_framebuf) & ~caching_bit;
    }
    else if (disp_bpp == 8)
    {
        if (get_model_id() == 0x349)
        {
            MEM(BMP_BUF_REG_5D4) = (uint32_t)(buf ? disp_framebuf_mirror : disp_framebuf) & ~caching_bit;
        }
        else
        {
            MEM(BMP_BUF_REG_D6) = (uint32_t)(buf ? disp_framebuf_mirror : disp_framebuf) >> 8;
        }
    }
    else if (disp_bpp == 4)
    {
        /* set frame buffer memory areas */
        MEM(0xC0F140D0) = (uint32_t)(buf ? disp_framebuf_mirror : disp_framebuf) & ~caching_bit;
        MEM(0xC0F140D4) = (uint32_t)(buf ? disp_framebuf_mirror : disp_framebuf) & ~caching_bit;
        MEM(0xC0F140E0) = (uint32_t)disp_yuvbuf & ~caching_bit;
        MEM(0xC0F140E4) = (uint32_t)disp_yuvbuf & ~caching_bit;
        
        /* trigger a display update */
        MEM(0xC0F14000) = 1;
    }
}

void disp_swap()
{
    disp_set_buf(disp_current_buf ^ 1);
}

void disp_init()
{
    if (is_digic6() || is_digic7())
    {
        disp_bpp = 8;
    }

    uint32_t id = get_model_id();
    if (id == 0x218 || id == 0x261 || id == 0x250)
    {
        /* 5D2, 50D, 7D */
        yuv_mode = YUV411;
    }

    if (id == 0x349)
    {
        /* 5D4 */
        disp_xres = 928;    /* fixme: 900 displayed */
        disp_xpad = 28;
        disp_yres = 600;
        PALETTE_REG_D6 = PALETTE_REG_5D4;
    }

    if (is_digic7())
    {
        BMP_BUF_REG_D6 = BMP_BUF_REG_D7;
    }

    if (is_digic8())
    {
        disp_bpp = 16;

        switch (get_model_id())
        {
            case 0x805: /* SX70 HS */
            case 0x801: /* SX740 HS, not sure */
                disp_xres = 640;
                break;
            case 0x412: /* M50 */
                disp_xres = 736;
                disp_xpad = 16;     /* 720 displayed */
                break;
            case 0x424: /* R */
                /* TODO */
                break;
        }
    }

    if (is_vxworks())
    {
        caching_bit = 0x10000000;
        disp_yres = 240;
        disp_yratio = 2;
    }
    
    /* make the image buffers uncacheable */
    *(uint32_t*)&disp_framebuf |= caching_bit;
    *(uint32_t*)&disp_framebuf_mirror |= caching_bit;
    *(uint32_t*)&disp_yuvbuf   |= caching_bit;

    uint32_t bmp_size = (disp_xres * disp_yres) * disp_bpp / 8;
    if (bmp_size > sizeof(__disp_framebuf_alloc))
    {
        while(1);
    }

    /* this should cover most (if not all) ML-supported cameras */
    /* and maybe most unsupported cameras as well :) */
    void (*fromutil_disp_init)(uint32_t) = disp_init_autodetect();

    /* do not continue if fromutil_disp_init could not be found */
    if (!fromutil_disp_init)
    {
        while(1);
    }

    /* this one initializes everyhting that is needed for display usage. PWM, PWR, GPIO, SIO and DISPLAY */
    fromutil_disp_init(0);

    /* we want our own palette */
    disp_set_palette();
    
    /* BMP foreground is transparent */
    disp_fill(COLOR_TRANSPARENT_BLACK);
    
    /* make a funny pattern in the YUV buffer*/
    disp_fill_yuv_gradient();
    
    disp_set_buf(0);
    
    /* from now on, everything you write on the display buffers
     * will appear on the screen without doing anything special */
}

static void memset32(uint32_t * buf, uint32_t val, size_t size)
{
    for (uint32_t i = 0; i < size / 4; i++)
    {
        buf[i] = val;
    }
}

uint32_t disp_direct_scroll_up(uint32_t height)
{
    /* some displays have non-square pixels */
    height /= disp_yratio;

    uint32_t start = (disp_xres * height) * disp_bpp / 8;
    uint32_t size = (disp_xres * (disp_yres - height)) * disp_bpp / 8;
    uint32_t color = color_word(COLOR_TRANSPARENT_BLACK);

    memcpy(disp_framebuf, &disp_framebuf[start], size);
    memset32((uint32_t*) &disp_framebuf[size], color, start);
    
    return height;
}
