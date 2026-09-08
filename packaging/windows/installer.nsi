Unicode True
RequestExecutionLevel user
!include "MUI2.nsh"
Name "Better PiP"
OutFile "${OUTPUT_FILE}"
InstallDir "$LOCALAPPDATA\Programs\Better PiP"
!define MUI_ABORTWARNING
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "${DIST_DIR}\licenses\LICENSE"
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_LANGUAGE "English"
Section "Better PiP"
    SetOutPath "$INSTDIR"
    File /r "${DIST_DIR}\*"
    WriteUninstaller "$INSTDIR\Uninstall.exe"
    CreateShortCut "$SMPROGRAMS\Better PiP.lnk" "$INSTDIR\bin\better-pip.exe"
    WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\BetterPiP" "DisplayName" "Better PiP"
    WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\BetterPiP" "DisplayVersion" "${VERSION}"
    WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\BetterPiP" "Publisher" "Better PiP contributors"
    WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\BetterPiP" "UninstallString" '$\"$INSTDIR\Uninstall.exe$\"'
    WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\BetterPiP" "DisplayIcon" "$INSTDIR\bin\better-pip.exe"
    WriteRegDWORD HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\BetterPiP" "NoModify" 1
    WriteRegDWORD HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\BetterPiP" "NoRepair" 1
SectionEnd
Section "Uninstall"
    !include "${UNINSTALL_FILES}"
    Delete "$SMPROGRAMS\Better PiP.lnk"
    DeleteRegKey HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\BetterPiP"
    Delete "$INSTDIR\Uninstall.exe"
    RMDir "$INSTDIR"
SectionEnd
