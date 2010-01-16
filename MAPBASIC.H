//============================================================================
// MapInfo version 4.0 - System defines
//----------------------------------------------------------------------------
// This file contains defines useful when programming in the MapBasic
// language.  There are three versions of this file:
//     MAPBASIC.DEF  - MapBasic syntax
//     MAPBASIC.BAS  - Visual Basic syntax
//     MAPBASIC.H    - C/C++ syntax
//----------------------------------------------------------------------------
// The defines in this file are organized into the following sections:
//     General Purpose defines:
//             Macros, Logical constants, Angle conversion, Colors
//     ButtonPadInfo() defines
//     ColumnInfo() defines
//     CommandInfo() defines
//     FileAttr() defines
//     IntersectNodes() parameters
//     LayerInfo() defines
//     MapperInfo() defines
//     MenuItemInfoByID() and MenuItemInfoByHandler() defines
//     ObjectGeography() defines
//     ObjectInfo() defines
//     SearchInfo() defines
//     SelectionInfo() defines
//     Server statement and function defines
//     StringCompare() return values
//     StyleAttr() defines
//     SystemInfo() defines
//     TableInfo() defines
//     WindowInfo() defines
//     Set Next Document Style defines
//     Abbreviated list of error codes
//     Backward Compatibility defines
//============================================================================
// This file is converted into MAPBASIC.H by doing the following:
//   - concatenate MAPBASIC.DEF and MENU.DEF into MAPBASIC.H
//   - search & replace "'" at begining of a line with "//"
//   - search & replace "Define" at begining of a line with "#define"
//   - delete the following sections:
//         * General Purpose defines: Macros, Logical Constants, Angle Conversions
//         * Abbreviated list of error codes
//         * Backward Compatibility defines
//         * Menu constants whose names have changed
//         * Obsolete menu items
//============================================================================
// This file is converted into MAPBASIC.BAS by doing the following:
//   - concatenate MAPBASIC.DEF and MENU.DEF into MAPBASIC.BAS
//   - search & replace "Define <name>" with "Global Const <name> ="
//     e.g. "<Define {[!-z]+} +{[!-z]}" with "Global Const \0 = \1" using Brief
//   - delete the following sections:
//         * General Purpose defines: Macros, Logical Constants, Angle Conversions
//         * Abbreviated list of error codes
//         * Backward Compatibility defines
//         * Menu constants whose names have changed
//         * Obsolete menu items
//============================================================================

//============================================================================
// General Purpose defines
//============================================================================
//----------------------------------------------------------------------------
// Colors
//----------------------------------------------------------------------------
#define BLACK                            0
#define WHITE                            16777215
#define RED                              16711680
#define GREEN                            65280
#define BLUE                             255
#define CYAN                             65535
#define MAGENTA                          16711935
#define YELLOW                           16776960

//============================================================================
// ButtonPadInfo() defines
//============================================================================
#define BTNPAD_INFO_FLOATING             1
#define BTNPAD_INFO_WIDTH                2
#define BTNPAD_INFO_NBTNS                3
#define BTNPAD_INFO_X                    4
#define BTNPAD_INFO_Y                    5
#define BTNPAD_INFO_WINID                6

//============================================================================
// ColumnInfo() defines
//============================================================================
#define COL_INFO_NAME                    1
#define COL_INFO_NUM                     2
#define COL_INFO_TYPE                    3
#define COL_INFO_WIDTH                   4
#define COL_INFO_DECPLACES               5
#define COL_INFO_INDEXED                 6
#define COL_INFO_EDITABLE                7

//----------------------------------------------------------------------------
// Column type defines, returned by ColumnInfo(<col_ref>, COL_INFO_TYPE)
//----------------------------------------------------------------------------
#define COL_TYPE_CHAR                    1
#define COL_TYPE_DECIMAL                 2
#define COL_TYPE_INTEGER                 3
#define COL_TYPE_SMALLINT                4
#define COL_TYPE_DATE                    5
#define COL_TYPE_LOGICAL                 6
#define COL_TYPE_GRAPHIC                 7
#define COL_TYPE_FLOAT                   8

//============================================================================
// CommandInfo() defines
//============================================================================
#define CMD_INFO_X                       1
#define CMD_INFO_Y                       2
#define CMD_INFO_SHIFT                   3
#define CMD_INFO_CTRL                    4
#define CMD_INFO_X2                      5
#define CMD_INFO_Y2                      6
#define CMD_INFO_TOOLBTN                 7
#define CMD_INFO_MENUITEM                8
#define CMD_INFO_WIN                     1
#define CMD_INFO_SELTYPE                 1
#define CMD_INFO_ROWID                   2
#define CMD_INFO_INTERRUPT               3
#define CMD_INFO_STATUS                  1
#define CMD_INFO_MSG                     1000
#define CMD_INFO_DLG_OK                  1
#define CMD_INFO_DLG_DBL                 1
#define CMD_INFO_FIND_RC                 3
#define CMD_INFO_FIND_ROWID              4
#define CMD_INFO_XCMD                    1
#define CMD_INFO_CUSTOM_OBJ              1
#define CMD_INFO_TASK_SWITCH             1

//----------------------------------------------------------------------------
// Task Switch, returned by CommandInfo(CMD_INFO_TASK_SWITCH)
//----------------------------------------------------------------------------
#define SWITCHING_OUT_OF_MAPINFO         0
#define SWITCHING_INTO_MAPINFO           1

//============================================================================
// FileAttr() defines
//============================================================================
#define FILE_ATTR_MODE                   1
#define FILE_ATTR_FILESIZE               2

//----------------------------------------------------------------------------
// File Access modes, returned by FileAttr(<file_id>, FILE_ATTR_MODE)
//----------------------------------------------------------------------------
#define MODE_INPUT                       0
#define MODE_OUTPUT                      1
#define MODE_APPEND                      2
#define MODE_RANDOM                      3
#define MODE_BINARY                      4

//============================================================================
// IntersectNodes(obj1, obj2, mode) parameters
//============================================================================
#define INCL_CROSSINGS                   1
#define INCL_COMMON                      6
#define INCL_ALL                         7

//============================================================================
// LayerInfo() defines
//============================================================================
#define LAYER_INFO_NAME                  1
#define LAYER_INFO_EDITABLE              2
#define LAYER_INFO_SELECTABLE            3
#define LAYER_INFO_ZOOM_LAYERED          4
#define LAYER_INFO_ZOOM_MIN              5
#define LAYER_INFO_ZOOM_MAX              6
#define LAYER_INFO_COSMETIC              7
#define LAYER_INFO_PATH                  8
#define LAYER_INFO_DISPLAY               9
#define LAYER_INFO_OVR_LINE              10
#define LAYER_INFO_OVR_PEN               11
#define LAYER_INFO_OVR_BRUSH             12
#define LAYER_INFO_OVR_SYMBOL            13
#define LAYER_INFO_OVR_FONT              14
#define LAYER_INFO_LBL_EXPR              15
#define LAYER_INFO_LBL_LT                16
#define LAYER_INFO_LBL_CURFONT           17
#define LAYER_INFO_LBL_FONT              18
#define LAYER_INFO_LBL_PARALLEL          19
#define LAYER_INFO_LBL_POS               20
#define LAYER_INFO_ARROWS                21
#define LAYER_INFO_NODES                 22
#define LAYER_INFO_CENTROIDS             23
#define LAYER_INFO_TYPE                  24
#define LAYER_INFO_LBL_VISIBILITY        25
#define LAYER_INFO_LBL_ZOOM_MIN          26
#define LAYER_INFO_LBL_ZOOM_MAX          27
#define LAYER_INFO_LBL_AUTODISPLAY       28
#define LAYER_INFO_LBL_OVERLAP           29
#define LAYER_INFO_LBL_DUPLICATES        30
#define LAYER_INFO_LBL_OFFSET            31
#define LAYER_INFO_LBL_MAX               32

//----------------------------------------------------------------------------
// Display Modes, returned by LayerInfo() for LAYER_INFO_DISPLAY
//----------------------------------------------------------------------------
#define LAYER_INFO_DISPLAY_OFF           0
#define LAYER_INFO_DISPLAY_GRAPHIC       1
#define LAYER_INFO_DISPLAY_GLOBAL        2
#define LAYER_INFO_DISPLAY_VALUE         3

//----------------------------------------------------------------------------
// Label Linetypes, returned by LayerInfo() for LAYER_INFO_LBL_LT
//----------------------------------------------------------------------------
#define LAYER_INFO_LBL_LT_NONE           0
#define LAYER_INFO_LBL_LT_SIMPLE         1
#define LAYER_INFO_LBL_LT_ARROW          2

//----------------------------------------------------------------------------
// Label Positions, returned by LayerInfo() for LAYER_INFO_LBL_POS
//----------------------------------------------------------------------------
#define LAYER_INFO_LBL_POS_CC            0
#define LAYER_INFO_LBL_POS_TL            1
#define LAYER_INFO_LBL_POS_TC            2
#define LAYER_INFO_LBL_POS_TR            3
#define LAYER_INFO_LBL_POS_CL            4
#define LAYER_INFO_LBL_POS_CR            5
#define LAYER_INFO_LBL_POS_BL            6
#define LAYER_INFO_LBL_POS_BC            7
#define LAYER_INFO_LBL_POS_BR            8

//----------------------------------------------------------------------------
// Layer Types, returned by LayerInfo() for LAYER_INFO_TYPE
//----------------------------------------------------------------------------
#define LAYER_INFO_TYPE_NORMAL           0
#define LAYER_INFO_TYPE_COSMETIC         1
#define LAYER_INFO_TYPE_IMAGE            2
#define LAYER_INFO_TYPE_THEMATIC         3

//----------------------------------------------------------------------------
// Label visibility modes, returned by LayerInfo() for LAYER_INFO_LBL_VISIBILITY
//----------------------------------------------------------------------------
#define LAYER_INFO_LBL_VIS_OFF           1
#define LAYER_INFO_LBL_VIS_ZOOM          2
#define LAYER_INFO_LBL_VIS_ON            3

//============================================================================
// MapperInfo() defines
//============================================================================
#define MAPPER_INFO_ZOOM                 1
#define MAPPER_INFO_SCALE                2
#define MAPPER_INFO_CENTERX              3
#define MAPPER_INFO_CENTERY              4 
#define MAPPER_INFO_MINX                 5
#define MAPPER_INFO_MINY                 6
#define MAPPER_INFO_MAXX                 7
#define MAPPER_INFO_MAXY                 8
#define MAPPER_INFO_LAYERS               9
#define MAPPER_INFO_EDIT_LAYER           10
#define MAPPER_INFO_XYUNITS              11
#define MAPPER_INFO_DISTUNITS            12
#define MAPPER_INFO_AREAUNITS            13
#define MAPPER_INFO_SCROLLBARS           14
#define MAPPER_INFO_DISPLAY              15
#define MAPPER_INFO_NUM_THEMATIC         16

//----------------------------------------------------------------------------
// Display Modes, returned by MapperInfo() for MAPPER_INFO_DISPLAY
//----------------------------------------------------------------------------
#define MAPPER_INFO_DISPLAY_SCALE        0
#define MAPPER_INFO_DISPLAY_ZOOM         1
#define MAPPER_INFO_DISPLAY_POSITION     2

//============================================================================
// MenuItemInfoByID() and MenuItemInfoByHandler() defines
//============================================================================
#define MENUITEM_INFO_ENABLED            1
#define MENUITEM_INFO_CHECKED            2
#define MENUITEM_INFO_CHECKABLE          3
#define MENUITEM_INFO_SHOWHIDEABLE       4
#define MENUITEM_INFO_ACCELERATOR        5
#define MENUITEM_INFO_TEXT               6
#define MENUITEM_INFO_HELPMSG            7
#define MENUITEM_INFO_HANDLER            8
#define MENUITEM_INFO_ID                 9

//============================================================================
// ObjectGeography() defines
//============================================================================
#define OBJ_GEO_MINX                     1
#define OBJ_GEO_LINEBEGX                 1
#define OBJ_GEO_POINTX                   1
#define OBJ_GEO_MINY                     2
#define OBJ_GEO_LINEBEGY                 2
#define OBJ_GEO_POINTY                   2
#define OBJ_GEO_MAXX                     3
#define OBJ_GEO_LINEENDX                 3
#define OBJ_GEO_MAXY                     4
#define OBJ_GEO_LINEENDY                 4
#define OBJ_GEO_ARCBEGANGLE              5
#define OBJ_GEO_TEXTLINEX                5
#define OBJ_GEO_ROUNDRADIUS              5
#define OBJ_GEO_ARCENDANGLE              6
#define OBJ_GEO_TEXTLINEY                6
#define OBJ_GEO_TEXTANGLE                7

//============================================================================
// ObjectInfo() defines
//============================================================================
#define OBJ_INFO_TYPE                    1
#define OBJ_INFO_PEN                     2
#define OBJ_INFO_SYMBOL                  2
#define OBJ_INFO_TEXTFONT                2
#define OBJ_INFO_BRUSH                   3
#define OBJ_INFO_NPNTS                   20
#define OBJ_INFO_TEXTSTRING              3
#define OBJ_INFO_SMOOTH                  4
#define OBJ_INFO_FRAMEWIN                4
#define OBJ_INFO_NPOLYGONS               21
#define OBJ_INFO_TEXTSPACING             4
#define OBJ_INFO_TEXTJUSTIFY             5
#define OBJ_INFO_FRAMETITLE              6
#define OBJ_INFO_TEXTARROW               6

//----------------------------------------------------------------------------
// Object types, returned by ObjectInfo(<obj>, OBJ_INFO_TYPE)
//----------------------------------------------------------------------------
#define OBJ_TYPE_ARC                     1
#define OBJ_TYPE_ELLIPSE                 2
#define OBJ_TYPE_LINE                    3
#define OBJ_TYPE_PLINE                   4
#define OBJ_TYPE_POINT                   5
#define OBJ_TYPE_FRAME                   6
#define OBJ_TYPE_REGION                  7
#define OBJ_TYPE_RECT                    8
#define OBJ_TYPE_ROUNDRECT               9
#define OBJ_TYPE_TEXT                    10

//============================================================================
// SearchInfo() defines
//============================================================================
#define SEARCH_INFO_TABLE                1
#define SEARCH_INFO_ROW                  2

//============================================================================
// SelectionInfo() defines
//============================================================================
#define SEL_INFO_TABLENAME               1
#define SEL_INFO_SELNAME                 2
#define SEL_INFO_NROWS                   3

//============================================================================
// Server statement and function defines
//============================================================================
//----------------------------------------------------------------------------
// Return Codes
//----------------------------------------------------------------------------
#define SRV_SUCCESS                      0
#define SRV_SUCCESS_WITH_INFO            1
#define SRV_ERROR                        -1
#define SRV_INVALID_HANDLE               -2
#define SRV_NEED_DATA                    99
#define SRV_NO_MORE_DATA                 100

//----------------------------------------------------------------------------
// Special values for the status associated with a fetched value
//----------------------------------------------------------------------------
#define SRV_NULL_DATA                    -1
#define SRV_TRUNCATED_DATA               -2

//----------------------------------------------------------------------------
// Server_ColumnInfo() Attr defines
//----------------------------------------------------------------------------
#define SRV_COL_INFO_NAME                1
#define SRV_COL_INFO_TYPE                2
#define SRV_COL_INFO_WIDTH               3
#define SRV_COL_INFO_PRECISION           4
#define SRV_COL_INFO_SCALE               5
#define SRV_COL_INFO_VALUE               6
#define SRV_COL_INFO_STATUS              7

//----------------------------------------------------------------------------
// Column types, returned by Server_ColumnInfo(<stmt>,<colno>,SRV_COL_INFO_TYPE)
//----------------------------------------------------------------------------
#define SRV_COL_TYPE_NONE                0
#define SRV_COL_TYPE_CHAR                1
#define SRV_COL_TYPE_DECIMAL             2
#define SRV_COL_TYPE_INTEGER             3
#define SRV_COL_TYPE_SMALLINT            4
#define SRV_COL_TYPE_DATE                5
#define SRV_COL_TYPE_LOGICAL             6
#define SRV_COL_TYPE_FLOAT               8
#define SRV_COL_TYPE_FIXED_LEN_STRING    16
#define SRV_COL_TYPE_BIN_STRING          17

//----------------------------------------------------------------------------
// Server_DriverInfo() Attr defines
//----------------------------------------------------------------------------
#define SRV_DRV_INFO_NAME                1
#define SRV_DRV_INFO_NAME_LIST           2
#define SRV_DRV_DATA_SOURCE              3

//----------------------------------------------------------------------------
// Fetch Directions used by Server_Fetch()
//----------------------------------------------------------------------------
#define SRV_FETCH_NEXT                   -1
#define SRV_FETCH_PREV                   -2
#define SRV_FETCH_FIRST                  -3
#define SRV_FETCH_LAST                   -4

//============================================================================
// StringCompare(<str_1>, <str_2>) return values
//============================================================================
#define STR_LT                           -1
#define STR_GT                           1
#define STR_EQ                           0

//============================================================================
// StyleAttr() defines
//============================================================================
#define PEN_WIDTH                        1
#define PEN_PATTERN                      2
#define PEN_COLOR                        4
#define BRUSH_PATTERN                    1
#define BRUSH_FORECOLOR                  2
#define BRUSH_BACKCOLOR                  3
#define FONT_NAME                        1
#define FONT_STYLE                       2
#define FONT_POINTSIZE                   3
#define FONT_FORECOLOR                   4
#define FONT_BACKCOLOR                   5
#define SYMBOL_CODE                      1
#define SYMBOL_COLOR                     2
#define SYMBOL_POINTSIZE                 3
#define SYMBOL_ANGLE                     4
#define SYMBOL_FONT_NAME                 5
#define SYMBOL_FONT_STYLE                6
#define SYMBOL_KIND                      7
#define SYMBOL_CUSTOM_NAME               8
#define SYMBOL_CUSTOM_STYLE              9

//----------------------------------------------------------------------------
// Symbol kinds returned by StyleAttr(<symbol>, SYMBOL_KIND)
//----------------------------------------------------------------------------
#define SYMBOL_KIND_VECTOR               1
#define SYMBOL_KIND_FONT                 2
#define SYMBOL_KIND_CUSTOM               3

//============================================================================
// SystemInfo() defines
//============================================================================
#define SYS_INFO_PLATFORM                1
#define SYS_INFO_APPVERSION              2
#define SYS_INFO_MIVERSION               3
#define SYS_INFO_RUNTIME                 4
#define SYS_INFO_CHARSET                 5
#define SYS_INFO_COPYPROTECTED           6
#define SYS_INFO_APPLICATIONWND          7
#define SYS_INFO_DDESTATUS               8
#define SYS_INFO_MAPINFOWND              9
#define SYS_INFO_NUMBER_FORMAT           10
#define SYS_INFO_DATE_FORMAT             11
#define SYS_INFO_DIG_INSTALLED           12
#define SYS_INFO_DIG_MODE                13
#define SYS_INFO_MIPLATFORM              14
#define SYS_INFO_MDICLIENTWND            15

//----------------------------------------------------------------------------
// Platform, returned by SystemInfo(SYS_INFO_PLATFORM)
//----------------------------------------------------------------------------
#define PLATFORM_SPECIAL                 0
#define PLATFORM_WIN                     1
#define PLATFORM_MAC                     2
#define PLATFORM_MOTIF                   3
#define PLATFORM_X11                     4
#define PLATFORM_XOL                     5

//----------------------------------------------------------------------------
// Version, returned by SystemInfo(SYS_INFO_MIPLATFORM)
//----------------------------------------------------------------------------
#define MIPLATFORM_SPECIAL               0
#define MIPLATFORM_WIN16                 1
#define MIPLATFORM_WIN32                 2
#define MIPLATFORM_POWERMAC              3
#define MIPLATFORM_MAC68K                4
#define MIPLATFORM_HP                    5
#define MIPLATFORM_SUN                   6

//============================================================================
// TableInfo() defines
//============================================================================
#define TAB_INFO_NAME                    1
#define TAB_INFO_NUM                     2
#define TAB_INFO_TYPE                    3
#define TAB_INFO_NCOLS                   4
#define TAB_INFO_MAPPABLE                5
#define TAB_INFO_READONLY                6
#define TAB_INFO_TEMP                    7
#define TAB_INFO_NROWS                   8
#define TAB_INFO_EDITED                  9
#define TAB_INFO_FASTEDIT                10
#define TAB_INFO_UNDO                    11
#define TAB_INFO_MAPPABLE_TABLE          12
#define TAB_INFO_USERMAP                 13
#define TAB_INFO_USERBROWSE              14
#define TAB_INFO_USERCLOSE               15
#define TAB_INFO_USEREDITABLE            16
#define TAB_INFO_USERREMOVEMAP           17
#define TAB_INFO_USERDISPLAYMAP          18
#define TAB_INFO_TABFILE                 19
#define TAB_INFO_MINX                    20
#define TAB_INFO_MINY                    21
#define TAB_INFO_MAXX                    22
#define TAB_INFO_MAXY                    23
#define TAB_INFO_SEAMLESS                24

//----------------------------------------------------------------------------
// Table type defines, returned by TableInfo(<tab_ref>, TAB_INFO_TYPE)
//----------------------------------------------------------------------------
#define TAB_TYPE_BASE                    1
#define TAB_TYPE_RESULT                  2
#define TAB_TYPE_VIEW                    3
#define TAB_TYPE_IMAGE                   4
#define TAB_TYPE_LINKED                  5

//============================================================================
// WindowInfo() defines
//============================================================================
#define WIN_INFO_NAME                    1
#define WIN_INFO_TYPE                    3
#define WIN_INFO_WIDTH                   4
#define WIN_INFO_HEIGHT                  5
#define WIN_INFO_X                       6
#define WIN_INFO_Y                       7
#define WIN_INFO_TOPMOST                 8
#define WIN_INFO_STATE                   9
#define WIN_INFO_TABLE                   10
#define WIN_INFO_LEGENDS_MAP             10
#define WIN_INFO_OPEN                    11
#define WIN_INFO_WND                     12
#define WIN_INFO_WINDOWID                13
#define WIN_INFO_WORKSPACE               14
#define WIN_INFO_CLONEWINDOW             15
#define WIN_INFO_SYSMENUCLOSE            16
#define WIN_INFO_AUTOSCROLL              17

//----------------------------------------------------------------------------
// Window types, returned by WindowInfo(<win_id>, WIN_INFO_TYPE)
//----------------------------------------------------------------------------
#define WIN_MAPPER                       1
#define WIN_BROWSER                      2
#define WIN_LAYOUT                       3
#define WIN_GRAPH                        4
#define WIN_BUTTONPAD                    19
#define WIN_HELP                         1001
#define WIN_MAPBASIC                     1002
#define WIN_MESSAGE                      1003
#define WIN_RULER                        1007
#define WIN_INFO                         1008
#define WIN_LEGEND                       1009
#define WIN_STATISTICS                   1010
#define WIN_MAPINFO                      1011
//----------------------------------------------------------------------------
// Version 2 window types no longer used in version 3 or version 4
//----------------------------------------------------------------------------
#define WIN_TOOLPICKER                   1004
#define WIN_PENPICKER                    1005
#define WIN_SYMBOLPICKER                 1006

//----------------------------------------------------------------------------
// Window states, returned by WindowInfo(<win_id>, WIN_INFO_STATE)
//----------------------------------------------------------------------------
#define WIN_STATE_NORMAL                 0
#define WIN_STATE_MINIMIZED              1
#define WIN_STATE_MAXIMIZED              2

//============================================================================
// Set Next Document Style defines
//============================================================================
#define WIN_STYLE_STANDARD               0
#define WIN_STYLE_CHILD                  1
#define WIN_STYLE_POPUP_FULLCAPTION      2
#define WIN_STYLE_POPUP                  3

//============================================================================
// end of MAPBASIC.DEF
//============================================================================

//============================================================================
// MapInfo version 4.0 - Menu Item Definitions
//----------------------------------------------------------------------------
// This file contains defines useful when programming in the MapBasic
// language.  The definitions in this file describe the standard MapInfo
// functionality available via the "Run Menu Command" MapBasic statement.
//----------------------------------------------------------------------------
// The defines in this file are organized to match the sequence of
// declarations in the MAPINFOW.MNU file, which in turn reflects the
// organization of the MapInfo menus and buttonpads.
//============================================================================

//----------------------------------------------------------------------------
// File & Send Mail menus
//----------------------------------------------------------------------------
#define M_FILE_NEW                       101
#define M_FILE_OPEN                      102
#define M_FILE_OPEN_ODBC                 116
#define M_FILE_ADD_WORKSPACE             108
#define M_FILE_CLOSE                     103
#define M_FILE_CLOSE_ALL                 104
#define M_FILE_SAVE                      105
#define M_FILE_SAVE_COPY_AS              106
#define M_FILE_SAVE_WORKSPACE            109
#define M_FILE_SAVE_WINDOW_AS            609
#define M_FILE_REVERT                    107
#define M_FILE_RUN                       110
#define M_FILE_PAGE_SETUP                111
#define M_FILE_PRINT                     112
#define M_FILE_EXIT                      113

#define M_SENDMAIL_CURRENTWINDOW         114
#define M_SENDMAIL_WORKSPACE             115

//----------------------------------------------------------------------------
// Edit menu
//----------------------------------------------------------------------------
#define M_EDIT_UNDO                      201
#define M_EDIT_CUT                       202
#define M_EDIT_COPY                      203
#define M_EDIT_PASTE                     204
#define M_EDIT_CLEAR                     205
#define M_EDIT_CLEAROBJ                  206
#define M_EDIT_RESHAPE                   1601
#define M_EDIT_NEW_ROW                   702
#define M_EDIT_GETINFO                   207

//----------------------------------------------------------------------------
// Objects menu
//----------------------------------------------------------------------------
#define M_OBJECTS_SET_TARGET             1610
#define M_OBJECTS_CLEAR_TARGET           1611
#define M_OBJECTS_COMBINE                1605
#define M_OBJECTS_SPLIT                  1612
#define M_OBJECTS_ERASE                  1613
#define M_OBJECTS_ERASE_OUT              1614
#define M_OBJECTS_OVERLAY                1615
#define M_OBJECTS_BUFFER                 1606
#define M_OBJECTS_SMOOTH                 1602
#define M_OBJECTS_UNSMOOTH               1603
#define M_OBJECTS_CVT_PGON               1607
#define M_OBJECTS_CVT_PLINE              1604

//----------------------------------------------------------------------------
// Query menu
//----------------------------------------------------------------------------
#define M_ANALYZE_SELECT                 301
#define M_ANALYZE_SQLQUERY               302
#define M_ANALYZE_SELECTALL              303
#define M_ANALYZE_UNSELECT               304
#define M_ANALYZE_FIND                   305
#define M_ANALYZE_FIND_SELECTION         306
#define M_ANALYZE_CALC_STATISTICS        309

//----------------------------------------------------------------------------
// Table, Maintenance, and Raster menus
//----------------------------------------------------------------------------
#define M_TABLE_UPDATE_COLUMN            405
#define M_TABLE_APPEND                   411
#define M_TABLE_GEOCODE                  407
#define M_TABLE_CREATE_POINTS            408
#define M_TABLE_MERGE_USING_COLUMN       406
#define M_TABLE_IMPORT                   401
#define M_TABLE_EXPORT                   402

#define M_TABLE_MODIFY_STRUCTURE         404
#define M_TABLE_DELETE                   409
#define M_TABLE_RENAME                   410
#define M_TABLE_PACK                     403
#define M_TABLE_MAKEMAPPABLE             415
#define M_TABLE_UNLINK                   416
#define M_TABLE_REFRESH                  417

#define M_TABLE_RASTER_STYLE             414
#define M_TABLE_RASTER_REG               413
#define M_TOOLS_RASTER_REG               1730

//----------------------------------------------------------------------------
// Options menu
//----------------------------------------------------------------------------
#define M_FORMAT_PICK_LINE               501
#define M_FORMAT_PICK_FILL               502
#define M_FORMAT_PICK_SYMBOL             503
#define M_FORMAT_PICK_FONT               504
#define M_WINDOW_BUTTONPAD               605
#define M_WINDOW_LEGEND                  606
#define M_WINDOW_STATISTICS              607
#define M_WINDOW_MAPBASIC                608
#define M_WINDOW_STATUSBAR               616
#define M_FORMAT_CUSTOM_COLORS           617
#define M_EDIT_PREFERENCES               208
#define M_EDIT_PREFERENCES_SYSTEM        210
#define M_EDIT_PREFERENCES_FILE          211
#define M_EDIT_PREFERENCES_MAP           212
#define M_EDIT_PREFERENCES_COUNTRY       213
#define M_EDIT_PREFERENCES_PATH          214

//----------------------------------------------------------------------------
// Window menu
//----------------------------------------------------------------------------
#define M_WINDOW_BROWSE                  601
#define M_WINDOW_MAP                     602
#define M_WINDOW_GRAPH                   603
#define M_WINDOW_LAYOUT                  604
#define M_WINDOW_REDISTRICT              615
#define M_WINDOW_REDRAW                  610
#define M_WINDOW_TILE                    611
#define M_WINDOW_CASCADE                 612
#define M_WINDOW_ARRANGEICONS            613
#define M_WINDOW_MORE                    614
#define M_WINDOW_FIRST                   620
// - - -- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Note: the 2nd through 80th windows can be accessed as (M_WINDOW_FIRST+i-1)
// - - -- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

//----------------------------------------------------------------------------
// Help menu
//----------------------------------------------------------------------------
#define M_HELP_CONTENTS                  1202
#define M_HELP_SEARCH                    1203
#define M_HELP_USE_HELP                  1204
#define M_HELP_TECHSUPPORT               1208
#define M_HELP_CONNECT_MIFORUM           1209
#define M_HELP_ABOUT                     1205

#define M_HELP_CONTEXTSENSITIVE          1201
#define M_HELP_HELPMODE                  1206

//----------------------------------------------------------------------------
// Browse menu
//----------------------------------------------------------------------------
#define M_BROWSE_PICK_FIELDS             704
#define M_BROWSE_OPTIONS                 703

//----------------------------------------------------------------------------
// Map menu
//----------------------------------------------------------------------------
#define M_MAP_LAYER_CONTROL              801
#define M_MAP_THEMATIC                   307
#define M_MAP_MODIFY_THEMATIC            308
#define M_MAP_CHANGE_VIEW                805
#define M_MAP_CLONE_MAPPER               811
#define M_MAP_PREVIOUS                   806
#define M_MAP_ENTIRE_LAYER               807
#define M_MAP_CLEAR_CUSTOM_LABELS        814
#define M_MAP_SAVE_COSMETIC              809
#define M_MAP_CLEAR_COSMETIC             810
#define M_MAP_SET_CLIP_REGION            812
#define M_MAP_CLIP_REGION_ONOFF          813
#define M_MAP_SETUPDIGITIZER             803
#define M_MAP_OPTIONS                    802

//----------------------------------------------------------------------------
// Layout menu
//----------------------------------------------------------------------------
#define M_LAYOUT_CHANGE_VIEW             902
#define M_LAYOUT_ACTUAL                  903
#define M_LAYOUT_ENTIRE                  904
#define M_LAYOUT_PREVIOUS                905
#define M_LAYOUT_BRING2FRONT             906
#define M_LAYOUT_SEND2BACK               907
#define M_LAYOUT_ALIGN                   908
#define M_LAYOUT_DROPSHADOWS             909
#define M_LAYOUT_DISPLAYOPTIONS          901

//----------------------------------------------------------------------------
// Graph menu
//----------------------------------------------------------------------------
#define M_GRAPH_TYPE                     1001
#define M_GRAPH_LABEL_AXIS               1002
#define M_GRAPH_VALUE_AXIS               1003
#define M_GRAPH_SERIES                   1004

//----------------------------------------------------------------------------
// MapBasic menu
//----------------------------------------------------------------------------
#define M_MAPBASIC_CLEAR                 1101
#define M_MAPBASIC_SAVECONTENTS          1102

//----------------------------------------------------------------------------
// Redistrict menu
//----------------------------------------------------------------------------
#define M_REDISTRICT_ASSIGN              705
#define M_REDISTRICT_TARGET              706
#define M_REDISTRICT_ADD                 707
#define M_REDISTRICT_DELETE              708
#define M_REDISTRICT_OPTIONS             709

//----------------------------------------------------------------------------
// Main Buttonpad
//----------------------------------------------------------------------------
#define M_TOOLS_SELECTOR                 1701
#define M_TOOLS_SEARCH_RECT              1722
#define M_TOOLS_SEARCH_RADIUS            1703
#define M_TOOLS_SEARCH_BOUNDARY          1704
#define M_TOOLS_EXPAND                   1705
#define M_TOOLS_SHRINK                   1706
#define M_TOOLS_RECENTER                 1702
#define M_TOOLS_PNT_QUERY                1707
#define M_TOOLS_LABELER                  1708
#define M_TOOLS_DRAGWINDOW               1734
#define M_TOOLS_RULER                    1710

//----------------------------------------------------------------------------
// Drawing Buttonpad
//----------------------------------------------------------------------------
#define M_TOOLS_POINT                    1711
#define M_TOOLS_LINE                     1712
#define M_TOOLS_POLYLINE                 1713
#define M_TOOLS_ARC                      1716
#define M_TOOLS_POLYGON                  1714
#define M_TOOLS_ELLIPSE                  1715
#define M_TOOLS_RECTANGLE                1717
#define M_TOOLS_ROUNDEDRECT              1718
#define M_TOOLS_TEXT                     1709
#define M_TOOLS_FRAME                    1719
#define M_TOOLS_ADD_NODE                 1723

//----------------------------------------------------------------------------
// Menu and ButtonPad items that do not appear in the standard menus
//----------------------------------------------------------------------------
#define M_TOOLS_MAPBASIC                 1720
#define M_TOOLS_SEARCH_POLYGON           1733

//============================================================================
// end of MENU.DEF
//============================================================================

