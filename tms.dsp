# Microsoft Developer Studio Project File - Name="tms" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=tms - Win32 Bounds Checker
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "tms.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "tms.mak" CFG="tms - Win32 Bounds Checker"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "tms - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "tms - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "tms - Win32 Bounds Checker" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "tms - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "." /D "__WIN32__" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /D "BTI_WIN_32" /FR /FD /Zm200 /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 /nologo /subsystem:windows /machine:I386
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "tms - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "." /D ".\inc" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /D "BTI_WIN_32" /D "__WIN32__" /FR".\Debug/" /Fo".\Debug/" /Fd".\Debug/" /FD /Zm200 /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /map /debug /machine:I386 /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "tms - Win32 Bounds Checker"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "tms___Win32_Bounds_Checker"
# PROP BASE Intermediate_Dir "tms___Win32_Bounds_Checker"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "BoundsChecker"
# PROP Intermediate_Dir "BoundsChecker"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D ".\inc" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /D "BTI_WIN_32" /D "__WIN32__" /FR".\Debug/" /Fo".\Debug/" /Fd".\Debug/" /FD /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "." /D ".\inc" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /D "BTI_WIN_32" /D "__WIN32__" /FR".\BoundsChecker/" /Fo".\BoundsChecker/" /Fd".\BoundsChecker/" /FD /Zm200 /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /map /debug /machine:I386 /pdbtype:sept
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /map /debug /machine:I386 /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "tms - Win32 Release"
# Name "tms - Win32 Debug"
# Name "tms - Win32 Bounds Checker"
# Begin Group "Source Files"

# PROP Default_Filter ".c;.cpp"
# Begin Source File

SOURCE=.\_line3dh.cpp
# End Source File
# Begin Source File

SOURCE=.\ADDBLOCK.c
# End Source File
# Begin Source File

SOURCE=.\AddBus.cpp
# End Source File
# Begin Source File

SOURCE=.\AddBustype.cpp
# End Source File
# Begin Source File

SOURCE=.\AddComment.cpp
# End Source File
# Begin Source File

SOURCE=.\AddConnection.cpp
# End Source File
# Begin Source File

SOURCE=.\AddConnectionSD.cpp
# End Source File
# Begin Source File

SOURCE=.\AddCrewOnly.cpp
# End Source File
# Begin Source File

SOURCE=.\AddDirection.cpp
# End Source File
# Begin Source File

SOURCE=.\AddDiscipline.cpp
# End Source File
# Begin Source File

SOURCE=.\AddDivision.cpp
# End Source File
# Begin Source File

SOURCE=.\AddDriver.cpp
# End Source File
# Begin Source File

SOURCE=.\AddDriver01.cpp
# End Source File
# Begin Source File

SOURCE=.\AddDriver02.cpp
# End Source File
# Begin Source File

SOURCE=.\AddDriver03.cpp
# End Source File
# Begin Source File

SOURCE=.\AddDriver04.cpp
# End Source File
# Begin Source File

SOURCE=.\AddDriverPropSheet.cpp
# End Source File
# Begin Source File

SOURCE=.\AddDriverType.cpp
# End Source File
# Begin Source File

SOURCE=.\AddExtraTrip.cpp
# End Source File
# Begin Source File

SOURCE=.\AddHoliday.cpp
# End Source File
# Begin Source File

SOURCE=.\ADDJURISDICTION.c
# End Source File
# Begin Source File

SOURCE=.\AddNode.cpp
# End Source File
# Begin Source File

SOURCE=.\ADDPATTERN.c
# End Source File
# Begin Source File

SOURCE=.\ADDROSTER.c
# End Source File
# Begin Source File

SOURCE=.\AddRoute.cpp
# End Source File
# Begin Source File

SOURCE=.\ADDRUN.c
# End Source File
# Begin Source File

SOURCE=.\ADDRUNCOMMENT.c
# End Source File
# Begin Source File

SOURCE=.\ADDRUNTYPE.c
# End Source File
# Begin Source File

SOURCE=.\ADDSERVICE.c
# End Source File
# Begin Source File

SOURCE=.\ADDSIGNCODE.c
# End Source File
# Begin Source File

SOURCE=.\AddTimecheck.cpp
# End Source File
# Begin Source File

SOURCE=.\ADDTRIP.c
# End Source File
# Begin Source File

SOURCE=.\Aggpiece.cpp
# End Source File
# Begin Source File

SOURCE=.\AllocationError.c
# End Source File
# Begin Source File

SOURCE=.\ALTERRUNTYPE.c
# End Source File
# Begin Source File

SOURCE=.\ANALYZERUN.c
# End Source File
# Begin Source File

SOURCE=.\AssembleRuns.c
# End Source File
# Begin Source File

SOURCE=.\AssignDriver.cpp
# End Source File
# Begin Source File

SOURCE=.\ASSIGNTRIP.c
# End Source File
# Begin Source File

SOURCE=.\ASSIGNVEHICLETYPE.c
# End Source File
# Begin Source File

SOURCE=.\ASSREMGARAGE.c
# End Source File
# Begin Source File

SOURCE=.\asTime.cpp
# End Source File
# Begin Source File

SOURCE=.\AuditFilter.cpp
# End Source File
# Begin Source File

SOURCE=.\BCTRAVEL.c
# End Source File
# Begin Source File

SOURCE=.\BIDDING.c
# End Source File
# Begin Source File

SOURCE=.\BitmapCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\BitmapDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\Block.cpp
# End Source File
# Begin Source File

SOURCE=.\BlockPairs.cpp
# End Source File
# Begin Source File

SOURCE=.\BlockProperties.cpp
# End Source File
# Begin Source File

SOURCE=.\BLOCKSUMMARY.c
# End Source File
# Begin Source File

SOURCE=.\bpnetwork.cpp
# End Source File
# Begin Source File

SOURCE=.\BrowseMenu.cpp
# End Source File
# Begin Source File

SOURCE=.\BTRAPI.C
# End Source File
# Begin Source File

SOURCE=.\btrieve.c
# End Source File
# Begin Source File

SOURCE=.\CalendarDate.cpp
# End Source File
# Begin Source File

SOURCE=.\CCDisplayLog.cpp
# End Source File
# Begin Source File

SOURCE=.\CCFrontEnd.cpp
# End Source File
# Begin Source File

SOURCE=.\CCLogin.cpp
# End Source File
# Begin Source File

SOURCE=.\CCProcess.cpp
# End Source File
# Begin Source File

SOURCE=.\CCReports.cpp
# End Source File
# Begin Source File

SOURCE=.\CCSupervisor.cpp
# End Source File
# Begin Source File

SOURCE=.\CHANGEBUSTYPES.c
# End Source File
# Begin Source File

SOURCE=.\CHANGECCOMMENTS.c
# End Source File
# Begin Source File

SOURCE=.\CHANGELAYOVER.c
# End Source File
# Begin Source File

SOURCE=.\CHANGEOCOMMENTS.c
# End Source File
# Begin Source File

SOURCE=.\CHANGEPATTERNS.c
# End Source File
# Begin Source File

SOURCE=.\CHANGESIGNCODES.c
# End Source File
# Begin Source File

SOURCE=.\CHANGETRIPNUMBER.c
# End Source File
# Begin Source File

SOURCE=.\CHANGETRIPSHIFT.c
# End Source File
# Begin Source File

SOURCE=.\ChildFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\ChooseDate.cpp
# End Source File
# Begin Source File

SOURCE=.\Cis.cpp
# End Source File
# Begin Source File

SOURCE=.\Cistms.cpp
# End Source File
# Begin Source File

SOURCE=.\colorbtn.cpp
# End Source File
# Begin Source File

SOURCE=.\CommandsMenu.cpp
# End Source File
# Begin Source File

SOURCE=.\Connexionz.cpp
# End Source File
# Begin Source File

SOURCE=.\convert_datum.cpp
# End Source File
# Begin Source File

SOURCE=.\COPYPATTERNS.c
# End Source File
# Begin Source File

SOURCE=.\CopyRoster.cpp
# End Source File
# Begin Source File

SOURCE=.\CopyRuncut.cpp
# End Source File
# Begin Source File

SOURCE=.\COPYTRIPS.c
# End Source File
# Begin Source File

SOURCE=.\COSTHOOKEDRUNAS.c
# End Source File
# Begin Source File

SOURCE=.\cover.cpp
# End Source File
# Begin Source File

SOURCE=.\Crewcut.c
# End Source File
# Begin Source File

SOURCE=.\crvtrackcursorinfo.cpp
# End Source File
# Begin Source File

SOURCE=.\Crystal.cpp
# End Source File
# Begin Source File

SOURCE=.\crystalreportviewer11.cpp
# End Source File
# Begin Source File

SOURCE=.\CustomControls.cpp
# End Source File
# Begin Source File

SOURCE=.\Customer.cpp
# End Source File
# Begin Source File

SOURCE=.\CustomerComment.cpp
# End Source File
# Begin Source File

SOURCE=.\CutCrew.cpp
# End Source File
# Begin Source File

SOURCE=.\CutRuns.cpp
# End Source File
# Begin Source File

SOURCE=.\DailyOps.cpp
# End Source File
# Begin Source File

SOURCE=.\DailyOpsAB.cpp
# End Source File
# Begin Source File

SOURCE=.\DailyOpsAL.cpp
# End Source File
# Begin Source File

SOURCE=.\DailyOpsBL.cpp
# End Source File
# Begin Source File

SOURCE=.\DailyOpsBP.cpp
# End Source File
# Begin Source File

SOURCE=.\DailyOpsBS.cpp
# End Source File
# Begin Source File

SOURCE=.\DailyOpsFU.cpp
# End Source File
# Begin Source File

SOURCE=.\DailyOpsHO.cpp
# End Source File
# Begin Source File

SOURCE=.\DailyOpsOOS.cpp
# End Source File
# Begin Source File

SOURCE=.\DailyOpsRL.cpp
# End Source File
# Begin Source File

SOURCE=.\DailyOpsRS.cpp
# End Source File
# Begin Source File

SOURCE=.\DailyOpsRSO.cpp
# End Source File
# Begin Source File

SOURCE=.\DailyOpsSO.cpp
# End Source File
# Begin Source File

SOURCE=.\DailyOpsSS.cpp
# End Source File
# Begin Source File

SOURCE=.\DailyOpsUA.cpp
# End Source File
# Begin Source File

SOURCE=.\DailyOpsXB.cpp
# End Source File
# Begin Source File

SOURCE=.\DatabaseStringRoutines.cpp
# End Source File
# Begin Source File

SOURCE=.\DATAFIELD.c
# End Source File
# Begin Source File

SOURCE=.\dataformatdisp.cpp
# End Source File
# Begin Source File

SOURCE=.\DateRange.cpp
# End Source File
# Begin Source File

SOURCE=.\DaysOffPatterns.cpp
# End Source File
# Begin Source File

SOURCE=.\DEBUG.C
# End Source File
# Begin Source File

SOURCE=.\Decide.c
# End Source File
# Begin Source File

SOURCE=.\DeleteRecord.c
# End Source File
# Begin Source File

SOURCE=.\DELETERUNS.c
# End Source File
# Begin Source File

SOURCE=.\dib256.cpp
# End Source File
# Begin Source File

SOURCE=.\dibpal.cpp
# End Source File
# Begin Source File

SOURCE=.\Discipline.cpp
# End Source File
# Begin Source File

SOURCE=.\DisciplineFilter.cpp
# End Source File
# Begin Source File

SOURCE=.\DisciplineLetter.cpp
# End Source File
# Begin Source File

SOURCE=.\DisciplineLetterTemplate.cpp
# End Source File
# Begin Source File

SOURCE=.\DispatchSheet.cpp
# End Source File
# Begin Source File

SOURCE=.\DisplayFlagRow.cpp
# End Source File
# Begin Source File

SOURCE=.\DisplayPatternRow.cpp
# End Source File
# Begin Source File

SOURCE=.\DisplayPlacement.c
# End Source File
# Begin Source File

SOURCE=.\DisplayRoster.cpp
# End Source File
# Begin Source File

SOURCE=.\DisplayTripRow.cpp
# End Source File
# Begin Source File

SOURCE=.\distance.cpp
# End Source File
# Begin Source File

SOURCE=.\DOCRoster.cpp
# End Source File
# Begin Source File

SOURCE=.\DriversToConsider.cpp
# End Source File
# Begin Source File

SOURCE=.\dtpicker.cpp
# End Source File
# Begin Source File

SOURCE=.\EditMenu.cpp
# End Source File
# Begin Source File

SOURCE=.\EnumerationRoutines.cpp
# End Source File
# Begin Source File

SOURCE=.\EstablishVRBitmaps.cpp
# End Source File
# Begin Source File

SOURCE=.\EventReport.cpp
# End Source File
# Begin Source File

SOURCE=.\Expiries.cpp
# End Source File
# Begin Source File

SOURCE=.\EXTRABOARDEDITOR.c
# End Source File
# Begin Source File

SOURCE=.\f1book.cpp
# End Source File
# Begin Source File

SOURCE=.\f1bookview.cpp
# End Source File
# Begin Source File

SOURCE=.\f1cellformat.cpp
# End Source File
# Begin Source File

SOURCE=.\F1Events.cpp
# End Source File
# Begin Source File

SOURCE=.\f1filespec.cpp
# End Source File
# Begin Source File

SOURCE=.\f1numberformat.cpp
# End Source File
# Begin Source File

SOURCE=.\f1objpos.cpp
# End Source File
# Begin Source File

SOURCE=.\f1odbcconnect.cpp
# End Source File
# Begin Source File

SOURCE=.\f1odbcquery.cpp
# End Source File
# Begin Source File

SOURCE=.\f1rangeref.cpp
# End Source File
# Begin Source File

SOURCE=.\f1rect.cpp
# End Source File
# Begin Source File

SOURCE=.\f1replaceresults.cpp
# End Source File
# Begin Source File

SOURCE=.\Fares.cpp
# End Source File
# Begin Source File

SOURCE=.\FIELDDEF.c
# End Source File
# Begin Source File

SOURCE=.\FileMenu.cpp
# End Source File
# Begin Source File

SOURCE=.\FindBlock.cpp
# End Source File
# Begin Source File

SOURCE=.\FindCustomer.cpp
# End Source File
# Begin Source File

SOURCE=.\flatsb.cpp
# End Source File
# Begin Source File

SOURCE=.\font.cpp
# End Source File
# Begin Source File

SOURCE=.\font1.cpp
# End Source File
# Begin Source File

SOURCE=.\FreeAll.c
# End Source File
# Begin Source File

SOURCE=.\Garages.cpp
# End Source File
# Begin Source File

SOURCE=.\GenerateTrip.c
# End Source File
# Begin Source File

SOURCE=.\GetDynamicTravelTime.c
# End Source File
# Begin Source File

SOURCE=.\GetLatLong.cpp
# End Source File
# Begin Source File

SOURCE=.\GetLLEN.c
# End Source File
# Begin Source File

SOURCE=.\GetNewBlockNumber.c
# End Source File
# Begin Source File

SOURCE=.\GetRouteQuery.cpp
# End Source File
# Begin Source File

SOURCE=.\GetRunElements.c
# End Source File
# Begin Source File

SOURCE=.\GetRunRecords.c
# End Source File
# Begin Source File

SOURCE=.\GetTimeAtNode.c
# End Source File
# Begin Source File

SOURCE=.\GetTMSData.cpp
# End Source File
# Begin Source File

SOURCE=.\GLIB.CPP
# End Source File
# Begin Source File

SOURCE=.\Hash.cpp
# End Source File
# Begin Source File

SOURCE=.\HashFI.cpp
# End Source File
# Begin Source File

SOURCE=.\HOOKBLOCKS.c
# End Source File
# Begin Source File

SOURCE=.\ILGBlock.cpp
# End Source File
# Begin Source File

SOURCE=.\ILGPattern.cpp
# End Source File
# Begin Source File

SOURCE=.\ILGRoster.cpp
# End Source File
# Begin Source File

SOURCE=.\ILGRouteDirection.cpp
# End Source File
# Begin Source File

SOURCE=.\ILGRoutes.cpp
# End Source File
# Begin Source File

SOURCE=.\ILGRouteStops.cpp
# End Source File
# Begin Source File

SOURCE=.\ILGRouteTimePoints.cpp
# End Source File
# Begin Source File

SOURCE=.\ILGRuns.cpp
# End Source File
# Begin Source File

SOURCE=.\ILGServiceType.cpp
# End Source File
# Begin Source File

SOURCE=.\ILGStops.cpp
# End Source File
# Begin Source File

SOURCE=.\ILGTimePoints.cpp
# End Source File
# Begin Source File

SOURCE=.\ILGTimeTable.cpp
# End Source File
# Begin Source File

SOURCE=.\ILGTrips.cpp
# End Source File
# Begin Source File

SOURCE=.\ILGTripStops.cpp
# End Source File
# Begin Source File

SOURCE=.\ILGVersionInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\IllegalRunCut.c
# End Source File
# Begin Source File

SOURCE=.\IMapi.cpp
# End Source File
# Begin Source File

SOURCE=.\InfoBar.cpp
# End Source File
# Begin Source File

SOURCE=.\JumpToRun.cpp
# End Source File
# Begin Source File

SOURCE=".\LatLong-UTMconversion.cpp"
# End Source File
# Begin Source File

SOURCE=.\LAYOVER.c
# End Source File
# Begin Source File

SOURCE=.\List.cpp
# End Source File
# Begin Source File

SOURCE=.\Logger.cpp
# End Source File
# Begin Source File

SOURCE=.\Lpcut.cpp
# End Source File
# Begin Source File

SOURCE=.\LPGen.cpp
# End Source File
# Begin Source File

SOURCE=.\Lpkit.c
# End Source File
# Begin Source File

SOURCE=.\MainFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\MakeRoster.cpp
# End Source File
# Begin Source File

SOURCE=.\MANCUT.c
# End Source File
# Begin Source File

SOURCE=.\ManualCut.cpp
# End Source File
# Begin Source File

SOURCE=.\Mapinfow.cpp
# End Source File
# Begin Source File

SOURCE=.\match.cpp
# End Source File
# Begin Source File

SOURCE=.\MatchGraph.cpp
# End Source File
# Begin Source File

SOURCE=.\mbpnetwork.cpp
# End Source File
# Begin Source File

SOURCE=.\MIFDigest.cpp
# End Source File
# Begin Source File

SOURCE=.\mm.cpp
# End Source File
# Begin Source File

SOURCE=.\msmask.cpp
# End Source File
# Begin Source File

SOURCE=.\Netblock.cpp
# End Source File
# Begin Source File

SOURCE=.\NETWORK.CPP
# End Source File
# Begin Source File

SOURCE=.\NetworkSimplex.cpp
# End Source File
# Begin Source File

SOURCE=.\NEWDATABASE.C
# End Source File
# Begin Source File

SOURCE=.\nextPrime.cpp
# End Source File
# Begin Source File

SOURCE=.\NodeAbbrFromRecID.c
# End Source File
# Begin Source File

SOURCE=.\NODEFINDER.c
# End Source File
# Begin Source File

SOURCE=.\NodesEquivalent.c
# End Source File
# Begin Source File

SOURCE=.\NumericEdit.cpp
# End Source File
# Begin Source File

SOURCE=.\OnTimePerformance.cpp
# End Source File
# Begin Source File

SOURCE=.\OnTimePerformance2.cpp
# End Source File
# Begin Source File

SOURCE=.\OpenDatabase.cpp
# End Source File
# Begin Source File

SOURCE=.\OVERTIME.c
# End Source File
# Begin Source File

SOURCE=.\PairingHeap.cpp
# End Source File
# Begin Source File

SOURCE=.\PAIRS.CPP
# End Source File
# Begin Source File

SOURCE=.\PARAMETERS.c
# End Source File
# Begin Source File

SOURCE=.\parseMIF.cpp
# End Source File
# Begin Source File

SOURCE=.\Pathify.cpp
# End Source File
# Begin Source File

SOURCE=.\PayrollLayout.cpp
# End Source File
# Begin Source File

SOURCE=.\PayrollSelections.cpp
# End Source File
# Begin Source File

SOURCE=.\PegboardColors.cpp
# End Source File
# Begin Source File

SOURCE=.\PickConnection.c
# End Source File
# Begin Source File

SOURCE=.\PickDate.cpp
# End Source File
# Begin Source File

SOURCE=.\picture.cpp
# End Source File
# Begin Source File

SOURCE=.\picture1.cpp
# End Source File
# Begin Source File

SOURCE=.\PlaceHolder.cpp
# End Source File
# Begin Source File

SOURCE=.\PlotLeft.cpp
# End Source File
# Begin Source File

SOURCE=.\POINTER.CPP
# End Source File
# Begin Source File

SOURCE=.\PoolCalloc.c
# End Source File
# Begin Source File

SOURCE=.\PREFERENCES.c
# End Source File
# Begin Source File

SOURCE=.\PremiumDefinition.cpp
# End Source File
# Begin Source File

SOURCE=.\PREMIUMS.c
# End Source File
# Begin Source File

SOURCE=.\PREVIEWT.c
# End Source File
# Begin Source File

SOURCE=.\PrintWindow.c
# End Source File
# Begin Source File

SOURCE=.\ProcessAbsenceRules.c
# End Source File
# Begin Source File

SOURCE=.\PROFILE.c
# End Source File
# Begin Source File

SOURCE=.\PULLOUTPULLIN.c
# End Source File
# Begin Source File

SOURCE=.\qtree.cpp
# End Source File
# Begin Source File

SOURCE=.\RBlock.cpp
# End Source File
# Begin Source File

SOURCE=.\Rc.c
# End Source File
# Begin Source File

SOURCE=.\rcimp.cpp
# End Source File
# Begin Source File

SOURCE=.\ReadWorkrules.c
# End Source File
# Begin Source File

SOURCE=.\ReassignRuntype.cpp
# End Source File
# Begin Source File

SOURCE=.\Recost.cpp
# End Source File
# Begin Source File

SOURCE=.\Recursive.cpp
# End Source File
# Begin Source File

SOURCE=.\RefCount.cpp
# End Source File
# Begin Source File

SOURCE=.\RELIEFPOINTS.c
# End Source File
# Begin Source File

SOURCE=.\RenderField.cpp
# End Source File
# Begin Source File

SOURCE=.\RENRUNS.c
# End Source File
# Begin Source File

SOURCE=.\RENUMBER.c
# End Source File
# Begin Source File

SOURCE=.\RENUMBLOCKS.c
# End Source File
# Begin Source File

SOURCE=.\RENUMROSTER.c
# End Source File
# Begin Source File

SOURCE=.\RENUMRUNS.c
# End Source File
# Begin Source File

SOURCE=".\Report Place Holder.c"
# End Source File
# Begin Source File

SOURCE=.\ResizingDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\Ridership.cpp
# End Source File
# Begin Source File

SOURCE=.\Roster.cpp
# End Source File
# Begin Source File

SOURCE=.\RosterCoster.c
# End Source File
# Begin Source File

SOURCE=.\RosterImprove.cpp
# End Source File
# Begin Source File

SOURCE=.\RosterMerge.cpp
# End Source File
# Begin Source File

SOURCE=.\RosterOrder.cpp
# End Source File
# Begin Source File

SOURCE=.\RosterParms.cpp
# End Source File
# Begin Source File

SOURCE=.\RosterWeek.cpp
# End Source File
# Begin Source File

SOURCE=.\RPTPARMS.c
# End Source File
# Begin Source File

SOURCE=.\RTATTRIB.c
# End Source File
# Begin Source File

SOURCE=.\RunCoster.c
# End Source File
# Begin Source File

SOURCE=.\RunDef.cpp
# End Source File
# Begin Source File

SOURCE=.\RunMenu.cpp
# End Source File
# Begin Source File

SOURCE=.\RunProperties.cpp
# End Source File
# Begin Source File

SOURCE=.\RunStartAndEnd.c
# End Source File
# Begin Source File

SOURCE=.\RunSummary.cpp
# End Source File
# Begin Source File

SOURCE=.\RUNTYPES.c
# End Source File
# Begin Source File

SOURCE=.\sdigrid.cpp
# End Source File
# Begin Source File

SOURCE=.\selbookmarks.cpp
# End Source File
# Begin Source File

SOURCE=.\SelectPatterns.cpp
# End Source File
# Begin Source File

SOURCE=.\ServiceCalendar.cpp
# End Source File
# Begin Source File

SOURCE=.\SERVICEDAYS.c
# End Source File
# Begin Source File

SOURCE=.\SETCOL.c
# End Source File
# Begin Source File

SOURCE=.\SetupMenu.cpp
# End Source File
# Begin Source File

SOURCE=.\SetupReliefPoints.c
# End Source File
# Begin Source File

SOURCE=.\Setups.c
# End Source File
# Begin Source File

SOURCE=.\SHIFTRUN.c
# End Source File
# Begin Source File

SOURCE=.\snbuild.cpp
# End Source File
# Begin Source File

SOURCE=.\snnetwork.cpp
# End Source File
# Begin Source File

SOURCE=.\SOLVE.C
# End Source File
# Begin Source File

SOURCE=.\SORTBYCOLUMN.c
# End Source File
# Begin Source File

SOURCE=.\SORTTRIP.c
# End Source File
# Begin Source File

SOURCE=.\Sphere.c
# End Source File
# Begin Source File

SOURCE=.\SPLITCUT.C
# End Source File
# Begin Source File

SOURCE=.\STARTEND.c
# End Source File
# Begin Source File

SOURCE=.\StatusBar.c
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# End Source File
# Begin Source File

SOURCE=.\stddataformatsdisp.cpp
# End Source File
# Begin Source File

SOURCE=.\StopConnection.cpp
# End Source File
# Begin Source File

SOURCE=.\StraightCutBackward.c
# End Source File
# Begin Source File

SOURCE=.\StraightCutForward.c
# End Source File
# Begin Source File

SOURCE=.\StrategicMapping.cpp
# End Source File
# Begin Source File

SOURCE=.\SuggestSuspension.cpp
# End Source File
# Begin Source File

SOURCE=.\SUMMARIZEBLOCK.c
# End Source File
# Begin Source File

SOURCE=.\SUPERVISOR.c
# End Source File
# Begin Source File

SOURCE=.\SuspensionPending.cpp
# End Source File
# Begin Source File

SOURCE=.\SWAPPIECES.c
# End Source File
# Begin Source File

SOURCE=.\TaskPanel.cpp
# End Source File
# Begin Source File

SOURCE=.\TERM.CPP
# End Source File
# Begin Source File

SOURCE=.\TextAbsenceReasons.cpp
# End Source File
# Begin Source File

SOURCE=.\TextActionCodes.cpp
# End Source File
# Begin Source File

SOURCE=.\TextBusSwapReasons.cpp
# End Source File
# Begin Source File

SOURCE=.\TextCategories.cpp
# End Source File
# Begin Source File

SOURCE=.\TextClassificationCodes.cpp
# End Source File
# Begin Source File

SOURCE=.\TextConvert.cpp
# End Source File
# Begin Source File

SOURCE=.\TextDepartments.cpp
# End Source File
# Begin Source File

SOURCE=.\TextExtraTimeReasons.cpp
# End Source File
# Begin Source File

SOURCE=.\TextLoad.cpp
# End Source File
# Begin Source File

SOURCE=.\TextOtherComments.cpp
# End Source File
# Begin Source File

SOURCE=.\TextPassengerLoads.cpp
# End Source File
# Begin Source File

SOURCE=.\TextPayrollLayout.cpp
# End Source File
# Begin Source File

SOURCE=.\TextReferrals.cpp
# End Source File
# Begin Source File

SOURCE=.\TextRoadSupervisors.cpp
# End Source File
# Begin Source File

SOURCE=.\TextStopFlags.cpp
# End Source File
# Begin Source File

SOURCE=.\TextSupervisor.cpp
# End Source File
# Begin Source File

SOURCE=.\TextTrafficConditions.cpp
# End Source File
# Begin Source File

SOURCE=.\TextTypical.cpp
# End Source File
# Begin Source File

SOURCE=.\TextViolationActions.cpp
# End Source File
# Begin Source File

SOURCE=.\TextViolationCategories.cpp
# End Source File
# Begin Source File

SOURCE=.\TextViolations.cpp
# End Source File
# Begin Source File

SOURCE=.\TextWeatherConditions.cpp
# End Source File
# Begin Source File

SOURCE=.\Themes.cpp
# End Source File
# Begin Source File

SOURCE=.\tinyxml.cpp
# End Source File
# Begin Source File

SOURCE=.\tinyxmlerror.cpp
# End Source File
# Begin Source File

SOURCE=.\tinyxmlparser.cpp
# End Source File
# Begin Source File

SOURCE=.\TMS.clw
# End Source File
# Begin Source File

SOURCE=.\tms.cpp
# End Source File
# Begin Source File

SOURCE=.\TmsError.c
# End Source File
# Begin Source File

SOURCE=.\Tmsrpt.cpp
# End Source File
# Begin Source File

SOURCE=.\Tmsrpt01.c
# End Source File
# Begin Source File

SOURCE=.\Tmsrpt02.c
# End Source File
# Begin Source File

SOURCE=.\Tmsrpt03.c
# End Source File
# Begin Source File

SOURCE=.\Tmsrpt04.c
# End Source File
# Begin Source File

SOURCE=.\Tmsrpt05.c
# End Source File
# Begin Source File

SOURCE=.\Tmsrpt06.c
# End Source File
# Begin Source File

SOURCE=.\Tmsrpt07.c
# End Source File
# Begin Source File

SOURCE=.\Tmsrpt08.c
# End Source File
# Begin Source File

SOURCE=.\Tmsrpt09.c
# End Source File
# Begin Source File

SOURCE=.\Tmsrpt10.c
# End Source File
# Begin Source File

SOURCE=.\Tmsrpt11.c
# End Source File
# Begin Source File

SOURCE=.\Tmsrpt12.c
# End Source File
# Begin Source File

SOURCE=.\Tmsrpt13.c
# End Source File
# Begin Source File

SOURCE=.\Tmsrpt14.c
# End Source File
# Begin Source File

SOURCE=.\Tmsrpt15.c
# End Source File
# Begin Source File

SOURCE=.\Tmsrpt16.c
# End Source File
# Begin Source File

SOURCE=.\Tmsrpt17.c
# End Source File
# Begin Source File

SOURCE=.\Tmsrpt18.c
# End Source File
# Begin Source File

SOURCE=.\TMSRpt19.c
# End Source File
# Begin Source File

SOURCE=.\TMSRPT20.c
# End Source File
# Begin Source File

SOURCE=.\Tmsrpt21.c
# End Source File
# Begin Source File

SOURCE=.\TMSRPT22.c
# End Source File
# Begin Source File

SOURCE=.\TMSRpt23.c
# End Source File
# Begin Source File

SOURCE=.\TMSRpt24.c
# End Source File
# Begin Source File

SOURCE=.\TMSRpt25.c
# End Source File
# Begin Source File

SOURCE=.\Tmsrpt26.c
# End Source File
# Begin Source File

SOURCE=.\Tmsrpt27.c
# End Source File
# Begin Source File

SOURCE=.\Tmsrpt28.c
# End Source File
# Begin Source File

SOURCE=.\Tmsrpt29.c
# End Source File
# Begin Source File

SOURCE=.\Tmsrpt30.c
# End Source File
# Begin Source File

SOURCE=.\TMSRpt31.c
# End Source File
# Begin Source File

SOURCE=.\Tmsrpt32.c
# End Source File
# Begin Source File

SOURCE=.\Tmsrpt33.c
# End Source File
# Begin Source File

SOURCE=.\Tmsrpt34.c
# End Source File
# Begin Source File

SOURCE=.\Tmsrpt35.c
# End Source File
# Begin Source File

SOURCE=.\Tmsrpt36.c
# End Source File
# Begin Source File

SOURCE=.\TMSRpt37.c
# End Source File
# Begin Source File

SOURCE=.\TMSRpt38.c
# End Source File
# Begin Source File

SOURCE=.\Tmsrpt39.c
# End Source File
# Begin Source File

SOURCE=.\Tmsrpt40.c
# End Source File
# Begin Source File

SOURCE=.\Tmsrpt41.c
# End Source File
# Begin Source File

SOURCE=.\TMSRpt42.c
# End Source File
# Begin Source File

SOURCE=.\TMSRPT43.c
# End Source File
# Begin Source File

SOURCE=.\TMSRpt44.c
# End Source File
# Begin Source File

SOURCE=.\TMSRpt45.c
# End Source File
# Begin Source File

SOURCE=.\TMSRpt46.c
# End Source File
# Begin Source File

SOURCE=.\Tmsrpt47.c
# End Source File
# Begin Source File

SOURCE=.\Tmsrpt48.c
# End Source File
# Begin Source File

SOURCE=.\TMSRpt49.c
# End Source File
# Begin Source File

SOURCE=.\TMSRpt50.c
# End Source File
# Begin Source File

SOURCE=.\TMSRpt51.c
# End Source File
# Begin Source File

SOURCE=.\TMSRpt52.c
# End Source File
# Begin Source File

SOURCE=.\Tmsrpt53.c
# End Source File
# Begin Source File

SOURCE=.\TMSRpt54.c
# End Source File
# Begin Source File

SOURCE=.\Tmsrpt55.c
# End Source File
# Begin Source File

SOURCE=.\Tmsrpt56.c
# End Source File
# Begin Source File

SOURCE=.\Tmsrpt57.c
# End Source File
# Begin Source File

SOURCE=.\TMSRpt58.c
# End Source File
# Begin Source File

SOURCE=.\Tmsrpt59.c
# End Source File
# Begin Source File

SOURCE=.\TMSRPT60.c
# End Source File
# Begin Source File

SOURCE=.\Tmsrpt61.c
# End Source File
# Begin Source File

SOURCE=.\Tmsrpt62.c
# End Source File
# Begin Source File

SOURCE=.\TMSRpt63.c
# End Source File
# Begin Source File

SOURCE=.\Tmsrpt64.c
# End Source File
# Begin Source File

SOURCE=.\Tmsrpt65.c
# End Source File
# Begin Source File

SOURCE=.\Tmsrpt66.c
# End Source File
# Begin Source File

SOURCE=.\Tmsrpt67.c
# End Source File
# Begin Source File

SOURCE=.\TMSRpt68.c
# End Source File
# Begin Source File

SOURCE=.\Tmsrpt69.c
# End Source File
# Begin Source File

SOURCE=.\Tmsrpt70.cpp
# End Source File
# Begin Source File

SOURCE=.\Tmsrpt71.c
# End Source File
# Begin Source File

SOURCE=.\Tmsrpt72.c
# End Source File
# Begin Source File

SOURCE=.\TMSRpt73.c
# End Source File
# Begin Source File

SOURCE=.\TMSRpt74.c
# End Source File
# Begin Source File

SOURCE=.\TMSRpt75.c
# End Source File
# Begin Source File

SOURCE=.\Tmsrpt76.cpp
# End Source File
# Begin Source File

SOURCE=.\Tmsrpt77.c
# End Source File
# Begin Source File

SOURCE=.\Tmsrpt78.c
# End Source File
# Begin Source File

SOURCE=.\TMSRpt79.cpp
# End Source File
# Begin Source File

SOURCE=.\TMSRpt80.cpp
# End Source File
# Begin Source File

SOURCE=.\Tmsrpt81.c
# End Source File
# Begin Source File

SOURCE=.\Tmsrpt82.c
# End Source File
# Begin Source File

SOURCE=.\TMSRpt83.cpp
# End Source File
# Begin Source File

SOURCE=.\Tmsrpt84.c
# End Source File
# Begin Source File

SOURCE=.\Tmsrpt85.c
# End Source File
# Begin Source File

SOURCE=.\TMSRpt86.c
# End Source File
# Begin Source File

SOURCE=.\Tmsrpt87.c
# End Source File
# Begin Source File

SOURCE=.\Tmsrpt88.c
# End Source File
# Begin Source File

SOURCE=.\Trapezoid.cpp
# End Source File
# Begin Source File

SOURCE=.\TRAVELINSTRUCTIONS.c
# End Source File
# Begin Source File

SOURCE=.\TripGraph.cpp
# End Source File
# Begin Source File

SOURCE=.\TripperRunCut.c
# End Source File
# Begin Source File

SOURCE=.\TRIPPLAN.c
# End Source File
# Begin Source File

SOURCE=.\TripPlanRequestReply.cpp
# End Source File
# Begin Source File

SOURCE=.\tstring.cpp
# End Source File
# Begin Source File

SOURCE=.\TwoPieceCleanup.cpp
# End Source File
# Begin Source File

SOURCE=.\UNBLOCKEDT.c
# End Source File
# Begin Source File

SOURCE=.\UNDOROSTER.c
# End Source File
# Begin Source File

SOURCE=.\UndoRuns.cpp
# End Source File
# Begin Source File

SOURCE=.\UNHOOKTRIPS.c
# End Source File
# Begin Source File

SOURCE=.\UnpaidTimeFilter.cpp
# End Source File
# Begin Source File

SOURCE=.\UNPAIRS.CPP
# End Source File
# Begin Source File

SOURCE=.\UntilTime.cpp
# End Source File
# Begin Source File

SOURCE=.\Username.c
# End Source File
# Begin Source File

SOURCE=.\utils.c
# End Source File
# Begin Source File

SOURCE=.\ViewMenu.cpp
# End Source File
# Begin Source File

SOURCE=.\VisualRuncutter.cpp
# End Source File
# Begin Source File

SOURCE=.\VRColors.cpp
# End Source File
# Begin Source File

SOURCE=.\WREAD.CPP
# End Source File
# Begin Source File

SOURCE=.\WriteExcel.c
# End Source File
# Begin Source File

SOURCE=.\WriteWorkrules.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter ".h"
# Begin Source File

SOURCE=.\_line3dh.h
# End Source File
# Begin Source File

SOURCE=.\AddBus.h
# End Source File
# Begin Source File

SOURCE=.\AddBustype.h
# End Source File
# Begin Source File

SOURCE=.\AddComment.h
# End Source File
# Begin Source File

SOURCE=.\AddConnection.h
# End Source File
# Begin Source File

SOURCE=.\AddConnectionSD.h
# End Source File
# Begin Source File

SOURCE=.\AddCrewOnly.h
# End Source File
# Begin Source File

SOURCE=.\AddDialogs.h
# End Source File
# Begin Source File

SOURCE=.\AddDirection.h
# End Source File
# Begin Source File

SOURCE=.\AddDiscipline.h
# End Source File
# Begin Source File

SOURCE=.\AddDivision.h
# End Source File
# Begin Source File

SOURCE=.\AddDriver.h
# End Source File
# Begin Source File

SOURCE=.\AddDriver01.h
# End Source File
# Begin Source File

SOURCE=.\AddDriver02.h
# End Source File
# Begin Source File

SOURCE=.\AddDriver03.h
# End Source File
# Begin Source File

SOURCE=.\AddDriver04.h
# End Source File
# Begin Source File

SOURCE=.\AddDriverPropSheet.h
# End Source File
# Begin Source File

SOURCE=.\AddDriverType.h
# End Source File
# Begin Source File

SOURCE=.\AddExtraTrip.h
# End Source File
# Begin Source File

SOURCE=.\AddHoliday.h
# End Source File
# Begin Source File

SOURCE=.\AddNode.h
# End Source File
# Begin Source File

SOURCE=.\AddRoute.h
# End Source File
# Begin Source File

SOURCE=.\AddTimecheck.h
# End Source File
# Begin Source File

SOURCE=.\AssignDriver.h
# End Source File
# Begin Source File

SOURCE=.\AuditFilter.h
# End Source File
# Begin Source File

SOURCE=.\BitmapCtrl.h
# End Source File
# Begin Source File

SOURCE=.\BitmapDialog.h
# End Source File
# Begin Source File

SOURCE=.\Block.h
# End Source File
# Begin Source File

SOURCE=.\BlockPairs.h
# End Source File
# Begin Source File

SOURCE=.\BlockProperties.h
# End Source File
# Begin Source File

SOURCE=.\Boundaries.h
# End Source File
# Begin Source File

SOURCE=.\bpsolve.h
# End Source File
# Begin Source File

SOURCE=.\Btitypes.h
# End Source File
# Begin Source File

SOURCE=.\BTRAPI.H
# End Source File
# Begin Source File

SOURCE=.\BTRCONST.H
# End Source File
# Begin Source File

SOURCE=.\btrieve.h
# End Source File
# Begin Source File

SOURCE=.\cc.h
# End Source File
# Begin Source File

SOURCE=.\CCDisplayLog.h
# End Source File
# Begin Source File

SOURCE=.\CCFrontEnd.h
# End Source File
# Begin Source File

SOURCE=.\CCLogin.h
# End Source File
# Begin Source File

SOURCE=.\CCProcess.h
# End Source File
# Begin Source File

SOURCE=.\CCReports.h
# End Source File
# Begin Source File

SOURCE=.\CCSupervisor.h
# End Source File
# Begin Source File

SOURCE=.\ChildFrm.h
# End Source File
# Begin Source File

SOURCE=.\ChooseDate.h
# End Source File
# Begin Source File

SOURCE=.\Cis.h
# End Source File
# Begin Source File

SOURCE=.\Cistms.h
# End Source File
# Begin Source File

SOURCE=.\colorbtn.h
# End Source File
# Begin Source File

SOURCE=.\COMPARE.H
# End Source File
# Begin Source File

SOURCE=.\constants.h
# End Source File
# Begin Source File

SOURCE=.\Context.HH
# End Source File
# Begin Source File

SOURCE=.\CopyRoster.h
# End Source File
# Begin Source File

SOURCE=.\CopyRuncut.h
# End Source File
# Begin Source File

SOURCE=.\cover.h
# End Source File
# Begin Source File

SOURCE=.\crvtrackcursorinfo.h
# End Source File
# Begin Source File

SOURCE=.\Crystal.h
# End Source File
# Begin Source File

SOURCE=.\crystalreportviewer11.h
# End Source File
# Begin Source File

SOURCE=.\CSHUSER.H
# End Source File
# Begin Source File

SOURCE=.\custcom.h
# End Source File
# Begin Source File

SOURCE=.\CustomControls.h
# End Source File
# Begin Source File

SOURCE=.\Customer.h
# End Source File
# Begin Source File

SOURCE=.\CustomerComment.h
# End Source File
# Begin Source File

SOURCE=.\CutRuns.h
# End Source File
# Begin Source File

SOURCE=.\DailyOps.h
# End Source File
# Begin Source File

SOURCE=.\DailyOpsAB.h
# End Source File
# Begin Source File

SOURCE=.\DailyOpsAL.h
# End Source File
# Begin Source File

SOURCE=.\DailyOpsBL.h
# End Source File
# Begin Source File

SOURCE=.\DailyOpsBP.h
# End Source File
# Begin Source File

SOURCE=.\DailyOpsBS.h
# End Source File
# Begin Source File

SOURCE=.\DailyOpsFU.h
# End Source File
# Begin Source File

SOURCE=.\DailyOpsHeader.h
# End Source File
# Begin Source File

SOURCE=.\DailyOpsHO.h
# End Source File
# Begin Source File

SOURCE=.\DailyOpsOOS.h
# End Source File
# Begin Source File

SOURCE=.\DailyOpsRL.h
# End Source File
# Begin Source File

SOURCE=.\DailyOpsRS.h
# End Source File
# Begin Source File

SOURCE=.\DailyOpsRSO.h
# End Source File
# Begin Source File

SOURCE=.\DailyOpsSO.h
# End Source File
# Begin Source File

SOURCE=.\DailyOpsSS.h
# End Source File
# Begin Source File

SOURCE=.\DailyOpsUA.h
# End Source File
# Begin Source File

SOURCE=.\DailyOpsXB.h
# End Source File
# Begin Source File

SOURCE=.\Database.h
# End Source File
# Begin Source File

SOURCE=.\dataformatdisp.h
# End Source File
# Begin Source File

SOURCE=.\DateRange.h
# End Source File
# Begin Source File

SOURCE=.\DaysOffPatterns.h
# End Source File
# Begin Source File

SOURCE=.\DEBUG.H
# End Source File
# Begin Source File

SOURCE=.\dheap.h
# End Source File
# Begin Source File

SOURCE=.\dib256.h
# End Source File
# Begin Source File

SOURCE=.\dibpal.h
# End Source File
# Begin Source File

SOURCE=.\Discipline.h
# End Source File
# Begin Source File

SOURCE=.\DisciplineFilter.h
# End Source File
# Begin Source File

SOURCE=.\DisciplineLetter.h
# End Source File
# Begin Source File

SOURCE=.\DisciplineLetterTemplate.h
# End Source File
# Begin Source File

SOURCE=.\DispatchSheet.h
# End Source File
# Begin Source File

SOURCE=.\DisplayRoster.h
# End Source File
# Begin Source File

SOURCE=.\distance.h
# End Source File
# Begin Source File

SOURCE=.\DOCRoster.h
# End Source File
# Begin Source File

SOURCE=.\driverprofile.h
# End Source File
# Begin Source File

SOURCE=.\DriversToConsider.h
# End Source File
# Begin Source File

SOURCE=.\dtpicker.h
# End Source File
# Begin Source File

SOURCE=.\EventReport.h
# End Source File
# Begin Source File

SOURCE=.\excel.h
# End Source File
# Begin Source File

SOURCE=.\Expiries.h
# End Source File
# Begin Source File

SOURCE=.\ExternalTextFiles.h
# End Source File
# Begin Source File

SOURCE=.\f1book.h
# End Source File
# Begin Source File

SOURCE=.\f1book1.h
# End Source File
# Begin Source File

SOURCE=.\f1bookview.h
# End Source File
# Begin Source File

SOURCE=.\f1bookview1.h
# End Source File
# Begin Source File

SOURCE=.\f1cellformat.h
# End Source File
# Begin Source File

SOURCE=.\f1cellformat1.h
# End Source File
# Begin Source File

SOURCE=.\F1ControlSpecs.h
# End Source File
# Begin Source File

SOURCE=.\f1filespec.h
# End Source File
# Begin Source File

SOURCE=.\f1filespec1.h
# End Source File
# Begin Source File

SOURCE=.\f1findreplaceinfo.h
# End Source File
# Begin Source File

SOURCE=.\f1numberformat.h
# End Source File
# Begin Source File

SOURCE=.\f1numberformat1.h
# End Source File
# Begin Source File

SOURCE=.\f1objpos.h
# End Source File
# Begin Source File

SOURCE=.\f1objpos1.h
# End Source File
# Begin Source File

SOURCE=.\f1odbcconnect.h
# End Source File
# Begin Source File

SOURCE=.\f1odbcconnect1.h
# End Source File
# Begin Source File

SOURCE=.\f1odbcquery.h
# End Source File
# Begin Source File

SOURCE=.\f1odbcquery1.h
# End Source File
# Begin Source File

SOURCE=.\f1pagesetup.h
# End Source File
# Begin Source File

SOURCE=.\f1rangeref.h
# End Source File
# Begin Source File

SOURCE=.\f1rangeref1.h
# End Source File
# Begin Source File

SOURCE=.\f1rect.h
# End Source File
# Begin Source File

SOURCE=.\f1rect1.h
# End Source File
# Begin Source File

SOURCE=.\f1replaceresults.h
# End Source File
# Begin Source File

SOURCE=.\f1replaceresults1.h
# End Source File
# Begin Source File

SOURCE=.\Fares.h
# End Source File
# Begin Source File

SOURCE=.\FastRand.h
# End Source File
# Begin Source File

SOURCE=.\FindBlock.h
# End Source File
# Begin Source File

SOURCE=.\FindCustomer.h
# End Source File
# Begin Source File

SOURCE=.\flatsb.h
# End Source File
# Begin Source File

SOURCE=.\font.h
# End Source File
# Begin Source File

SOURCE=.\font1.h
# End Source File
# Begin Source File

SOURCE=.\Garages.h
# End Source File
# Begin Source File

SOURCE=.\Hash.h
# End Source File
# Begin Source File

SOURCE=.\HashFI.h
# End Source File
# Begin Source File

SOURCE=.\HashObj.h
# End Source File
# Begin Source File

SOURCE=.\HashObjFI.h
# End Source File
# Begin Source File

SOURCE=.\ILGBlock.h
# End Source File
# Begin Source File

SOURCE=.\ILGPattern.h
# End Source File
# Begin Source File

SOURCE=.\ILGRoster.h
# End Source File
# Begin Source File

SOURCE=.\ILGRouteDirection.h
# End Source File
# Begin Source File

SOURCE=.\ILGRoutes.h
# End Source File
# Begin Source File

SOURCE=.\ILGRouteStops.h
# End Source File
# Begin Source File

SOURCE=.\ILGRouteTimePoints.h
# End Source File
# Begin Source File

SOURCE=.\ILGRuns.h
# End Source File
# Begin Source File

SOURCE=.\ILGServiceType.h
# End Source File
# Begin Source File

SOURCE=.\ILGStops.h
# End Source File
# Begin Source File

SOURCE=.\ILGTimePoints.h
# End Source File
# Begin Source File

SOURCE=.\ILGTimeTable.h
# End Source File
# Begin Source File

SOURCE=.\ILGTrips.h
# End Source File
# Begin Source File

SOURCE=.\ILGTripStops.h
# End Source File
# Begin Source File

SOURCE=.\ILGVersionInfo.h
# End Source File
# Begin Source File

SOURCE=.\IMapi.h
# End Source File
# Begin Source File

SOURCE=.\InfoBar.h
# End Source File
# Begin Source File

SOURCE=.\insectc.h
# End Source File
# Begin Source File

SOURCE=.\JumpToRun.h
# End Source File
# Begin Source File

SOURCE=".\LatLong-UTMconversion.h"
# End Source File
# Begin Source File

SOURCE=.\List.H
# End Source File
# Begin Source File

SOURCE=.\LPFILE.H
# End Source File
# Begin Source File

SOURCE=.\LPGEN.H
# End Source File
# Begin Source File

SOURCE=.\LPGLOB.H
# End Source File
# Begin Source File

SOURCE=.\Lpkit.h
# End Source File
# Begin Source File

SOURCE=.\MainFrm.h
# End Source File
# Begin Source File

SOURCE=.\ManualCut.h
# End Source File
# Begin Source File

SOURCE=.\MAPBASIC.H
# End Source File
# Begin Source File

SOURCE=.\Mapinfow.h
# End Source File
# Begin Source File

SOURCE=.\MATCH.H
# End Source File
# Begin Source File

SOURCE=.\mbpsolve.h
# End Source File
# Begin Source File

SOURCE=.\mm.h
# End Source File
# Begin Source File

SOURCE=.\msmask.h
# End Source File
# Begin Source File

SOURCE=.\Netblock.h
# End Source File
# Begin Source File

SOURCE=.\NETWORK.H
# End Source File
# Begin Source File

SOURCE=.\NetworkSimplex.h
# End Source File
# Begin Source File

SOURCE=.\nextPrime.h
# End Source File
# Begin Source File

SOURCE=.\NumericEdit.h
# End Source File
# Begin Source File

SOURCE=.\OnTimePerformance.h
# End Source File
# Begin Source File

SOURCE=.\OnTimePerformance2.h
# End Source File
# Begin Source File

SOURCE=.\OpenDatabase.h
# End Source File
# Begin Source File

SOURCE=.\pagesetup.h
# End Source File
# Begin Source File

SOURCE=.\PairingHeap.h
# End Source File
# Begin Source File

SOURCE=.\parseMIF.h
# End Source File
# Begin Source File

SOURCE=.\ParseStream.h
# End Source File
# Begin Source File

SOURCE=.\PayrollLayout.h
# End Source File
# Begin Source File

SOURCE=.\PayrollSelections.h
# End Source File
# Begin Source File

SOURCE=.\PegboardColors.h
# End Source File
# Begin Source File

SOURCE=.\PickDate.h
# End Source File
# Begin Source File

SOURCE=.\picture.h
# End Source File
# Begin Source File

SOURCE=.\picture1.h
# End Source File
# Begin Source File

SOURCE=.\PlaceHolder.h
# End Source File
# Begin Source File

SOURCE=.\PlotLeft.h
# End Source File
# Begin Source File

SOURCE=.\PremiumDefinition.h
# End Source File
# Begin Source File

SOURCE=.\Prototypes.h
# End Source File
# Begin Source File

SOURCE=.\qtree.h
# End Source File
# Begin Source File

SOURCE=.\quickSort.h
# End Source File
# Begin Source File

SOURCE=.\RBLOCK.H
# End Source File
# Begin Source File

SOURCE=.\rbtreemap.h
# End Source File
# Begin Source File

SOURCE=.\rbtreeset.h
# End Source File
# Begin Source File

SOURCE=.\RC.H
# End Source File
# Begin Source File

SOURCE=.\RCPROTO.H
# End Source File
# Begin Source File

SOURCE=.\ReassignRuntype.h
# End Source File
# Begin Source File

SOURCE=.\Recost.h
# End Source File
# Begin Source File

SOURCE=.\Recursive.h
# End Source File
# Begin Source File

SOURCE=.\RefCount.h
# End Source File
# Begin Source File

SOURCE=.\ResizingDialog.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\Ridership.h
# End Source File
# Begin Source File

SOURCE=.\Roster.h
# End Source File
# Begin Source File

SOURCE=.\RosterImprove.h
# End Source File
# Begin Source File

SOURCE=.\RosterMerge.h
# End Source File
# Begin Source File

SOURCE=.\RosterOrder.h
# End Source File
# Begin Source File

SOURCE=.\RosterParms.h
# End Source File
# Begin Source File

SOURCE=.\RosterWeek.h
# End Source File
# Begin Source File

SOURCE=.\Runcut.h
# End Source File
# Begin Source File

SOURCE=.\Rundef.h
# End Source File
# Begin Source File

SOURCE=.\RunProperties.h
# End Source File
# Begin Source File

SOURCE=.\RunSummary.h
# End Source File
# Begin Source File

SOURCE=.\sdigrid.h
# End Source File
# Begin Source File

SOURCE=.\selbookmarks.h
# End Source File
# Begin Source File

SOURCE=.\SelectPatterns.h
# End Source File
# Begin Source File

SOURCE=.\Skiplist.h
# End Source File
# Begin Source File

SOURCE=.\Snbuild.h
# End Source File
# Begin Source File

SOURCE=.\snbuildp.h
# End Source File
# Begin Source File

SOURCE=.\snsolve.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\stddataformatsdisp.h
# End Source File
# Begin Source File

SOURCE=.\StopConnection.h
# End Source File
# Begin Source File

SOURCE=.\StrStripCmp.h
# End Source File
# Begin Source File

SOURCE=.\StrTrim.h
# End Source File
# Begin Source File

SOURCE=.\SuggestSuspension.h
# End Source File
# Begin Source File

SOURCE=.\SuspensionPending.h
# End Source File
# Begin Source File

SOURCE=.\TaskPanel.h
# End Source File
# Begin Source File

SOURCE=.\TextAbsenceReasons.h
# End Source File
# Begin Source File

SOURCE=.\TextActionCodes.h
# End Source File
# Begin Source File

SOURCE=.\TextBusSwapReasons.h
# End Source File
# Begin Source File

SOURCE=.\TextCategories.h
# End Source File
# Begin Source File

SOURCE=.\TextClassificationCodes.h
# End Source File
# Begin Source File

SOURCE=.\TextDepartments.h
# End Source File
# Begin Source File

SOURCE=.\TextExtraTimeReasons.h
# End Source File
# Begin Source File

SOURCE=.\TextOtherComments.h
# End Source File
# Begin Source File

SOURCE=.\TextPassengerLoads.h
# End Source File
# Begin Source File

SOURCE=.\TextPayrollLayout.h
# End Source File
# Begin Source File

SOURCE=.\TextReferrals.h
# End Source File
# Begin Source File

SOURCE=.\TextRoadSupervisors.h
# End Source File
# Begin Source File

SOURCE=.\TextStopFlags.h
# End Source File
# Begin Source File

SOURCE=.\TextSupervisor.h
# End Source File
# Begin Source File

SOURCE=.\TextTrafficConditions.h
# End Source File
# Begin Source File

SOURCE=.\TextTypical.h
# End Source File
# Begin Source File

SOURCE=.\TextViolationActions.h
# End Source File
# Begin Source File

SOURCE=.\TextViolationCategories.h
# End Source File
# Begin Source File

SOURCE=.\TextViolations.h
# End Source File
# Begin Source File

SOURCE=.\TextWeatherConditions.h
# End Source File
# Begin Source File

SOURCE=.\TFormat.h
# End Source File
# Begin Source File

SOURCE=.\Themes.h
# End Source File
# Begin Source File

SOURCE=.\TimeInterval.h
# End Source File
# Begin Source File

SOURCE=.\Timer.h
# End Source File
# Begin Source File

SOURCE=.\tms.h
# End Source File
# Begin Source File

SOURCE=.\tms.hh
# End Source File
# Begin Source File

SOURCE=.\TmsHeader.h
# End Source File
# Begin Source File

SOURCE=.\Tmsrpt.h
# End Source File
# Begin Source File

SOURCE=.\toolbar.h
# End Source File
# Begin Source File

SOURCE=.\toolbars.h
# End Source File
# Begin Source File

SOURCE=.\Trapezoid.h
# End Source File
# Begin Source File

SOURCE=.\TripGraph.h
# End Source File
# Begin Source File

SOURCE=.\tstring.h
# End Source File
# Begin Source File

SOURCE=.\UndoRuns.h
# End Source File
# Begin Source File

SOURCE=.\UnpaidTimeFilter.h
# End Source File
# Begin Source File

SOURCE=.\UntilTime.h
# End Source File
# Begin Source File

SOURCE=.\VCF15.h
# End Source File
# Begin Source File

SOURCE=.\VisualRuncutter.h
# End Source File
# Begin Source File

SOURCE=.\VRColors.h
# End Source File
# End Group
# Begin Group "Resources"

# PROP Default_Filter ".rc"
# Begin Source File

SOURCE=.\tms.rc
# End Source File
# End Group
# Begin Group "Libraries"

# PROP Default_Filter ".lib"
# Begin Source File

SOURCE=.\lib\crpe32.lib
# End Source File
# Begin Source File

SOURCE=.\lib\crpe32m.lib
# End Source File
# Begin Source File

SOURCE=.\lib\Csh.lib
# End Source File
# Begin Source File

SOURCE=.\lib\LZ32.LIB
# End Source File
# Begin Source File

SOURCE=.\lib\Otmdll.lib
# End Source File
# Begin Source File

SOURCE=.\lib\VERSION.LIB
# End Source File
# Begin Source File

SOURCE=.\lib\WBTRV32.LIB
# End Source File
# Begin Source File

SOURCE=.\lib\msxml6.lib
# End Source File
# Begin Source File

SOURCE=.\lib\WININET.LIB
# End Source File
# Begin Source File

SOURCE=.\lib\RTTExternal.lib
# End Source File
# End Group
# Begin Group "Graphics"

# PROP Default_Filter "*.bmp;*.cur;*.ico"
# Begin Source File

SOURCE=.\res\bitmap1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\BMP00001.BMP
# End Source File
# Begin Source File

SOURCE=.\res\bmp00002.bmp
# End Source File
# Begin Source File

SOURCE=.\res\cur00001.cur
# End Source File
# Begin Source File

SOURCE=.\res\cur00002.cur
# End Source File
# Begin Source File

SOURCE=.\res\cur00003.cur
# End Source File
# Begin Source File

SOURCE=.\res\cursor1.cur
# End Source File
# Begin Source File

SOURCE=.\res\icon1.ico
# End Source File
# Begin Source File

SOURCE=.\res\nophoto.bmp
# End Source File
# Begin Source File

SOURCE=.\res\runcutti.ico
# End Source File
# Begin Source File

SOURCE=.\res\system_t.ico
# End Source File
# Begin Source File

SOURCE=.\res\TMS.ico
# End Source File
# Begin Source File

SOURCE=.\res\TMS4.ico
# End Source File
# Begin Source File

SOURCE=.\res\TMSDoc.ico
# End Source File
# Begin Source File

SOURCE=.\res\Toolbar.bmp
# End Source File
# Begin Source File

SOURCE=.\res\TOOLBAR1.BMP
# End Source File
# Begin Source File

SOURCE=.\res\toolbar3.bmp
# End Source File
# Begin Source File

SOURCE=.\res\trips.ico
# End Source File
# Begin Source File

SOURCE=.\res\vrcolorb.bmp
# End Source File
# Begin Source File

SOURCE=.\res\watch.bmp
# End Source File
# End Group
# End Target
# End Project
# Section tms : {E675F3F0-91B5-11D0-9484-00A0C91110ED}
# 	2:5:Class:CDataFormatDisp
# 	2:10:HeaderFile:dataformatdisp.h
# 	2:8:ImplFile:dataformatdisp.cpp
# End Section
# Section tms : {CDE57A54-8B86-11D0-B3C6-00A0C90AEA82}
# 	2:5:Class:CSplit
# 	2:10:HeaderFile:split.h
# 	2:8:ImplFile:split.cpp
# End Section
# Section tms : {99FF4676-FFC3-11D0-BD02-00C04FC2FB86}
# 	2:5:Class:CStdDataFormatsDisp
# 	2:10:HeaderFile:stddataformatsdisp.h
# 	2:8:ImplFile:stddataformatsdisp.cpp
# End Section
# Section tms : {13E5102B-A52B-11D0-86DA-00608CB9FBFB}
# 	2:5:Class:CF1ReplaceResults1
# 	2:10:HeaderFile:f1replaceresults1.h
# 	2:8:ImplFile:f1replaceresults1.cpp
# End Section
# Section tms : {3FA3EA6B-D886-4689-87E5-2EFB3101F16B}
# 	2:5:Class:CCrystalReportViewer11
# 	2:10:HeaderFile:crystalreportviewer11.h
# 	2:8:ImplFile:crystalreportviewer11.cpp
# End Section
# Section tms : {13E51003-A52B-11D0-86DA-00608CB9FBFB}
# 	2:21:DefaultSinkHeaderFile:f1book1.h
# 	2:16:DefaultSinkClass:CF1Book1
# End Section
# Section tms : {CDE57A53-8B86-11D0-B3C6-00A0C90AEA82}
# 	2:5:Class:CSplits
# 	2:10:HeaderFile:splits.h
# 	2:8:ImplFile:splits.cpp
# End Section
# Section tms : {CDE57A43-8B86-11D0-B3C6-00A0C90AEA82}
# 	2:21:DefaultSinkHeaderFile:datagrid.h
# 	2:16:DefaultSinkClass:CDataGrid
# End Section
# Section tms : {CDE57A4F-8B86-11D0-B3C6-00A0C90AEA82}
# 	2:5:Class:CColumn
# 	2:10:HeaderFile:column.h
# 	2:8:ImplFile:column.cpp
# End Section
# Section tms : {13E51023-A52B-11D0-86DA-00608CB9FBFB}
# 	2:5:Class:CF1FileSpec1
# 	2:10:HeaderFile:f1filespec1.h
# 	2:8:ImplFile:f1filespec1.cpp
# End Section
# Section tms : {C3E95CF4-64BC-11D3-8D9C-4EE4B9000000}
# 	2:5:Class:CSDIGrid
# 	2:10:HeaderFile:sdigrid.h
# 	2:8:ImplFile:sdigrid.cpp
# End Section
# Section tms : {BEF6E003-A874-101A-8BBA-00AA00300CAB}
# 	2:5:Class:COleFont1
# 	2:10:HeaderFile:font1.h
# 	2:8:ImplFile:font1.cpp
# End Section
# Section tms : {13E51027-A52B-11D0-86DA-00608CB9FBFB}
# 	2:5:Class:CF1ODBCConnect1
# 	2:10:HeaderFile:f1odbcconnect1.h
# 	2:8:ImplFile:f1odbcconnect1.cpp
# End Section
# Section tms : {CDE57A52-8B86-11D0-B3C6-00A0C90AEA82}
# 	2:5:Class:CSelBookmarks
# 	2:10:HeaderFile:selbookmarks.h
# 	2:8:ImplFile:selbookmarks.cpp
# End Section
# Section tms : {7BF80981-BF32-101A-8BBB-00AA00300CAB}
# 	2:5:Class:CPicture
# 	2:10:HeaderFile:picture1.h
# 	2:8:ImplFile:picture1.cpp
# End Section
# Section tms : {13FA5946-561C-11D1-BE3F-00A0C95A6A5C}
# 	2:5:Class:CCRVTrackCursorInfo
# 	2:10:HeaderFile:crvtrackcursorinfo.h
# 	2:8:ImplFile:crvtrackcursorinfo.cpp
# End Section
# Section tms : {13E51032-A52B-11D0-86DA-00608CB9FBFB}
# 	2:5:Class:CF1BookView1
# 	2:10:HeaderFile:f1bookview1.h
# 	2:8:ImplFile:f1bookview1.cpp
# End Section
# Section tms : {13E51042-A52B-11D0-86DA-00608CB9FBFB}
# 	2:5:Class:CF1ObjPos1
# 	2:10:HeaderFile:f1objpos1.h
# 	2:8:ImplFile:f1objpos1.cpp
# End Section
# Section tms : {CDE57A41-8B86-11D0-B3C6-00A0C90AEA82}
# 	2:5:Class:CDataGrid
# 	2:10:HeaderFile:datagrid.h
# 	2:8:ImplFile:datagrid.cpp
# End Section
# Section tms : {13E51001-A52B-11D0-86DA-00608CB9FBFB}
# 	2:5:Class:CF1Book1
# 	2:10:HeaderFile:f1book1.h
# 	2:8:ImplFile:f1book1.cpp
# End Section
# Section tms : {13E5103D-A52B-11D0-86DA-00608CB9FBFB}
# 	2:5:Class:CF1CellFormat1
# 	2:10:HeaderFile:f1cellformat1.h
# 	2:8:ImplFile:f1cellformat1.cpp
# End Section
# Section tms : {FE387538-44A3-11D1-B5B7-0000C09000C4}
# 	2:5:Class:CFlatSB1
# 	2:10:HeaderFile:flatsb1.h
# 	2:8:ImplFile:flatsb1.cpp
# End Section
# Section tms : {6F0892F7-0D44-41C3-BF07-7599873FAA04}
# 	2:21:DefaultSinkHeaderFile:crystalreportviewer11.h
# 	2:16:DefaultSinkClass:CCrystalReportViewer11
# End Section
# Section tms : {CDE57A50-8B86-11D0-B3C6-00A0C90AEA82}
# 	2:5:Class:CColumns
# 	2:10:HeaderFile:columns.h
# 	2:8:ImplFile:columns.cpp
# End Section
# Section tms : {FE38753A-44A3-11D1-B5B7-0000C09000C4}
# 	2:21:DefaultSinkHeaderFile:flatsb1.h
# 	2:16:DefaultSinkClass:CFlatSB1
# End Section
# Section tms : {C3E95CF6-64BC-11D3-8D9C-4EE4B9000000}
# 	2:21:DefaultSinkHeaderFile:sdigrid.h
# 	2:16:DefaultSinkClass:CSDIGrid
# End Section
# Section tms : {13E51035-A52B-11D0-86DA-00608CB9FBFB}
# 	2:5:Class:CF1NumberFormat1
# 	2:10:HeaderFile:f1numberformat1.h
# 	2:8:ImplFile:f1numberformat1.cpp
# End Section
# Section tms : {13E51040-A52B-11D0-86DA-00608CB9FBFB}
# 	2:5:Class:CF1Rect1
# 	2:10:HeaderFile:f1rect1.h
# 	2:8:ImplFile:f1rect1.cpp
# End Section
# Section tms : {13E51020-A52B-11D0-86DA-00608CB9FBFB}
# 	2:5:Class:CF1RangeRef1
# 	2:10:HeaderFile:f1rangeref1.h
# 	2:8:ImplFile:f1rangeref1.cpp
# End Section
# Section tms : {13E51029-A52B-11D0-86DA-00608CB9FBFB}
# 	2:5:Class:CF1ODBCQuery1
# 	2:10:HeaderFile:f1odbcquery1.h
# 	2:8:ImplFile:f1odbcquery1.cpp
# End Section
