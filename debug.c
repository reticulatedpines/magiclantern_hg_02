/** \file
 * Magic Lantern debugging and reverse engineering code
 */
#include "dryos.h"
#include "bmp.h"
#include "tasks.h"
#include "debug.h"
#include "menu.h"
#include "property.h"
#include "config.h"
//#include "lua.h"

#if 0
void
display_full_hd(
	void *			priv,
	int			x,
	int			y,
	int			selected
)
{
	unsigned * gui_struct = (void*) 0x3548;
	unsigned * ps_struct = (void*) 0x11bb8;
	// disp=0x08 == 3 == 1080p, 5 == vga
	// vid=0x0c == 0
	// hdmi=0x10 == 3, 5 == vga
	// c4=0xc4 == 0, 1 == vga

	unsigned (*GetDisplayType)(void) = (void*) 0xFF863590;

	uint32_t * lv_struct = (void*) 0x37fc;

	bmp_printf(
		FONT_MED,
		x, y,
		//23456789012
#if 0
		"disp=%x vid=%x hdmi=%x c4=%x ps=%x type=%x/%x",
		gui_struct[0x08 / 4],
		gui_struct[0x0C / 4],
		gui_struct[0x10 / 4],
		gui_struct[0xC4 / 4],
		ps_struct[0x230 / 4],
		hdmi_config.disp_type,
		hdmi_config.off_0x0c
#else
		"mvr %d/%x",
		mvr_struct->fps,
		mvr_struct->bit_rate
#endif
	);

	bmp_hexdump( FONT_SMALL, 300, 400,
		(void*) 0x7b40,
		64
	);
}


void enable_full_hd( void * priv )
{
	call("mvrSetPrintMovieLog");
#if 0
	if( mvr_struct->fps == 0 )
		mvr_struct->fps = 30;
	uint8_t bitrate = 60;
	//call( "mvrSetBitRate", &bitrate );
	void (*mvrSetBitRate)( uint8_t * bitrate ) = (void*) 0xff84f990;
	mvrSetBitRate( &bitrate );
#endif
	void (*mvrSetQScale)( int8_t * ) = (void*) 0xff9715e0;
	int8_t scale = -30;
	mvrSetQScale( &scale );
	return;

	DebugMsg( DM_MAGIC, 3, "Attempting to set HDMI to full HD" );

#if 1
	thunk ChangeHDMIOutputSizeToFULLHD = (thunk) 0xFFA96260;
	//void (*SetDisplayType)(int) = (void*) 0xFF8620DC;
	void (*SetDisplayType)(int) = (void*) 0xFFB4835C;

	//SetDisplayType( 0 );
	unsigned * gui_struct = (void*) 0x3548;
	unsigned * ps_struct = (void*) 0x11bb8;
	ChangeHDMIOutputSizeToFULLHD();
	//gui_struct[ 0xC4/4 ] = 0; // not resized?
	//ps_struct[ 0x230/4 ] = 6; // who knows

	DebugMsg( DM_MAGIC, 3, "Full HD done?" );
#else
	// This will select the image on the full screen.
	// but not change the output resolution
	void (*lv_output_device)( const char ** arg ) = (void*) 0xFF833218;
	const char * full_hd = "1080full";
	lv_output_device( &full_hd );
#endif
}
#endif


void call_dispcheck( void * priv )
{
	call( "dispcheck" );
}

static uint8_t * const mvr_config_struct = (uint8_t*) 0x86b0;
static uint32_t * const jpcore_struct = (void*) 0x8fbc;

static int16_t qscale = -8;

void set_vbr( void * priv )
{
	void (*mvrFixQScale)(uint16_t *) = (void*) 0xff9905d4; // 2.0.8
	void (*mvrSetDefQScale)(int16_t *) = (void*) 0xff990188; // 2.0.8

	uint16_t param=1; // select fixed rate
	mvrFixQScale(&param);

	qscale -= 1;
	if (qscale < -16)
		qscale = 16;

	mvrSetDefQScale(&qscale);
}

static void
print_vbr(
	void *			priv,
	int			x,
	int			y,
	int			selected
)
{
	bmp_printf(
		selected ? MENU_FONT_SEL : MENU_FONT,
		x, y,
		//23456789012
		"QScale:     %s%3d",
		qscale < 0 ? "-" : "+",
		qscale < 0 ? -qscale : qscale
	);
}



void call_setprintmovielog( void * priv )
{
	gui_stop_menu();

	//int i = 1;
	//void (*mvrSetPrintMovieLog)(int *val) = (void*) 0xFF9905f4; // 2.0.8;
	//mvrSetPrintMovieLog(&i);

	uint8_t bps = 100;
	void (*mvrSetBitRate)(uint8_t *val) = (void*) 0xFF852FAC; // 2.0.8;
	void (*mvrFixQScale)(uint8_t *val) = (void*) 0xFF9905D4; // 2.0.8;
	void (*mvrSetQScale)(uint8_t *val) = (void*) 0xFF9900B8; // 2.0.8;
	uint32_t (*div)(uint32_t x, uint32_t y) = (void*) 0xFFC47ABC; // 2.0.8
	
	uint8_t * mvr_struct = ((uint8_t**) 0x1eec)[1];

	int y = 40;
	bmp_printf( FONT_MED, 0, y += 17,
		"mvr.bitrate=%d mvr.fps=%d frame=%d",
		*(int16_t*)(mvr_struct + 0x1d8),
		*(int16_t*)(mvr_struct + 0x178),
		*(int32_t*)(mvr_struct + 0x150)
	);
	bmp_printf( FONT_MED, 0, y += 17,
		"LimitQScale(L=%d, H=%d)",
		*(int16_t*)(mvr_config_struct + 0xc),
		*(int16_t*)(mvr_config_struct + 0xe)
	);
	bmp_printf( FONT_MED, 0, y += 17,
		"DefQscale(%d)",
		*(int16_t*)(mvr_config_struct + 0x8)
	);
	bmp_printf( FONT_MED, 0, y += 17,
		"TimeConst(%d)",
		*(int16_t*)(mvr_config_struct + 0x10)
	);
	bmp_printf( FONT_MED, 0, y += 17,
		"FullHDOptSize(I_OPT=%d, P_OPT=%d)",
		*(int32_t*)(mvr_struct + 0x14),
		*(int32_t*)(mvr_struct + 0x18)
	);
	uint8_t * gop_struct = (uint8_t*) 0x875c;
	bmp_printf( FONT_MED, 0, y += 17,
		"GopOpt(%d, %d, %d, %d, %d)",
		*(int32_t*)(gop_struct + 0x0),
		*(int32_t*)(gop_struct + 0x4),
		*(int32_t*)(gop_struct + 0x8),
		*(int32_t*)(gop_struct + 0xc),
		*(int32_t*)(gop_struct + 0x10)
	);
	bmp_printf( FONT_MED, 0, y += 17,
		"D_FULLHD(H=%d, L=%d)",
		*(int32_t*)(mvr_struct + 0x1c),
		*(int32_t*)(mvr_struct + 0x20)
	);
	bmp_printf( FONT_MED, 0, y += 17,
		"DefDB(A=%d, B=%d)",
		*(int32_t*)(mvr_config_struct + 0x1c),
		*(int32_t*)(mvr_config_struct + 0x20)
	);
	bmp_printf( FONT_MED, 0, y += 17,
		"FixQscale(%s)",
		*(int16_t*)(mvr_config_struct + 0x2) ? "FIX" : "VBR"
	);

	return;

/*
	mvrSetBitRate(&bps);

	bmp_hexdump( FONT_SMALL, 10, 300,
		(void*) mvr_struct[1],
		0x200
	);
*/
}


static PROP_INT( PROP_EFIC_TEMP, efic_temp );


PROP_HANDLER( PROP_HDMI_CHANGE_CODE )
{
	DebugMsg( DM_MAGIC, 3, "They try to set code to %d", buf[0] );
	return prop_cleanup( token, property );
}


static void
efic_temp_display(
	void *			priv,
	int			x,
	int			y,
	int			selected
)
{
	bmp_printf(
		selected ? MENU_FONT_SEL : MENU_FONT,
		x, y,
		//23456789012
		"CMOS temp:  %d",
		efic_temp
	);
}

#if 0
static void
mvr_time_const_display(
	void *			priv,
	int			x,
	int			y,
	int			selected
)
{
	uint8_t * mvr_struct = (void*) 0x1ed4;
	uint8_t * mvr_hdr = *(void**)( 0x1ed4 + 4 );
	struct state_object ** const mvr_state_object = (void*) 0x68a4;

	struct tm now;
	LoadCalendarFromRTC( &now );

	bmp_printf(
		FONT_MED, // selected ? MENU_FONT_SEL : MENU_FONT,
		x, y,
		"Date %4d/%2d/%2d %02d:%02d:%02d",
		now.tm_year + 1900,
		now.tm_mon,
		now.tm_mday,
		now.tm_hour,
		now.tm_min,
		now.tm_sec
	);
}
#endif


#if 0
static void
mvr_time_const_select( void * priv )
{
/*
	void (*mvr_set_time_const)(int *) = (void*) 0xff9716cc;
	void (*mvr_setd_fullhd)(int *) = (void*) 0xff9716cc;
	//int args[] = { 640, 480 };
	//DebugMsg( DM_MAGIC, 3, "calling mvr_setd_fullhd %d %d", args[0], args[1] );
	//mvr_setd_fullhd( args );

	uint32_t buf[] = { 8 };
	//prop_request_change( 0x207000c, buf, sizeof(buf) );
	void (*lv_magnify)( int * ) = (void*) 0xff83359c;
	lv_magnify( buf );
	void (*mvrSetBitRate)( int * ) = (void*) 0xff84f990;
	//int rate = 24;
	//mvrSetBitRate( &rate );
	mvr_struct->is_vga	= 0;
	mvr_struct->width	= 1920;
	mvr_struct->height	= 1080;
	mvr_struct->fps		= 24;
*/

	uint8_t * mvr_hdr = *(void**)( 0x1ed4 + 4 );
	*(unsigned *)( mvr_hdr + 0x60 ) = 2400;
	*(unsigned *)( mvr_hdr + 0x64 ) = 100;
	*(unsigned *)( mvr_hdr + 0x68 ) = 24;
}
#endif


CONFIG_INT( "debug.draw-prop",		draw_prop, 0 );

static void
draw_prop_select( void * priv )
{
	draw_prop = !draw_prop;
}


static void
save_config( void * priv )
{
	config_save_file( global_config, "A:/magiclantern.cfg" );
}


struct menu_entry debug_menus[] = {
	{
		.display	= efic_temp_display,
	},
	{
		.priv		= "Save config",
		.select		= save_config,
		.display	= menu_print,
	},
	{
		.select		= set_vbr,
		.display	= print_vbr,
	},
	{
		.priv		= "Print mvr_config",
		.select		= call_setprintmovielog,
		.display	= menu_print,
	},
	{
		.priv		= "Draw palette",
		.select		= bmp_draw_palette,
		.display	= menu_print,
	},
	{
		.priv		= "Toggle draw_prop",
		.select		= draw_prop_select,
		.display	= menu_print,
	},
	{
		.priv		= "Dump dmlog",
		.select		= (void*) dumpf,
		.display	= menu_print,
	},
	{
		.priv		= "Screenshot",
		.select		= call_dispcheck,
		.display	= menu_print,
	},

#if 0
	{
		.display	= mvr_time_const_display,
		.select		= mvr_time_const_select,
	},
#endif
};


static void * debug_token;

static void
debug_token_handler(
	void *			token,
	void *			arg1,
	void *			arg2,
	void *			arg3
)
{
	debug_token = token;
	DebugMsg( DM_MAGIC, 3, "token %08x arg=%08x %08x %08x",
		(unsigned) token,
		(unsigned) arg1,
		(unsigned) arg2,
		(unsigned) arg3
	);
}

static void *
debug_property_handler(
	unsigned		property,
	void *			UNUSED( priv ),
	void *			buf,
	unsigned		len
)
{
	const uint32_t * const addr = buf;

	DebugMsg( DM_MAGIC, 3, "Prop %08x: %d: %08x %08x %08x %08x",
		property,
		len,
		len > 0x00 ? addr[0] : 0,
		len > 0x04 ? addr[1] : 0,
		len > 0x08 ? addr[2] : 0,
		len > 0x0c ? addr[3] : 0
	);
		
	if( !draw_prop )
		goto ack;

	const unsigned x = 80;
	static unsigned y = 32;

	bmp_printf( FONT_SMALL, x, y,
		"%08x %04x: %08x %08x %08x %08x %08x %08x",
		property,
		len,
		len > 0x00 ? addr[0] : 0,
		len > 0x04 ? addr[1] : 0,
		len > 0x08 ? addr[2] : 0,
		len > 0x0c ? addr[3] : 0,
		len > 0x10 ? addr[4] : 0,
		len > 0x14 ? addr[5] : 0
	);
	y += font_small.height;

	bmp_fill( COLOR_RED, x, y, 100, 1 );

	if( y > 400 )
		y = 32;

ack:
	return prop_cleanup( debug_token, property );
}



#define num_properties 8192
unsigned property_list[ num_properties ];


void
debug_init( void )
{
	draw_prop = 0;

#if 1
	unsigned i, j, k;
	unsigned actual_num_properties = 0;
	//for( i=0 ; i<=0x8 ; i+=8 )
	i = 8;
	{
		for( j=0 ; j<=0x8 ; j++ )
		{
			for( k=0 ; k<0x40 ; k++ )
			{
				unsigned prop = 0
					| (i << 28) 
					| (j << 16)
					| (k <<  0);

				if( prop != 0x80030014
				&&  prop != 0x80030015
				&&  prop != 0x80050000
				&&  prop != 0x80050004
				&&  prop != 0x80050005
				&&  prop != 0x80050010
				&&  prop != 0x8005000f
				) {
					property_list[ actual_num_properties++ ] = prop;
				}

/*
				if( i !=0 && j != 0 )
				property_list[ actual_num_properties++ ] = 0
					| (2 << 24) 
					| (j << 16)
					| (k <<  0);
*/

				if( actual_num_properties > num_properties )
					goto thats_all;
			}
		}
	}

thats_all:
#else
	int actual_num_properties = 0;
	int i;
	for( i=0 ;i<0xFF ; i++ )
		property_list[actual_num_properties++] = 
			0x02010000 | i;
#endif

	prop_register_slave(
		property_list,
		actual_num_properties,
		debug_property_handler,
		(void*) 0xdeadbeef,
		(void*) debug_token_handler // wrong signature
	);

	menu_add( "Debug", debug_menus, COUNT(debug_menus) );
}

CONFIG_INT( "debug.timed-dump",		timed_dump, 0 );

static void
dump_task( void * unused )
{
	//lua_State * L = lua_open();

	// It was too early to turn these down in debug_init().
	// Only record important events for the display and face detect
	dm_set_store_level( DM_DISP, 4 );
	dm_set_store_level( DM_LVFD, 4 );
	dm_set_store_level( DM_LVCFG, 4 );
	dm_set_store_level( DM_LVCDEV, 4 );
	dm_set_store_level( DM_LV, 4 );
	dm_set_store_level( DM_RSC, 4 );
	dm_set_store_level( 0, 4 ); // catch all?

	// increase jpcore debugging (breaks liveview?)
	//dm_set_store_level( 0x15, 2 );
	//dm_set_store_level( 0x2f, 0x16 );

	if( timed_dump == 0 )
		return;

	int sec = timed_dump;

	DebugMsg( DM_MAGIC, 3, "%s: Will do debug dump in %d sec",
		__func__,
		sec
	);

	while( sec-- )
	{
		//bmp_printf( FONT_SMALL, 600, 400, "dump %2d", sec );
		msleep( 1000 );
	}

	DebugMsg( DM_MAGIC, 3, "%s: calling dumpf", __func__ );

	unsigned long *adr = (unsigned long*)0x7cfc; // 0x5bb4 on 550d 108;
	unsigned long *adr2 = (unsigned long*)*(adr + 2);
	//unsigned char *pa =( unsigned char *)adr2;

	int i;
	for (i=0; i<35; i++)
		DebugMsg( DM_MAGIC, 3, "@ 0x%08X = %08X", adr2+i, *(adr2+i) );

	dumpf();
}


TASK_CREATE( "dump_task", dump_task, 0, 0x1f, 0x1000 );


CONFIG_INT( "debug.timed-start",	timed_start, 0 );

static void
movie_start( void )
{
	int sec = timed_start;
	if( sec == 0 )
		return;

	const int x = 320;
	const int y = 150;

	while( --sec > 0 )
	{
		msleep( 1000 );
		bmp_printf(
			FONT(
				FONT_HUGE,
				sec > 4 ? COLOR_WHITE : COLOR_RED,
				0
			),
			x, y,
			"T-%d",
			sec
		);
	}

	bmp_printf( FONT(FONT_HUGE,COLOR_WHITE,0), x, y, "GO!" );

	call( "MovieStart" );

	msleep( 1000 );

	bmp_printf( FONT(FONT_HUGE,COLOR_WHITE,0), x, y, "   " );
}

TASK_CREATE( "movie_start", movie_start, 0, 0x1f, 0x1000 );
