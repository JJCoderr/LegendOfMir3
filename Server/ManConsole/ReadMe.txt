========================================================================
       WIN32 APPLICATION : ManConsole
========================================================================

AppWizard has created this ManConsole application for you.  This file
contains a summary of what you will find in each of the files that make
up your ManConsole application.

ManConsole.h
    This is the main header file for the application.  It includes other
    project specific headers (including Resource.h) and declares the
    global window handles, the log message helpers and the dialog
    procedures.

ManConsole.cpp
    This is the main application source file that contains WinMain, the
    instance initialization and the log message helpers.

ManConsole.rc
    This is a listing of all of the Microsoft Windows resources that the
    program uses.  It includes the icons, bitmaps, and cursors that are stored
    in the RES subdirectory.  This file can be directly edited in Microsoft
	Visual C++.

res\ManConsole.ico
    This is an icon file, which is used as the application's icon.  This
    icon is included by the main resource file ManConsole.rc.




/////////////////////////////////////////////////////////////////////////////

AppWizard creates one dialog:

ManConsoleDlg.cpp - the dialog
    This file implements the main dialog as a plain dialog procedure
    (ManConsoleDlgProc) plus the About box procedure.  The dialog's
    template is in ManConsole.rc, which can be edited in Microsoft
	Visual C++.

ConsoleSocket.h, ConsoleSocket.cpp
    Blocking socket wrapper (CConsoleSocket) used by the console to talk
    to the game servers.  It only uses WinSock2, no framework classes.


/////////////////////////////////////////////////////////////////////////////
Other standard files:

StdAfx.h, StdAfx.cpp
    These files are used to build a precompiled header (PCH) file
    named ManConsole.pch and a precompiled types file named StdAfx.obj.

Resource.h
    This is the standard header file, which defines new resource IDs.
    Microsoft Visual C++ reads and updates this file.

/////////////////////////////////////////////////////////////////////////////
Other notes:

AppWizard uses "TODO:" to indicate parts of the source code you
should add to or customize.

/////////////////////////////////////////////////////////////////////////////

