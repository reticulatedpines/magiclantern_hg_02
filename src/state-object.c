/** 
 * Experiments on state objects 
 * 
 * http://magiclantern.wikia.com/wiki/StateObjects
 * 
 **/

#include "dryos.h"
#include "bmp.h"
#include "state-object.h"
#include "property.h"

#ifdef CONFIG_7D
#include "cache_hacks.h"
#endif

#ifdef CONFIG_550D
#define DISPLAY_STATE DISPLAY_STATEOBJ
#define INPUT_ENABLE_IMAGE_PHYSICAL_SCREEN_PARAMETER 19
#define MOVREC_STATE (*(struct state_object **)0x5B34)
#define LV_STATE (*(struct state_object **)0x4B74)
#define LVCAE_STATE (*(struct state_object **)0x51E4)
#endif

#ifdef CONFIG_60D
#define DISPLAY_STATE DISPLAY_STATEOBJ
#define INPUT_ENABLE_IMAGE_PHYSICAL_SCREEN_PARAMETER 20
#define VOI_STATE (*(struct state_object **)0x269D8)
#define EVF_STATE (*(struct state_object **)0x4ff8)
#define MOVREC_STATE (*(struct state_object **)0x5A40)
#endif

#ifdef CONFIG_600D
#define DISPLAY_STATE DISPLAY_STATEOBJ
#define INPUT_ENABLE_IMAGE_PHYSICAL_SCREEN_PARAMETER 20
#define EVF_STATE (*(struct state_object **)0x51CC)
#define MOVREC_STATE (*(struct state_object **)0x5EF8)
#endif

#ifdef CONFIG_5D2
#define MOVREC_STATE (*(struct state_object **)0x7C90)
#define LV_STATE (*(struct state_object **)0x4528)
#endif

#ifdef CONFIG_500D
#define MOVREC_STATE (*(struct state_object **)0x7AF4)
#define LV_STATE (*(struct state_object **)0x4804)
#endif

#ifdef CONFIG_50D
#define MOVREC_STATE (*(struct state_object **)0x6CDC)
#define LV_STATE (*(struct state_object **)0x4580)
#endif

#ifdef CONFIG_5D3
#define DISPLAY_STATE DISPLAY_STATEOBJ
#define INPUT_ENABLE_IMAGE_PHYSICAL_SCREEN_PARAMETER 20
#define EVF_STATE (*(struct state_object **)0x2600c)
#define MOVREC_STATE (*(struct state_object **)0x27850)
#endif

#ifdef CONFIG_1100D
#define DISPLAY_STATE DISPLAY_STATEOBJ
#define INPUT_ENABLE_IMAGE_PHYSICAL_SCREEN_PARAMETER 20
#define EVF_STATE (*(struct state_object **)0x4C34)
#define MOVREC_STATE (*(struct state_object **)0x5720)
#endif

#ifdef CONFIG_5DC
// we need to detect halfshutter press from EMState.
#define EMState (*(struct state_object **)0x4f24)
#endif

/*
static void stateobj_matrix_copy_for_patching(struct state_object * stateobj)
{
    int size = stateobj->max_inputs * stateobj->max_states * sizeof(struct state_transition);
    struct state_transition * new_matrix = (struct state_transition *)AllocateMemory(size);
    memcpy(new_matrix, stateobj->state_matrix, size);
    stateobj->state_matrix = new_matrix;
}

static void stateobj_install_hook(struct state_object * stateobj, int input, int state, void* newfunc)
{
    if ((uint32_t)(stateobj->state_matrix) & 0xFF000000) // that's in ROM, make a copy to allow patching
        stateobj_matrix_copy_for_patching(stateobj);
    STATE_FUNC(stateobj,input,state) = newfunc;
}
*/

static void* hd_buf_dst = 0;
static void* hd_buf_size = 0;
static int hd_buf_memcpy_flag = 0;
void sync_hd_buf_memcpy(void* dst, int size)
{
    hd_buf_dst = dst;
    hd_buf_size = size;
    hd_buf_memcpy_flag = 1;
    while (hd_buf_memcpy_flag) msleep(20);
}

static void vsync_func() // called once per frame.. in theory :)
{
    #if !defined(CONFIG_60D) && !defined(CONFIG_600D) && !defined(CONFIG_1100D) && !defined(CONFIG_5D3) // for those cameras, it's called from a different spot of the evf state object
    hdr_step();
    #endif
    
    digic_iso_step();
    image_effects_step();
    
    if (hd_buf_memcpy_flag)
    {
        memcpy(hd_buf_dst, CACHEABLE(YUV422_HD_BUFFER_DMA_ADDR), hd_buf_size);
        hd_buf_memcpy_flag = 0;
    }
    
    //~ display_shake_step();
}

#ifdef CONFIG_550D
int display_is_on_550D = 0;
int get_display_is_on_550D() { return display_is_on_550D; }
#endif

int (*StateTransition)(void*,int,int,int,int) = 0;
static int stateobj_spy(struct state_object * self, int x, int input, int z, int t)
{
    int old_state = self->current_state;

#ifdef CONFIG_550D
    if (self == DISPLAY_STATE && old_state != 0 && input == 0) // TurnOffDisplay_action
        display_is_on_550D = 0;
#endif

// sync ML overlay tools (especially Magic Zoom) with LiveView
// this is tricky...
#ifdef CONFIG_5D3
    if (self == DISPLAY_STATE && input == INPUT_ENABLE_IMAGE_PHYSICAL_SCREEN_PARAMETER)
        lv_vsync_signal();
#elif defined(CONFIG_5D2)
    if (self == LV_STATE)//&& old_state == 4)
    {
        //~ lv_vsync_signal();
    }
#elif defined(CONFIG_60D)
    if (self == DISPLAY_STATE && input >= 19)
        lv_vsync_signal();
#endif

// sync display filters (for these, we need to redirect display buffers
    #ifdef DISPLAY_STATE
    if (self == DISPLAY_STATE && input == INPUT_ENABLE_IMAGE_PHYSICAL_SCREEN_PARAMETER)
    {
        hdr_kill_flicker();
        display_filter_lv_vsync(old_state, x, input, z, t);
        digic_zoom_overlay_step();
    }
    #endif
    
#ifdef CONFIG_5D2
    if (self == LV_STATE && old_state == 2 && input == 2) // lvVdInterrupt
    {
        display_filter_lv_vsync(old_state, x, input, z, t);
    }
#endif

    int ans = StateTransition(self, x, input, z, t);

#ifdef CONFIG_550D
    if (self == DISPLAY_STATE)
        display_is_on_550D = (self->current_state == 1);
#endif


// sync digic functions (like overriding ISO or image effects)

    #if defined(CONFIG_5D2) || defined(CONFIG_50D) || defined(CONFIG_500D)
    if (self == LV_STATE && input==4 && old_state==4) // AJ_ResetPSave_n_WB_n_LVREC_MVR_EV_EXPOSURESTARTED => perfect sync for digic on 5D2 :)
    #endif

    #ifdef CONFIG_550D
    if (self == LV_STATE && input==5 && old_state == 5) // SYNC_GetEngineResource => perfect sync for digic :)
    #endif

    #if defined(CONFIG_60D) || defined(CONFIG_600D) || defined(CONFIG_1100D) || defined(CONFIG_5D3)
    if (self == EVF_STATE && input == 5 && old_state == 5) // evfReadOutDoneInterrupt => perfect sync for digic :)
    #endif
    
#ifndef CONFIG_5DC
        vsync_func();
#endif

    #if defined(CONFIG_60D) || defined(CONFIG_600D) || defined(CONFIG_1100D) || defined(CONFIG_5D3) // exception for overriding ISO
    if (self == EVF_STATE && input == 4 && old_state == 5) // evfSetParamInterrupt
        hdr_step();
    #endif

#ifdef CONFIG_5DC
    if (z == 0x0) { fake_simple_button(BGMT_PRESS_HALFSHUTTER); }
    if (z == 0xB) { fake_simple_button(BGMT_UNPRESS_HALFSHUTTER); }
#endif
    
    return ans;
}

static int stateobj_start_spy(struct state_object * stateobj)
{
  StateTransition = (void *)stateobj->StateTransition_maybe;
  stateobj->StateTransition_maybe = (void *)stateobj_spy;
  return 0; //not used currently
}

static void state_init(void* unused)
{
    #ifdef DISPLAY_STATE
        stateobj_start_spy(DISPLAY_STATE);
    #endif
    #ifdef LV_STATE
        stateobj_start_spy(LV_STATE);
    #endif
    //~ #ifdef MOVREC_STATE
        //~ stateobj_start_spy(MOVREC_STATE);
    //~ #endif
    #ifdef EVF_STATE
        stateobj_start_spy(EVF_STATE);
    #endif
    
    #ifdef EMState
        stateobj_start_spy(EMState);
    #endif
    
    #ifdef CONFIG_7D
        /* will work, but this is LV only - not recorded */
        // cache_fake(0xFF10D2F4, BL_INSTR(0xFF10D2F4, &vsync_func), TYPE_ICACHE);
    #endif
}

INIT_FUNC("state_init", state_init);
