/*
 * BRIEF compatibility module
 * nicholas christopoulos (nereus@freemail.gr)
 *
 * screen.c -- error message color, selection lines/columns inclusive or not colorize
 * intrin.c -- add cbrief_slang_init()
 * misc.c -- add set_last_macro()
*/

#include "config.h"
#include "jed-feat.h"

#include <stdio.h>
#include <slang.h>
#include "jdmacros.h"

#ifdef HAVE_STDLIB_H
# include <stdlib.h>
#endif
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#include <string.h>

#if JED_HAS_LINE_ATTRIBUTES
# include "lineattr.h"
#endif

#if JED_HAS_SUBPROCESSES
# include "jprocess.h"
#endif

#if defined(__DECC) && defined(VMS)
# include <unixlib.h>
#endif

#if defined(__MSDOS__) || defined(__os2__) || defined(__WIN32__)
# include <process.h>
#endif

#include "buffer.h"
#include "keymap.h"
#include "file.h"
#include "ins.h"
#include "ledit.h"
#include "screen.h"
#include "window.h"
#include "display.h"
#include "search.h"
#include "misc.h"
#include "replace.h"
#include "paste.h"
#include "sysdep.h"
#include "cmds.h"
#include "text.h"
#include "abbrev.h"
#include "indent.h"
#include "colors.h"

#define BUFSZ		1024
#define BIGBUFSZ	4096
#define MIN(a,b) 	((a<b)?a:b)
#define MAX(a,b) 	((a>b)?a:b)

/* CBRIEF flags:
 * 0x01 = use reversed block for cursor. (visual only)
 * 0x02 = activate BRIEF's inclusive selection. (visual only)
 * 0x04 = activate BRIEF's line-mode selection. (visual only)
 * 0x08 = activate column-mode selection. (visual only)
 */
int cbrief_flags = 0;					/* ndc: just an integer to pass parameters */
int cbrief_select_column_pos = 1;		/* ndc: column selection mode, column begins */
int cbrief_api_ver = 5;					/* ndc: cbrief module version */
extern void set_last_macro(char *);		/* ndc: copies a keystroke string to macro buffer (misc.c) */

/*
 * Keymap for our User Interface
 */
static SLKeyMap_List_Type *CBMenuKMap = NULL;
static SLKeymap_Function_Type CBMenuKMapFuncs [] = { {NULL, NULL} };

static int cbrief_init_menu_keymap()
{
	int ch;
	unsigned char simple[3];

	if ( (CBMenuKMap = SLang_create_keymap ("cbriefpopup", NULL)) == NULL )
		return -1;
	CBMenuKMap->functions = NULL; // CBMenuKMapFuncs;
	simple[2] = '\0';
	for ( ch = 0; ch < 256; ch ++ ) {
		// raw & ctrl
		simple[1] =  0; simple[0] = (unsigned char) ch;
		SLkm_define_keysym(simple, ch, CBMenuKMap);
		}
	for ( ch = 'a'; ch <= 'z'; ch ++ ) {
		// alt
		simple[0] = 27; simple[1] = (unsigned char) ch;
		SLkm_define_keysym(simple, 0x100 | ch, CBMenuKMap);
		}
//	SLkm_define_keysym("\033\033", 0x1B, CBMenuKMap);
#ifndef IBMPC_SYSTEM
	SLkm_define_keysym("\033[A",  0x201, CBMenuKMap);	// up
	SLkm_define_keysym("\033OA",  0x201, CBMenuKMap);
	SLkm_define_keysym("\033[B",  0x202, CBMenuKMap);	// down
	SLkm_define_keysym("\033OB",  0x202, CBMenuKMap);
	SLkm_define_keysym("\033[D",  0x203, CBMenuKMap);	// left
	SLkm_define_keysym("\033OD",  0x203, CBMenuKMap);
	SLkm_define_keysym("\033[C",  0x204, CBMenuKMap);	// right
	SLkm_define_keysym("\033OC",  0x204, CBMenuKMap);
	SLkm_define_keysym("\033[1~", 0x205, CBMenuKMap);	// home
	SLkm_define_keysym("\033[4~", 0x206, CBMenuKMap);	// end
	SLkm_define_keysym("\033[5~", 0x207, CBMenuKMap);	// pgup
	SLkm_define_keysym("\033[6~", 0x208, CBMenuKMap);	// pgdn
	SLkm_define_keysym("\033[2~", 0x209, CBMenuKMap);	// insert
	SLkm_define_keysym("\033[3~", 0x20A, CBMenuKMap);	// delete
#ifdef __unix__
	SLkm_define_keysym("^(ku)",   0x201, CBMenuKMap);
	SLkm_define_keysym("^(kd)",   0x202, CBMenuKMap);
	SLkm_define_keysym("^(kl)",   0x203, CBMenuKMap);
	SLkm_define_keysym("^(kr)",   0x204, CBMenuKMap);
	SLkm_define_keysym("^(kh)",   0x205, CBMenuKMap);
	SLkm_define_keysym("^(@7)",   0x206, CBMenuKMap);
	SLkm_define_keysym("^(kP)",   0x207, CBMenuKMap);
	SLkm_define_keysym("^(kN)",   0x208, CBMenuKMap);
	SLkm_define_keysym("^(kA)",   0x209, CBMenuKMap);
	SLkm_define_keysym("^(kD)",   0x20A, CBMenuKMap);
#endif
#else				       /* IBMPC_SYSTEM */
#include "doskeys.h"

	SLkm_define_key (PC_UP,     0x201, CBMenuKMap);
	SLkm_define_key (PC_UP1,    0x201, CBMenuKMap);
	SLkm_define_key (PC_DN,     0x202, CBMenuKMap);
	SLkm_define_key (PC_DN1,    0x202, CBMenuKMap);
	SLkm_define_key (PC_LT,     0x203, CBMenuKMap);
	SLkm_define_key (PC_LT1,    0x203, CBMenuKMap);
	SLkm_define_key (PC_RT,     0x204, CBMenuKMap);
	SLkm_define_key (PC_RT1,    0x204, CBMenuKMap);
	SLkm_define_key (PC_HOME,   0x205, CBMenuKMap);
	SLkm_define_key (PC_HOME1,  0x205, CBMenuKMap);
	SLkm_define_key (PC_END,    0x206, CBMenuKMap);
	SLkm_define_key (PC_END1,   0x206, CBMenuKMap);
	SLkm_define_key (PC_PGUP,   0x207, CBMenuKMap);
	SLkm_define_key (PC_PGUP1,  0x207, CBMenuKMap);
	SLkm_define_key (PC_PGDN,   0x208, CBMenuKMap);
	SLkm_define_key (PC_PGDN1,  0x208, CBMenuKMap);
	SLkm_define_key (PC_INS,    0x209, CBMenuKMap);
	SLkm_define_key (PC_INS1,   0x209, CBMenuKMap);
	SLkm_define_key (PC_DEL,    0x20A, CBMenuKMap);
	SLkm_define_key (PC_DEL1,   0x20A, CBMenuKMap);
#endif
	return 0;
}

static SLang_Key_Type *cbrief_tui_getkey()
{
	int ch;

	if ( Executing_Keyboard_Macro || (Read_This_Character != NULL) )
		return SLang_do_key(CBMenuKMap, jed_getkey);
	ch = jed_getkey ();
	if ( SLKeyBoard_Quit || SLang_get_error () )
		return NULL;
	if ( ch == 27 ) {
		int tsecs = 1;
		if ( input_pending(&tsecs) == 0 )
			ungetkey(&ch); // second ESC
		}
	ungetkey(&ch);
	return SLang_do_key(CBMenuKMap, jed_getkey);
}

/*
 * waits and returns the key code
 * 0-255 = character or control code
 * 0x100 = ALT mask
 * 0x200 = Special keys mask
 */
static int cbm_getkey()
{
	SLang_Key_Type *key;

	if ( (key = cbrief_tui_getkey()) == NULL )
		return -1;
	return key->f.keysym;
}

/* --- keymap configuration ends here --- */

// The abort() primitive immediately exit.
void cbm_abort()
{
	abort();
}

// try to save screen to run terminal command that may or may not uses curses
void cbm_save_screen()
{
	SLsmg_suspend_smg();
//	SLsmg_cls();
}

// restore screen from previous save_screen
void cbm_restore_screen()
{
	SLsmg_resume_smg();
	jed_redraw_screen(0);
}

// full redraw the jed
void cbm_redraw()
{
	jed_redraw_screen(1);
}

// returns the width of the screen
int cbm_scr_width()
{
	int r, c;
	jed_get_screen_size(&r, &c);
	return c;
}

// returns the heigh of the screen
int cbm_scr_height()
{
	int r, c;
	jed_get_screen_size(&r, &c);
	return r;
}

// the inq_debug() primitive returns an integer representing the debug flags which are currently in effect
// no debug capabilities are supported yet
int cbm_inq_debug()
{
	return 0;
}

// msgbox
static int cbm_msgbox(int flags, const char *title, const char *fmt, ...)
{
	char	*msg = (char *) malloc(BUFSZ);
	int		rows, cols, x, y, w, h, i, count, maxc, rv, alloc;
	char	**list, *elem, *ps, *p;
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(msg, BUFSZ, fmt, ap);
	va_end(ap);

	// create an array of options (break source at \n)
	alloc = 128;
	list = (char **) malloc(sizeof(char *) * (alloc + 1));
	count = 0;
	ps = p = msg;
	while ( *p ) {
		if ( *p == '\n' ) {
			elem = (char *) malloc((p - ps) + 1);
			strncpy(elem, ps, p - ps);
			elem[p - ps] = '\0';
			list[count] = elem;
			count ++;
			if ( count == alloc ) {
				alloc += 128;
				list = (char **) realloc(list, sizeof(char *) * (alloc + 1));
				}
			ps = p + 1;
			}
		p ++;
		}
	if ( *ps && count < 128 ) {
		list[count] = strdup(ps);
		count ++;
		}

	// calculate the size and the width
	maxc = 0;
	if ( title ) {
		if ( *title )
			maxc = strlen(title) + 2;
		}
	for ( i = 0; i < count; i ++ )
		maxc = MAX(maxc, strlen(list[i]));

	// calculate size and position
	jed_get_screen_size(&rows, &cols);
	w = MIN(maxc,  cols - 8);
	h = MIN(count, rows - 4);
	y = rows / 2 - h / 2;
	x = cols / 2 - w / 2;

	rv = -1;
	do {
		// draw the whole box
		SLsmg_set_color(JMENU_POPUP_COLOR);
		SLsmg_draw_box(y - 1, x - 2, h + 2, w + 4);
		if ( title ) {
			if ( *title ) {
				SLsmg_draw_object(y - 1, x, SLSMG_RTEE_CHAR);
				SLsmg_gotorc(y - 1, x + 1);
				SLsmg_write_string((char *) title);
				SLsmg_draw_object(y - 1, x + strlen(title) + 1, SLSMG_LTEE_CHAR);
				}
			}
		for ( i = 0; i < h; i ++ ) {
			SLsmg_gotorc(y + i, x - 1);
			SLsmg_write_nstring(NULL, w + 2);
			}
		for ( i = 0; i < count && i < h; i ++ ) {
			SLsmg_gotorc(y + i, x);
			SLsmg_write_nstring(list[i], w);
			}
		SLsmg_set_color(JNORMAL_COLOR);
		SLsmg_refresh();

		// input
		switch ( cbm_getkey() ) {
		case 0x0D: rv = 1; break;
		case -1: case 'q': case 'Q':
		case 0x1B: rv = 0; break;
			}
		} while ( rv == -1 );
	cbm_redraw();

	// clean up
	for ( i = 0; i < count; i ++ )
		free(list[i]);
	free(list);
	free(msg);
	return rv;
}

// interface for S-Lang
static int cbm_msgbox_sl(const char *title, const char *msg, int *flags)
{ return cbm_msgbox(*flags, title, "%s", msg); }

// popup menu
// the 'source' is the list of items separated by '\n'
// the 'defsel' is the preselected item (default is 0)
// returns the index of the selected item or -1 for cancel
// flags 0x00 = normal
//       0x01 = allow delete item

#define PUM_FIX_SCROLL() {\
		if ( selected >= count ) selected = count - 1;\
		if ( selected >= start_pos + menu_items ) start_pos = (selected+1) - menu_items;\
		}

static int cbm_popup_menu(int flags, const char *source, int defsel,
				const char *title, const char *footer,
				int (*hook)(const char*,int,int,int), const char *hookpar)
{
	int 	count, alloc, maxc, start_pos = 0, selected = 0;
	int 	i, rows, cols, loop_exit, key;
	int		menu_x, menu_y, menu_w, menu_h, menu_items;
	char	**list, *elem;
	const char *ps, *p;

	// create an array of options (break source at \n)
	alloc = 128;
	list = (char **) malloc(sizeof(char *) * (alloc + 1));
	count = 0;
	ps = p = source;
	while ( *p ) {
		if ( *p == '\n' ) {
			elem = (char *) malloc((p - ps) + 1);
			strncpy(elem, ps, p - ps);
			elem[p - ps] = '\0';
			list[count] = elem;
			count ++;
			if ( count == alloc ) {
				alloc += 128;
				list = (char **) realloc(list, sizeof(char *) * (alloc + 1));
				}
			ps = p + 1;
			}
		p ++;
		}
	if ( *ps ) {
		list[count] = strdup(ps);
		count ++;
		}
	for ( i = count; i < alloc; i ++ )
		list[i] = NULL;

	// calculate the size and position
	jed_get_screen_size(&rows, &cols);
	maxc = 0;
	if ( title )	maxc = strlen(title) + 2;
	if ( footer )	MAX(maxc, strlen(footer) + 2);
	for ( i = 0; i < count; i ++ )
		maxc = MAX(maxc, strlen(list[i]));

	// calculate menu width, height and position
	menu_w = MIN(maxc,  cols - 8);
	menu_h = MIN(count, rows - 6);
	menu_x = cols / 2 - menu_w / 2;
//	menu_y = (rows - menu_h) - 2;
	menu_y = ((rows - menu_h) - 2) / 2;
	menu_items = menu_h;

	// menu loop
	selected  = 0;
	start_pos = 0;
	if ( defsel > 0 && defsel < count ) {
		selected = defsel;
		PUM_FIX_SCROLL();
		}
	loop_exit = 0;
	while ( loop_exit == 0 ) {
		// draw static items...
		// this was outside of the loop but needed for redraw
		SLsmg_set_color(JMENU_POPUP_COLOR);
		SLsmg_draw_box(menu_y - 1, menu_x - 2, menu_h + 2, menu_w + 4);
		if ( title ) {
			if ( *title ) {
				SLsmg_draw_object(menu_y - 1, menu_x, SLSMG_RTEE_CHAR);
				SLsmg_gotorc(menu_y - 1, menu_x + 1);
				SLsmg_write_string((char *) title);
				SLsmg_draw_object(menu_y - 1, menu_x + strlen(title) + 1, SLSMG_LTEE_CHAR);
				}
			}
		if ( footer ) {
			if ( *footer ) {
				SLsmg_gotorc(menu_y + menu_h, menu_x + menu_w - strlen(footer));
				SLsmg_write_string((char *) footer);
				}
			}
		for ( i = 0; i < menu_items; i ++ ) {
			SLsmg_gotorc(menu_y + i, menu_x - 1);
			SLsmg_write_nstring(NULL, menu_w + 2);
			}

		// draw menu
		for ( i = 0; i < menu_items; i ++ ) {
			if ( i + start_pos == selected )
				SLsmg_set_color(JMENU_SELECTION_COLOR);
			else
				SLsmg_set_color(JMENU_POPUP_COLOR);
			SLsmg_gotorc(menu_y + i, menu_x);
			if ( start_pos + i < count )
				SLsmg_write_nstring(list[start_pos + i], menu_w);
			else
				SLsmg_write_nstring(NULL, menu_w);
			}
		SLsmg_set_color(JNORMAL_COLOR);
 		SLsmg_refresh();

		// get key
		key = cbm_getkey();
		switch ( key ) {
		case 0x201: // up
			if ( selected > 0 ) {
				selected --;
				if ( start_pos > selected )
					start_pos = selected;
				}
			break;
		case 0x202: // down
			selected ++;
			PUM_FIX_SCROLL();
			break;
		case 0x205: // home
			selected = start_pos = 0;
			break;
		case 0x206: // end
			selected = count - 1;
			PUM_FIX_SCROLL();
			break;
		case 0x207: // pgup
 			selected -= menu_items;
 			if ( selected < 0 ) selected = 0;
 			start_pos -= menu_items;
			if ( start_pos < 0 ) start_pos = 0;
			break; 
		case 0x208: // pgdn
 			selected += menu_items;
			PUM_FIX_SCROLL();
			break;
		case 0x20A: case 'd': case 'D': // delete
			if ( flags & 0x01 ) {
				if ( hook ) {
					int r = hook(hookpar, selected, 'd', key);
					if ( r < 0 ) {
						selected = r;
						goto pum_exit;
						}
					}
				free(list[selected]);
				list[selected] = NULL;
				count --;
	 			if ( selected < count ) {
					for ( i = selected; i < count; i ++ )
						list[i] = list[i+1];
					list[count] = NULL;
					}
				PUM_FIX_SCROLL();
				}
			break;
		case 0x0D: // ENTER (select)
			if ( hook )
				hook(hookpar, selected, 's', key);
 			loop_exit ++;
			break;
		case 0x1B: case -1: case 'q': case 'Q': // cancel
			selected = -1;
			loop_exit ++;
			break;
		default:
			if ( hook ) {
				int r = hook(hookpar, selected, 'k', key);
				if ( r < 0 ) {
					selected = r;
					goto pum_exit;
					}
				}
			}
		} // while

	// cleanup and return
pum_exit:
	for ( i = 0; i < count; i ++ )
		free(list[i]);
	free(list);
	return selected;
}

// interface for S-Lang
static int cbm_popup_menu_sl(const char *source, int *defsel)
{ return cbm_popup_menu(0, source, *defsel, NULL, NULL, NULL, NULL); }

// interface for S-Lang
int pum_hook(const char *slfunc, int item, int code, int key)
{
	SLang_Name_Type *slhook = NULL;
	int	rv = 0;
	
	if ( *slfunc )
		slhook = SLang_get_function((char *)slfunc);
	else
		slhook = NULL;
	if ( slhook ) {
		SLang_push_integer(item);
		SLang_push_integer(code);
		SLang_push_integer(key);
		SLexecute_function(slhook);
		if (SLang_get_error () == 0) SLang_pop_integer(&rv);
		}
	return rv;
}

static int cbm_popup_menu5_sl(int *flags, const char *source, int *defsel, const char *title, const char *footer, const char *hookfunc)
{
	if ( *hookfunc )
		return cbm_popup_menu(*flags, source, *defsel, title, footer, pum_hook, hookfunc);
	return cbm_popup_menu(*flags, source, *defsel, title, footer, NULL, NULL);
}

/*
 * setup S-Lang interface
 */
static SLang_Intrin_Fun_Type CBRIEF_Intrinsics [] = {
	MAKE_INTRINSIC_S("set_last_macro", set_last_macro, VOID_TYPE),
	MAKE_INTRINSIC_0("abort", cbm_abort, VOID_TYPE),
	MAKE_INTRINSIC_0("inq_debug", cbm_inq_debug, INT_TYPE),
	MAKE_INTRINSIC_0("save_screen", cbm_save_screen, VOID_TYPE),
	MAKE_INTRINSIC_0("restore_screen", cbm_restore_screen, VOID_TYPE),
	MAKE_INTRINSIC_0("redraw_screen", cbm_redraw, VOID_TYPE),
	MAKE_INTRINSIC_0("touch_screen", touch_screen, VOID_TYPE),
	MAKE_INTRINSIC_0("screen_width", cbm_scr_width, INT_TYPE),
	MAKE_INTRINSIC_0("screen_height", cbm_scr_height, INT_TYPE),
	MAKE_INTRINSIC_0("cgetkey",     cbm_getkey,        INT_TYPE),
	MAKE_INTRINSIC_SI("popup_menu", cbm_popup_menu_sl, INT_TYPE),
	MAKE_INTRINSIC_6("popup_menu5", cbm_popup_menu5_sl, INT_TYPE, INT_TYPE, STRING_TYPE, INT_TYPE, STRING_TYPE, STRING_TYPE, STRING_TYPE),
	MAKE_INTRINSIC_SSI("msgbox",    cbm_msgbox_sl,     INT_TYPE),
	SLANG_END_INTRIN_FUN_TABLE
	};

static SLang_Intrin_Var_Type CBRIEF_Variables [] = {
	MAKE_VARIABLE("CBRIEF_API",       &cbrief_api_ver,	INT_TYPE, 0),
	MAKE_VARIABLE("CBRIEF_FLAGS",     &cbrief_flags,	INT_TYPE, 0),
	MAKE_VARIABLE("CBRIEF_SELCOLPOS", &cbrief_select_column_pos, INT_TYPE, 0),
	MAKE_VARIABLE(NULL, NULL, 0, 0)
	};

/*
 *	initialize slang members
 */
int cbrief_slang_init()
{
	if ( SLadd_intrin_fun_table(CBRIEF_Intrinsics, NULL) == -1 )	return 0;
	if ( SLadd_intrin_var_table(CBRIEF_Variables,  NULL) == -1 )	return 0;
	if ( cbrief_init_menu_keymap() == -1 )                          return 0;
	SLdefine_for_ifdef("CBRIEF_PATCH_V1"); /* ndc: statusline fmt, line/incl sel mode, x11 rev. cursor, set_last_macro()  */
	SLdefine_for_ifdef("CBRIEF_PATCH_V5"); /* ndc: tui */
	return 1;
}
