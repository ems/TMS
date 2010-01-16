#ifndef __VCF15_H__
#define __VCF15_H__

#include <objbase.h>

typedef enum {
    F1ColWidthUnitsCharacters = 0,
    F1ColWidthUnitsTwips = 1
} F1ColWidthUnitsConstants;

typedef enum {
    F1RowHeightAuto = -1
} F1RowHeightConstants;

typedef enum {
    F1Off = 0,
    F1On = 1,
    F1Auto = 2
} F1ShowOffOnAutoConstants;

typedef enum {
    F1TabsOff = 0,
    F1TabsBottom = 1,
    F1TabsTop = 2
} F1ShowTabsConstants;

typedef enum {
    F1ModeNormal = 0,
    F1ModeLine = 1,
    F1ModeRectangle = 2,
    F1ModeOval = 3,
    F1ModeArc = 4,
    F1ModeChart = 5,
    F1ModeField = 6,
    F1ModeButton = 7,
    F1ModePolygon = 8,
    F1ModeCheckBox = 9,
    F1ModeDropDown = 10
} F1ModeConstants;

typedef enum {
    F1PolyEditModeNormal = 0,
    F1PolyEditModePoints = 1
} F1PolyEditModeConstants;

typedef enum {
    F1ShiftHorizontal = 1,
    F1ShiftVertical = 2,
    F1ShiftRows = 3,
    F1ShiftCols = 4,
    F1FixupNormal = 0,
    F1FixupPrepend = 16,
    F1FixupAppend = 32
} F1ShiftTypeConstants;

typedef enum {
    F1ODBCErrorAbort = 0,
    F1ODBCErrorSkipRow = 1,
    F1ODBCErrorTryAgain = 2
} F1ODBCExecuteErrorConstants;

typedef enum {
    F1CDataChar = 0,
    F1CDataDouble = 1,
    F1CDataDate = 2,
    F1CDataTime = 3,
    F1CDataTimeStamp = 4,
    F1CDataBool = 5,
    F1CDataLong = 6
} F1CDataTypesConstants;

typedef enum {
    F1HAlignGeneral = 1,
    F1HAlignLeft = 2,
    F1HAlignCenter = 3,
    F1HAlignRight = 4,
    F1HAlignFill = 5,
    F1HAlignJustify = 6,
    F1HAlignCenterAcrossCells = 7
} F1HAlignConstants;

typedef enum {
    F1VAlignTop = 1,
    F1VAlignCenter = 2,
    F1VAlignBottom = 3
} F1VAlignConstants;

typedef enum {
    F1ClearDlg = 0,
    F1ClearAll = 1,
    F1ClearFormats = 2,
    F1ClearValues = 3
} F1ClearTypeConstants;

typedef enum {
    F1FileFormulaOne = 1,
    F1FileExcel4 = 2,
    F1FileTabbedText = 3,
    F1FileExcel5 = 4,
    F1FileFormulaOne3 = 5,
    F1FileTabbedTextValuesOnly = 6,
    F1FileHTML = 9,
    F1FileHTMLDataOnly = 10
} F1FileTypeConstants;

typedef enum {
    F1ObjLine = 1,
    F1ObjRectangle = 2,
    F1ObjOval = 3,
    F1ObjArc = 4,
    F1ObjChart = 5,
    F1ObjButton = 7,
    F1ObjPolygon = 8,
    F1ObjCheckBox = 9,
    F1ObjDropDown = 10,
    F1ObjPicture = 11
} F1ObjTypeConstants;

typedef enum {
    F1FindMatchCase = 1,
    F1FindEntireCells = 2,
    F1FindMatchBytes = 4,
    F1FindByRows = 0,
    F1FindByColumns = 8,
    F1FindInFormulas = 0,
    F1FindInValues = 16,
    F1FindReplaceAll = 32
} F1FindReplaceConstants;

typedef enum {
    F1ReplaceYes = 0,
    F1ReplaceNo = 1,
    F1ReplaceCancel = 2
} F1BeforeReplaceConstants;

typedef enum {
    F1ErrorNone = 0,
    F1ErrorGeneral = 20001,
    F1ErrorBadArgument = 20002,
    F1ErrorNoMemory = 20003,
    F1ErrorBadFormula = 20004,
    F1ErrorBufTooShort = 20005,
    F1ErrorNotFound = 20006,
    F1ErrorBadRC = 20007,
    F1ErrorBadHSS = 20008,
    F1ErrorTooManyHSS = 20009,
    F1ErrorNoTable = 20010,
    F1ErrorUnableToOpenFile = 20011,
    F1ErrorInvalidFile = 20012,
    F1ErrorInsertShiftOffTable = 20013,
    F1ErrorOnlyOneRange = 20014,
    F1ErrorNothingToPaste = 20015,
    F1ErrorBadNumberFormat = 20016,
    F1ErrorTooManyFonts = 20017,
    F1ErrorTooManySelectedRanges = 20018,
    F1ErrorUnableToWriteFile = 20019,
    F1ErrorNoTransaction = 20020,
    F1ErrorNothingToPrint = 20021,
    F1ErrorPrintMarginsDontFit = 20022,
    F1ErrorCancel = 20023,
    F1ErrorUnableToInitializePrinter = 20024,
    F1ErrorStringTooLong = 20025,
    F1ErrorFormulaTooLong = 20026,
    F1ErrorUnableToOpenClipboard = 20027,
    F1ErrorPasteWouldOverflowSheet = 20028,
    F1ErrorLockedCellsCannotBeModified = 20029,
    F1ErrorLockedDocCannotBeModified = 20030,
    F1ErrorInvalidName = 20031,
    F1ErrorCannotDeleteNameInUse = 20032,
    F1ErrorUnableToFindName = 20033,
    F1ErrorNoWindow = 20034,
    F1ErrorSelection = 20035,
    F1ErrorTooManyObjects = 20036,
    F1ErrorInvalidObjectType = 20037,
    F1ErrorObjectNotFound = 20038,
    F1ErrorInvalidRequest = 20039,
    F1ErrorBadValidationRule = 20040,
    F1ErrorBadInputMask = 20041,
    F1ErrorValidationFailed = 20042,
    F1ErrorNoODBCConnection = 20043,
    F1ErrorUnableToLoadODBC = 20044,
    F1ErrorUnsupportedFeature = 20045,
    F1ErrorBadArray = 20046,
    F1InvalidODBCParameterBinding = 20047,
    F1InvalidStatementHandle = 20048,
    F1BadPrepareStatement = 20049,
    F1NotAvailableInSafeMode = 20050
} F1ErrorConstants;

typedef enum {
    F1ControlNoCell = 0,
    F1ControlCellValue = 1,
    F1ControlCellText = 2
} F1ControlCellConstants;

typedef enum {
    F1HInsideBorder = -2,
    F1VInsideBorder = -1,
    F1TopBorder = 0,
    F1LeftBorder = 1,
    F1BottomBorder = 2,
    F1RightBorder = 3
} F1BorderConstants;

typedef enum {
    F1PasteAll = 0,
    F1PasteFormulas = 1,
    F1PasteValues = 2,
    F1PasteFormats = 3
} F1PasteWhatConstants;

typedef enum {
    F1PasteOpNone = 0
} F1PasteOpConstants;

typedef enum {
    F1BorderNone = 0,
    F1BorderThin = 1,
    F1BorderMedium = 2,
    F1BorderDashed = 3,
    F1BorderDotted = 4,
    F1BorderThick = 5,
    F1BorderDouble = 6,
    F1BorderHair = 7
} F1BorderStyleConstants;

typedef enum {
    F1BookBorderNone = 0,
    F1BookBorderThin = 1
} F1BookBorderConstants;

typedef enum {
    F1AllPages = 2147483647,
    F1AllExcept = 0x80000000,
    F1NumberPage = 1,
    F1AlignmentPage = 2,
    F1FontPage = 4,
    F1BorderPage = 8,
    F1PatternsPage = 16,
    F1ProtectionPage = 32,
    F1ValidationPage = 64,
    F1LineStylePage = 128,
    F1NamePage = 256,
    F1OptionsPage = 512,
    F1AutoFillPage = 1024,
    F1CalculationPage = 2048,
    F1ColorPage = 4096,
    F1EditPage = 8192,
    F1GeneralPage = 16384,
    F1ViewPage = 32768,
    F1SelectionPage = 65536
} F1DialogPageConstants;

typedef enum {
    F1NoType = -1,
    F1GeneralType = 0,
    F1NumberType = 1,
    F1CurrencyType = 2,
    F1DateType = 3,
    F1DateTimeType = 4,
    F1PercentType = 5,
    F1FractionType = 6,
    F1ScientificType = 7,
    F1StringType = 8
} F1FormatTypeConstants;

typedef enum {
    F1AnsiCharSet = 0,
    F1SymbolCharSet = 2,
    F1ShiftJisCharSet = 128,
    F1HangeulCharSet = 129,
    F1GB2312CharSet = 134,
    F1ChineseBig5CharSet = 136,
    F1OemCharSet = 255,
    F1JohabCharSet = 130,
    F1HebrewCharSet = 177,
    F1ArabicCharSet = 178,
    F1GreekCharSet = 161,
    F1TurkishCharSet = 162,
    F1VietnameseCharSet = 163,
    F1ThaiCharSet = 222,
    F1EastEuropeCharSet = 238,
    F1RussianCharSet = 204,
    F1MacCharSet = 77,
    F1BalticCharSet = 186
} F1CharSetConstants;

typedef enum {
    F1CellAlignHorizontal = 8,
    F1CellWordWrap = 9,
    F1CellAlignVertical = 10,
    F1CellFontName = 11,
    F1CellFontCharSet = 12,
    F1CellFontSize = 13,
    F1CellFontBold = 14,
    F1CellFontItalic = 15,
    F1CellFontUnderline = 16,
    F1CellFontStrikeout = 17,
    F1CellFontColor = 18,
    F1CellNumberFormat = 19,
    F1CellPatternStyle = 20,
    F1CellPatternFG = 21,
    F1CellPatternBG = 22,
    F1CellProtectionLocked = 23,
    F1CellProtectionHidden = 24,
    F1CellValidationRule = 25,
    F1CellValidationText = 26
} F1CellAttrConstants;

typedef enum {
    F1Default = 0,
    F1Arrow = 1,
    F1Cross = 2,
    F1IBeam = 3,
    F1Icon = 4,
    F1Size = 5,
    F1SizeNESW = 6,
    F1SizeNS = 7,
    F1SizeNWSE = 8,
    F1SizeWE = 9,
    F1UpArrow = 10,
    F1Hourglass = 11,
    F1NoDrop = 12,
    F1ArrowAndHourglass = 13,
    F1ArrowAndQuestion = 14,
    F1SizeAll = 15,
    F1Custom = 99
} F1MousePointerConstants;


DEFINE_GUID(CLSID_F1FileSpec,0x13e51024,0xa52b,0x11d0,0x86,0xda,0x00,0x60,0x8c,0xb9,0xfb,0xfb);
DEFINE_GUID(CLSID_F1ODBCConnect,0x13e51028,0xa52b,0x11d0,0x86,0xda,0x00,0x60,0x8c,0xb9,0xfb,0xfb);
DEFINE_GUID(CLSID_F1ODBCQuery,0x13e5102a,0xa52b,0x11d0,0x86,0xda,0x00,0x60,0x8c,0xb9,0xfb,0xfb);
DEFINE_GUID(CLSID_F1BookView,0x13e51031,0xa52b,0x11d0,0x86,0xda,0x00,0x60,0x8c,0xb9,0xfb,0xfb);

#endif // __VCF15_H__
