//
//  This program code is a part of The Master Scheduler (TMS)
//  and is Copyright (C) 1991-2004 Schedule Masters, Inc.
//  All rights reserved.
//
#include "TMSHeader.h"

//
//  WriteExcel.c - Developed initially for Grand Rapids to create a bus stops
//                 output file in Excel format.  This routine has been designed
//                 to be added to (functionality-wise) as newer needs demand.
//
//  NB: Most comment text ripped off from "Microsoft Excel File Format",
//      dutifully lifted from http://www.wotsit.org.
//
//  Microsoft Excel is a popular spreadsheet.  It uses a file format called BIFF (Binary
//  File Format).  There are many types of BIFF records.  Each has a 4 byte header.  The
//  first two bytes are an opcode that specifies the record type.  The second two bytes
//  specify record length.  Header values are stored in byte-reversed form (less significant
//  byte first).  The rest of the record is the data itself.
//
//  BIFF record header:
//
//                  |  Record Header    |  Record Body
//  Byte Number     |  0    1    2    3 |  0    1   ...
//                  -----------------------------------
//  Record Contents | XX | XX | XX | XX | XX | XX | ...
//                  -----------------------------------
//                  | opcode  | length  | data
//				
//  Each X represents a hexadecimal digit
//
//  Two X's form a byte.  The least significant (low) byte of the opcode is byte 0 and the
//  most significant (high) byte is byte 1.  Similarly, the low byte of the record length
//  field is byte 2 and the high byte is byte 3.
//
//
//  Relating Spreadsheet Cells to Record Data Bytes
//
//  A spreadsheet appears on a screen or printout as a matrix of rectangular cells.  Each
//  column is identified by a letter at its top, and each row is identified by a number.
//  Thus cell A1 is in the first column and the first row.  Cell C240 is in the third column
//  and the 240th row.  This scheme identifies cells in a way easily understood by people.
//  However, it is not particularly convenient for computers, as they do not handle letters
//  efficiently.  They are best at dealing with binary numbers.  Thus, Excel stores cell
//  identifiers as binary numbers, that people can read as hexadecimal.  The first number in
//  the system is 0 rather than 1.
//
//  The following example, which shows the form of an INTEGER record, illustrates the
//  storage of column and row information.
//
//  INTEGER record.
//
//        |  Record Header    |  Record Body
//  Byte  |  0    1    2    3 |  0    1    2    3    4    5    6    7    8 |
//        ------------------------------------------------------------------
//  Value | 02 | 00 | 09 | 00 | 00 | 00 | 02 | 00 | 00 | 00 | 00 | 39 | 00 |
//        ------------------------------------------------------------------
//        | opcode  | length  |   row   | column  |   rgbAttr    |    w    |
//
//  Opcode 2 indicates an integer record.  The length bytes show that the record body is 9
//  bytes long.  Row 0 in the body corresponds to spreadsheet row 1.  Row 1 corresponds to
//  spreadsheet row 2, and so on.  Column 2 corresponds to spreadsheet column C.  Thus,
//  This example deals with cell C1.  The next three bytes, labelled "rgbAttr," specify cell
//  attributes (see below).  The final pair of bytes, (labelled "w") holds the integer's
//  value.  Here it is 39H or 57 decimal.  Thus the record specifies that cell C1 of the
//  spreadsheet contains an integer with the value 57.
//
//  Standard File Record Order
//
//  Excel worksheet files have each record type in a predetermined position.  A file need
//  not have all types, but the ones that are present are always be in the same order.
//  The file excel.h contains the #defines for the record types for Excel document
//  (spreadsheet) files.
//
//  Several record types in a BIFF file, namely, ROW, BLANK, INTEGER, NUMBER, LABEL,
//  BOOLERR, FORMULA, and COLUMN DEFAULT, describe the contents of a cell.  These records
//  contain a 3 byte attribute field labelled "rgbAttr".  The following table describes how
//  the  bits in the field correspond to cell attributes.
//
//  Cell Attributes (rgbAttr)
//  
//  Byte Offset     Bit   Description                     Contents
//       0          7     Cell is not hidden              0b
//                        Cell is hidden                  1b
//                  6     Cell is not locked              0b
//                        Cell is locked                  1b
//                  5-0   Reserved, must be 0             000000b
//                  7-6   Font number (4 possible)
//                  5-0   Cell format code
//       2          7     Cell is not shaded              0b
//                        Cell is shaded                  1b
//                  6     Cell has no bottom border       0b
//                        Cell has a bottom border        1b
//                  5     Cell has no top border          0b
//                        Cell has a top border           1b
//                  4     Cell has no right border        0b
//                        Cell has a right border         1b
//                  3     Cell has no left border         0b
//                        Cell has a left border          1b
//                  2-0   Cell alignment code 
//                             general                    000b
//                             left                       001b
//                             center                     010b
//                             right                      011b
//                             fill                       100b
//                             Multiplan default align.   111b
//
//  The font number field is a zero-based index into the document's table of fonts.  the
//  cell format code is a zero-based index into the document's table of picture formats.
//  There are 21 different standard formats.  Additional custom formats may be defined by
//  the user.  See the FONT and FORMAT record descriptions form additional details.
//

void WriteExcel(short int recordType, HANDLE hOutputFile, BYTE *recordBody, int recordBodyLength)
{
  DWORD dwBytesWritten;
  BYTE  recordHeader[4];

  switch(recordType)
  {
//
//  Record Type: EXCEL_DIMENSIONS
//
//  Description: Entire dimensions or range of a spreadsheet
//
//  Record Body Length: 8 bytes
//
//  Record Body Byte Structure:
//      Byte Number     Byte Description               Contents (hex)
//           0-1        First row
//           2-3        Last row plus 1
//           4-5        First column
//           6-7        Last column plus 1
//
//  Note: The last row and column in the record are both one greater
//        than the highest numbered occupied ones.
//
    case EXCEL_DIMENSIONS:
      break;
//
//  Record Type: EXCEL_BLANK
//
//  Description: Cell with no formula or value
//
//  Record Body Length: 7 bytes
//
//  Record Body Byte Structure:
//      Byte Number     Byte Description               Contents (hex)
//           0-1        Row
//           2-3        Column
//           4-6        Cell attributes (rgbAttr) (See above)
//
    case EXCEL_BLANK:
      break;
//
//  Record Type: EXCEL_INTEGER
//
//  Description: Constant unsigned integer
//
//  Record Body Length: 9 bytes
//
//  Record Body Byte Structure:
//      Byte Number     Byte Description               Contents (hex)
//           0-1        Row
//           2-3        Column
//           4-6        Cell attributes (rgbAttr) (See above)
//           7-8        Unsigned integer value (w)
//
    case EXCEL_INTEGER:
      recordHeader[0] = EXCEL_INTEGER;
      recordHeader[1] = 0x00;
      recordHeader[2] = recordBodyLength;
      recordHeader[3] = 0x00;
      WriteFile(hOutputFile, (LPCVOID *)recordHeader, 4, &dwBytesWritten, NULL);
      break;
//
//  Record Type: EXCEL_NUMBER
//
//  Description: Constant floating point number
//
//  Record Body Length: 15 bytes
//
//  Record Body Byte Structure:
//      Byte Number     Byte Description               Contents (hex)
//           0-1        Row
//           2-3        Column
//           4-6        Cell attributes (rgbAttr) (See above)
//           7-14       Floating point number value (IEEE format)
//
    case EXCEL_NUMBER:
      recordHeader[0] = EXCEL_NUMBER;
      recordHeader[1] = 0x00;
      recordHeader[2] = recordBodyLength;
      recordHeader[3] = 0x00;
      WriteFile(hOutputFile, (LPCVOID *)recordHeader, 4, &dwBytesWritten, NULL);
      break;
//
//  Record Type: EXCEL_LABEL
//
//  Description: Constant string
//
//  Record Body Length: 8 to 263 bytes
//
//  Record Body Byte Structure:
//      Byte Number     Byte Description               Contents (hex)
//           0-1        Row
//           2-3        Column
//           4-6        Cell attributes (rgbAttr) (See above)
//           7          Length of string
//           8-263      ASCII string, 0 to 255 bytes long
//
    case EXCEL_LABEL:
      recordHeader[0] = EXCEL_LABEL;
      recordHeader[1] = 0x00;
      recordHeader[2] = recordBodyLength;
      recordHeader[3] = 0x00;
      WriteFile(hOutputFile, (LPCVOID *)recordHeader, 4, &dwBytesWritten, NULL);
      break;
//
//  Record Type: EXCEL_BOOLERR
//
//  Description: Boolean constant or error value
//
//  Record Body Length: 9 bytes
//
//  Record Body Byte Structure:
//      Byte Number     Byte Description               Contents (hex)
//           0-1        Row
//           2-3        Column
//           4-6        Cell attributes (rgbAttr) (See above)
//           7          Boolean or error value
//                                Boolean
//                                      true                 1
//                                      false                0
//                                Error
//                                      #NULL!               0
//                                      #DIV/0!              7
//                                      #VALUE!              0Fh
//                                      #REF!                17h
//                                      #NAME?               1Dh
//                                      #NUM!                24h
//                                      #N/A                 2Ah
//           8          Specifies Boolean or error
//                                Boolean                    0
//                                Error                      1
//
    case EXCEL_BOOLERR:
      break;
//
//  Record Type: EXCEL_FORMULA
//
//  Description: Name, size, and contents of a formula cell
//
//  Record Body Length: 17-272 bytes
//
//  Record Body Byte Structure:
//      Byte Number     Byte Description               Contents (hex)
//           0-1        Row
//           2-3        Column
//           4-6        Cell attributes (rgbAttr) (See above)
//           7          Current value of formula (IEEE format)
//           15         Recalc flag
//           16         Length of parsed expression
//           17         Parsed expression
//
//  If a formula must be recalculated whenever it is loaded, the recalc flag (byte 15) must
//  be set.  Any nonzero value is a set recalc flag.  However, a flag value of 3 indicates
//  that the cell is a part of a matrix, and the entire matrix must be recalculated. Bytes 7
//  through 14 may contain a number, a Boolean value, an error code, or a string.  The
//  following tables apply.
//
//  Case 1: Bytes 7 - 14 contain a Boolean value.
//
//     Byte Number     Byte Description          Contents (hex)
//           7         otBool                          1
//           8         Reserved                        0
//           9         Boolean value
//           10-12     Reserved                        0
//           13-14     fExprO                          FFFFh
//
//  Case 2: Bytes 7 - 14 contain an error code.
//
//     Byte Number     Byte Description          Contents (hex)
//           7         otErr                           2
//           8         Reserved                        0
//           9         error code
//           10-12     Reserved                        0
//           13-14     fExprO                          FFFFh
//
//  Case 3: Bytes 7 - 14 contain a string.
//
//     Byte Number     Byte Description          Contents (hex)
//           7         otString                        0
//           8-12      Reserved                        0
//           13-14     fExprO                          FFFFh
//
//  The string value itself is not stored in the field, but rather in a separate record of
//  the STRING type.
//
    case EXCEL_FORMULA:
      break;
//
//  Record Type: EXCEL_STRING
//
//  Description: Value of a string in a formula
//
//  Record Body Length: variable
//
//  Record Body Byte Structure:
//      Byte Number     Byte Description           Contents (hex)
//           0          Length of the string
//           1-256 (max) The string itself
//
//  The STRING record appears immediately after the FORMULA record that evaluates to the
//  string, unless the formula is in an array.  In that case, the string record immediately
//  follows the ARRAY record.
//
    case EXCEL_STRING:
      break;
//
//  Record Type: EXCEL_ROW
//
//  Description: Specifies a spreadsheet row 
//
//  Record Body Length: 16 bytes
//
//  Record Body Byte Structure:
//      Byte Number     Byte Description               Contents (hex)
//           0-1        Row number
//           2-3        First defined column in the row
//           4-5        Last defined column in the row plus 1
//           6-7        Row height
//           8-9        RESERVED                                0
//           10         Default cell attributes byte
//                      Default attributes                      1
//                      Not default attributes                  0
//           11-12      Offset to cell records for this row
//           13-15      Cell attributes (rgbAttr) (See above)
//
    case EXCEL_ROW:
      break;
//
//  Record Type: EXCEL_BOF
//
//  Description: Beginning of file
//
//  Record Body Length: 4 bytes
//
//  Record Body Byte Structure:
//      Byte Number     Byte Description               Contents (hex)
//           0-1        Version number
//                           Excel                           2
//                           Multiplan                       3
//           2-3        Document type
//                           worksheet                       10h
//                           chart                           20h
//                           macro sheet                     40h
//
//  If bit 8 of the version number byte pair is high (mask with 0100h to find out), the BIFF
//  file is a Multiplan document.
//
    case EXCEL_BOF:
      recordHeader[0] = EXCEL_BOF;
      recordHeader[1] = 0x00;
      recordHeader[2] = 0x04;
      recordHeader[3] = 0x00;
      WriteFile(hOutputFile, (LPCVOID *)recordHeader, 4, &dwBytesWritten, NULL);
      break;
//
//  Record Type: EXCEL_EOF
//
//  Description: End of file
//
//  Record Body Length: 0 bytes
//
//  The EOF record is the last one in a BIFF file.  It always takes the form 0A000000h.
//
    case EXCEL_EOF:
      recordHeader[0] = EXCEL_EOF;
      recordHeader[1] = 0x00;
      recordHeader[2] = 0x00;
      recordHeader[3] = 0x00;
      WriteFile(hOutputFile, (LPCVOID *)recordHeader, 4, &dwBytesWritten, NULL);
      break;
//
//  Record Type: EXCEL_INDEX
//
//  Description: Contains pointers to other records in the BIFF file, and defines the range
//  of rows used by the document.  It is used to simplify searching a file for a particular
//  cell or name.
//
//  Record Body Length: variable
//
//  Record Body Byte Structure:
//      Byte Number     Byte Description               Contents (hex)
//           0-3        Absolute file position of first NAME record
//           4-5        First row that exists
//           6-7        Last row that exists plus 1
//           8-on       Array of absolute file positions of the 
//                      blocks of ROW records.
//
//  The INDEX record is optional.  If present, it must immediately follow the FILEPASS
//  record.  IF there is no FILEPASS record, it must follow the BOF record.
//
    case EXCEL_INDEX:
      break;
//
//  Record Type: EXCEL_CALCCOUNT
//
//  Description: Specifies the iteration count
//
//  Record Body Length: 2
//
//  Record Body Byte Structure:
//      Byte Number     Byte Description               Contents (hex)
//           0-1        Iteration Count
//
    case EXCEL_CALCCOUNT:
      break;
//
//  Record Type: EXCEL_CALCMODE
//
//  Description: Specifies the calculation mode
//
//  Record Body Length: 2
//
//  Record Body Byte Structure:
//      Byte Number     Byte Description               Contents (hex)
//           0-1        Calculation mode
//                        Manual                           0
//                        Automatic                        1
//                        Automatic, no tables            -1
//
    case EXCEL_CALCMODE:
      break;
//
//  Record Type: EXCEL_PRECISION
//
//  Description: Specifies precision of calculations for document
//
//  Record Body Length: 2
//
//  Record Body Byte Structure:
//      Byte Number     Byte Description               Contents (hex)
//           0-1        Document precision
//                        precision as displayed           0
//                        full precision                   1
//
    case EXCEL_PRECISION:
      break;
//
//  Record Type: EXCEL_REFMODE
//
//  Description: Specifies location reference mode
//
//  Record Body Length: 2
//
//  Record Body Byte Structure:
//      Byte Number     Byte Description            Contents (hex)
//          0-1         Reference mode
//                        R1C1 mode                      0
//                        A1 mode                        1
//
    case EXCEL_REFMODE:
      break;
//
//  Record Type: EXCEL_DELTA
//
//  Description: Maximum change for an iterative model
//
//  Record Body Length: 8
//
//  Record Body Byte Structure:
//      Byte Number           Byte Description      Contents (hex)
//          0-7               Maximum change (IEEE format)
//
    case EXCEL_DELTA:
      break;
//
//  Record Type: EXCEL_ITERATION
//
//  Description: Specifies whether iteration is on
//
//  Record Body Length: 2
//
//  Record Body Byte Structure:
//      Byte Number           Byte Description         Contents (hex)
//          0-1               Iteration flag
//                              Iteration off               0
//                              Iteration on                1
//
    case EXCEL_ITERATION:
      break;
//
//  Record Type: EXCEL_PROTECT
//
//  Description: Specifies whether the document is protected with a document password
//
//  Record Body Length: 2
//
//  Record Body Byte Structure:
//      Byte Number           Byte Description         Contents (hex)
//          0-1               Document protection
//                              Not protected               0
//                              Protected                   1
//
    case EXCEL_PROTECT:
      break;
//
//  Record Type: EXCEL_PASSWORD
//
//  Description: Contains encrypted document password
//
//  Record Body Length: 2
//
//  Record Body Byte Structure:
//      Byte Number           Byte Description         Contents (hex)
//          0-1               Encrypted password
//
    case EXCEL_PASSWORD:
      break;
//
//  Record Type: EXCEL_HEADER
//
//  Description: Specifies header string that appears at the top of every page when the
//  document is printed
//
//  Record Body Length: variable
//
//  Record Body Byte Structure:
//      Byte Number           Byte Description         Contents (hex)
//          0                 Length of string (bytes)
//         1-on               Header string (ASCII)
//
    case EXCEL_HEADER:
      break;
//
//  Record Type: EXCEL_FOOTER
//
//  Description: Specifies footer string that appears at the bottom of every page when the
//  document is printed
//
//  Record Body Length: variable
//
//  Record Body Byte Structure:
//      Byte Number           Byte Description         Contents (hex)
//          0                 Length of string (bytes)
//        1-on                Footer string (ASCII)
//
    case EXCEL_FOOTER:
      break;
//
//  Record Type: EXCEL_EXTERNCOUNT
//
//  Description: Specifies the number of documents referenced externally by an Excel
//  document
//
//  Record Body Length: 2
//
//  Record Body Byte Structure:
//      Byte Number           Byte Description         Contents (hex)
//          0-1               Number of externally referenced documents
//
    case EXCEL_EXTERNCOUNT:
      break;
//
//  Record Type: EXCEL_EXTERNSHEET
//
//  Description: Specifies a document that is referenced externally by the Excel file.
//  There must be an EXTERNSHEET record for every external file counted by the EXTERNCOUNT
//  record.
//
//  Record Body Length: variable
//
//  Record Body Byte Structure:
//      Byte Number           Byte Description         Contents (hex)
//           0                Length of document name
//          1-on              Document name
//
//  The document name may be encoded.  If so, its first character will be 0, 1 or 2.
//
//  0 indicates the document name is an external reference to an empty sheet.
//
//  1 indicates the document name has been translated to a less system-dependent name.
//  This feature is valuable for documents intended for a non-DOS environment.
//
//  2 indicates that the externally referenced document is, in fact, the current document.
//
    case EXCEL_EXTERNSHEET:
      break;
//
//  Record Type: EXCEL_NAME
//
//  Description: User-defined name on the document
//
//  Record Body Length: variable
//
//  Record Body Byte Structure:
//      Byte Number           Byte Description              Contents (hex)
//           0                Name attribute
//                              Only bits 1 and 2 are ever nonzero.
//
//                              Bit 1 is 1 if the name is a function or 
//                              command name on a macro sheet.
//
//                              Bit 2 is 1 if the name definition 
//                              includes:
//                              * A function that returns an array, such 
//                                 as TREND or MINVERSE
//                              * A ROW or COLUMN function
//                              * A user-defined function
//
//                            Name attribute
//                              Meaningful only if bit 1 of 
//                              byte 0 is 1 (the name is a function or 
//                              command name).  Only bits 0 and 1 are  
//                              ever nonzero.
//
//                              Bit 0 is 1 if the name is a function.
//
//                              Bit 1 is 1 if the name is a command.
//
//           2                Keyboard shortcut.  Meaningful only if the  
//                            name is a command.
//                              If no keyboard shortcut     0
//                              If shortcut exists          ASCII value
//
//           3                Length of the name text
//           4                Length of the name's definition
//           5-?              Text of the name
//           ?-?              Name's definition (parsed) in internal
//                            compressed format
//           ?                Length of the name's definition (duplicate)
//
//  All NAME records should appear together in a BIFF file.
//
    case EXCEL_NAME:
      break;
//
//  Record Type: EXCEL_WINDOW_PROTECT
//
//  Description: Specifies whether a document's windows are protected
//
//  Record Body Length: 2 bytes
//
//  Record Body Byte Structure:
//      Byte Number           Byte Description         Contents (hex)
//          0-1               Window protect flag
//                              Not protected               0
//                              Protected                   1
//
    case EXCEL_WINDOW_PROTECT:
      break;
//
//  Record Type: EXCEL_VERTICAL_PAGE_BREAKS
//
//  Description: Lists all column page breaks
//
//  Record Body Length: variable
//
//  Record Body Byte Structure:
//      Byte Number           Byte Description         Contents (hex)
//          0-1               Number of page breaks
//          2-on              Array containing a 2-byte integer for each
//                            column that immediately follows a column page
//                            break.  Columns must be sorted in ascending
//                            order.
//
    case EXCEL_VERTICAL_PAGE_BREAKS:
      break;
//
//  Record Type: EXCEL_HORIZONTAL_PAGE_BREAKS
//
//  Description: Lists all row page breaks
//
//  Record Body Length: variable
//
//  Record Body Byte Structure:
//      Byte Number           Byte Description         Contents (hex)
//          0-1               Number of page breaks
//          2-on              Array containing a 2-byte integer for each
//                            row that immediately follows a row page
//                            break.  Rows must be sorted in ascending
//                            order.
//
    case EXCEL_HORIZONTAL_PAGE_BREAKS:
      break;
//
//  Record Type: EXCEL_NOTE
//
//  Description: Note associated with a cell
//
//  Record Body Length: Variable, maximum of 254
//
//  Record Body Byte Structure:
//      Byte Number           Byte Description         Contents (hex)
//          0-1               Row of the note
//          2-3               Column of the note
//          4-5               Length of the note part of the record
//          6-on               Text of the note
//
//  Notes longer than 2048 characters must be split among multiple records.  All except the
//  last one will contain 2048 text characters.  The last one will contain the overflow.
//
    case EXCEL_NOTE:
      break;
//
//  Record Type: EXCEL_SELECTION
//
//  Description: Specifies which cells are selected in a pane of a split window.  It can
//  also specify selected cells in a window that is not split.
//
//  Record Body Length: Variable
//
//  Record Body Byte Structure:
//      Byte Number           Byte Description         Contents (hex)
//           0                Number of pane
//                              bottom right                0
//                              top right                   1
//                              bottom left                 2
//                              top left                    3
//                              no splits                   3
//          1-2               Row number of the active cell
//          3-4               Column number of the active cell
//          5-6               Reference number of the active cell
//          7-8               Number of references in the selection
//          9-on              Array of references
//
//  Each reference in the array consists of 6 bytes arranged as follows:
//      Byte Number           Byte Description
//          0-1               First row in the reference
//          2-3               Last row in the reference
//           4                First column in the reference
//           5                Last column in the reference 
//
    case EXCEL_SELECTION:
      break;
//
//  Record Type: EXCEL_FORMAT
//
//  Description: Describes a picture format in a document.  All FORMAT records must appear
//  together in a BIFF file.
//
//  Record Body Length: Variable
//
//  Record Body Byte Structure:
//      Byte Number           Byte Description         Contents (hex)
//           0                Length of format string
//          1-on              Picture format string
//
    case EXCEL_FORMAT:
      break;
//
//  Record Type: EXCEL_FORMATCOUNT
//
//  Description: The number of standard FORMAT records in the file.  There are 21 different
//  format records.
//
//  Record Body Length: 2 bytes
//
//  Record Body Byte Structure:
//      Byte Number           Byte Description         Contents (hex)
//          0-1               Number of built in format records.
//
    case EXCEL_FORMATCOUNT:
      break;
//
//  Record Type: EXCEL_COLUMN_DEFAULT
//
//  Description: Specifies default cell attributes for cells in a particular column.  The
//  default value is overridden for individual cells by a subsequent explicit definition.
//
//  Record Body Length: Variable
//
//  Record Body Byte Structure:
//      Byte Number           Byte Description         Contents (hex)
//          0-1               Column number of first column for which a 
//                              default cell is being defined
//          2-3               Column number of last column for which a 
//                              default cell is being defined, plus 1.
//          4-on              Array of cell attributes
//
    case EXCEL_COLUMN_DEFAULT:
      break;
//
//  Record Type: EXCEL_ARRAY
//
//  Description: Describes a formula entered into a range of cells as an array.  Occurs
//  immediately after the FORMULA record for the upper left corner of the array.
//
//  Record Body Length: variable
//
//  Record Body Byte Structure:
//      Byte Number           Byte Description              Contents (hex)
//          0-1               First row of the array
//          2-3               Last row of the array
//           4                First column of the array
//           5                Last column of the array
//           6                Recalculation flag
//                              Array is calculated             0
//                              Needs to be calculated         nonzero
//           7                Length of parsed expression
//          8-on              Parsed expression (array formula)
//
    case EXCEL_ARRAY:
      break;
//
//  Record Type: EXCEL_1904
//
//  Description: Specifies date system used on this spreadsheet
//
//  Record Body Length: 2 bytes
//
//  Record Body Byte Structure:
//      Byte Number           Byte Description              Contents (hex)
//          0-1               Specifies date system used
//                              1904 date system                 1
//                              anything else                    0
//
    case EXCEL_1904:
      break;
//
//  Record Type: EXCEL_EXTERNNAME
//
//  Description: An externally referenced name, referring to a work-sheet or macro sheet or
//  to a DDE topic.  All EXTERNNAME records associated with a supporting document must
//  directly follow its EXTERNSHEET record.
//
//  Record Body Length: Variable
//
//  Record Body Byte Structure:
//      Byte Number           Byte Description         Contents (hex)
//           0                Length of the name
//          1-on              The name
//
//  When EXTERNNAME references a DDE topic, Excel may append its most recent values to the
//  EXTERNNAME record.  If the record becomes too long to be contained in a single record,
//  it is split into multiple records, with CONTINUE records holding the excess.
//
    case EXCEL_EXTERNNAME:
      break;
//
//  Record Type: EXCEL_COLWIDTH
//
//  Description: Sets column width for a range of columns
//
//  Record Body Length: 3 bytes
//
//  Record Body Byte Structure:
//      Byte Number           Byte Description         Contents (hex)
//           0                First column in the range
//           1                Last column in the range
//          2-3               Column width in units of 1/256th of a 
//                            character
//
    case EXCEL_COLWIDTH:
      break;
//
//  Record Type: EXCEL_DEFAULT_ROW_HEIGHT
//
//  Description: Specifies the height of all rows that are not defined explicitly
//
//  Record Body Length: 2 bytes
//
//  Record Body Byte Structure:
//      Byte Number           Byte Description         Contents  (hex)
//          0-1               Default row height in units of 1/20th of a 
//                            point
//
    case EXCEL_DEFAULT_ROW_HEIGHT:
      break;
//
//  Record Type: EXCEL_LEFT_MARGIN
//
//  Description: Specifies the left margin in inches when the document is printed
//
//  Record Body Length: 8 bytes
//
//  Record Body Byte Structure:
//      Byte Number           Byte Description         Contents (hex)
//          0-7               Left margin (IEEE format)
//
    case EXCEL_LEFT_MARGIN:
      break;
//
//  Record Type: EXCEL_RIGHT_MARGIN
//
//  Description: Specifies the right margin in inches when the document is printed
//
//  Record Body Length: 8 bytes
//
//  Record Body Byte Structure:
//      Byte Number           Byte Description         Contents (hex)
//          0-7               Right margin (IEEE format)
//
    case EXCEL_RIGHT_MARGIN:
      break;
//
//  Record Type: EXCEL_TOP_MARGIN
//
//  Description: Specifies the top margin in inches when the document is printed
//
//  Record Body Length: 8 bytes
//
//  Record Body Byte Structure:
//      Byte Number           Byte Description         Contents (hex)
//          0-7               Top margin (IEEE format)
//
    case EXCEL_TOP_MARGIN:
      break;
//
//  Record Type: EXCEL_BOTTOM_MARGIN
//
//  Description: Specifies the bottom margin in inches when the document is printed
//
//  Record Body Length: 8 bytes
//
//  Record Body Byte Structure:
//      Byte Number           Byte Description         Contents (hex)
//          0-7               Bottom margin (IEEE format)
//
    case EXCEL_BOTTOM_MARGIN:
      break;
//
//  Record Type: EXCEL_PRINT_ROW_HEADERS
//
//  Description: Flag determines whether to include row and column headers on printout of
//  document
//
//  Record Body Length: 2 bytes
//
//  Record Body Byte Structure:
//      Byte Number           Byte Description              Contents (hex)
//          0-1               Row and Column Header Print Flag
//                              Do not print headers             0
//                              Print headers                    1
//
    case EXCEL_PRINT_ROW_HEADERS:
      break;
//
//  Record Type: EXCEL_PRINT_GRIDLINES
//
//  Description: Flag determines whether to print gridlines on print-out of document
//
//  Record Body Length: 2
//
//  Record Body Byte Structure:
//      Byte Number           Byte Description              Contents (hex)
//          0-1               Gridline Print Flag
//                              Do not print gridlines           0
//                              Print gridlines                  1
//
    case EXCEL_PRINT_GRIDLINES:
      break;
//
//  Record Type: EXCEL_FILEPASS
//
//  Description: Specifies a file password.  If this record is present, the rest of the file
//  is encrypted.  The file password specified here is distinct from the document password
//  specified by the PASSWORD record.  If present, the FILEPASS record must immediately
//  follow the BOF record.
//
//  Record Body Length: ?
//
//  Record Body Byte Structure:
//      Byte Number           Byte Description         Contents (hex)
//          0-on              ?
//
    case EXCEL_FILEPASS:
      break;
//
//  Record Type: EXCEL_FONT
//
//  Description: Describes an entry in the document's font table.  A document may have up to
//  4 different fonts, numbered 0 to 3.  Font records are written in the font table in the
//  order in which they are encountered in the file.
//
//  Record Body Length: variable
//
//  Record Body Byte Structure:
//      Byte Number           Byte Description              Contents (binary)
//          0-1               Height of the font (in 1/20ths of a point)
//          2-3               Font Attributes
//                              First byte (reserved)             00000000b
//                              Second byte
//                                Bit 0 - bold                        1b
//                                Bit 1 - italic                      1b
//                                Bit 2 - underline                   1b
//                                Bit 2 - strikeout                   1b
//                                Bits 4-7 (reserved)               0000b
//           4                Length of font name
//          5-?               Font name
//
    case EXCEL_FONT:
      break;
//
//  Record Type: EXCEL_FONT2
//
//  Description:  System specific information about the font defined in the previous FONT
//  record.  The FONT2 record is optional.
//
//  Record Body Length: Variable
//
//  Record Body Byte Structure:
//      Byte Number           Byte Description         Contents (hex)
//          0-on              ?
//
    case EXCEL_FONT2:
      break;
//
//  Record Type: EXCEL_TABLE
//
//  Description: Describes a one-input row or column table created through the Data Table
//  command
//
//  Record Body Length: 12 bytes
//
//  Record Body Byte Structure:
//      Byte Number           Byte Description              Contents (hex)
//          0-1               First row of the table
//          2-3               Last row of the table
//           4                First column of the table
//           5                Last column of the table
//           6                Recalculation flag
//                              Table is recalculated           0
//                              Not recalculated              nonzero
//           7                Row or column input table flag
//                              Column input table              0
//                              Row input table                 1
//          8-9               Row of the input cell
//          10-11             Column of the input cell
//
//  The area given by the first and last rows and columns does not include the outer row or
//  column, which contains table formulas or input values.  If the input cell is a deleted
//  reference, the row of the input cell, given by the bytes at offset 8 and 9, is -1.
//
    case EXCEL_TABLE:
      break;
//
//  Record Type: EXCEL_TABLE2
//
//  Description: Describes a two-input table created by the Data Table command.  It is the
//  same as the TABLE record, except there is no distinction between a row input table and a
//  column input table, there are two input cells rather than one, and either may have a
//  value of -1, indicating a deleted reference.
//
//  Record Body Length: 16 bytes
//
//  Record Body Byte Structure:
//      Byte Number           Byte Description              Contents (hex)
//          0-1               First row of the table
//          2-3               Last row of the table
//           4                First column of the table
//           5                Last column of the table
//           6                Recalculation flag
//                              Table is calculated              0
//                              Needs recalculation           nonzero
//           7                RESERVED - must be zero            0
//          8-9               Row of the row input cell
//          10-11             Column of the row input cell
//          12-13             Row of the column input cell
//          14-15             Column of the column input cell
//
    case EXCEL_TABLE2:
       break;
//
//  Record Type: EXCEL_CONTINUE
//
//  Description: Continuation of FORMULA, ARRAY, or EXTERNNAME records that are too long to
//  fit in a single record.
//
//  Record Body Length: variable
//
//  Record Body Byte Structure:
//      Byte Number           Byte Description         Contents (hex)
//          0-on              Parsed expression
//
    case EXCEL_CONTINUE:
      break;
//
//  Record Type: EXCEL_WINDOW1
//
//  Description: Basic window information.  Locations are relative to the upper left corner
//  of the Microsoft Windows desktop, and are measured in units of 1/20th of a point.
//
//  Record Body Length: 9 bytes
//
//  Record Body Byte Structure:
//      Byte Number           Byte Description              Contents (hex)
//          0-1               Horizontal position of the window
//          2-3               Vertical position of the window
//          4-5               Width of the window
//          6-7               Height of the window
//           8                Hidden attribute
//                              Window is not hidden             0
//                              Window is hidden                 1
//
//  If you do not include a WINDOW1 record in your BIFF file, Excel will create a default
//  window in your document.
//
    case EXCEL_WINDOW1:
      break;
//
//  Record Type: EXCEL_WINDOW2
//
//  Description: Advanced window information.  The WINDOW2 record is optional.  If present,
//  it must immediately follow the WINDOW1 record.
//
//  Record Body Length: 14 bytes
//
//  Record Body Byte Structure:
//      Byte Number           Byte Description              Contents (hex)
//           0                Display Formulas
//                              Display values                   0
//                              Display formulas                 1
//           1                Display Grid
//                              Do not display gridlines         0
//                              Display gridlines                1
//           2                Display Row and Column Headers
//                              Do not display headers           0
//                              Display headers
//           3                Freeze window panes
//                              Do not freeze panes              0
//                              Freeze panes                     1
//           4                Display zero values
//                              Suppress display                 0
//                              Display zero values              1
//          5-6               Top row visible in the window
//          7-8               Leftmost column visible in the window
//           9                Row/column header and gridline color
//                              Specified in next four bytes 0 
//                              Use window's default             1
//                              foreground color.
//          10-13             Row/column headers and gridline color (RGB)
//
    case EXCEL_WINDOW2:
      break;
//
//  Record Type: EXCEL_BACKUP
//
//  Description: Specifies whether a BIFF file should be backed up
//
//  Record Body Length: 2 bytes
//
//  Record Body Byte Structure:
//      Byte Number           Byte Description         Contents (hex)
//          0-1               Backup flag
//                              Do not back up              0
//                              Back up                     1
//
    case EXCEL_BACKUP:
      break;
//
//  Record Type: EXCEL_PANE
//
//  Description:  Describes the number and position of unfrozen panes in a window. Panes are
//  created by horizontal and vertical splits, which are measured in units of 1/20th of a
//  point.
//
//  Record Body Length: 9 bytes
//
//  Record Body Byte Structure:
//      Byte Number           Byte Description         Contents (hex)
//          0-1               Horizontal position of the split, zero if none
//          2-3               Vertical position of the split, zero if none
//          4-5               Top row visible in the bottom pane
//          6-7               Leftmost column visible in the right pane
//           8                Pane number of the active pane
//
    case EXCEL_PANE:
      break;
  }
//
//  Now, write out the record body
//
  if(recordBodyLength != NO_RECORD)
  {
    WriteFile(hOutputFile, (LPCVOID *)recordBody, recordBodyLength, &dwBytesWritten, NULL);
  }
//
//  All done
//
  return;
}
