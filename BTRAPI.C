/*************************************************************************
**
**  Copyright 1982-1997 Pervasive Software Inc. All Rights Reserved
**
*************************************************************************/
/*************************************************************************
   BTRAPI.C
     This module implements the Btrieve Interface for C/C++ applications
     using MS Windows, Windows 95, Windows NT, OS2, DOS, Extended DOS.  An
     NLM application does not need to compile and link this module.

     You must select a target platform switch.  See the prologue inside
     'btrapi.h' for a list of platform switches.

     IMPORTANT
     ---------
     Pervasive Software Inc., invites you to modify this file
     if you find it necessary for your particular situation.  However,
     we cannot provide technical support for this module if you
     do modify it.

*************************************************************************/
#if !defined(BTI_WIN) && !defined(BTI_OS2) && !defined(BTI_DOS) \
&& !defined(BTI_NLM) && !defined(BTI_DOS_32R) && !defined(BTI_DOS_32P) \
&& !defined(BTI_DOS_32B) && !defined(BTI_WIN_32) && !defined(BTI_OS2_32)
#error You must define one of the following: BTI_WIN, BTI_OS2, BTI_DOS, BTI_NLM, BTI_DOS_32R, BTI_DOS_32P, BTI_DOS_32B, BTI_WIN_32, BTI_OS2_32
#endif

#include <btrconst.h>
#include <btrapi.h>
#include <string.h>

#if defined( BTI_DOS )     || defined( BTI_DOS_32R ) || \
    defined( BTI_DOS_32P ) || defined( BTI_DOS_32B )
#include <dos.h>

#define BTR_INTRPT       0x7B                   /* Btrieve interrupt vector */
#define BTR_OFFSET       0x33              /* Btrieve offset within segment */
#define VARIABLE_ID      0x6176    /* id for variable length records - 'va' */
#define PROTECTED_ID     0x6370             /* id for protected call - 'pc' */
#define VERSION_OFFSET   0
#define REVISION_OFFSET  2
#define TYPE_OFFSET      4
#define VERSION_BUF_SIZE 5
#define BTRV_CODE        7
#define TRUE             1
#define FALSE            0

typedef unsigned char    BOOLEAN;

/* Protected Mode switch parameter block */
typedef struct
{
   BTI_CHAR      sign[ 4 ];
   BTI_ULONG     flags;
   BTI_ULONG     functionCode;
   BTI_LONG      pmSwitchStatus;
   BTI_LONG      dataLength;
   BTI_VOID_PTR  dataPtr;
} pmswParmBlock;
#endif

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
   PROCESS7B IS IN PMSWITCH, AND IS RESOLVED AT RUN-TIME
***************************************************************************/
#if defined( BTI_DOS_32R )
typedef struct
{
   char  *Name;
   void  cdecl (*functionPtr) ( pmswParmBlock *bP );
} IMPORTS[];
#define PROCESS7B 75
IMPORTS * _ImportedFunctions_ = 0;
#endif

/****************************************************************************
   RQSHELLINIT() Function for MS Windows
***************************************************************************/
#if defined( BTI_WIN ) || defined( BTI_WIN_32 )
BTI_API RQSHELLINIT( BTI_CHAR_PTR option )
{
   return WBRQSHELLINIT( option );
}
#endif

/****************************************************************************
   BTRV() Function for 16-Bit MS Windows or OS2
***************************************************************************/
#if defined( BTI_WIN ) || defined( BTI_OS2 )
BTI_API BTRV(
           BTI_WORD     operation,
           BTI_VOID_PTR posBlock,
           BTI_VOID_PTR dataBuffer,
           BTI_WORD_PTR dataLength,
           BTI_VOID_PTR keyBuffer,
           BTI_SINT     keyNumber )
{
   BTI_BYTE keyLength = MAX_KEY_SIZE;
   BTI_CHAR ckeynum = (BTI_CHAR)keyNumber;
   return BTRCALL (
             operation,
             posBlock,
             dataBuffer,
             dataLength,
             keyBuffer,
             keyLength,
             ckeynum );
}
#endif

/****************************************************************************
   BTRVID() Function for 16-Bit MS Windows or OS2
***************************************************************************/
#if defined( BTI_WIN ) || defined( BTI_OS2 )
BTI_API BTRVID(
           BTI_WORD       operation,
           BTI_VOID_PTR   posBlock,
           BTI_VOID_PTR   dataBuffer,
           BTI_WORD_PTR   dataLength,
           BTI_VOID_PTR   keyBuffer,
           BTI_SINT       keyNumber,
           BTI_BUFFER_PTR clientID )
{
   BTI_BYTE keyLength = MAX_KEY_SIZE;
   BTI_CHAR ckeynum = (BTI_CHAR)keyNumber;
   return BTRCALLID (
             operation,
             posBlock,
             dataBuffer,
             dataLength,
             keyBuffer,
             keyLength,
             ckeynum,
             clientID );
}
#endif

/****************************************************************************
   BTRV() Function for NT, Win32s, or 32-bit OS2
***************************************************************************/
#if defined( BTI_WIN_32 ) || defined( BTI_OS2_32 )
#if defined( BTI_OS2_32 )
#define BTRCALL BTRCALL32
#endif
BTI_API BTRV(
           BTI_WORD     operation,
           BTI_VOID_PTR posBlock,
           BTI_VOID_PTR dataBuffer,
           BTI_WORD_PTR dataLength,
           BTI_VOID_PTR keyBuffer,
           BTI_SINT     keyNumber )
{
   BTI_BYTE keyLength  = MAX_KEY_SIZE;
   BTI_CHAR ckeynum    = (BTI_CHAR)keyNumber;
   BTI_ULONG dataLen32 = 0;
   BTI_SINT status;

   if ( dataLength != NULL )
      dataLen32 = *dataLength;

   status = BTRCALL (
              operation,
              posBlock,
              dataBuffer,
              &dataLen32,
              keyBuffer,
              keyLength,
              ckeynum );

   if ( dataLength != NULL )
      *dataLength = (BTI_WORD)dataLen32;

   return status;
}
#endif


/****************************************************************************
   BTRVID() Function for NT, Win32s or 32-bit OS2
***************************************************************************/
#if defined( BTI_WIN_32 ) || defined( BTI_OS2_32 )
#if defined( BTI_OS2_32 )
#define BTRCALLID BTRCALLID32
#endif
BTI_API BTRVID(
           BTI_WORD       operation,
           BTI_VOID_PTR   posBlock,
           BTI_VOID_PTR   dataBuffer,
           BTI_WORD_PTR   dataLength,
           BTI_VOID_PTR   keyBuffer,
           BTI_SINT       keyNumber,
           BTI_BUFFER_PTR clientID )
{
   BTI_BYTE keyLength  = MAX_KEY_SIZE;
   BTI_CHAR ckeynum    = (BTI_CHAR)keyNumber;
   BTI_ULONG dataLen32 = 0;
   BTI_SINT status;

   if ( dataLength != NULL )
      dataLen32 = *dataLength;

   status = BTRCALLID (
              operation,
              posBlock,
              dataBuffer,
              &dataLen32,
              keyBuffer,
              keyLength,
              ckeynum,
              clientID );

   if ( dataLength != NULL )
      *dataLength = (BTI_WORD)dataLen32;

   return status;
}
#endif


/****************************************************************************
   BTRVID() Function for Real Mode DOS ( Btrieve 6.15 or later )
***************************************************************************/
#if defined( BTI_DOS )
BTI_API BTRVID(
           BTI_WORD       operation,
           BTI_VOID_PTR   posBlock,
           BTI_VOID_PTR   dataBuffer,
           BTI_WORD_PTR   dataLength,
           BTI_VOID_PTR   keyBuffer,
           BTI_SINT       keyNumber,
           BTI_BUFFER_PTR clientID )
{
   /* Microsoft changed function and structure names in 7.x . */
   #if defined( _MSC_VER )
      #if ( _MSC_VER > 600 )
         #define segread _segread
         #define int86x  _int86x
         #define REGS    _REGS
         #define SREGS   _SREGS
      #endif
   #endif

   static BOOLEAN btrieveVersionOkay = FALSE;
   BTI_BYTE       versionOffset;
   BTI_BYTE       revisionOffset;
   BTI_BYTE       typeOffset;
   BOOLEAN        done;
   union  REGS    b_regs;
   struct SREGS   b_sregs;
   BTI_SINT       stat = B_NO_ERROR;
   BTI_VOID_PTR   fptr;
   BTI_VOID_PTR   twoPointersPtr;
   pmswParmBlock  newParms;
   BTI_WORD       version;
   BTI_WORD       revision;

   struct                     /* structure passed to Btrieve */
   {
      BTI_VOID_PTR   DATA_BUF;                       /* callers data buffer */
      BTI_WORD       BUF_LEN;                      /* length of data buffer */
      BTI_VOID_PTR   POS_BLOCK;                      /* user position block */
      BTI_VOID_PTR   FCB;                                       /* disk FCB */
      BTI_WORD       FUNCTION;                        /* requested function */
      BTI_VOID_PTR   KEY_BUFFER;                       /* user's key buffer */
      BTI_BYTE       KEY_LENGTH;             /* length of user's key buffer */
      BTI_CHAR       KEY_NUMBER;            /* key of reference for request */
      BTI_SINT_PTR   STATUS;                                 /* status word */
      BTI_SINT       XFACE_ID;                       /* language identifier */
    } XDATA;

   struct
   {
      BTI_VOID_PTR xdataPtr;
      BTI_VOID_PTR clientIdPtr;
   } twoPointers;

   /* Btrieve Parameters for stat call */
   BTI_BYTE posBlockx[ 128 ];
   BTI_BYTE dataBufx[ 255 ];
   BTI_WORD dataLenx;
   BTI_BYTE keyBufx[ 255 ];
   BTI_WORD keyNumx = 0;


   segread( &b_sregs );

   /*=========================================================================
   Check to see that Btrieve has been started.
   =========================================================================*/
   b_regs.x.ax = 0x3500 + BTR_INTRPT;
   int86x( 0x21, &b_regs, &b_regs, &b_sregs );
   if ( b_regs.x.bx != BTR_OFFSET )
      stat = B_RECORD_MANAGER_INACTIVE;

   /*=========================================================================
    Check for correct versions of requester and engine.  This check is done
    only once per application.
   =========================================================================*/
   if ( ( stat == B_NO_ERROR ) && ( !btrieveVersionOkay ) )
   {
      versionOffset  = VERSION_OFFSET;
      revisionOffset = REVISION_OFFSET;
      typeOffset     = TYPE_OFFSET;
      done           = FALSE;
      dataLenx       = sizeof( dataBufx );
      stat = BTRV( B_VERSION, posBlockx, &dataBufx, &dataLenx, keyBufx,
                keyNumx );
      if ( stat == B_NO_ERROR )
      {
         while ( !done )
         {
            version  = (*(BTI_WORD *)&dataBufx[ versionOffset ] );
            revision = (*(BTI_WORD *)&dataBufx[ revisionOffset ] );
            switch( *(BTI_BYTE *)&dataBufx[ typeOffset ] )
            {
               /* Must have requestor version 6.16 or higher. */
               case 'N':
                  if ( ( ( version == 6 ) && ( revision < 16 ) ) ||
                       ( version < 6 ) )
                     {
                        stat = B_INVALID_INTERFACE;
                        done = TRUE;
                     }
                     break;
               /* Must have engine version 6 or higher. */
               case 'D':
                  if ( version < 6 )
                     {
                        stat = B_INVALID_INTERFACE;
                        done = TRUE;
                     }
                     break;
               case 0:
                  done = TRUE;
                  break;
            }
            if ( !done )
            {
               versionOffset  = versionOffset  + VERSION_BUF_SIZE;
               revisionOffset = revisionOffset + VERSION_BUF_SIZE;
               typeOffset     = typeOffset     + VERSION_BUF_SIZE;
               continue;
            }
         }
      }
      else
         stat = B_INVALID_INTERFACE;
   }

   if ( stat == B_NO_ERROR )
   {
      /*======================================================================
      Move user parameters to XDATA, the block where Btrieve expects them.
      ======================================================================*/
      XDATA.FUNCTION    = operation;
      XDATA.STATUS      = &stat;
      XDATA.FCB         = posBlock;
      XDATA.POS_BLOCK   = (BTI_BUFFER_PTR)posBlock + 38;
      XDATA.DATA_BUF    = dataBuffer;
      XDATA.BUF_LEN     = *dataLength;
      XDATA.KEY_BUFFER  = keyBuffer;
      XDATA.KEY_LENGTH  = 255;               /* use max since we don't know */
      XDATA.KEY_NUMBER  = (BTI_CHAR)keyNumber;
      XDATA.XFACE_ID    = VARIABLE_ID;

      btrieveVersionOkay = TRUE;
      twoPointers.xdataPtr = &XDATA;
      twoPointers.clientIdPtr = clientID;
      twoPointersPtr = &twoPointers;
      *((BTI_LONG_PTR) &(newParms.sign)) = *((BTI_LONG_PTR) "PMSW" );
      newParms.flags = 0;
      newParms.functionCode = BTRV_CODE;
      newParms.dataLength = sizeof( newParms );
      newParms.dataPtr = twoPointersPtr;

      /*======================================================================
      Make call to Btrieve.
      ======================================================================*/
      fptr = &newParms;
      b_regs.x.dx = FP_OFF(fptr);
      b_sregs.ds = FP_SEG(fptr);

      int86x( BTR_INTRPT, &b_regs, &b_regs, &b_sregs );
      *dataLength = XDATA.BUF_LEN;
   }
   return( stat );
}
#endif


/***************************************************************************
   BTRV() Function for Real Mode DOS
***************************************************************************/
#if defined( BTI_DOS )
BTI_API BTRV(
           BTI_WORD     operation,
           BTI_VOID_PTR posBlock,
           BTI_VOID_PTR dataBuffer,
           BTI_WORD_PTR dataLength,
           BTI_VOID_PTR keyBuffer,
           BTI_SINT     keyNumber )
{
   /* Microsoft changed function and structure names in 7.x . */
   #if defined( _MSC_VER )
      #if ( _MSC_VER > 600 )
         #define segread _segread
         #define int86x  _int86x
         #define REGS    _REGS
         #define SREGS   _SREGS
      #endif
   #endif

   union REGS b_regs;
   struct SREGS b_sregs;

   struct                     /* structure passed to Btrieve */
   {
      BTI_VOID_PTR   DATA_BUF;                       /* callers data buffer */
      BTI_WORD       BUF_LEN;                      /* length of data buffer */
      BTI_VOID_PTR   POS_BLOCK;                      /* user position block */
      BTI_VOID_PTR   FCB;                                       /* disk FCB */
      BTI_WORD       FUNCTION;                        /* requested function */
      BTI_VOID_PTR   KEY_BUFFER;                       /* user's key buffer */
      BTI_BYTE       KEY_LENGTH;             /* length of user's key buffer */
      BTI_CHAR       KEY_NUMBER;            /* key of reference for request */
      BTI_SINT_PTR   STATUS;                                 /* status word */
      BTI_SINT       XFACE_ID;                       /* language identifier */
    } XDATA;

   BTI_VOID_PTR fptr;
   BTI_SINT     stat = B_NO_ERROR;

   segread( &b_sregs );

   /*=========================================================================
   Check to see that Btrieve has been started.
   =========================================================================*/
   b_regs.x.ax = 0x3500 + BTR_INTRPT;
   int86x( 0x21, &b_regs, &b_regs, &b_sregs );
   if ( b_regs.x.bx != BTR_OFFSET )
      stat = B_RECORD_MANAGER_INACTIVE;

   /*=========================================================================
   Move user parameters to XDATA, the block where Btrieve expects them.
   =========================================================================*/
   if ( stat == B_NO_ERROR )
   {
      XDATA.FUNCTION    = operation;
      XDATA.STATUS      = &stat;
      XDATA.FCB         = posBlock;
      XDATA.POS_BLOCK   = (BTI_BUFFER_PTR)posBlock + 38;
      XDATA.DATA_BUF    = dataBuffer;
      XDATA.BUF_LEN     = *dataLength;
      XDATA.KEY_BUFFER  = keyBuffer;
      XDATA.KEY_LENGTH  = 255;               /* use max since we don't know */
      XDATA.KEY_NUMBER  = (BTI_CHAR)keyNumber;
      XDATA.XFACE_ID    = VARIABLE_ID;

      /*======================================================================
      Make call to Btrieve.
      ======================================================================*/
      fptr = &XDATA;
      b_regs.x.dx = FP_OFF(fptr);    /*parameter block is expected to be in DX */
      b_sregs.ds = FP_SEG(fptr);

      int86x( BTR_INTRPT, &b_regs, &b_regs, &b_sregs );
      *dataLength = XDATA.BUF_LEN;
   }
   return( stat );
}
#endif


/***************************************************************************
   BTRVID() Function for Extended DOS Using Rational + BSTUB.EXE
***************************************************************************/
#if defined( BTI_DOS_32R )
BTI_API BTRVID(
           BTI_WORD       operation,
           BTI_VOID_PTR   posBlock,
           BTI_VOID_PTR   dataBuffer,
           BTI_WORD_PTR   dataLength,
           BTI_VOID_PTR   keyBuffer,
           BTI_SINT       keyNumber,
           BTI_BUFFER_PTR clientID )
{

   BTI_SINT      stat = B_NO_ERROR;
   BTI_VOID_PTR  twoPointersPtr;
   pmswParmBlock newParms;

   struct
   {
      BTI_VOID_PTR xdataPtr;
      BTI_VOID_PTR clientIdPtr;
   } twoPointers;

   /*=========================================================================
   Check to see that Btrieve has been started.
   =========================================================================*/
   /* bstub code will check for presence of Btrieve; no need to check ver */

   /*=========================================================================
   Move user parameters to XDATA, the block where Btrieve expects them.
   =========================================================================*/
   struct                  /* structure passed to Btrieve */
   {
      BTI_VOID_PTR   DATA_BUF;                    /* callers data buffer */
      BTI_WORD       BUF_LEN;                   /* length of data buffer */
      BTI_VOID_PTR   POS_BLOCK;                   /* user position block */
      BTI_VOID_PTR   FCB;                                    /* disk FCB */
      BTI_WORD       FUNCTION;                     /* requested function */
      BTI_VOID_PTR   KEY_BUFFER;                    /* user's key buffer */
      BTI_BYTE       KEY_LENGTH;          /* length of user's key buffer */
      BTI_CHAR       KEY_NUMBER;         /* key of reference for request */
      BTI_SINT_PTR   STATUS;                              /* status word */
      BTI_SINT       XFACE_ID;                    /* language identifier */
    } XDATA;

   /*======================================================================
   Move user parameters to XDATA, the block where Btrieve expects them.
   ======================================================================*/
   XDATA.FUNCTION    = operation;
   XDATA.STATUS      = &stat;
   XDATA.FCB         = posBlock;
   XDATA.POS_BLOCK   = (BTI_BUFFER_PTR)posBlock + 38;
   XDATA.DATA_BUF    = dataBuffer;
   XDATA.BUF_LEN     = *dataLength;
   XDATA.KEY_BUFFER  = keyBuffer;
   XDATA.KEY_LENGTH  = 255;               /* use max since we don't know */
   XDATA.KEY_NUMBER  = (BTI_CHAR)keyNumber;
   XDATA.XFACE_ID    = PROTECTED_ID;

   /*=========================================================================
   Set up the new parmeter block.
   =========================================================================*/
   twoPointers.xdataPtr = &XDATA;
   twoPointers.clientIdPtr = clientID;
   twoPointersPtr = &twoPointers;
   *((BTI_LONG_PTR) &(newParms.sign)) = *((BTI_LONG_PTR) "PMSW" );
   newParms.flags = 0;
   newParms.functionCode = BTRV_CODE;
   newParms.dataLength = sizeof( newParms );
   newParms.dataPtr = twoPointersPtr;

   /*======================================================================
   Make call to Btrieve.
   ======================================================================*/
   (*_ImportedFunctions_)[ PROCESS7B ].functionPtr(
                                                (pmswParmBlock *)&newParms );

   *dataLength = XDATA.BUF_LEN;
   return( stat );
}
#endif


/***************************************************************************
   BTRV() Function for Extended DOS Using Rational + BSTUB.EXE
***************************************************************************/
#if defined( BTI_DOS_32R )
BTI_API BTRV(
           BTI_WORD     operation,
           BTI_VOID_PTR posBlock,
           BTI_VOID_PTR dataBuffer,
           BTI_WORD_PTR dataLength,
           BTI_VOID_PTR keyBuffer,
           BTI_SINT     keyNumber )
{
   BTI_SINT      stat = B_NO_ERROR;

   /*=========================================================================
   Check to see that Btrieve has been started.
   =========================================================================*/
   /* bstub code will check for presence of Btrieve */

   /*=========================================================================
   Move user parameters to XDATA, the block where Btrieve expects them.
   =========================================================================*/
   struct                  /* structure passed to Btrieve */
   {
      BTI_VOID_PTR   DATA_BUF;                    /* callers data buffer */
      BTI_WORD       BUF_LEN;                   /* length of data buffer */
      BTI_VOID_PTR   POS_BLOCK;                   /* user position block */
      BTI_VOID_PTR   FCB;                                    /* disk FCB */
      BTI_WORD       FUNCTION;                     /* requested function */
      BTI_VOID_PTR   KEY_BUFFER;                    /* user's key buffer */
      BTI_BYTE       KEY_LENGTH;          /* length of user's key buffer */
      BTI_CHAR       KEY_NUMBER;         /* key of reference for request */
      BTI_SINT_PTR   STATUS;                              /* status word */
      BTI_SINT       XFACE_ID;                    /* language identifier */
    } XDATA;

   /*======================================================================
   Move user parameters to XDATA, the block where Btrieve expects them.
   ======================================================================*/
   XDATA.FUNCTION    = operation;
   XDATA.STATUS      = &stat;
   XDATA.FCB         = posBlock;
   XDATA.POS_BLOCK   = (BTI_BUFFER_PTR)posBlock + 38;
   XDATA.DATA_BUF    = dataBuffer;
   XDATA.BUF_LEN     = *dataLength;
   XDATA.KEY_BUFFER  = keyBuffer;
   XDATA.KEY_LENGTH  = 255;               /* use max since we don't know */
   XDATA.KEY_NUMBER  = (BTI_CHAR)keyNumber;
   XDATA.XFACE_ID    = PROTECTED_ID;

   /*======================================================================
   Make call to Btrieve.
   ======================================================================*/
   (*_ImportedFunctions_)[ PROCESS7B ].functionPtr(
                                                (pmswParmBlock *)&XDATA );
   *dataLength = XDATA.BUF_LEN;
   return( stat );
}
#endif


/***************************************************************************
   'Borland PowerPack' and 'Phar-Lap TNT 6.0' 32-Bit Interfaces
***************************************************************************/
#if defined( BTI_DOS_32B ) || defined( BTI_DOS_32P )

#define min(a,b)    (((a) < (b)) ? (a) : (b))

#if defined( BTI_DOS_32B )
/***************************************************************************
** Define Far pointer data type (selector:offset32)
**
** The Borland C 32-bit compiler does not support far pointers.  For this
** compiler, a FARPTR is defined as a structure which has the same
** organization in memory as a standard 48-bit far pointer.
**
** The following macros are used to get and set the selector and offset
** fields of far pointers, and work both for compilers that directly
** support far pointers and compilers that don't have far pointer support.
** The FP_SEL macro returns the 16-bit selector number field
** of a far pointer.  The FP_OFF macro returns the 32-bit offset
** field of a far pointer.  The FP_SET macro is used to construct a far
** pointer from a selector number and offset.  It takes three arguments:
** the name of the far pointer to be set, an offset, and a selector number.
** The FP_INCR macro increments the offset field of a far pointer.
***************************************************************************/
   typedef struct
   {
      BTI_ULONG	Off;
      BTI_WORD	Sel;
   } FARPTR;

   #ifndef FP_SEL
      #define FP_SEL(fp) ((fp).Sel)
   #endif

   #ifndef FP_OFF
      #define FP_OFF(fp) ((fp).Off)
   #endif

   #ifndef FP_SET
      #define FP_SET(fp, off, sel) \
	{\
		(fp).Sel = (BTI_WORD) (sel);\
		(fp).Off = (BTI_ULONG) (off);\
	}
   #endif

   #ifndef FP_INCR
      #define FP_INCR(fp, incr) ((fp).Off += (BTI_ULONG) (incr))
   #endif

   /*
   ** Include files for Borland Compiler
   */
   #include <windows.h>

#else
   /*
   ** Include files for PharLap extender
   */
   #include <i86.h>
   #include <pharlap.h>

#endif

#include <blobhdr.h>          /* structure definitions for chunk operations */

/*
** Constant definitions
*/
#define structoffset(s,m) (unsigned)(((char *)(&((s *)0)->m))-((char *)0))
#define Normalize(x)     ((BTI_ULONG)(x))
#define CLIENT_ID_SIZE 16

/*============================================================================
The XDATASTRUCT defines the Btrieve parameter block
============================================================================*/
typedef struct                /* structure passed to Btrieve */
{
   BTI_ULONG      DATA_BUF;                          /* callers data buffer */
   BTI_WORD       BUF_LEN;                         /* length of data buffer */
   BTI_ULONG      POS_BLOCK;                         /* user position block */
   BTI_ULONG      FCB;                                          /* disk FCB */
   BTI_WORD       FUNCTION;                           /* requested function */
   BTI_ULONG      KEY_BUFFER;                          /* user's key buffer */
   BTI_BYTE       KEY_LENGTH;                /* length of user's key buffer */
   BTI_CHAR       KEY_NUMBER;               /* key of reference for request */
   BTI_ULONG      STATUS;                                    /* status word */
   BTI_SINT       XFACE_ID;                          /* language identifier */
} XDATASTRUCT;

/*============================================================================
This structure defines the real mode buffer.  The DATA_BUF parameter is
set up as one byte for reference.  Memory needs to be declared large enough
for data buffers that will be returned.
============================================================================*/
typedef struct
{
   XDATASTRUCT XDATA;
   BTI_SINT    STATUS;
   BTI_BYTE    POS_BLOCK[128];
   BTI_BYTE    KEY_BUFFER[255];
   BTI_BYTE    DATA_BUF[1];
} RMBUFF;


/*============================================================================
Prototypes for functions in protected mode interface
=============================================================================*/
BTI_SINT BtrLoaded( void );

BTI_VOID CallBTRV(
            BTI_ULONG );

BTI_SINT GetRealModeBuffer(
            FARPTR *,
            BTI_ULONG *,
            BTI_ULONG );

BTI_SINT SetUpBTRVData(
            FARPTR *,
            BTI_VOID_PTR,
            BTI_VOID_PTR,
            BTI_VOID_PTR );

BTI_VOID RetrieveBTRVData(
            FARPTR *,
            BTI_VOID_PTR,
            BTI_VOID_PTR,
            BTI_VOID_PTR );

BTI_SINT BlobInputDBlen(
            register XTRACTR *);

BTI_SINT ProcessIndirect(
            FARPTR *,
            BTI_VOID_PTR,
            BTI_WORD,
            BTI_SINT,
            BTI_WORD * );

BTI_SINT VerifyChunk(
            BTI_VOID_PTR,
            BTI_WORD,
            BTI_VOID_PTR,
            BTI_WORD,
            BTI_WORD,
            BTI_WORD );

/*
** The following function are in assembly language- define with CDECL
*/

extern BTI_CHAR  cdecl getLowMemByte(
                          BTI_WORD,
                          BTI_ULONG );

extern BTI_CHAR  cdecl setLowMemByte(
                          BTI_WORD,
                          BTI_ULONG,
                          BTI_BYTE );

extern BTI_WORD  cdecl getLowMemWord(
                          BTI_WORD,
                          BTI_ULONG );

extern BTI_WORD  cdecl setLowMemWord(
                          BTI_WORD,
                          BTI_ULONG,
                          BTI_WORD );

extern BTI_ULONG cdecl getLowMemLong(
                          BTI_WORD,
                          BTI_ULONG );

extern BTI_ULONG cdecl setLowMemLong(
                          BTI_WORD,
                          BTI_ULONG,
                          BTI_ULONG );

extern void cdecl copyToLow(
                     unsigned short dst_sel,
                     unsigned dst_offset,
                     void * src,
                     unsigned int size);

extern void cdecl copyFromLow(
                     void * dst,
                     unsigned short src_sel,
                     unsigned src_offset,
                     unsigned int size);

/*
*****************************************************************************
**
**  Prototype:
**
**      BTI_SINT GetRealModeBuffer(
**                  FARPTR *protectedP,
**                  BTI_ULONG *realAddr,
**                  BTI_ULONG requestedSize )
**
**  Description:
**
**      This function returns a protected mode pointer and a real mode
**      pointer to a block of memory allocated in DOS real memory.
**      This memory is used for the Btrieve parameter block information.
**      When BTI_DOS_32B is defined, this function allocates DOS real
**      memory.  When BTI_DOS_32P is defined, this function uses a
**      DOS buffer that is preallocated by PharLap.
**
**  Preconditions:
**
**      None.
**
**  Parameters:
**
**      *protectedP:        On output, protectedP is the protected mode
**        <output>          pointer to the real memory Btrieve parameter
**                          block.
**
**      *realAddr:          On output, realAddr contains the real mode
**        <output>          pointer the Btrieve parameter block in DOS
**                          real memory.
**
**      requestedSize:      Contains the size of memory to allocate.
**        <input>
**
**
**  Return value:
**
**      B_NO_ERROR          GetRealModeBuffer is successful.
**
**      B_DATALENGTH_ERROR  If BTI_DOS_32B is defined, this status code is
**                          returned if the int386 call fails to allocate
**                          the DOS real memory block.
**
**                          If BTI_DOS_32P is defined, this status is
**                          returned when the requestedSize + dataLength
**                          is smaller than the size of the DOS buffer.
**                          As an enhancement to the interface, a call to
**                          _dx_dosbuf_set could be made to set the
**                          size of the PharLap DOS buffer before the call
**                          to _dx_dosbuf_get instead of relying on the
**                          default buffer size.
**
**
**  Globals:
**
**      None.
**
**  Called Functions:
**
**      BTI_DOS_32B:
**         int386() - Use DPMI services to allocate DOS memory.
**
**      BTI_DOS_32P:
**         _dx_dosbuf_get - PharLap function to return protected mode and
**                          real mode pointers to a DOS buffer that PharLap
**                          maintains.
**
**  Comments:
**
**      None.
**
*****************************************************************************
*/
BTI_SINT GetRealModeBuffer(
            FARPTR *protectedP,    /* protected mode pointer to real memory */
            BTI_ULONG *realAddr,        /* real mode pointer to real memory */
            BTI_ULONG requestedSize )         /* size of buffer to allocate */

#if defined( BTI_DOS_32B )
{
   /*=========================================================================
   GetRealModeBuffer - Borland Power Pack
   =========================================================================*/

   BTI_SINT allocStatus;

   union REGS inRegs, outRegs;

   /*
   ** Allocate DOS Memory
   ** Interrupt 31h, function 100h
   ** Note: The DPMI server must support version 0.9 or later of the DPMI
   **       specification to support this this function.  An error code
   **       will be returned in the AX register and returned to caller
   **       as B_DATALENGTH_ERROR.
   **
   ** AX = 100h
   ** BX = Number of 16-byte paragraphs to allocate
   **
   ** Returns:
   **
   ** if carry flag is clear
   **    AX = real mode segment base address of allocated block
   **    DX = selector for allocated block
   **
   ** if carry flag is set
   **    AX = error code
   **
   */
   inRegs.x.eax = 0x100;
   inRegs.x.ebx = (requestedSize + 15) / 16;        /* Number of paragraphs */

   int386( 0x31, &inRegs, &outRegs );

   if ( outRegs.x.cflag )
      allocStatus = B_DATALENGTH_ERROR;
   else
   {
      *realAddr = (outRegs.x.eax << 16) & 0xFFFF0000;
      FP_SET( *protectedP, 0, (BTI_WORD) outRegs.x.edx );
      allocStatus = B_NO_ERROR;
   }

   return allocStatus;
}
#else
{
   /*=========================================================================
   GetRealModeBuffer - Phar Lap Extender
   =========================================================================*/
   BTI_SINT stat;                                     /* Return status code */
   BTI_ULONG bufferSize;

   /*
   ** Get Information about the DOS Data Buffer
   **
   ** The _dx_dos_buf_get function returns real and protected-mode pointers
   ** to the buffer used to buffer data on MS-DOS and BIOS system calls.  The
   ** size of the buffer is returned in the size parameter.  Note: need to
   ** make this call each time because address of the MS_DOS buffer changes
   ** when anyone calls Load for Debug(_dx_dbg_ld) or the Set DOS Data Buffer
   ** Size(_dx_dosbuf_set).
   */

   _dx_dosbuf_get( (FARPTR *) protectedP, realAddr, &bufferSize);

   if ( bufferSize < requestedSize )
      stat = B_DATALENGTH_ERROR;
   else
      stat = B_NO_ERROR;

   return stat;
}
#endif

/*
*****************************************************************************
**
**  Prototype:
**
**      static void FreeRealModeBuffer( BTI_WORD sel )
**
**  Description:
**
**      FreeRealModeBuffer frees the memory allocated by GetRealModeBuffer
**      when BTI_DOS_32B is defined.  For BTI_DOS_32P there is nothing to do.
**
**  Preconditions:
**
**      None.
**
**  Parameters:
**
**      sel:            Protected mode selector of the Real Mode buffer to
**        <input>       free.
**
**
**  Return value:
**
**      None.
**
**
**  Globals:
**
**      None.
**
**  Called Functions:
**
**      int386
**
**  Comments:
**
**      None.
**
*****************************************************************************
*/
static void FreeRealModeBuffer( BTI_WORD sel )
{
#if defined( BTI_DOS_32B )
   union REGS inRegs, outRegs;


   inRegs.x.edx = sel;                              /* Selector to be freed */
   inRegs.x.eax = 0x101;

   int386( 0x31, &inRegs, &outRegs );

#else

   /*
   ** Phar Lap specific code
   */

   /* do nothing */
#endif
}

/*
*****************************************************************************
**
**  Prototype:
**
**      BTI_SINT BtrLoaded( )
**
**  Description:
**
**      This function checks to see if Btrieve is loaded.
**
**  Preconditions:
**
**      None.
**
**  Parameters:
**
**      None.
**
**  Return value:
**
**      B_NO_ERROR                      Btrieve is loaded
**
**      B_RECORD_MANAGER_INACTIVE       Btrieve is not loaded.
**
**  Globals:
**
**      None.
**
**  Called Functions:
**
**      BTI_DOS_32B             BTI_DOS_32P
**
**        int386()              _dx_rmiv_get()
**
**
**  Comments:
**
**      None.
**
*****************************************************************************
*/
BTI_SINT BtrLoaded( )
{
   BTI_ULONG             btrAddress;
   BTI_SINT              stat;

#if defined( BTI_DOS_32B )
   union REGS            inRegs, outRegs;

   inRegs.x.eax = 0x200;
   inRegs.x.ebx = BTR_INTRPT;

   int386( 0x31, &inRegs, &outRegs );
   btrAddress = outRegs.x.edx;

   if (( (BTI_WORD) (((BTI_ULONG)(btrAddress)) & 0xFFFF) ) != BTR_OFFSET )
      stat = B_RECORD_MANAGER_INACTIVE;
   else
      stat = B_NO_ERROR;

   return ( stat );
}
#else
   /*
   **   The _dx_rmiv_get routine returns the real-mode interrupt
   **   vector for interupt 7b
   */

   _dx_rmiv_get( BTR_INTRPT, &btrAddress );

   if (( (BTI_WORD) (((BTI_ULONG)(btrAddress)) & 0xFFFF) ) != BTR_OFFSET )
      stat = B_RECORD_MANAGER_INACTIVE;
   else
      stat = B_NO_ERROR;

   return (stat);
}
#endif

/*
*****************************************************************************
**
**  Prototype:
**
**      BTI_VOID CallBTRV( BTI_ULONG realPtr )
**
**  Description:
**
**      CallBTRV calls Btrieve using real mode interrupt 7B.
**
**  Preconditions:
**
**      None.
**
**  Parameters:
**
**      realPtr:                Real mode pointer to the Btrieve parameter
**        <input>               block in real memory.
**
**
**  Return value:
**
**      None.
**
**
**  Globals:
**
**      None.
**
**  Called Functions:
**
**      BTI_DOS_32B             BTI_DOS_32P
**
**        int386()              _dx_real_int()
**
**
**  Comments:
**
**      None.
**
**
*****************************************************************************
*/
BTI_VOID CallBTRV( BTI_ULONG realPtr )
#if defined( BTI_DOS_32B )
{
   /*
   **   Call Btrieve using Borland Power Pack DOS Extender
   */
   union  REGS           b_regs;
   struct   REALMODEREGS
   {
      BTI_ULONG edi;
      BTI_ULONG esi;
      BTI_ULONG ebp;
      BTI_ULONG reserved;
      BTI_ULONG ebx;
      BTI_ULONG edx;
      BTI_ULONG ecx;
      BTI_ULONG eax;
      BTI_WORD  CPUflag;
      BTI_WORD  es;
      BTI_WORD  ds;
      BTI_WORD  fs;
      BTI_WORD  gs;
      BTI_WORD  ip;
      BTI_WORD  cs;
      BTI_WORD  sp;
      BTI_WORD  ss;
   } realmodeRegs;

   b_regs.x.eax = 0x300;
   b_regs.h.bl  = BTR_INTRPT;
   b_regs.h.bh  = 0;
     /* CX = Number of words to copy from protected-mode to real-mode stack */
   b_regs.x.ecx = 0;

   /*
   ** Initialize real mode segment registers for call to Btrieve
   */
   memset( &realmodeRegs, 0, sizeof(struct REALMODEREGS) );

   realmodeRegs.ds  = (BTI_WORD) (realPtr >> 16);  /* Segment of parm block */
   realmodeRegs.edx = (realPtr << 16);         /* Offset of parameter block */

   b_regs.x.edi = (BTI_ULONG) &realmodeRegs;

   int386( 0x31, &b_regs, &b_regs );
}
#else
{
   /*
   ** Phar Lap DOS extender specific code
   */
   SWI_REGS              b_regs;

   memset( &b_regs, 0, sizeof( b_regs ) );
   b_regs.ds  = RP_SEG( realPtr );
   b_regs.edx = RP_OFF( realPtr );

   /*
   **   The _dx_real_int routine issues the 7b interrupt in real mode
   */
   _dx_real_int( BTR_INTRPT, &b_regs );
}
#endif


/***************************************************************************
    BTRVID() Function for Extended DOS Using 'Borland PowerPack' or
    'Phar-Lap TNT 6.0' DOS Extenders
***************************************************************************/
BTI_API BTRVID(
           BTI_WORD       operation,
           BTI_VOID_PTR   posBlock,
           BTI_VOID_PTR   dataBuffer,
           BTI_WORD_PTR   dataLength,
           BTI_VOID_PTR   keyBuffer,
           BTI_SINT       keyNumber,
           BTI_BUFFER_PTR clientID )
{
   /* Microsoft changed function and structure names in 7.x . */
   #if defined( _MSC_VER )
      #if ( _MSC_VER > 600 )
         #define segread _segread
         #define int86x  _int86x
         #define REGS    _REGS
         #define SREGS   _SREGS
      #endif
   #endif

   typedef struct
   {
      BTI_ULONG xdataPtr;
      BTI_ULONG clientIdPtr;
   } TWOPTRSTRUCT;

   typedef struct
   {
      pmswParmBlock  newParms;
      TWOPTRSTRUCT   twoPointers;
      BTI_BYTE       clientID[CLIENT_ID_SIZE];
      RMBUFF         btrv;
   } RMBTRVID;

   static BOOLEAN btrieveVersionOkay = FALSE;
   BTI_BYTE       versionOffset;
   BTI_BYTE       revisionOffset;
   BTI_BYTE       typeOffset;
   BOOLEAN        done;
   BTI_SINT       stat = B_NO_ERROR;
   BTI_ULONG      bufferSize;
   FARPTR         protectedP;
   FARPTR         tmpProtectedP;
   BTI_ULONG      realPtr;


   /*
   **   Btrieve Parameters for stat call
   */
   BTI_BYTE posBlockx[ 128 ];
   BTI_BYTE dataBufx[ 255 ];
   BTI_WORD dataLenx;
   BTI_BYTE keyBufx[ 255 ];
   BTI_WORD keyNumx = 0;

   /*=========================================================================
   Check to see that Btrieve has been started.
   =========================================================================*/
   stat = BtrLoaded( );

   /*=========================================================================
    Check for correct versions of requester and engine.  This check is done
    only once per application.
   =========================================================================*/
   if ( ( stat == B_NO_ERROR ) && ( !btrieveVersionOkay ) )
   {
      versionOffset  = VERSION_OFFSET;
      revisionOffset = REVISION_OFFSET;
      typeOffset     = TYPE_OFFSET;
      done           = FALSE;
      dataLenx       = sizeof( dataBufx );
      stat = BTRV(
                B_VERSION,
                posBlockx,
                &dataBufx,
                &dataLenx,
                keyBufx,
                keyNumx );
      if ( stat == B_NO_ERROR )
      {
         while ( !done )
         {
            switch( *(BTI_BYTE *)&dataBufx[ typeOffset ] )
            {
               /* Must have requestor version 6.16 or higher. */
               case 'N':
                  if ( ( (*(BTI_WORD *)&dataBufx[ versionOffset ] ) < 6 ) ||
                         ( ( (*(BTI_WORD *)&dataBufx[ versionOffset ] ) == 6 ) &&
                         ( (*(BTI_WORD *)&dataBufx[ revisionOffset ] ) < 16 ) ) )
                  {
                     stat = B_INVALID_INTERFACE;
                     done = TRUE;
                  }
                  break;

               /* Must have engine version 6 or higher. */
               case 'D':
                  if ( (*(BTI_WORD *)&dataBufx[ versionOffset ] ) < 6 )
                  {
                     stat = B_INVALID_INTERFACE;
                     done = TRUE;
                  }
                  break;

               case 0:
                  done = TRUE;
                  break;
            }
            if ( !done )
            {
               versionOffset  = versionOffset  + VERSION_BUF_SIZE;
               revisionOffset = revisionOffset + VERSION_BUF_SIZE;
               typeOffset     = typeOffset     + VERSION_BUF_SIZE;
               continue;
            }
         }
      }
      else
         stat = B_INVALID_INTERFACE;
   }

   /*=========================================================================
   Get pointers to real memory which is used to transfer data on the
   call to BTRVID
   =========================================================================*/
   if ( stat == B_NO_ERROR )
   {
      bufferSize = sizeof( RMBTRVID ) + *dataLength;
      stat = GetRealModeBuffer( &protectedP, &realPtr, bufferSize );

      if (stat == B_NO_ERROR)
      {
         btrieveVersionOkay = TRUE;

         /*===================================================================
         Establish pointer links inside real mode buffer.  Note: at first
         glance you might think to do this only one time.  However, if you are
         not the only one using the DOS buffer these relations would not be
         there next time around.
         ===================================================================*/

         copyToLow(                                   /* Use PMSW signature */
            FP_SEL(protectedP),
            FP_OFF(protectedP) + structoffset( RMBTRVID, newParms.sign ),
            "PMSW",
            4 );

         setLowMemLong(                                      /* Clear flags */
            FP_SEL(protectedP),
            FP_OFF(protectedP)
               + structoffset( RMBTRVID, newParms.flags ),
            0 );

         setLowMemLong(                 /* set function code to BTRVID code */
            FP_SEL(protectedP),
            FP_OFF(protectedP)
               + structoffset( RMBTRVID, newParms.functionCode ),
            BTRV_CODE );

         setLowMemLong(                             /* clear pmSwitchStatus */
            FP_SEL(protectedP),
            FP_OFF(protectedP)
               + structoffset( RMBTRVID, newParms.pmSwitchStatus ),
            0 );

         setLowMemLong(                                  /* set data length */
            FP_SEL(protectedP),
            FP_OFF(protectedP)
               + structoffset( RMBTRVID, newParms.dataLength ),
            sizeof( pmswParmBlock ) );

         setLowMemLong(    /* set dataPtr to point to twoPointers structure */
            FP_SEL(protectedP),
            FP_OFF(protectedP)
               + structoffset( RMBTRVID, newParms.dataPtr ),
            realPtr + structoffset( RMBTRVID, twoPointers ) );

         setLowMemLong(          /* set xDataPtr to Btrieve parameter block */
            FP_SEL(protectedP),
            FP_OFF(protectedP)
               + structoffset( RMBTRVID, twoPointers.xdataPtr ),
            realPtr + structoffset( RMBTRVID, btrv.XDATA ) );

         setLowMemLong(            /* set pointer to clientID in low memory */
            FP_SEL(protectedP),
            FP_OFF(protectedP)
               + structoffset( RMBTRVID, twoPointers.clientIdPtr ),
            realPtr + structoffset( RMBTRVID, clientID ) );

         copyToLow(                   /* copy clientID from input parameter */
            FP_SEL(protectedP),
            FP_OFF(protectedP) + structoffset( RMBTRVID, clientID ),
            clientID,
            CLIENT_ID_SIZE );

         setLowMemLong(                          /* set data buffer pointer */
            FP_SEL(protectedP),
            FP_OFF(protectedP)
               + structoffset( RMBTRVID, btrv.XDATA.DATA_BUF ),
            realPtr + structoffset( RMBTRVID, btrv.DATA_BUF ) );

         setLowMemWord(             /* set data buffer length in parm block */
            FP_SEL(protectedP),
            FP_OFF(protectedP)
               + structoffset( RMBTRVID, btrv.XDATA.BUF_LEN ),
            *dataLength );

         setLowMemLong(                                   /* Position block */
            FP_SEL(protectedP),
            FP_OFF(protectedP)
               + structoffset( RMBTRVID, btrv.XDATA.POS_BLOCK ),
            realPtr + structoffset( RMBTRVID, btrv.POS_BLOCK) + 38 );

         setLowMemLong(
            FP_SEL(protectedP),
            FP_OFF(protectedP)
               + structoffset( RMBTRVID, btrv.XDATA.FCB ),
            realPtr + structoffset( RMBTRVID, btrv.POS_BLOCK ) );

         setLowMemWord(
            FP_SEL(protectedP),
            FP_OFF(protectedP)
               + structoffset( RMBTRVID, btrv.XDATA.FUNCTION ),
            operation );

         setLowMemLong(
            FP_SEL(protectedP),
            FP_OFF(protectedP)
               + structoffset( RMBTRVID, btrv.XDATA.KEY_BUFFER ),
            realPtr + structoffset( RMBTRVID, btrv.KEY_BUFFER) );

         setLowMemByte(
            FP_SEL(protectedP),
            FP_OFF(protectedP)
               + structoffset( RMBTRVID, btrv.XDATA.KEY_LENGTH ),
            255 );

         setLowMemByte(
            FP_SEL(protectedP),
            FP_OFF(protectedP)
               + structoffset( RMBTRVID, btrv.XDATA.KEY_NUMBER ),
            keyNumber );

         setLowMemLong(
            FP_SEL(protectedP),
            FP_OFF(protectedP)
               + structoffset( RMBTRVID, btrv.XDATA.STATUS ),
            realPtr + structoffset( RMBTRVID, btrv.STATUS ) );

         setLowMemWord(
            FP_SEL(protectedP),
            FP_OFF(protectedP)
               + structoffset( RMBTRVID, btrv.XDATA.XFACE_ID ),
            VARIABLE_ID );

         tmpProtectedP = protectedP;
         FP_INCR(tmpProtectedP, structoffset( RMBTRVID, btrv ));
         stat = SetUpBTRVData(
                   &tmpProtectedP,
                   posBlock,
                   dataBuffer,
                   keyBuffer );

         /*===================================================================
         Make call to Btrieve.
         ===================================================================*/
         if ( stat == B_NO_ERROR )
         {
            CallBTRV( realPtr );

            *dataLength = getLowMemWord(
                             FP_SEL(protectedP),
                             FP_OFF(protectedP)
                                + structoffset(
                                     RMBTRVID,
                                     btrv.XDATA.BUF_LEN ) );

            stat = getLowMemLong(
                      FP_SEL(protectedP),
                      FP_OFF(protectedP)
                         + structoffset( RMBTRVID, btrv.STATUS ) ) ;

            /*================================================================
            Copy data from protected mode back to user's data
            ================================================================*/
            RetrieveBTRVData(
               &tmpProtectedP,
               posBlock,
               dataBuffer,
               keyBuffer );
         }
      }
   }

   return( stat );
}


/***************************************************************************
    BTRV() Function for Extended DOS Using 'Borland PowerPack' or
    'Phar-Lap TNT 6.0' DOS Extenders
***************************************************************************/
BTI_API BTRV(
           BTI_WORD     operation,
           BTI_VOID_PTR posBlock,
           BTI_VOID_PTR dataBuffer,
           BTI_WORD_PTR dataLength,
           BTI_VOID_PTR keyBuffer,
           BTI_SINT     keyNumber )
{
   /* Microsoft changed function and structure names in post 6.0 versions. */
   #if defined( _MSC_VER )
      #if ( _MSC_VER > 600 )
         #define int386x _int386x
         #define REGS    _REGS
      #endif
   #endif

   BTI_SINT              stat = B_NO_ERROR;
   BTI_ULONG             bufferSize;
   FARPTR                protectedP;
   BTI_ULONG             realPtr;

   /*=========================================================================
   Check to see that Btrieve has been started.
   =========================================================================*/
   stat = BtrLoaded( );

   /*=========================================================================
   Move user parameters to XDATA, the block where Btrieve expects them.
   =========================================================================*/
   if ( stat == B_NO_ERROR )
   {
      bufferSize = sizeof( RMBUFF ) + *dataLength;
      stat = GetRealModeBuffer( &protectedP, &realPtr, bufferSize );

      if (stat == B_NO_ERROR)
      {
         /*===================================================================
         Establish pointer links inside real mode buffer.  Note: at first
         glance you might think to do this only one time.  However, if you are
         not the only one using the DOS buffer these relations would not be
         there next time around.
         ===================================================================*/

         setLowMemLong(
            FP_SEL(protectedP),
            FP_OFF(protectedP) + structoffset( RMBUFF, XDATA.POS_BLOCK ),
            realPtr + structoffset(RMBUFF, POS_BLOCK) + 38 );

         setLowMemLong(
            FP_SEL(protectedP),
            FP_OFF(protectedP) + structoffset(RMBUFF, XDATA.FCB),
            realPtr + structoffset( RMBUFF, POS_BLOCK ) );

         setLowMemLong(
            FP_SEL(protectedP),
            FP_OFF(protectedP) + structoffset(RMBUFF, XDATA.STATUS),
            realPtr + structoffset( RMBUFF, STATUS ) );

         setLowMemLong(
            FP_SEL(protectedP),
            FP_OFF(protectedP) + structoffset( RMBUFF, XDATA.KEY_BUFFER ),
            realPtr + structoffset( RMBUFF, KEY_BUFFER) );

         setLowMemLong(
            FP_SEL(protectedP),
            FP_OFF(protectedP) + structoffset( RMBUFF, XDATA.DATA_BUF ),
            realPtr + structoffset( RMBUFF, DATA_BUF ) );

         setLowMemWord(
            FP_SEL(protectedP),
            FP_OFF(protectedP) + structoffset( RMBUFF, XDATA.XFACE_ID ),
            VARIABLE_ID );

         setLowMemWord(
            FP_SEL(protectedP),
            FP_OFF(protectedP) + structoffset( RMBUFF, XDATA.FUNCTION ),
            operation );

         setLowMemWord(
            FP_SEL(protectedP),
            FP_OFF(protectedP) + structoffset( RMBUFF, XDATA.BUF_LEN ),
            *dataLength );

         setLowMemByte(
            FP_SEL(protectedP),
            FP_OFF(protectedP) + structoffset( RMBUFF, XDATA.KEY_LENGTH ),
            255 );

         setLowMemByte(
            FP_SEL(protectedP),
            FP_OFF(protectedP) + structoffset( RMBUFF, XDATA.KEY_NUMBER ),
            keyNumber );

         stat = SetUpBTRVData( &protectedP, posBlock, dataBuffer, keyBuffer );

         /*===================================================================
         Make call to Btrieve.
         ===================================================================*/
         if ( stat == B_NO_ERROR )
         {
            CallBTRV( realPtr );

            *dataLength = getLowMemWord(
                             FP_SEL(protectedP),
                             FP_OFF(protectedP)
                                + structoffset( RMBUFF, XDATA.BUF_LEN) );

            stat = getLowMemLong(
                      FP_SEL(protectedP),
                      FP_OFF(protectedP)
                         + structoffset( RMBUFF, STATUS ) );

            /*================================================================
            Copy data from protected mode back to user's data
            ================================================================*/
            RetrieveBTRVData( &protectedP, posBlock, dataBuffer, keyBuffer );
         }
         FreeRealModeBuffer( FP_SEL(protectedP) );
      }
   }
   return( stat );
}

/*
*****************************************************************************
**
**  Prototype:
**
**      BTI_SINT SetUpBTRVData(
**                  FARPTR *protectedP,
**                  BTI_VOID_PTR posBlock,
**                  BTI_VOID_PTR dataBuffer,
**                  BTI_VOID_PTR keyBuffer )
**
**  Description:
**
**      This function copies Btrieve parameter data from the user's
**      application to the Btrieve parameter block in DOS real memory.
**      It checks the Btrieve function code and key number parameters
**      that are already in the parameter block to see if the operation
**      processes indirect chunk data.  If so, ProcessIndirect() is called
**      to process the indirect Btrieve chunk data.
**
**  Preconditions:
**
**      Real Mode Btrieve parameter block is already initialized with
**      Function code, Key Number, Data buffer length.
**
**  Parameters:
**
**      *protectedP:    Protected mode pointer to the Btrieve parameter block.
**       <input/output>
**
**      posBlock:       Application's parameter block
**        <input>
**
**      dataBuffer:     Application's data buffer
**        <input>
**
**      keyBuffer:      Application's key buffer
**        <input>
**
**
**  Return value:
**
**      B_NO_ERROR      SetUpBTRVData() is successful.
**
**      B_DATA_MESSAGE_TOO_SMALL Chunk cannot fit in the user's data buffer.
**
**      B_INVALID_GET_EXPRESSION Chunk data overwrites chunk descriptors.
**
**
**  Globals:
**
**      None.
**
**  Called Functions:
**
**      BlobInputDBlen()
**      copyToLow()
**      getLowMemByte()
**      getLowMemWord()
**      ProcessIndirect()
**
**  Comments:
**
**      None.
**
*****************************************************************************
*/
BTI_SINT SetUpBTRVData(
            FARPTR *protectedP,
            BTI_VOID_PTR posBlock,
            BTI_VOID_PTR dataBuffer,
            BTI_VOID_PTR keyBuffer )
{
   BTI_WORD function;
   BTI_WORD copyLen;
   BTI_CHAR keyNumber;
   BTI_WORD stat;
   BTI_WORD bufLen;

   function = getLowMemWord(
                 FP_SEL(*protectedP),
                 FP_OFF(*protectedP)
                    + structoffset( RMBUFF, XDATA.FUNCTION) );
   function = function % S_WAIT_LOCK;

   copyLen  = getLowMemWord(
                 FP_SEL(*protectedP),
                 FP_OFF(*protectedP)
                    + structoffset( RMBUFF, XDATA.BUF_LEN ) );

   if (function == B_GET_DIRECT)
   {
      keyNumber = getLowMemByte(
                     FP_SEL(*protectedP),
                     FP_OFF(*protectedP)
                        + structoffset( RMBUFF, XDATA.KEY_NUMBER ) );

      if ( keyNumber == (BTI_CHAR) GET_DRTC_XTRACTOR_KEY )
      {

         PUT_XTRACTR *xtr;

         xtr = (PUT_XTRACTR *)((BTI_BYTE *) dataBuffer + 4);
         if ( xtr->Signature & INDIRECT_BIT)
         {
            bufLen = getLowMemWord(
                        FP_SEL(*protectedP),
                        FP_OFF(*protectedP)
                           + structoffset( RMBUFF, XDATA.BUF_LEN ) );

            if (stat = ProcessIndirect(
                          protectedP,
                          dataBuffer,          /* application's data buffer */
                          bufLen,
                          (BTI_SINT) PREPROCESS_BLOBGET,
                          &copyLen))
            {
               return (stat);
            }
            /*
            ** Data was already copied in ProcessIndirect, so set
            ** copyLen to 0
            */
            copyLen = 0;
         }
         else
         {               /* Only copy the extractors & the record address. */
            copyLen = (BTI_WORD) BlobInputDBlen ((XTRACTR *) xtr) + 4;
         }
      }
   }

   if (function == B_CHUNK_UPDATE)
   {
      PUT_XTRACTR *xtr;

      xtr = (PUT_XTRACTR *)(dataBuffer);

      if ( xtr->Signature & INDIRECT_BIT)
      {
         bufLen = getLowMemWord(
                     FP_SEL(*protectedP),
                     FP_OFF(*protectedP)
                        + structoffset( RMBUFF, XDATA.BUF_LEN ) );

         if (stat = ProcessIndirect(
                       protectedP,
                       dataBuffer,            /* application's data buffer  */
                       bufLen,
                       PREPROCESS_BLOBPUT,
                       &copyLen))
         {
            return (stat);
         }
         /* Data was already copied in ProcessIndirect, so set copyLen to 0 */
         copyLen = 0;
      }
   }

   copyToLow(
      FP_SEL(*protectedP),
      FP_OFF(*protectedP) + structoffset( RMBUFF, POS_BLOCK),
      posBlock,
      128 );

   copyToLow(
      FP_SEL(*protectedP),
      FP_OFF(*protectedP) + structoffset( RMBUFF, KEY_BUFFER),
      keyBuffer,
      255 );

   copyToLow(
      FP_SEL(*protectedP),
      FP_OFF(*protectedP) + structoffset( RMBUFF, DATA_BUF),
      dataBuffer,
      copyLen );

   return (0);
}


/*
*****************************************************************************
**
**  Prototype:
**
**      BTI_VOID RetrieveBTRVData(
**                  FARPTR *protectedP,
**                  BTI_VOID_PTR posBlock,
**                  BTI_VOID_PTR dataBuffer,
**                  BTI_VOID_PTR keyBuffer )
**
**  Description:
**
**      RetrieveBTRVData() copies data from the Btrieve parameter block
**      in DOS real memory to the application's data area.  When there
**      is indirection in the destination of the data, then
**      RetrieveBTRVData() calls ProcessIndirect() to pull the data out
**      of the parameter block and place it at the correct address.
**
**  Preconditions:
**
**      None.
**
**  Parameters:
**
**      *protectedP:            Protected-mode pointer to the Btrieve
**        <input>               parameter block which contains data from
**                              the last Btrieve call.
**
**      posBlock:               Application's position block.
**        <output>
**
**      dataBuffer:             Application's data buffer.
**        <output>
**
**      keyBuffer:              Application's data buffer.
**        <output>
**
**
**  Return value:
**
**      B_NO_ERROR               RetrieveBTRVData() is successful.
**
**      B_DATA_MESSAGE_TOO_SMALL Chunk cannot fit in the user's data buffer.
**
**      B_INVALID_GET_EXPRESSION Chunk data overwrites chunk descriptors.
**
**  Globals:
**
**      None.
**
**  Called Functions:
**
**      copyFromLow()
**      getLowMemWord()
**      ProcessIndirect()
**
**  Comments:
**
**      None.
**
*****************************************************************************
*/
BTI_VOID RetrieveBTRVData(
            FARPTR *protectedP,
            BTI_VOID_PTR posBlock,
            BTI_VOID_PTR dataBuffer,
            BTI_VOID_PTR keyBuffer )
{
   BTI_WORD function;
   BTI_CHAR keyNumber;
   BTI_WORD bufLen;

   function = getLowMemWord(
                 FP_SEL(*protectedP),
                 FP_OFF(*protectedP)
                    + structoffset( RMBUFF, XDATA.FUNCTION) );
   function = function % S_WAIT_LOCK;

   keyNumber = getLowMemWord(
                  FP_SEL(*protectedP),
                  FP_OFF(*protectedP)
                     + structoffset( RMBUFF, XDATA.KEY_NUMBER));

   bufLen = getLowMemWord(
               FP_SEL(*protectedP),
               FP_OFF(*protectedP)
                  + structoffset( RMBUFF, XDATA.BUF_LEN ) );

   if ( (function == B_GET_DIRECT) &&
        (keyNumber == (BTI_CHAR) GET_DRTC_XTRACTOR_KEY ) &&
        (((GET_XTRACTR *) dataBuffer)->Signature & INDIRECT_BIT))
   {
      BTI_WORD ignoredDataLen;

      ProcessIndirect(
         protectedP,
         dataBuffer,
         bufLen,
         POSTPROCESS_BLOBGET,
         &ignoredDataLen );
   }
   else
   {

      copyFromLow(
         dataBuffer,
         FP_SEL(*protectedP),
         FP_OFF(*protectedP) + structoffset( RMBUFF, DATA_BUF ),
         bufLen );

   }

   copyFromLow(
      posBlock,
      FP_SEL(*protectedP),
      FP_OFF(*protectedP) + structoffset( RMBUFF, POS_BLOCK ),
      128 );

   copyFromLow(
      keyBuffer,
      FP_SEL(*protectedP),
      FP_OFF(*protectedP) + structoffset( RMBUFF, KEY_BUFFER ),
      255 );

}

/*
*****************************************************************************
**
**  Prototype:
**
**      BTI_SINT BlobInputDBlen( register XTRACTR * xtractP )
**
**  Description:
**
**      Returns the size of the Chunk Extractors based on the signature
**      field.
**
**  Preconditions:
**
**      None.
**
**  Parameters:
**
**      xtractP:        Pointer to the chunk extractor.
**        <input>
**
**
**  Return value:
**
**      Returns the number of bytes in the extractor descriptor.
**
**  Globals:
**
**      None.
**
**  Called Functions:
**
**      None.
**
**  Comments:
**
**      None.
**
**
*****************************************************************************
*/
BTI_SINT BlobInputDBlen( register XTRACTR * xtractP )
{
   register BTI_WORD ret;

   /*
   ** The size returned is based on the Signature field.
   */
   if ( xtractP->Signature & RECTANGLE_BIT )
   {
      ret = sizeof (PUT_RECTANGLE);
   }
   else
   {                                               /* Signature & numfields */
      ret = (xtractP->NumChunks * sizeof(CHUNK)) + (2 * sizeof(BTI_ULONG));
   }
   return (ret);
}

/*
*****************************************************************************
**
**  Prototype:
**
**      BTI_SINT ProcessIndirect(
**                  FARPTR *protectedP,
**                  BTI_VOID_PTR usersDataBuf,
**                  BTI_WORD maxlen,
**                  BTI_SINT Action,
**                  BTI_WORD *bytesDone )
**
**  Description:
**
**      ProcessIndirect() copies data to/from the address of the
**      application's chunk data.
**
**  Preconditions:
**
**      None.
**
**  Parameters:
**
**      *protectedP:            Protected-mode pointer to Btrieve parameter
**        <input/output>        block (RMBUFF data structure) in DOS real
**                              memory.
**
**      usersDataBuf:           Pointer to application's data buffer.
**        <input/output>
**
**      maxlen:                 Maximum length of application's data buffer.
**        <input>
**
**      Action:                 Type of action to take (See below)
**        <input>
**
**      *bytesDone:             Number of bytes copied.
**        <input/output>
**
**
**  Action can be one of the following:
**
**  PREPROCESS_BLOBGET
**      Chunk extractor information is copied from the user's data buffer
**      to the data buffer in the Btrieve parameter block.  The request
**      type is changed to a direct data request so that we do not
**      have to mess with pointer conversions between real and protected
**      mode.  It also makes it easier for us to copy data to/from
**      real-mode request buffer and user's buffer when request is
**      pre/post processed.
**
**
**  POSTPROCESS_BLOBGET
**      Chunk data is copied from the Btrieve parameter block to the
**      application's address given in the chunk extractor information
**      (still in the application's data buffer)
**
**
**  PREPROCESS_BLOBPUT
**
**      Chunk extractor information is copied from the user's data buffer
**      to the data buffer in the Btrieve parameter block.  The request
**      type is changed from indirect to direct so that the application's
**      data can be copied into the Btrieve parameter block.
**
**  Return value:
**
**      B_NO_ERROR               Success
**
**      B_DATA_MESSAGE_TOO_SMALL Chunk cannot fit in the user's data buffer
**
**      B_INVALID_GET_EXPRESSION Chunk data overwrites chunk descriptors
**
**
**  Globals:
**
**      None.
**
**  Called Functions:
**
**      BlobInputDBlen()
**      copyFromLow()
**      copyToLow()
**      getLowMemLong()
**      setLowMemLong()
**      VerifyChunk()
**
**  Comments:
**
**      None.
**
*****************************************************************************
*/
BTI_SINT ProcessIndirect(
            FARPTR *protectedP,
            BTI_VOID_PTR usersDataBuf,
            BTI_WORD maxlen,
            BTI_SINT Action,
            BTI_WORD *bytesDone )
{
   PUT_XTRACTR   *xtr;
   PUT_RECTANGLE *rectP;
   BTI_WORD      protectSize;
   BTI_WORD      len = 0;                           /* Len of current chunk */
   BTI_WORD      i, limit;                                       /* Helpers */
   BTI_WORD      stat = 0;
   CHUNK         *iP;                        /* Will point to current chunk */
   BTI_SINT      rectangle;
   BTI_ULONG     chunkFlag;
   FARPTR        reqB;

   reqB = *protectedP;

   FP_INCR(reqB, structoffset(RMBUFF, DATA_BUF));

   if ( (Action == PREPROCESS_BLOBGET) || (Action == POSTPROCESS_BLOBGET) )
   {
      xtr = (PUT_XTRACTR *)(((BTI_BYTE *) usersDataBuf) + 4);
   }
   else
   {
      xtr = (PUT_XTRACTR *)usersDataBuf;
   }

   rectP = (PUT_RECTANGLE *) xtr;
     /* protectSize is the size of req. buffer that must not be overwritten */
   *bytesDone = protectSize = (BTI_WORD) BlobInputDBlen ((XTRACTR *)xtr);

   rectangle = xtr->Signature & RECTANGLE_BIT;
   if (rectangle)
      limit = (BTI_WORD)rectP->NumRows;
   else
      limit = (BTI_WORD)xtr->NumChunks;

   switch (Action)
   {
      case PREPROCESS_BLOBGET:

         *bytesDone += 4;                          /* four byte positioning */
         /*
         **    move record positioning information
         */
         setLowMemLong(
            FP_SEL(reqB),
            FP_OFF(reqB),
            *((BTI_ULONG *) usersDataBuf) );
         FP_INCR(reqB,sizeof( BTI_ULONG ));

                         /* Control flows down to copyToLow() and beyond... */

      case PREPROCESS_BLOBPUT:
                                                    /* move over extractors */
         copyToLow(
            FP_SEL(reqB),
            FP_OFF(reqB),
            xtr,
            protectSize );
         /*
         ** Change the request to a direct data request so that we do not
         ** have to mess with pointer conversions between real and protectSize
         ** mode.  It also makes it easier for us to copy data to/from
         ** real-mode request buffer and user's buffer when request is
         ** pre/post processed.
         */

         chunkFlag = getLowMemLong(
                        FP_SEL(reqB),
                        FP_OFF(reqB) );
         setLowMemLong(
            FP_SEL(reqB),
            FP_OFF(reqB),
            chunkFlag & ~INDIRECT_BIT );
         FP_INCR(reqB,protectSize);

         break;

      case POSTPROCESS_BLOBGET:
         *bytesDone = 0;
         break;
   }

   if (Action == PREPROCESS_BLOBGET)
   {
      /*
      ** Zero bytesDone so that it can now accumulate the number of bytes
      ** expected to be returned, looking for status 97.  This is done by
      ** calling VerifyChunk who also catches attempts to overwrite the
      ** descriptors with the returned data.
      */
      *bytesDone = 0;
   }

   /*=========================================================================
   Tack the chunks together at the end of the request buffer.
   =========================================================================*/
   /*
   ** iP is always initialized and maintained, but not used if chunk type
   ** is rectangle.
   */
   iP = &(xtr->Chunk[0]);
   for (i = 0; i < limit; i++)          /* limit is the same for both types */
   {
      register BTI_BYTE *chunkP;

      if ( rectangle )
      {
         if ( rectP->BytesPerRow & 0xffff0000L )   /* must be 16-bit number */
            return ( B_INVALID_GET_EXPRESSION );            /* return error */
         len = (BTI_WORD) rectP->BytesPerRow;
         chunkP = rectP->dataP + (i * rectP->AppDistanceBetweenRows);
      }
      else
      {
         chunkP = iP->dataP;

         if (iP->ChunkLen & 0xffff0000L)           /* must be 16-bit number */
            return (B_INVALID_GET_EXPRESSION);              /* return error */

         len = (BTI_WORD) iP->ChunkLen;
      }


      /*======================================================================
      Does the chunk about to be written overlap with the extractors?
      ======================================================================*/
      stat = VerifyChunk(
                xtr,
                protectSize,
                chunkP,
                len,
                *bytesDone,
                Action == POSTPROCESS_BLOBGET ? 0xffff : maxlen );

      if ( stat != B_NO_ERROR )
      {
         return ( stat );
      }

      if ( *bytesDone >= maxlen )               /* Already consumed buffer? */
      {
         return ( B_DATA_MESSAGE_TOO_SMALL );
      }

      if ( Action != PREPROCESS_BLOBGET )
      {
         switch ( Action )
         {
            case POSTPROCESS_BLOBGET:
               copyFromLow(
                  chunkP,
                  FP_SEL(reqB),
                  FP_OFF(reqB),
                  min( len, maxlen - *bytesDone ) );
               break;

            case PREPROCESS_BLOBPUT:
               copyToLow(
                  FP_SEL(reqB),
                  FP_OFF(reqB),
                  chunkP,
                  min( len, maxlen - *bytesDone ) );
               break;
         }
      }
      FP_INCR(reqB,len);
      *bytesDone += len;
      iP++;
   }

   return ( B_NO_ERROR );
}

/*
*****************************************************************************
**
**  Prototype:
**
**      BTI_SINT VerifyChunk(
**                  BTI_VOID_PTR userDB,
**                  BTI_WORD bytesToProtect,
**                  BTI_VOID_PTR ChunkP,
**                  BTI_WORD ChunkLen,
**                  BTI_WORD bytesDone,
**                  BTI_WORD maxlen )
**
**  Description:
**
**      VerifyChunk makes sure the data will not overwrite the end of the
**      output buffer.  It also catches attempts to overwrite the
**      chunk descriptors with the returned data.
**
**
**  Preconditions:
**
**      None.
**
**  Parameters:
**
**      userDB:                 pointer to the user's data buffer.
**        <input>
**
**      bytesToProtect:         Size of request buffer that must not be
**        <input>               overwritten with returned data.
**
**      ChunkP:                 Pointer to the chunk data.
**        <input>
**
**      ChunkLen:
**        <input>               Length of the chunk data.
**
**      bytesDone:              Number of bytes copied so far to user's
**        <input>               data buffer.
**
**      maxlen:                 Maximum length of the user's data buffer.
**        <input>
**
**
**  Return value:
**
**      B_NO_ERROR               Success
**
**      B_DATA_MESSAGE_TOO_SMALL Chunk cannot fit in the user's data buffer
**
**      B_INVALID_GET_EXPRESSION Chunk data overwrites chunk descriptors
**
**  Globals:
**
**      None.
**
**  Called Functions:
**
**      None.
**
**  Comments:
**
**      None.
**
*****************************************************************************
*/
BTI_SINT VerifyChunk(
            BTI_VOID_PTR userDB,
            BTI_WORD bytesToProtect,
            BTI_VOID_PTR ChunkP,
            BTI_WORD ChunkLen,
            BTI_WORD bytesDone,
            BTI_WORD maxlen )
{
   register BTI_ULONG uNorm = Normalize(userDB),
                      cNorm = Normalize(ChunkP);

   /*
   **   Check for buffer overrun
   */
   if ((BTI_ULONG)bytesDone + (BTI_ULONG)ChunkLen > maxlen)
   {
      return (B_DATA_MESSAGE_TOO_SMALL);
   }
   /*
   **   Check if chunk about to be written overlaps with the extractors
   */
   if ((cNorm + ChunkLen < uNorm) || (cNorm >= uNorm + bytesToProtect))
   {
      return (0);
   }

   return (B_INVALID_GET_EXPRESSION);
}
#endif

#ifdef __cplusplus
}
#endif

