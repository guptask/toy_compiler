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
bool_t fillVarType( tokenListEntry_t *psToken )
{
    variable_t  *psTemp = NULL;
    procedure_t *psNode = NULL;

    if(uiNestingLevel < 1)
    {
        printf("Incorrect nesting scenario for variable. Needs to be tested-1.\n");
        return FALSE;
    }
    else
    {
        psTemp = (variable_t *) malloc( sizeof(variable_t) );
        if(!psTemp)
        {
            printf("Failed to allocate space for variable.\n");
            return FALSE;
        }
        psTemp->pcVarName   = NULL;

        if      ( 0 == strcmp(psToken->pcToken, "integer") ) psTemp->eDataType = INTEGER_TYPE;
        else if ( 0 == strcmp(psToken->pcToken,   "float") ) psTemp->eDataType = FLOAT_TYPE;
        else if ( 0 == strcmp(psToken->pcToken,    "bool") ) psTemp->eDataType = BOOL_TYPE;
        else if ( 0 == strcmp(psToken->pcToken,  "string") ) psTemp->eDataType = STRING_TYPE;
        else                                                 psTemp->eDataType = UNDEFINED_TYPE;

        psTemp->pcArrSize   = NULL;
        psTemp->bIsParam    = FALSE;
        psTemp->bIsOutParam = FALSE;

        if(uiNestingLevel == 1)
        {
            if(TRUE == bIsCurrDeclGlobal)
            {
                psProgram->arrpsGlobalVar[psProgram->ucGlobalVarCnt++] = psTemp;
            }
            else
            {
                psProgram->arrpsLocalVar[psProgram->ucLocalVarCnt++] = psTemp;
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

            unsigned int uiTempCount = uiNestingLevel-2;
            while( psNode && (uiTempCount-- > 0) )
            {
                psNode = psNode->arrpsIntrnlProc[psNode->ucIntrnlProcCnt-1];
            }

            if(!psNode)
            {
                printf("Error in type checking for variable type.\n");
                return FALSE;
            }

            psNode->arrpsParam[psNode->ucParamCnt++] = psTemp;
        }
    }
    return TRUE;
}

/* API: Fill variable name */
bool_t fillVarName( tokenListEntry_t *psToken )
{
    variable_t    *psTemp     = NULL;
    procedure_t   *psNode     = NULL;
    bool_t        bRetStatus  = TRUE;
    unsigned char ucTempCount = 0;
    unsigned int  uiNestCount = 0;

    if(uiNestingLevel < 1)
    {
        printf("Incorrect nesting scenario for variable. Needs to be tested-2.\n");
        return FALSE;
    }
    else
    {
        if(uiNestingLevel == 1)
        {
            if(TRUE == bIsCurrDeclGlobal)
            {
                psTemp = psProgram->arrpsGlobalVar[psProgram->ucGlobalVarCnt-1];
            }
            else
            {
                psTemp = psProgram->arrpsLocalVar[psProgram->ucLocalVarCnt-1];
            }

            for(ucTempCount = 0; psProgram->ucGlobalVarCnt > 0; ucTempCount++)
            {
                if( (TRUE == bIsCurrDeclGlobal) && (ucTempCount >= psProgram->ucGlobalVarCnt-1) ) break;
                else if( (FALSE == bIsCurrDeclGlobal) && (ucTempCount >= psProgram->ucGlobalVarCnt) ) break;
                else
                {
                    if( 0 == strcmp(psToken->pcToken, psProgram->arrpsGlobalVar[ucTempCount]->pcVarName) )
                    {
                        printf("Multiple declarations of variable '%s' in same scope on line %u.\n", 
                                                                psToken->pcToken, psToken->uiLineNum);
                        bRetStatus = FALSE;
                    }
                }
            }
            for(ucTempCount = 0; psProgram->ucLocalVarCnt > 0; ucTempCount++)
            {
                if( (TRUE == bIsCurrDeclGlobal) && (ucTempCount >= psProgram->ucLocalVarCnt) ) break;
                else if( (FALSE == bIsCurrDeclGlobal) && (ucTempCount >= psProgram->ucLocalVarCnt-1) ) break;
                else
                {
                    if( 0 == strcmp(psToken->pcToken, psProgram->arrpsLocalVar[ucTempCount]->pcVarName) )
                    {
                        printf("Multiple declarations of variable '%s' in same scope on line %u.\n", 
                                                                psToken->pcToken, psToken->uiLineNum);
                        bRetStatus = FALSE;
                    }
                }
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

            uiNestCount = uiNestingLevel-2;
            while( psNode && (uiNestCount-- > 0) )
            {
                psNode = psNode->arrpsIntrnlProc[psNode->ucIntrnlProcCnt-1];
            }

            if(!psNode)
            {
                printf("Error in type checking for variable name.\n");
                return FALSE;
            }

            for(ucTempCount = 0; ucTempCount < psNode->ucParamCnt-1; ucTempCount++)
            {
                if( 0 == strcmp(psToken->pcToken, psNode->arrpsParam[ucTempCount]->pcVarName) )
                {
                    printf("Multiple declarations of variable '%s' in same scope on line %u.\n", 
                                                            psToken->pcToken, psToken->uiLineNum);
                    bRetStatus = FALSE;
                }
            }
            psTemp = psNode->arrpsParam[psNode->ucParamCnt-1];
        }

        if(!psTemp)
        {
            printf("Error2 in type checking for variable name.\n");
            return FALSE;
        }
        psTemp->pcVarName = psToken->pcToken;
    }
    return bRetStatus;
}

/* API: Fill arr size */
bool_t fillArrSize( tokenListEntry_t *psToken )
{
    variable_t  *psTemp = NULL;
    procedure_t *psNode = NULL;

    if(uiNestingLevel < 1)
    {
        printf("Incorrect nesting scenario for variable. Needs to be tested-3.\n");
        return FALSE;
    }
    else
    {
        if(uiNestingLevel == 1)
        {
            if(TRUE == bIsCurrDeclGlobal)
            {
                psTemp = psProgram->arrpsGlobalVar[psProgram->ucGlobalVarCnt-1];
            }
            else
            {
                psTemp = psProgram->arrpsLocalVar[psProgram->ucLocalVarCnt-1];
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

            unsigned int uiTempCount = uiNestingLevel-2;
            while( psNode && (uiTempCount-- > 0) )
            {
                psNode = psNode->arrpsIntrnlProc[psNode->ucIntrnlProcCnt-1];
            }

            if(!psNode)
            {
                printf("Error in type checking for variable arr size.\n");
                return FALSE;
            }
            psTemp = psNode->arrpsParam[psNode->ucParamCnt-1];
        }

        if(!psTemp)
        {
            printf("Error2 in type checking for variable arr size.\n");
            return FALSE;
        }
        psTemp->pcArrSize = psToken->pcToken;
    }
    return TRUE;
}

/* API: Fill variable in or out */
bool_t fillParamType( tokenListEntry_t *psToken )
{
    variable_t  *psTemp = NULL;
    procedure_t *psNode = NULL;

    if(uiNestingLevel < 2)
    {
        printf("Incorrect nesting scenario for variable. Needs to be tested-4.\n");
        return FALSE;
    }
    else
    {
        if(uiNestingLevel == 2)
        {
            if(TRUE == bIsCurrDeclGlobal)
            {
                psTemp = psProgram->arrpsGlobalVar[psProgram->ucGlobalVarCnt-1];
            }
            else
            {
                psTemp = psProgram->arrpsLocalVar[psProgram->ucLocalVarCnt-1];
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

            unsigned int uiTempCount = uiNestingLevel-2;
            while(uiTempCount-- > 0)
            {
                psNode = psNode->arrpsIntrnlProc[psNode->ucIntrnlProcCnt-1];
            }

            if(!psNode)
            {
                printf("Error in type checking for param type.\n");
                return FALSE;
            }
            psTemp = psNode->arrpsParam[psNode->ucParamCnt-1];
        }

        if(!psTemp)
        {
            printf("Error2 in type checking for param type.\n");
            return FALSE;
        }

        if( 0 == strcmp(psToken->pcToken, "in") )
        {
            psTemp->bIsParam    = TRUE;
            psTemp->bIsOutParam = FALSE;
        }
        else if( 0 == strcmp(psToken->pcToken, "out") )
        {
            psTemp->bIsParam    = TRUE;
            psTemp->bIsOutParam = TRUE;
        }
        else
        {
            printf("Error3 in type checking for param type.\n");
            return FALSE;
        }
    }
    return TRUE;
}

/* API: Fill procedure name */
bool_t fillProcName( tokenListEntry_t *psToken )
{
    procedure_t *psTemp = NULL, *psNode = NULL;
    unsigned int  uiNestCount = 0;
    unsigned char ucTempCount = 0;

    if(uiNestingLevel < 2)
    {
        printf("Incorrect nesting scenario for procedure. Needs to be tested-1.\n");
        return FALSE;
    }
    else
    {
        psTemp = (procedure_t *) malloc( sizeof(procedure_t) );
        if(!psTemp)
        {
            printf("Failed to allocate space for procedure.\n");
            return FALSE;
        }
        psTemp->pcProcName = psToken->pcToken;
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

            for(ucTempCount = 0; psProgram->ucGlobalProcCnt > 0; ucTempCount++)
            {
                if( (TRUE == bIsCurrDeclGlobal) && (ucTempCount >= psProgram->ucGlobalProcCnt-1) ) break;
                else if( (FALSE == bIsCurrDeclGlobal) && (ucTempCount >= psProgram->ucGlobalProcCnt) ) break;
                else
                {
                    if( 0 == strcmp(psToken->pcToken, psProgram->arrpsGlobalProc[ucTempCount]->pcProcName) )
                    {
                        printf("Multiple declarations of procedure '%s' in same scope on line %u.\n", 
                                                                psToken->pcToken, psToken->uiLineNum);
                        return FALSE;
                    }
                }
            }
            for(ucTempCount = 0; psProgram->ucLocalProcCnt > 0; ucTempCount++)
            {
                if( (TRUE == bIsCurrDeclGlobal) && (ucTempCount >= psProgram->ucLocalProcCnt) ) break;
                else if( (FALSE == bIsCurrDeclGlobal) && (ucTempCount >= psProgram->ucLocalProcCnt-1) ) break;
                else
                {
                    if( 0 == strcmp(psToken->pcToken, psProgram->arrpsLocalProc[ucTempCount]->pcProcName) )
                    {
                        printf("Multiple declarations of procedure '%s' in same scope on line %u.\n", 
                                                                psToken->pcToken, psToken->uiLineNum);
                        return FALSE;
                    }
                }
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

            uiNestCount = uiNestingLevel-3;
            while(uiNestCount-- > 0)
            {
                psNode = psNode->arrpsIntrnlProc[psNode->ucIntrnlProcCnt-1];
            }
            psNode->arrpsIntrnlProc[psNode->ucIntrnlProcCnt++] = psTemp;

            for(ucTempCount = 0; ucTempCount < psNode->ucIntrnlProcCnt-1; ucTempCount++)
            {
                if( 0 == strcmp(psToken->pcToken, psNode->arrpsIntrnlProc[ucTempCount]->pcProcName) )
                {
                    printf("Multiple declarations of procedure '%s' in same scope on line %u.\n", 
                                                            psToken->pcToken, psToken->uiLineNum);
                    return FALSE;
                }
            }
        }
    }
    return TRUE;
}

/* API: Fill program name */
bool_t fillProgName( tokenListEntry_t *psToken )
{
    psProgram->pcProgName = psToken->pcToken;
    return TRUE;
}

/* API: Authenticate variable scope */
bool_t authVar()
{
    procedure_t   *psProc     = NULL;
    bool_t        bRetStatus  = FALSE;
    unsigned char ucTempCount = 0;
    unsigned int  uiNestCount = 0;

    if(uiNestingLevel < 1)
    {
        printf("Incorrect nesting scenario for variable. Needs to be tested-5.\n");
        return FALSE;
    }

    /* Check for program local variable */
    else if(uiNestingLevel == 1)
    {
        for(ucTempCount = 0; ucTempCount < psProgram->ucLocalVarCnt; ucTempCount++)
        {
            if( 0 == strcmp(psVariable->pcToken, psProgram->arrpsLocalVar[ucTempCount]->pcVarName) )
            {
                bRetStatus = TRUE;
                break;
            }
        }
    }

    /* Check for procedure local variable */
    else
    {
        if(TRUE == bIsGlobalChain)
        {
            psProc = psProgram->arrpsGlobalProc[psProgram->ucGlobalProcCnt-1];
        }
        else
        {
            psProc = psProgram->arrpsLocalProc[psProgram->ucLocalProcCnt-1];
        }

        uiNestCount = uiNestingLevel-2;
        while( psProc && (uiNestCount-- > 0) )
        {
            psProc = psProc->arrpsIntrnlProc[psProc->ucIntrnlProcCnt-1];
        }

        if(!psProc)
        {
            printf("Error in type checking for variable authentication.\n");
            return FALSE;
        }

        for(ucTempCount = 0; ucTempCount < psProc->ucParamCnt; ucTempCount++)
        {
            if( 0 == strcmp(psVariable->pcToken, psProc->arrpsParam[ucTempCount]->pcVarName) )
            {
                bRetStatus = TRUE;
                break;
            }
        }
    }

    /* Check for global variable */
    for(ucTempCount = 0; (FALSE == bRetStatus) && (ucTempCount < psProgram->ucGlobalVarCnt); ucTempCount++)
    {
        if( 0 == strcmp(psVariable->pcToken, psProgram->arrpsGlobalVar[ucTempCount]->pcVarName) )
        {
            bRetStatus = TRUE;
        }
    }

    if(FALSE == bRetStatus)
    { 
        printf("Undeclared variable '%s' on line %u.\n", psVariable->pcToken, psVariable->uiLineNum);
    }

    return bRetStatus;
}

/* API: Authenticate procedure scope */
bool_t authProc()
{
    procedure_t   *psProc     = NULL;
    bool_t        bRetStatus  = FALSE;
    unsigned char ucTempCount = 0;
    unsigned int  uiNestCount = 0;

    if(uiNestingLevel < 1)
    {
        printf("Incorrect nesting scenario for procedure. Needs to be tested-6.\n");
        return FALSE;
    }

    /* Check for program local procedure */
    else if(uiNestingLevel == 1)
    {
        for(ucTempCount = 0; ucTempCount < psProgram->ucLocalProcCnt; ucTempCount++)
        {
            if( 0 == strcmp(psVariable->pcToken, psProgram->arrpsLocalProc[ucTempCount]->pcProcName) )
            {
                bRetStatus = TRUE;
                break;
            }
        }
    }

    /* Check for procedure local variable */
    else
    {
        if(TRUE == bIsGlobalChain)
        {
            psProc = psProgram->arrpsGlobalProc[psProgram->ucGlobalProcCnt-1];
        }
        else
        {
            psProc = psProgram->arrpsLocalProc[psProgram->ucLocalProcCnt-1];
        }

        uiNestCount = uiNestingLevel-2;
        while( psProc && (uiNestCount-- > 0) )
        {
            psProc = psProc->arrpsIntrnlProc[psProc->ucIntrnlProcCnt-1];
        }

        if(!psProc)
        {
            printf("Error in type checking for procedure authentication.\n");
            return FALSE;
        }

        for(ucTempCount = 0; ucTempCount < psProc->ucIntrnlProcCnt; ucTempCount++)
        {
            if( 0 == strcmp(psVariable->pcToken, psProc->arrpsIntrnlProc[ucTempCount]->pcProcName) )
            {
                bRetStatus = TRUE;
                break;
            }
        }
    }

    /* Check for global variable */
    for(ucTempCount = 0; (FALSE == bRetStatus) && (ucTempCount < psProgram->ucGlobalProcCnt); ucTempCount++)
    {
        if( 0 == strcmp(psVariable->pcToken, psProgram->arrpsGlobalProc[ucTempCount]->pcProcName) )
        {
            bRetStatus = TRUE;
        }
    }

    if(FALSE == bRetStatus)
    { 
        printf("Undeclared procedure '%s' on line %u.\n", psVariable->pcToken, psVariable->uiLineNum);
    }

    return bRetStatus;
}


