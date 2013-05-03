/*---------------------------*
 |       TYPE CHECKING       |
 *---------------------------*/

#ifndef TYPE_CHECK_H_
#define TYPE_CHECK_H_

/* Header section */
#include "scan_parse_common.h"


/* Macro section */
#define MAX_IN_VAR_COUNT      10
#define MAX_OUT_VAR_COUNT     10
#define MAX_GLOBAL_VAR_COUNT  20
#define MAX_GLOBAL_PROC_COUNT 20


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
    char            *pcVarName;
    unsigned int    uiVarNameLen;
    dataType_t      eDataType;
    bool_t          bIsArray;
    unsigned int    uiArrSize;

} variable_t;

/* Procedure */
typedef struct procedure_s
{
    char            *pcProcName;
    unsigned int    uiProcNameLen;
    unsigned int    uiInParamNum;
    variable_t      *arrpsInParam[MAX_IN_VAR_COUNT];
    unsigned int    uiOutParamNum;
    variable_t      *arrpsOutParam[MAX_OUT_VAR_COUNT];

} procedure_t;

/* Global Constructs */
typedef struct globalConstruct_s
{
    unsigned int    uiVarNum;
    variable_t      *arrpsVar[MAX_GLOBAL_VAR_COUNT];
    unsigned int    uiProcNum;
    procedure_t     *arrpsProc[MAX_GLOBAL_PROC_COUNT];

} globalConstruct_t;














#endif /* TYPE_CHECK_H_ */

