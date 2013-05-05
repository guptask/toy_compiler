/* Include section */
#include "type_check.h"

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

/* Fill variable type */
bool_t fillVarType(char *pcType)
{
    return TRUE;
}

/* Fill variable name */
bool_t fillVarName(char *pcName)
{
    return TRUE;
}

/* Fill arr size */
bool_t fillArrSize(char *pcSize)
{
    return TRUE;
}

/* Fill variable in or out */
bool_t fillParamType(char *pcParamType)
{
    return TRUE;
}

/* Fill procedure name */
bool_t fillProcName(char *pcName)
{
    return TRUE;
}

/* Fill program name */
bool_t fillProgName(char *pcName)
{
    return TRUE;
}


