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

/* CBRIEF flags:
 * 0x01 = use reversed block for cursor. (visual only)
 * 0x02 = activate BRIEF's inclusive selection. (visual only)
 * 0x04 = activate BRIEF's line-mode selection. (visual only)
 * 0x08 = activate column-mode selection. (visual only)
 */
int cbrief_flags = 0;					/* ndc: just an integer to pass parameters */
int cbrief_select_column_pos = 1;		/* ndc: column selection mode, column begins */
int cbrief_api_ver = 4;					/* ndc: cbrief module version */
extern void set_last_macro(char *);		/* ndc: copies a keystroke string to macro buffer (misc.c) */

// The abort() primitive immediately exits GriefEdit.
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

//
void cbm_redraw()
{
	jed_redraw_screen(1);
}

int cbm_scr_width()
{
	int r, c;
	jed_get_screen_size(&r, &c);
	return c;
}

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
	SLdefine_for_ifdef("CBRIEF_PATCH_V1"); /* ndc: statusline fmt, line/incl sel mode, x11 rev. cursor, set_last_macro()  */
	return 1;
}
