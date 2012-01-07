;NSIS Modern User Interface

!define PRODUCT_NAME "ImageWalker"
!define PRODUCT_EXE "ImageWalkerU.exe"
!define PRODUCT_VERSION "231" 
!define PRODUCT_PUBLISHER "ImageWalker"
!define PRODUCT_WEB_SITE "http://wwwImageWalker.com/"
!define PRODUCT_DIR_REGKEY "Software\Microsoft\Windows\CurrentVersion\App Paths\${PRODUCT_EXE}"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}${PRODUCT_VERSION}"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"
!define PRODUCT_STARTMENU_REGVAL "NSIS:StartMenuDir"
!define PRODUCT_DEFAULT_DIR_KEY "Software\${PRODUCT_PUBLISHER}\${PRODUCT_NAME}${PRODUCT_VERSION}\InstallDir"

;--------------------------------
;Include Modern UI

  !include "MUI.nsh"


;--------------------------------
;General

  Name "${PRODUCT_NAME}"
  OutFile "${PRODUCT_NAME}Setup${PRODUCT_VERSION}.exe"
  InstallDir "$PROGRAMFILES\ImageWalker\${PRODUCT_NAME}${PRODUCT_VERSION}"
  InstallDirRegKey HKLM "${PRODUCT_DEFAULT_DIR_KEY}" ""
  ShowInstDetails show
  ShowUnInstDetails show
  SetCompressor lzma
  Icon "Setup.ico"
  XPStyle on

;--------------------------------
;Interface Settings

!define MUI_ABORTWARNING
!define MUI_ICON "setup.ico"
!define MUI_UNICON "setup.ico"
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP "logo.bmp"
!define MUI_HEADERIMAGE_BITMAP_NOSTRETCH
!define MUI_HEADERIMAGE_RIGHT
!define MUI_WELCOMEFINISHPAGE_BITMAP "install.bmp"
!define MUI_UNWELCOMEFINISHPAGE_BITMAP "uninstall.bmp"
!define MUI_COMPONENTSPAGE_SMALLDESC

;--------------------------------
;Pages

  !insertmacro MUI_PAGE_WELCOME
  !insertmacro MUI_PAGE_LICENSE "License.txt"
  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY
  !insertmacro MUI_PAGE_INSTFILES
  !define MUI_FINISHPAGE_RUN "$INSTDIR\${PRODUCT_EXE}"
  !insertmacro MUI_PAGE_FINISH

  
  !insertmacro MUI_UNPAGE_WELCOME
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES
  !insertmacro MUI_UNPAGE_FINISH
 

Function .onInstSuccess

    ExecWait "$INSTDIR\${PRODUCT_EXE} /RegServer"

FunctionEnd

;--------------------------------
;Languages
 
  !insertmacro MUI_LANGUAGE "English"

;--------------------------------
;Installer Sections

Section "ImageWalker Program (required)"
    
  SetOverwrite ifnewer
  
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR

  ; Put file there
  File "..\exe\ImageWalker.chm"
  File "..\exe\${PRODUCT_EXE}"
  File "..\exe\Hooks.dll"
  File "..\exe\unicows.dll"


  ;Do the plugins
  ;SetOutPath $INSTDIR\Plugins

  ;FILE "..\exe\Plugins\toolFtpUpload.dll"
  ;FILE "..\exe\Plugins\toolImportXML.dll"
  ;FILE "..\exe\Plugins\LoadJBIG.dll"
  ;FILE "..\exe\Plugins\AndrewsFilters.ffl"  

  ; may be locked so handel special
  File /oname=_ShellExtensions.dll "..\exe\ShellExtensions.dll" 
  Rename /REBOOTOK $OUTDIR\_ShellExtensions.dll $OUTDIR\ShellExtensions.dll
  RegDLL "$OUTDIR\ShellExtensions.dll"

   
  ;Store installation folder
  WriteRegStr HKCU "Software\ImageWalker\ImageWalker${PRODUCT_VERSION}" "" $INSTDIR
  
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "DisplayName" "ImageWalker 2.31 (remove only)"
  WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "${PRODUCT_UNINST_KEY}" "NoModify" 1
  WriteRegDWORD HKLM "${PRODUCT_UNINST_KEY}" "NoRepair" 1
  WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "" "$INSTDIR\Server\${PRODUCT_EXE}"
  WriteRegStr HKLM "${PRODUCT_DEFAULT_DIR_KEY}" "" "$INSTDIR"
  
  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"

SectionEnd


Section "Spell check dictionaries"

  SetOverwrite ifnewer

  ;Do the templates first
  SetOutPath $INSTDIR\dic


File "..\exe\dic\en_GB.aff"
File "..\exe\dic\en_GB.dic"
File "..\exe\dic\en_US.aff"
File "..\exe\dic\en_US.dic"


SectionEnd

; The stuff to install
Section "Web Page Templates"

  SetOverwrite ifnewer

  ;Do the templates first
  SetOutPath $INSTDIR\WebTemplates

	FILE "..\exe\WebTemplates\Imagewalker.js"
	FILE "..\exe\WebTemplates\Blue.css"
	FILE "..\exe\WebTemplates\Brown.css"
	FILE "..\exe\WebTemplates\brown1.gif"
	FILE "..\exe\WebTemplates\brown2.jpg"
	FILE "..\exe\WebTemplates\Cherry.css"
	FILE "..\exe\WebTemplates\cherry.gif"
	FILE "..\exe\WebTemplates\Gray.css"
	FILE "..\exe\WebTemplates\gray.gif"
	FILE "..\exe\WebTemplates\Groovy.css"
	FILE "..\exe\WebTemplates\groovy1.gif"
	FILE "..\exe\WebTemplates\groovy2.gif"
	FILE "..\exe\WebTemplates\Large Text.css"
	FILE "..\exe\WebTemplates\logo.gif"
	FILE "..\exe\WebTemplates\Orange.css"
	FILE "..\exe\WebTemplates\orange.gif"
	FILE "..\exe\WebTemplates\Red and Black.css"
	FILE "..\exe\WebTemplates\Tech.css"
	FILE "..\exe\WebTemplates\tech1.gif"
	FILE "..\exe\WebTemplates\tech2.gif"
	FILE "..\exe\WebTemplates\Wedding.css"
	FILE "..\exe\WebTemplates\wedding1.jpg"
	FILE "..\exe\WebTemplates\wedding2.gif"
	FILE "..\exe\WebTemplates\White and Black.css"
	FILE "..\exe\WebTemplates\Start.exe"
	FILE "..\exe\WebTemplates\iw.ico"
	FILE "..\exe\WebTemplates\Direct Link.ini"
	FILE "..\exe\WebTemplates\Thumb Navigation.ini"
	FILE "..\exe\WebTemplates\Standard.ini"
	FILE "..\exe\WebTemplates\Resize.ini"
	FILE "..\exe\WebTemplates\Popup.ini"
	FILE "..\exe\WebTemplates\Slide Show.ini"
	FILE "..\exe\WebTemplates\Index and Image with Highlighting.ini"
	FILE "..\exe\WebTemplates\Index and Image.ini"
	FILE "..\exe\WebTemplates\slideshow.image.html"
	FILE "..\exe\WebTemplates\slideshow.index.html"
	FILE "..\exe\WebTemplates\standard.image.html"
	FILE "..\exe\WebTemplates\standard.index.html"
	FILE "..\exe\WebTemplates\standardtn.image.html"
	FILE "..\exe\WebTemplates\combined.frame.html"
	FILE "..\exe\WebTemplates\combined.image.html"
	FILE "..\exe\WebTemplates\combined.index.html"
	FILE "..\exe\WebTemplates\combinedhighlight.frame.html"
	FILE "..\exe\WebTemplates\combinedhighlight.image.html"
	FILE "..\exe\WebTemplates\combinedhighlight.index.html"
	FILE "..\exe\WebTemplates\resize.image.html"
	FILE "..\exe\WebTemplates\popup.image.html"
	FILE "..\exe\WebTemplates\popup.index.html"
	FILE "..\exe\WebTemplates\black.jpg"
	FILE "..\exe\WebTemplates\blue.jpg"
	FILE "..\exe\WebTemplates\red.jpg"
  
SectionEnd

; optional section
Section "Start Menu Shortcuts"

  CreateDirectory "$SMPROGRAMS\ImageWalker"

  CreateShortCut "$SMPROGRAMS\ImageWalker\ImageWalker 2.31.lnk" "$INSTDIR\${PRODUCT_EXE}" "" "$INSTDIR\${PRODUCT_EXE}" 0
  CreateShortCut "$SMPROGRAMS\ImageWalker\ImageWalker 2.31 Help.lnk" "$INSTDIR\ImageWalker.chm" "" "$INSTDIR\ImageWalker.chm" 0
  CreateShortCut "$SMPROGRAMS\ImageWalker\Uninstall 2.31.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0

  CreateShortCut "$DESKTOP\ImageWalker 2.31.lnk" "$INSTDIR\${PRODUCT_EXE}" "" "$INSTDIR\${PRODUCT_EXE}" 0

SectionEnd

 
;--------------------------------
;Uninstaller Section

Section "Uninstall"

  ; Unregister controls
  ExecWait "$INSTDIR\${PRODUCT_EXE} /UnregServer"
  
  ; remove registry keys
  DeleteRegKey HKLM "${PRODUCT_UNINST_KEY}"
  DeleteRegKey HKLM "Software\Walker\ImageWalker${PRODUCT_VERSION}"
  DeleteRegKey HKLM "${PRODUCT_DIR_REGKEY}"

  ; remove files    

	Delete "$INSTDIR\WebTemplates\Imagewalker.js"
	Delete "$INSTDIR\WebTemplates\Blue.css"
	Delete "$INSTDIR\WebTemplates\Brown.css"
	Delete "$INSTDIR\WebTemplates\brown1.gif"
	Delete "$INSTDIR\WebTemplates\brown2.jpg"
	Delete "$INSTDIR\WebTemplates\Cherry.css"
	Delete "$INSTDIR\WebTemplates\cherry.gif"
	Delete "$INSTDIR\WebTemplates\Gray.css"
	Delete "$INSTDIR\WebTemplates\gray.gif"
	Delete "$INSTDIR\WebTemplates\Groovy.css"
	Delete "$INSTDIR\WebTemplates\groovy1.gif"
	Delete "$INSTDIR\WebTemplates\groovy2.gif"
	Delete "$INSTDIR\WebTemplates\Large Text.css"
	Delete "$INSTDIR\WebTemplates\logo.gif"
	Delete "$INSTDIR\WebTemplates\Orange.css"
	Delete "$INSTDIR\WebTemplates\orange.gif"
	Delete "$INSTDIR\WebTemplates\Red and Black.css"
	Delete "$INSTDIR\WebTemplates\Tech.css"
	Delete "$INSTDIR\WebTemplates\tech1.gif"
	Delete "$INSTDIR\WebTemplates\tech2.gif"
	Delete "$INSTDIR\WebTemplates\Wedding.css"
	Delete "$INSTDIR\WebTemplates\wedding1.jpg"
	Delete "$INSTDIR\WebTemplates\wedding2.gif"
	Delete "$INSTDIR\WebTemplates\White and Black.css"
	Delete "$INSTDIR\WebTemplates\Start.exe"
	Delete "$INSTDIR\WebTemplates\iw.ico"
	Delete "$INSTDIR\WebTemplates\Direct Link.ini"
	Delete "$INSTDIR\WebTemplates\Thumb Navigation.ini"
	Delete "$INSTDIR\WebTemplates\Standard.ini"
	Delete "$INSTDIR\WebTemplates\Resize.ini"
	Delete "$INSTDIR\WebTemplates\Popup.ini"
	Delete "$INSTDIR\WebTemplates\Slide Show.ini"
	Delete "$INSTDIR\WebTemplates\Index and Image with Highlighting.ini"
	Delete "$INSTDIR\WebTemplates\Index and Image.ini"
	Delete "$INSTDIR\WebTemplates\slideshow.image.html"
	Delete "$INSTDIR\WebTemplates\slideshow.index.html"
	Delete "$INSTDIR\WebTemplates\standard.image.html"
	Delete "$INSTDIR\WebTemplates\standard.index.html"
	Delete "$INSTDIR\WebTemplates\standardtn.image.html"
	Delete "$INSTDIR\WebTemplates\combined.frame.html"
	Delete "$INSTDIR\WebTemplates\combined.image.html"
	Delete "$INSTDIR\WebTemplates\combined.index.html"
	Delete "$INSTDIR\WebTemplates\combinedhighlight.frame.html"
	Delete "$INSTDIR\WebTemplates\combinedhighlight.image.html"
	Delete "$INSTDIR\WebTemplates\combinedhighlight.index.html"
	Delete "$INSTDIR\WebTemplates\resize.image.html"
	Delete "$INSTDIR\WebTemplates\popup.image.html"
	Delete "$INSTDIR\WebTemplates\popup.index.html"
	Delete "$INSTDIR\WebTemplates\black.jpg"
	Delete "$INSTDIR\WebTemplates\blue.jpg"
	Delete "$INSTDIR\WebTemplates\red.jpg"

	Delete "$INSTDIR\dic\en_GB.aff"
	Delete "$INSTDIR\dic\en_GB.dic"
	Delete "$INSTDIR\dic\en_US.aff"
	Delete "$INSTDIR\dic\en_US.dic"

	;Delete /REBOOTOK "$INSTDIR\Plugins\toolFtpUpload.dll"
	;Delete /REBOOTOK "$INSTDIR\Plugins\toolImportXML.dll"
	;Delete /REBOOTOK "$INSTDIR\Plugins\LoadJBIG.dll"
	;Delete /REBOOTOK "$INSTDIR\Plugins\AndrewsFilters.ffl"

	UnregDLL "$INSTDIR\ShellExtensions.dll"

	Delete /REBOOTOK "$INSTDIR\ShellExtensions.dll"
	Delete /REBOOTOK "$INSTDIR\ImageWalker.chm"
	Delete /REBOOTOK "$INSTDIR\${PRODUCT_EXE}"	
	Delete /REBOOTOK "$INSTDIR\Hooks.dll"	
	Delete /REBOOTOK "$INSTDIR\unicows.dll"

  ; Screen saver
  ;Delete "$WINDIR\ImageWalkerScreenSaver.scr"

  ; MUST REMOVE UNINSTALLER, too
  Delete $INSTDIR\uninstall.exe

  ; remove shortcuts, if any.
  Delete "$SMPROGRAMS\ImageWalker\Uninstall 2.31.lnk"
  Delete "$SMPROGRAMS\ImageWalker\ImageWalker 2.31.lnk"
  Delete "$SMPROGRAMS\ImageWalker\ImageWalker 2.31 Help.lnk"
  Delete "$DESKTOP\ImageWalker 2.31.lnk"

  ; remove directories used.
  RMDir "$SMPROGRAMS\ImageWalker"
  RMDir "$INSTDIR\WebTemplates"
  RMDir "$INSTDIR\Plugins"
  RMDir "$INSTDIR"

  DeleteRegKey /ifempty HKCU "Software\ImageWalker\ImageWalker${PRODUCT_VERSION}"

SectionEnd  