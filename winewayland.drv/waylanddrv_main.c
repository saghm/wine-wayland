/*
 * WAYLANDDRV initialization code
 *
 * Copyright 1998 Patrik Stridvall
 * Copyright 2000 Alexandre Julliard
 * Copyright 2020 varmd
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include "config.h"


#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rpc.h>

#ifdef HAVE_SYS_TIME_H
# include <sys/time.h>
#endif
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#define DESKTOP_CLASS_ATOM ((LPCWSTR)MAKEINTATOM(32769))
#define DESKTOP_ALL_ACCESS 0x01ff

#include "windef.h"
#include "winbase.h"
#include "winreg.h"

#include "waylanddrv.h"

#include "wine/server.h"
#include "wine/unicode.h"
#include "wine/debug.h"

//#include "ntstatus.h"


WINE_DEFAULT_DEBUG_CHANNEL(waylanddrv);

unsigned int screen_bpp;
Window root_window;

BOOL use_xkb = FALSE;
BOOL use_take_focus = FALSE;
BOOL use_primary_selection = FALSE;
BOOL use_system_cursors = TRUE;
BOOL show_systray = FALSE;
BOOL grab_pointer = TRUE;
BOOL grab_fullscreen = FALSE;
BOOL managed_mode = FALSE;
BOOL decorated_mode = FALSE;
BOOL private_color_map = FALSE;
int primary_monitor = 0;
BOOL client_side_graphics = TRUE;
BOOL client_side_with_render = TRUE;
BOOL shape_layered_windows = FALSE;
int copy_default_colors = 128;
int alloc_system_colors = 256;
int default_display_frequency = 0;
DWORD thread_data_tls_index = TLS_OUT_OF_INDEXES;
int xrender_error_base = 0;
HMODULE waylanddrv_module = 0;
char *process_name = NULL;




#define IS_OPTION_TRUE(ch) \
    ((ch) == 'y' || (ch) == 'Y' || (ch) == 't' || (ch) == 'T' || (ch) == '1')
#define IS_OPTION_FALSE(ch) \
    ((ch) == 'n' || (ch) == 'N' || (ch) == 'f' || (ch) == 'F' || (ch) == '0')



/***********************************************************************
 *		get_config_key
 *
 * Get a config key from either the app-specific or the default config
 */
static inline DWORD get_config_key( HKEY defkey, HKEY appkey, const char *name,
                                    char *buffer, DWORD size )
{
    if (appkey && !RegQueryValueExA( appkey, name, 0, NULL, (LPBYTE)buffer, &size )) return 0;
    if (defkey && !RegQueryValueExA( defkey, name, 0, NULL, (LPBYTE)buffer, &size )) return 0;
    return ERROR_FILE_NOT_FOUND;
}





#if 0
void create_desktop( int is_one )
{
    static const WCHAR messageW[] = {'M','e','s','s','a','g','e',0};
    HDESK desktop = 0;

    HWND hwnd;

    GUID guid;
    const WCHAR *name = NULL;

    static const WCHAR desktopW[] = {'D','e','s','k','t','o','p',0};
    static const WCHAR desktopW1[] = {'D','e','s','k','t','o','p','1',0};
    static const WCHAR *desktopw = NULL;
    desktopw = desktopW;
    //WCHAR desktopW[] = {'D','e','s','k','t','o','p',0};
    if(is_one)
      desktopw = desktopW1;



        if (!(desktop = CreateDesktopW( desktopw, NULL, NULL, 0, DESKTOP_ALL_ACCESS, NULL )))
        {
            TRACE( "failed to create desktop %s error %d\n", wine_dbgstr_w(name), GetLastError() );
            ExitProcess( 1 );
        }
        SetThreadDesktop( desktop );



    UuidCreate( &guid );
    /* create the desktop window */
    hwnd = CreateWindowExW( 0, DESKTOP_CLASS_ATOM, NULL,
                            WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
                            0, 0, 0, 0, 0, 0, 0, &guid );

    if (hwnd)
    {
        /* create the HWND_MESSAGE parent */
        CreateWindowExW( 0, messageW, NULL, WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0, 0, 100, 100, 0, 0, 0, NULL );


        SetWindowPos( hwnd, 0, GetSystemMetrics(SM_XVIRTUALSCREEN), GetSystemMetrics(SM_YVIRTUALSCREEN),
                      GetSystemMetrics(SM_CXVIRTUALSCREEN), GetSystemMetrics(SM_CYVIRTUALSCREEN),
                      SWP_SHOWWINDOW );

        ClipCursor( NULL );



    } else {
       TRACE( "failed to create desktop hwnd %s error %d\n", wine_dbgstr_w(name), GetLastError() );
    }

}
#endif

/***************************************************************************
 *	get_basename
 *
 * Return the base name of a file name (i.e. remove the path components).
 */
static const WCHAR *get_basename( const WCHAR *name )
{
    const WCHAR *ptr;

    if (name[0] && name[1] == ':') name += 2;  /* strip drive specification */
    if ((ptr = strrchrW( name, '\\' ))) name = ptr + 1;
    if ((ptr = strrchrW( name, '/' ))) name = ptr + 1;
    return name;
}


/***********************************************************************
 *           WAYLANDDRV process initialisation routine
 */
static BOOL process_attach(void)
{


  TRACE("Entering wayland \n");

  static WCHAR *current_exe = NULL;
  char *env_width, *env_height;
  static const WCHAR zfgamebrowser_exe[] = {'Z','F','G','a','m','e','B','r','o','w','s','e','r','.','e','x','e', 0};
  static WCHAR current_exepath[MAX_PATH] = {0};

  int screen_width = 1920;
  int screen_height = 1080;

  GetModuleFileNameW(NULL, current_exepath, ARRAY_SIZE(current_exepath));
  current_exe = (WCHAR *)get_basename(current_exepath);


  TRACE("current exe path %s \n", debugstr_wn(current_exepath, strlenW( current_exepath )));
  TRACE("current exe %s \n", debugstr_wn(current_exe, strlenW( current_exe )));

  env_width = getenv( "WINE_VK_WAYLAND_WIDTH" );
  env_height = getenv( "WINE_VK_WAYLAND_HEIGHT" );



  if(env_width) {
    screen_width = atoi(env_width);
  }
  if(env_height) {
    screen_height = atoi(env_height);
  }

  screen_bpp = 32;

//  if( !is_vulkan_desktop_only ) {
//    TRACE( "Creating desktop %d %d \n\n", screen_width , screen_height );
//  }
  xinerama_init( screen_width , screen_height);


  WAYLANDDRV_Settings_Init();


  //setup registry for display devices
  wayland_init_display_devices(1);

//  if( !is_vulkan_desktop_only ) {
//    TRACE( "Creating desktop done %d %d \n\n", screen_width , screen_height );
//  }

  //if ((thread_data_tls_index = TlsAlloc()) == TLS_OUT_OF_INDEXES) {
  //  return FALSE;
  //}


  //Hack for GenshinImpact ZFGameBrowser
//  if(!lstrcmpiW(current_exe, zfgamebrowser_exe)) {
//    create_desktop(1);
//  } else {
//    create_desktop( 0 );
//  }


  TRACE("Entering wayland 2 \n");
  return TRUE;
}






/***********************************************************************
 *           WAYLANDDRV thread initialisation routine
 */
struct waylanddrv_thread_data *waylanddrv_init_thread_data(void)
{




    struct waylanddrv_thread_data *data = waylanddrv_thread_data();

    if (data)
      return data;

    return NULL;

}


/***********************************************************************
 *           WAYLANDDRV initialisation routine
 */
BOOL WINAPI DllMain( HINSTANCE hinst, DWORD reason, LPVOID reserved )
{
    BOOL ret = TRUE;

    switch(reason)
    {
    case DLL_PROCESS_ATTACH:
        init_user_driver();
        DisableThreadLibraryCalls( hinst );
        waylanddrv_module = hinst;
        ret = process_attach();

        break;
    }
    return ret;
}
