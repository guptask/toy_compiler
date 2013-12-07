/*---------------------------*
 |       TYPE CHECKING       |
 *---------------------------*/

#ifndef TYPE_CHECK_H_
#define TYPE_CHECK_H_

/* Header section */
#include "scan_parse_common.h"


/* Macro section */
/* Note: Ideally these count values should be 
         set dynamically. But max count fixed 
         to retain simplicity                 */
#define MAX_PROC_PARAM_CNT    80
#define MAX_INTRNL_PROC_CNT   80
#define MAX_GLOBAL_VAR_CNT    80
#define MAX_GLOBAL_PROC_CNT   80
#define MAX_LOCAL_VAR_CNT     80
#define MAX_LOCAL_PROC_CNT    80
#define MAX_EXPR_OPERATOR_CNT 40
#define MAX_EXPR_OPERAND_CNT  80
#define EXPR_DEBUG_FLAG       0
#define MAX_EXPR_NEST_CNT     80


/* Enumeration section */
/* Data Types */
typedef enum dataType_e
{
    UNDEFINED_TYPE = 0,
    INTEGER_TYPE   = 1,
    FLOAT_TYPE     = 2,
    BOOL_TYPE      = 4,
    STRING_TYPE    = 8

} dataType_t;


/* Structure/union section */
/* Variable */
typedef struct variable_s
{
    char                *pcVarName;
    dataType_t          eDataType;
    char                *pcArrSize;
    bool_t              bIsMemAllocated;
    bool_t              bIsParam;
    bool_t              bIsOutParam;
    unsigned char       ucCallStkDisp;

} variable_t;

/* Procedure */
typedef struct procedure_s
{
    char                *pcProcName;
    unsigned char       ucVariableCnt;
    variable_t          *arrpsVariable[MAX_PROC_PARAM_CNT];
    unsigned char       ucIntrnlProcCnt;
    struct procedure_s  *arrpsIntrnlProc[MAX_INTRNL_PROC_CNT];

} procedure_t;

/* Program */
typedef struct program_s
{
    char                *pcProgName;
    unsigned char       ucGlobalVarCnt;
    variable_t          *arrpsGlobalVar[MAX_GLOBAL_VAR_CNT];
    unsigned char       ucGlobalProcCnt;
    procedure_t         *arrpsGlobalProc[MAX_GLOBAL_PROC_CNT];
    unsigned char       ucLocalVarCnt;
    variable_t          *arrpsLocalVar[MAX_LOCAL_VAR_CNT];
    unsigned char       ucLocalProcCnt;
    procedure_t         *arrpsLocalProc[MAX_LOCAL_PROC_CNT];

} program_t;

/* Expression Tree */
typedef struct exprTree_s
{
    unsigned char ucOperandStkTop;
    dataType_t arreOperandStk[MAX_EXPR_OPERAND_CNT];
    unsigned int arruiOperandRegCnt[MAX_EXPR_OPERAND_CNT];
    unsigned char ucOperatorStkTop;
    char *arrpcOperatorStk[MAX_EXPR_OPERATOR_CNT];
    bool_t arrbOperatorType[MAX_EXPR_OPERATOR_CNT];

} exprTree_t;

/* Extern variable declaration section */
extern program_t        *psProgram;
extern unsigned int     uiNestingLevel;
extern bool_t           bIsCurrDeclGlobal;
extern bool_t           bIsCurrProc;
extern tokenListEntry_t *psAuthToken;

/** Declaration section **/

/* Type check init */
bool_t initTypeChecking();

/* Fill variable type */
bool_t fillVarType( tokenListEntry_t *psNode );

/* Fill variable name */
bool_t fillVarName( tokenListEntry_t *psNode );

/* Fill arr size */
bool_t fillArrSize( tokenListEntry_t *psNode );

/* Fill memory allocation status */
bool_t fillMemAlloStatus();

/* Fill variable in or out */
bool_t fillParamType( tokenListEntry_t *psNode );

/* Fill procedure name */
bool_t fillProcName( tokenListEntry_t *psNode );

/* Fill program name */
bool_t fillProgName( tokenListEntry_t *psNode );

/* Fetch program name */
char *fetchProgName();

/* Authenticate variable scope */
bool_t authVar();

/* Authenticate array */
bool_t authArr( bool_t bIsArrNotDesired );

/* Fetch array size */
char *fetchArrSize();

/* Authenticate data type */
bool_t authDataType();

/* Fetch variable SP displacement */
unsigned char fetchVarSPDisp();

/* Fetch memory allocation status */
bool_t fetchMemAlloStatus();

/* Fetch variable name */
char *fetchVarName();

/* Fetch procedure name */
char *fetchProcName();

/* Fetch data type */
dataType_t fetchDataType();

/* Fetch parameter count */
unsigned char fetchParamCnt();

/* Generate the runtime procedure code */
bool_t genRuntimeProcCode();

/* Fetch parameter data type */
dataType_t fetchParamDataType( unsigned char ucParamNum );

/* Fetch out paramter status */
bool_t fetchOutParamStatus( unsigned char ucParamNum );

/* Authenticate procedure scope */
bool_t authProc();

/* Create the expression tree */
bool_t createExprTree();

/* Destroy the expression tree */
bool_t destroyExprTree();

/* Populate the expression tree operand */
bool_t popuExprTreeOperand( dataType_t eOperand );

/* Populate the expression tree operator */
bool_t popuExprTreeOperator( char *pcOperator, bool_t bIsUnaryOperator );

/* Evaluate the expression tree */
dataType_t evalExprTree( unsigned int *puiRegCount );

/* Generate the code for procedure label */
bool_t writeProcLabel();

/* Generate the code for procedure return */
bool_t writeProcReturn();

/* Generate the code for procedure argument */
bool_t writeProcArgs( unsigned char ucArgSPDisp );

/* Generate the code for procedure call */
bool_t writeProcCall( tokenListEntry_t *psNode );


#endif /* TYPE_CHECK_H_ */

