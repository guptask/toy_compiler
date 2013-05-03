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
    unsigned int        uiVarNameLen;
    dataType_t          eDataType;
    bool_t              bIsArray;
    unsigned int        uiArrSize;

} variable_t;

/* Procedure */
typedef struct procedure_s
{
    char                *pcProcName;
    unsigned int        uiProcNameLen;
    unsigned int        uiInParamCnt;
    variable_t          *arrpsInParam[MAX_IN_VAR_CNT];
    unsigned int        uiOutParamCnt;
    variable_t          *arrpsOutParam[MAX_OUT_VAR_CNT];
    unsigned int        uiIntrnlProcCnt;
    struct procedure_s  *arrpsIntrnlProc[MAX_INTRNL_PROC_CNT];

} procedure_t;

/* Program */
typedef struct program_s
{
    unsigned int        uiGlobalVarCnt;
    variable_t          *arrpsGlobalVar[MAX_GLOBAL_VAR_CNT];
    unsigned int        uiGlobalProcCnt;
    procedure_t         *arrpsGlobalProc[MAX_GLOBAL_PROC_CNT];
    unsigned int        uiLocalVarCnt;
    procedure_t         *arrpsLocalVar[MAX_LOCAL_VAR_CNT];
    unsigned int        uiLocalProcCnt;
    procedure_t         *arrpsLocalProc[MAX_LOCAL_PROC_CNT];

} program_t;


/* Extern declaration section */
extern program_t *psProgram;











#endif /* TYPE_CHECK_H_ */

