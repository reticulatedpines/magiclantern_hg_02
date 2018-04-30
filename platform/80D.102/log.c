/* DIGIC 6 logging experiments
 * based on dm-spy-experiments  code */

#include "dryos.h"

/* comment out one of these to see anything in QEMU */
/* on real hardware, more interrupts are expected */
static char* isr_names[0x200] = {
    [0x0A]  = "Timer",
    [0x1B]  = "Timer",
    [0x10]  = "HPTimer",
    [0x28]  = "HPTimer",
    [0x2E]  = "Term-RD",
    [0x3A]  = "Term-WR",
    [0x15D] = "Term-RD",
    [0x16D] = "Term-WR",
    [0x34]  = "SIO1",
    [0x35]  = "SIO2",
    [0x36]  = "SIO3",
    [0x147] = "SIO3",
    [0x50]  = "MREQ",
  //[0x2A]  = "MREQ",
    [0x2f]  = "DMA1",
    [0x74]  = "DMA2",
    [0x75]  = "DMA3",
    [0x76]  = "DMA4",
    [0xFE]  = "SF",
    [0xEE]  = "SD",
    [0xBE]  = "SDDMA",
    [0x13E] = "XDMAC",
    [0x14E] = "XDMAC",
    [0x15E] = "XDMAC",
    [0x16E] = "XDMAC",

    /* sorry, couldn't get dynamic code
     * to run without stack overflows...
     * (list autogenerated from qemu)
     */
    [0x58] = "EDMAC#0",
    [0x59] = "EDMAC#1",
    [0x5A] = "EDMAC#2",
    [0x5B] = "EDMAC#3",
    [0x5C] = "EDMAC#4",
    [0x6D] = "EDMAC#5",
    [0xC0] = "EDMAC#6",
    [0x5D] = "EDMAC#8",
    [0x5E] = "EDMAC#9",
    [0x5F] = "EDMAC#10",
    [0x6E] = "EDMAC#11",
    [0xC1] = "EDMAC#12",
    [0xC8] = "EDMAC#13",
    [0xF9] = "EDMAC#16",
    [0x83] = "EDMAC#17",
    [0x8A] = "EDMAC#18",
    [0xCA] = "EDMAC#19",
    [0xCB] = "EDMAC#20",
    [0xD2] = "EDMAC#21",
    [0xD3] = "EDMAC#22",
    [0x8B] = "EDMAC#24",
    [0x92] = "EDMAC#25",
    [0xE2] = "EDMAC#26",
    [0x95] = "EDMAC#27",
    [0x96] = "EDMAC#28",
    [0x97] = "EDMAC#29",
    [0xDA] = "EDMAC#32",
    [0xDB] = "EDMAC#33",
    [0x9D] = "EDMAC#40",
    [0x9E] = "EDMAC#41",
    [0x9F] = "EDMAC#42",
    [0xA5] = "EDMAC#43",
};

static void pre_isr_log(uint32_t isr)
{
#ifdef CONFIG_DIGIC_VI
    extern uint32_t isr_table_handler[];
    extern uint32_t isr_table_param[];
    uint32_t handler = isr_table_handler[2 * isr];
    uint32_t arg     = isr_table_param  [2 * isr];
#endif

    /* log only unknown interrupts */
    char* name = isr_names[isr & 0x1FF];
    if (name) return;

    DryosDebugMsg(0, 15, "INT-%03Xh %X(%X)", isr, handler, arg);
}

static void post_isr_log(uint32_t isr)
{
}

extern void (*pre_isr_hook)();
extern void (*post_isr_hook)();

static void mpu_decode(char* in, char* out, int max_len)
{
    int len = 0;
    int size = in[0];

    /* print each byte as hex */
    for (char* c = in; c < in + size; c++)
    {
        len += snprintf(out+len, max_len-len, "%02x ", *c);
    }
    
    /* trim the last space */
    if (len) out[len-1] = 0;
}

extern int (*mpu_recv_cbr)(char * buf, int size);
extern int __attribute__((long_call)) mpu_recv(char * buf);

static int mpu_recv_log(char * buf)
{
    int size = buf[-1];
    char msg[256];
    mpu_decode(buf, msg, sizeof(msg));
    DryosDebugMsg(0, 15, "*** mpu_recv(%02x %s)", size, msg);

    /* call the original */
    return mpu_recv(buf);
}

void log_start()
{
    //pre_isr_hook = &pre_isr_log;
    //post_isr_hook = &post_isr_log;

    /* wait for InitializeIntercom to complete
     * then install our own hook quickly
     * this assumes Canon's init_task is already running */
    while (!mpu_recv_cbr)
    {
        msleep(10);
    }
    mpu_recv_cbr = &mpu_recv_log;

    dm_set_store_level(255, 1);
    DryosDebugMsg(0, 15, "Logging started.");

    sync_caches();
}

void log_finish()
{
    pre_isr_hook = 0;
    post_isr_hook = 0;
    sync_caches();
    dm_set_store_level(255, 15);
    DryosDebugMsg(0, 15, "Logging finished.");
}