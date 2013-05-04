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
#define MAX_IN_VAR_CNT        20
#define MAX_OUT_VAR_CNT       20
#define MAX_INTRNL_PROC_CNT   20
#define MAX_GLOBAL_VAR_CNT    20
#define MAX_GLOBAL_PROC_CNT   20
#define MAX_LOCAL_VAR_CNT     20
#define MAX_LOCAL_PROC_CNT    20


/* Enumeration section */
/* Data Types */
typedef enum dataType_e
{
    UNDEFINED_TYPE = 0,
    INTEGER_TYPE,
    FLOAT_TYPE,
    BOOL_TYPE,
    STRING_TYPE

} dataType_t;


/* Structure/union section */
/* Variable */
typedef struct variable_s
{
    char                *pcVarName;
    dataType_t          eDataType;
    bool_t              bIsArray;
    unsigned int        uiArrSize;

} variable_t;

/* Procedure */
typedef struct procedure_s
{
    char                *pcProcName;
    unsigned char       ucInParamCnt;
    variable_t          *arrpsInParam[MAX_IN_VAR_CNT];
    unsigned char       ucOutParamCnt;
    variable_t          *arrpsOutParam[MAX_OUT_VAR_CNT];
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
    procedure_t         *arrpsLocalVar[MAX_LOCAL_VAR_CNT];
    unsigned char       ucLocalProcCnt;
    procedure_t         *arrpsLocalProc[MAX_LOCAL_PROC_CNT];

} program_t;


/* Extern variable declaration section */
extern program_t    *psProgram;
extern unsigned int uiNestingLevel;
extern bool_t       bIsCurrDeclGlobal;


/** Declaration section **/

/* Type check init */
void typeChkInit();

/* Type Check Name */
bool_t typeChkName(char *pcName, bool_t bIsProc);








#endif /* TYPE_CHECK_H_ */

