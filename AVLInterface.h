//
//  Stuff for DailyOps Interfaces to AVL Systems
//
typedef void (FAR * AVLInterfaceRoutineInitDef)(char *, char *, char *);
typedef void (FAR * AVLInterfaceRoutineAddAssignmentDef)(long, long, long, long, long, long, char *);
typedef void (FAR * AVLInterfaceRoutineDelAssignmentDef)(long);
typedef void (FAR * AVLInterfaceRoutineAddDriverDef)(long);
typedef void (FAR * AVLInterfaceRoutineAddBusDef)(long);

EXTERN AVLInterfaceRoutineAddAssignmentDef  m_AddAssignment;
EXTERN AVLInterfaceRoutineDelAssignmentDef  m_DeleteAssignment;
EXTERN AVLInterfaceRoutineAddDriverDef      m_AddDriver;
EXTERN AVLInterfaceRoutineAddBusDef         m_AddBus;
EXTERN AVLInterfaceRoutineInitDef           m_InitClient;

EXTERN char szPostAddress[128];
EXTERN char szHostAddress[128];

EXTERN BOOL  m_bUseStrategicMapping;
EXTERN BOOL  m_bUseConnexionz;

EXTERN CStdioFile BusAssignmentTransactions;

EXTERN void SMAddAssignment(long, long, long, long, long, long, char *);
EXTERN void SMDelAssignment(long);
EXTERN void SMAddDriver(long);
EXTERN void SMAddBus(long);

EXTERN void CNZAddAssignment(long, long, long, long, long, long, char *);
EXTERN void CNZDelAssignment(long);
EXTERN void CNZAddDriver(long);
EXTERN void CNZAddBus(long);
