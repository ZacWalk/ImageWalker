///////////////////////////////////////////////////////////////////////
//
// This file is part of the ImageWalker code base.
// For more information on ImageWalker see www.ImageWalker.com
//
// Copyright (C) 1998-2001 Zac Walker.  All rights reserved.
//
///////////////////////////////////////////////////////////////////////

#define WINVER 0x0502
#define _WIN32_WINNT 0x0502
#define _WIN32_WINDOWS 0x0502
#define _WIN32_IE 0x0700 

#define _ATL_APARTMENT_THREADED
#define _ATL_ALL_WARNINGS
#define _WTL_NO_CSTRING
#define _WTL_CMDBAR_VISTA_MENUS 0

#include <AtlStr.h>
#include <AtlBase.h>
#include <atlApp.h>

extern CServerAppModule _Module;

#include <string>
#include <map>
#include <vector>
#include <set>
#include <algorithm>
#include <list>
#include <iostream>
#include <queue>
#include <iterator>

#include "Memory.h"

#include <WinInet.h>
#include <ShlObj.h>

//#include <AtlTypes.h>
#include <AtlComTime.h>
#include <AtlCom.h>
#include <Atlctl.h>
#include <AtlCtrls.h>
#include <atlctrlw.h>
#include <AtlCtrlx.h>
#include <AtlDlgs.h>
#include <atlframe.h>
#include <AtlGdi.h>
#include <AtlMisc.h>
#include <atlPrint.h>
#include <AtlScrl.h>
#include <AtlSync.h>
#include <AtlTheme.h>
#include <AtlMem.h>


#define _USE_MATH_DEFINES  1
#include <Math.h> 
#include <time.h>
#include <urlmon.h>
#include <process.h>

#include "resource.h"
#include "Ver.h"
#include "Strings.h"
#include "Core.h"
#include "Shell.h"
#include "Streams.h"
#include "Serialize.h"
#include "Imaging.h"
#include "Render.h"
#include "ThumbnailCache.h"
#include "Application.h"
#include "Help.h"
#include "Coupling.h"
#include "Icons.h"
#include "AtlSplit2.h"
#include "PropertyArchiveRegistry.h"
//#include "AtlCtrlXP.h"
//#include "AtlCtrlXP2.h"