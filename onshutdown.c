/****************************************************************************

    OnShutdown.c

    (C) Copyright Claudio Fahey, 1994

    e-mail: claudio@uclink.berkeley.edu

    Permission granted to use, distribute, and modify, provided that this
    notice remains intact.  If you distribute a modified version, you must
    identify your modifications as such.

****************************************************************************/

#define INCL_WIN
#define INCL_DOS
#include <os2.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static HAB hab;
static HMQ hmq;
static char szCmdParams[1024];
static char *szCmd;
static char szUsage[] =
    "Usage:\n"
    "  OnShutdown <program> [arguments]\n\n"
    "Example:\n"
    "  OnShutdown c:\\shutdown.cmd\n\n"
    "Purpose:\n"
    "  OnShutdown will start the specified\n"
    "  program when the system shuts\n"
    "  down.";

MRESULT EXPENTRY ShutdownObject( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 );

int main( int argc, char *argv[] )
    {
    QMSG qmsg;
    HWND hwnd;
    int i;

    hab = WinInitialize( 0L );
    hmq = WinCreateMsgQueue( hab, 0 );

    WinRegisterClass( hab, "Shutdown Object", ShutdownObject, 0L, 0 );

    // Setup application parameters
    if ( argc >= 2 )
        szCmd = argv[1];
    else
        {
        WinMessageBox( HWND_DESKTOP, HWND_DESKTOP, szUsage, "On Shutdown", 0L,
            MB_OK | MB_ERROR | MB_MOVEABLE );
        return 1;
        }

    szCmdParams[0] = '\0';
    for ( i = 2 ; i < argc - 1; i++ )
        {
        strcat( szCmdParams, argv[i] );
        strcat( szCmdParams, " " );
        }
    if ( argc >= 3 )
        strcat( szCmdParams, argv[argc-1] );

    hwnd = WinCreateWindow( HWND_OBJECT, "Shutdown Object", 0L, 0L, 0, 0, 0, 0, 0L,
        HWND_TOP, 0, 0L, 0L );

    while ( WinGetMsg( hab, &qmsg, 0L, 0, 0 ) )
        WinDispatchMsg( hab, &qmsg );

    WinDestroyWindow( hwnd );
    WinDestroyMsgQueue( hmq );
    WinTerminate( hab );

    return 0;
    }

MRESULT EXPENTRY ShutdownObject( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
    {
    QMSG qmsg;
    PROGDETAILS Details;
    HAPP happ;
    char szMessage[1024];

    switch( msg )
        {
        case WM_DESTROY:
            // Start application
            Details.Length          = sizeof(PROGDETAILS);
            Details.progt.progc     = PROG_DEFAULT;
            Details.progt.fbVisible = SHE_VISIBLE;
            Details.pszTitle        = "Shutdown Process";
            Details.pszExecutable   = szCmd;
            Details.pszParameters   = NULL;
            Details.pszStartupDir   = "";
            Details.pszIcon         = "";
            Details.pszEnvironment  = "";
            Details.swpInitial.fl   = SWP_ACTIVATE;
            Details.swpInitial.cy   = 0;
            Details.swpInitial.cx   = 0;
            Details.swpInitial.y    = 0;
            Details.swpInitial.x    = 0;
            Details.swpInitial.hwndInsertBehind = HWND_TOP;
            Details.swpInitial.hwnd             = 0L;
            Details.swpInitial.ulReserved1      = 0;
            Details.swpInitial.ulReserved2      = 0;
            happ = WinStartApp( hwnd, &Details, szCmdParams, NULL, SAF_STARTCHILDAPP );

            if ( happ )
                {
                // Process message queue because we need to wait for WM_APPTERMINATENOTIFY
                while ( WinGetMsg( hab, &qmsg, 0L, 0, 0 ) )
                    WinDispatchMsg( hab, &qmsg );
                }
            else
                {
                sprintf( szMessage, "Error starting application \"%s\".\n\n"
                    "Do you want to cancel shutdown?", szCmd );
                if ( WinMessageBox( HWND_DESKTOP, HWND_DESKTOP, szMessage, "On Shutdown Error", 0L,
                        MB_YESNO | MB_ERROR | MB_MOVEABLE ) == MBID_YES )
                    WinCancelShutdown( hmq, FALSE );
                }
            break;

        case WM_APPTERMINATENOTIFY:
            WinPostMsg( hwnd, WM_QUIT, 0L, 0L );
            break;

        default:
            return WinDefWindowProc( hwnd, msg, mp1, mp2 );
        }
    return 0L;
    }

