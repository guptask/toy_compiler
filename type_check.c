/* Include section */
#include "type_check.h"

/* Static variable */
bool_t bIsGlobalChain = FALSE;


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
    procedure_t *psTemp = NULL, *psNode = NULL;

    if(uiNestingLevel < 2)
    {
        printf("Incorrect nesting scenario. Needs to be tested.\n");
        return FALSE;
    }
    else
    {
        psTemp = (procedure_t *) malloc( sizeof(procedure_t) );
        if(!psTemp)
        {
            printf("Failed to allocate space for procedure name.\n");
            return FALSE;
        }
        psTemp->pcProcName = pcName;
        psTemp->ucParamCnt = 0;
        psTemp->ucIntrnlProcCnt = 0;

        if(uiNestingLevel == 2)
        {
            if(TRUE == bIsCurrDeclGlobal)
            {
                psProgram->arrpsGlobalProc[psProgram->ucGlobalProcCnt++] = psTemp;
                bIsGlobalChain = TRUE;
            }
            else
            {
                psProgram->arrpsLocalProc[psProgram->ucLocalProcCnt++] = psTemp;
                bIsGlobalChain = FALSE;
            }
        }
        else
        {
            if(TRUE == bIsGlobalChain)
            {
                psNode = psProgram->arrpsGlobalProc[psProgram->ucGlobalProcCnt-1];
            }
            else
            {
                psNode = psProgram->arrpsLocalProc[psProgram->ucLocalProcCnt-1];
            }

            unsigned int uiTempCount = uiNestingLevel-3;
            while(uiTempCount-- > 0)
            {
                psNode = psNode->arrpsIntrnlProc[psTemp->ucIntrnlProcCnt-1];
            }
            psNode->arrpsIntrnlProc[psNode->ucIntrnlProcCnt++] = psTemp;
        }
    }
    return TRUE;
}

/* API: Fill program name */
bool_t fillProgName(char *pcName)
{
    psProgram->pcProgName = pcName;
    return TRUE;
}


