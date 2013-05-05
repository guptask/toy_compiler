/* Include section */
#include "type_check.h"

/* API: Type check init */
bool_t initTypeChecking()
{
    if( NULL == (psProgram = (program_t *) malloc(sizeof(program_t))) )
    {
        printf("Failed to initialize type checking mechanism.\n");
        return FALSE;
    }
    psProgram->ucGlobalVarCnt = psProgram->ucGlobalProcCnt = 0;
    psProgram->ucLocalVarCnt  = psProgram->ucLocalProcCnt  = 0;

    return TRUE;
}

/* API: Fill variable type */
bool_t fillVarType(char *pcType)
{
    return TRUE;
}

/* API: Fill variable name */
bool_t fillVarName(char *pcName)
{
    return TRUE;
}

/* API: Fill arr size */
bool_t fillArrSize(char *pcSize)
{
    return TRUE;
}

/* API: Fill variable in or out */
bool_t fillParamType(char *pcParamType)
{
    return TRUE;
}

/* API: Fill procedure name */
bool_t fillProcName(char *pcName)
{
    return TRUE;
}

/* API: Fill program name */
bool_t fillProgName(char *pcName)
{
    return TRUE;
}


