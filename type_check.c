/* Include section */
#include "type_check.h"

void typeChkInit()
{
    psProgram = (program_t *) malloc(sizeof(program_t));
    psProgram->ucGlobalVarCnt = psProgram->ucGlobalProcCnt = 
        psProgram->ucLocalVarCnt = psProgram->ucLocalProcCnt = 0;
}

/* Type checking procedure or program name */
bool_t typeChkName(char *pcName, bool_t bIsProc)
{
    switch(uiNestingLevel)
    {
        case 0:
        {
            printf("Type check nesting counter error.\n");
            return FALSE;
        } break;

        case 1:
        {
            psProgram->pcProgName = pcName;
        } break;

        default:
        {
            if(TRUE == bIsProc)
            {
                if(2 == uiNestingLevel)
                {
                    if(TRUE == bIsProc)
                    {
                        procedure_t *psProcTemp = (procedure_t *) malloc(sizeof(procedure_t));
                        psProcTemp->pcProcName = pcName;
                        psProcTemp->ucInParamCnt = psProcTemp->ucOutParamCnt = 
                                                       psProcTemp->ucIntrnlProcCnt = 0;
                    }
                    else
                    {
                        variable_t *psVarTemp = (variable_t *) malloc(sizeof(variable_t));
                        psVarTemp->pcVarName = pcName;
                    }
                }
                else
                {
                    printf("Type check for global procedure error.\n");
                    return FALSE;
                }
            }
        }
    }
    return TRUE;
}

