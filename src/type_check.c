/* Include section */
#include "type_check.h"
#include "code_generation.h"

/* Macro Section */
#define MAX_EXPR_TREE_ARR_LEN 20
#define MAX_GOTO_TAGS         20

/* Static variable */
static bool_t        bIsGlobalChain = FALSE;
static unsigned char ucExpressionTreeCnt = 0;
static unsigned int  uiArgNum = 0;
static unsigned char ucGotoTagCount = 0;
static char          arrcGotoTag[MAX_GOTO_TAGS][LENGTH_OF_EACH_LINE] = {{0}};
static exprTree_t    *expressionTree[MAX_EXPR_TREE_ARR_LEN];
static variable_t    *psVariable  = NULL;
static procedure_t   *psProcedure = NULL;

/* API: Type check init */
bool_t initTypeChecking()
{
    procedure_t *psProc = NULL;
    variable_t  *psVar  = NULL;

    if( NULL == (psProgram = (program_t *) malloc(sizeof(program_t))) )
    {
        printf("Failed to initialize type checking mechanism.\n");
        return FALSE;
    }
    psProgram->ucGlobalVarCnt = psProgram->ucGlobalProcCnt = 0;
    psProgram->ucLocalVarCnt  = psProgram->ucLocalProcCnt  = 0;

    /* Pre-populate the runtime APIs */
    /* getInteger(integer a out) */
    psProc = (procedure_t *) malloc( sizeof(procedure_t) );
    if(!psProc)
    {
        printf("Failed to allocate space for procedure.\n");
        return FALSE;
    }
    psProc->pcProcName = (char *) malloc( 11*sizeof(char *) );
    strcpy( psProc->pcProcName, "getInteger" );
    psProc->ucIntrnlProcCnt = 0;
    psProc->ucVariableCnt = 1;
    psVar = (variable_t *) malloc( sizeof(variable_t) );
    if(!psVar)
    {
        printf("Failed to allocate space for variable.\n");
        return FALSE;
    }
    psProc->arrpsVariable[0] = psVar;
    psVar->pcVarName         = NULL;
    psVar->eDataType         = INTEGER_TYPE;
    psVar->pcArrSize         = NULL;
    psVar->bIsParam          = TRUE;
    psVar->bIsOutParam       = TRUE;
    psVar->uiCallStkDisp     = 1;
    psProgram->arrpsGlobalProc[(psProgram->ucGlobalProcCnt)++] = psProc;
    psProc->uiReturnAddrDisp = 2;

    /* getBool(bool a out) */
    psProc = NULL;
    psVar  = NULL;
    psProc = (procedure_t *) malloc( sizeof(procedure_t) );
    if(!psProc)
    {
        printf("Failed to allocate space for procedure.\n");
        return FALSE;
    }
    psProc->pcProcName = (char *) malloc( 8*sizeof(char *) );
    strcpy( psProc->pcProcName, "getBool" );
    psProc->ucIntrnlProcCnt = 0;
    psProc->ucVariableCnt = 1;
    psVar = (variable_t *) malloc( sizeof(variable_t) );
    if(!psVar)
    {
        printf("Failed to allocate space for variable.\n");
        return FALSE;
    }
    psProc->arrpsVariable[0] = psVar;
    psVar->pcVarName         = NULL;
    psVar->eDataType         = BOOL_TYPE;
    psVar->pcArrSize         = NULL;
    psVar->bIsParam          = TRUE;
    psVar->bIsOutParam       = TRUE;
    psVar->uiCallStkDisp     = 1;
    psProgram->arrpsGlobalProc[(psProgram->ucGlobalProcCnt)++] = psProc;
    psProc->uiReturnAddrDisp = 2;

    /* getFloat(float a out) */
    psProc = NULL;
    psVar  = NULL;
    psProc = (procedure_t *) malloc( sizeof(procedure_t) );
    if(!psProc)
    {
        printf("Failed to allocate space for procedure.\n");
        return FALSE;
    }
    psProc->pcProcName = (char *) malloc( 9*sizeof(char *) );
    strcpy( psProc->pcProcName, "getFloat" );
    psProc->ucIntrnlProcCnt = 0;
    psProc->ucVariableCnt = 1;
    psVar = (variable_t *) malloc( sizeof(variable_t) );
    if(!psVar)
    {
        printf("Failed to allocate space for variable.\n");
        return FALSE;
    }
    psProc->arrpsVariable[0] = psVar;
    psVar->pcVarName         = NULL;
    psVar->eDataType         = FLOAT_TYPE;
    psVar->pcArrSize         = NULL;
    psVar->bIsParam          = TRUE;
    psVar->bIsOutParam       = TRUE;
    psVar->uiCallStkDisp     = 1;
    psProgram->arrpsGlobalProc[(psProgram->ucGlobalProcCnt)++] = psProc;
    psProc->uiReturnAddrDisp = 2;

    /* getString(string a in, integer b out) */
    psProc = NULL;
    psVar  = NULL;
    psProc = (procedure_t *) malloc( sizeof(procedure_t) );
    if(!psProc)
    {
        printf("Failed to allocate space for procedure.\n");
        return FALSE;
    }
    psProc->pcProcName = (char *) malloc( 10*sizeof(char *) );
    strcpy( psProc->pcProcName, "getString" );
    psProc->ucIntrnlProcCnt = 0;
    psProc->ucVariableCnt = 2;
    psVar = (variable_t *) malloc( sizeof(variable_t) );
    if(!psVar)
    {
        printf("Failed to allocate space for variable.\n");
        return FALSE;
    }
    psProc->arrpsVariable[0] = psVar;
    psVar->pcVarName         = NULL;
    psVar->eDataType         = STRING_TYPE;
    psVar->pcArrSize         = NULL;
    psVar->bIsParam          = TRUE;
    psVar->bIsOutParam       = FALSE;
    psVar->uiCallStkDisp     = 1;

    psVar = NULL;
    psVar = (variable_t *) malloc( sizeof(variable_t) );
    if(!psVar)
    {
        printf("Failed to allocate space for variable.\n");
        return FALSE;
    }
    psProc->arrpsVariable[1] = psVar;
    psVar->pcVarName         = NULL;
    psVar->eDataType         = INTEGER_TYPE;
    psVar->pcArrSize         = NULL;
    psVar->bIsParam          = TRUE;
    psVar->bIsOutParam       = TRUE;
    psVar->uiCallStkDisp     = 2;

    psProgram->arrpsGlobalProc[(psProgram->ucGlobalProcCnt)++] = psProc;
    psProc->uiReturnAddrDisp = 3;

    /* putInteger(integer a in) */
    psProc = (procedure_t *) malloc( sizeof(procedure_t) );
    if(!psProc)
    {
        printf("Failed to allocate space for procedure.\n");
        return FALSE;
    }
    psProc->pcProcName = (char *) malloc( 11*sizeof(char *) );
    strcpy( psProc->pcProcName, "putInteger" );
    psProc->ucIntrnlProcCnt = 0;
    psProc->ucVariableCnt = 1;
    psVar = (variable_t *) malloc( sizeof(variable_t) );
    if(!psVar)
    {
        printf("Failed to allocate space for variable.\n");
        return FALSE;
    }
    psProc->arrpsVariable[0] = psVar;
    psVar->pcVarName         = NULL;
    psVar->eDataType         = INTEGER_TYPE;
    psVar->pcArrSize         = NULL;
    psVar->bIsParam          = TRUE;
    psVar->bIsOutParam       = FALSE;
    psVar->uiCallStkDisp     = 1;
    psProgram->arrpsGlobalProc[(psProgram->ucGlobalProcCnt)++] = psProc;
    psProc->uiReturnAddrDisp = 2;

    /* putBool(bool a in) */
    psProc = NULL;
    psVar  = NULL;
    psProc = (procedure_t *) malloc( sizeof(procedure_t) );
    if(!psProc)
    {
        printf("Failed to allocate space for procedure.\n");
        return FALSE;
    }
    psProc->pcProcName = (char *) malloc( 8*sizeof(char *) );
    strcpy( psProc->pcProcName, "putBool" );
    psProc->ucIntrnlProcCnt = 0;
    psProc->ucVariableCnt = 1;
    psVar = (variable_t *) malloc( sizeof(variable_t) );
    if(!psVar)
    {
        printf("Failed to allocate space for variable.\n");
        return FALSE;
    }
    psProc->arrpsVariable[0] = psVar;
    psVar->pcVarName         = NULL;
    psVar->eDataType         = BOOL_TYPE;
    psVar->pcArrSize         = NULL;
    psVar->bIsParam          = TRUE;
    psVar->bIsOutParam       = FALSE;
    psVar->uiCallStkDisp     = 1;
    psProgram->arrpsGlobalProc[(psProgram->ucGlobalProcCnt)++] = psProc;
    psProc->uiReturnAddrDisp = 2;

    /* putFloat(float a in) */
    psProc = NULL;
    psVar  = NULL;
    psProc = (procedure_t *) malloc( sizeof(procedure_t) );
    if(!psProc)
    {
        printf("Failed to allocate space for procedure.\n");
        return FALSE;
    }
    psProc->pcProcName = (char *) malloc( 9*sizeof(char *) );
    strcpy( psProc->pcProcName, "putFloat" );
    psProc->ucIntrnlProcCnt = 0;
    psProc->ucVariableCnt = 1;
    psVar = (variable_t *) malloc( sizeof(variable_t) );
    if(!psVar)
    {
        printf("Failed to allocate space for variable.\n");
        return FALSE;
    }
    psProc->arrpsVariable[0] = psVar;
    psVar->pcVarName         = NULL;
    psVar->eDataType         = FLOAT_TYPE;
    psVar->pcArrSize         = NULL;
    psVar->bIsParam          = TRUE;
    psVar->bIsOutParam       = FALSE;
    psVar->uiCallStkDisp     = 1;
    psProgram->arrpsGlobalProc[(psProgram->ucGlobalProcCnt)++] = psProc;
    psProc->uiReturnAddrDisp = 2;

    /* putString(string a in) */
    psProc = NULL;
    psVar  = NULL;
    psProc = (procedure_t *) malloc( sizeof(procedure_t) );
    if(!psProc)
    {
        printf("Failed to allocate space for procedure.\n");
        return FALSE;
    }
    psProc->pcProcName = (char *) malloc( 10*sizeof(char *) );
    strcpy( psProc->pcProcName, "putString" );
    psProc->ucIntrnlProcCnt = 0;
    psProc->ucVariableCnt = 1;
    psVar = (variable_t *) malloc( sizeof(variable_t) );
    if(!psVar)
    {
        printf("Failed to allocate space for variable.\n");
        return FALSE;
    }
    psProc->arrpsVariable[0] = psVar;
    psVar->pcVarName         = NULL;
    psVar->eDataType         = STRING_TYPE;
    psVar->pcArrSize         = NULL;
    psVar->bIsParam          = TRUE;
    psVar->bIsOutParam       = FALSE;
    psVar->uiCallStkDisp     = 1;
    psProgram->arrpsGlobalProc[(psProgram->ucGlobalProcCnt)++] = psProc;
    psProc->uiReturnAddrDisp = 2;

    return TRUE;
}

/* API: Fill variable type */
bool_t fillVarType( tokenListEntry_t *psToken )
{
    variable_t  *psTemp = NULL;
    procedure_t *psNode = NULL;
    psVariable          = NULL;

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

        psTemp->pcArrSize     = NULL;
        psTemp->bIsParam      = FALSE;
        psTemp->bIsOutParam   = FALSE;
        psTemp->uiCallStkDisp = 0;

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

            psNode->arrpsVariable[psNode->ucVariableCnt++] = psTemp;
        }
        psVariable = psTemp;
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
        /* If at program level */
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

            /* Compare variable name with global variable names */
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

            /* Compare variable name with local variable names */
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
        else /* when at procedure level */
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

            for(ucTempCount = 0; ucTempCount < psNode->ucVariableCnt-1; ucTempCount++)
            {
                if( 0 == strcmp(psToken->pcToken, psNode->arrpsVariable[ucTempCount]->pcVarName) )
                {
                    printf("Multiple declarations of variable '%s' in same scope on line %u.\n", 
                                                            psToken->pcToken, psToken->uiLineNum);
                    bRetStatus = FALSE;
                }
            }
            psTemp = psNode->arrpsVariable[psNode->ucVariableCnt-1];
        }

        if(!psTemp)
        {
            printf("Error2 in type checking for variable name.\n");
            return FALSE;
        }
        if( psVariable != psTemp )
        {
            printf("Internally something is wrong in variable addressing-1.\n");
            return FALSE;
        }
        psTemp->pcVarName = psToken->pcToken;
    }
    return bRetStatus;
}

/* API: Fill arr size */
bool_t fillArrSize( tokenListEntry_t *psToken )
{
    if( !psVariable )
    {
        printf("Internally something is wrong in variable addressing-2.\n");
        return FALSE;
    }
    psVariable->pcArrSize = psToken->pcToken;
    return TRUE;
}

/* API: Fill variable in or out */
bool_t fillParamType( tokenListEntry_t *psToken )
{
    if( !psVariable )
    {
        printf("Internally something is wrong in variable addressing-3.\n");
        return FALSE;
    }

    if( 0 == strcmp(psToken->pcToken, "in") )
    {
        psVariable->bIsParam    = TRUE;
        psVariable->bIsOutParam = FALSE;
    }
    else if( 0 == strcmp(psToken->pcToken, "out") )
    {
        psVariable->bIsParam    = TRUE;
        psVariable->bIsOutParam = TRUE;
    }
    else
    {
        printf("Internally something is wrong in variable addressing-4.\n");
        return FALSE;
    }

    psVariable->uiCallStkDisp     = ++uiArgNum;
    psProcedure->uiReturnAddrDisp = uiArgNum + 1;

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
        psTemp->pcProcName       = psToken->pcToken;
        psTemp->ucVariableCnt    = 0;
        psTemp->ucIntrnlProcCnt  = 0;
        psTemp->uiReturnAddrDisp = 0;

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

            /* Check for name conflict with global procedure */
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

            /* Check for name conflict with local procedure */
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
            while( psNode && (uiNestCount-- > 0) )
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
        psProcedure = psTemp;
        uiArgNum = 0;
    }

    /* Store the goto tag */
    arrcGotoTag[ucGotoTagCount][0] = 0;
    sprintf(arrcGotoTag[ucGotoTagCount++], "_%p_%s_ :\n", psTemp, psToken->pcToken);

    return TRUE;
}

/* API: Fill program name */
bool_t fillProgName( tokenListEntry_t *psToken )
{
    psProgram->pcProgName = psToken->pcToken;
    return TRUE;
}

/* API: Fetch program name */
char *fetchProgName()
{
    return psProgram->pcProgName;
}

/* API: Authenticate variable scope */
bool_t authVar()
{
    procedure_t   *psProc     = NULL;
    bool_t        bRetStatus  = FALSE;
    unsigned char ucTempCount = 0;
    unsigned int  uiNestCount = 0;
    psVariable                = NULL;

    if(uiNestingLevel < 1)
    {
        printf("Incorrect nesting scenario for variable. Needs to be tested-3.\n");
        return FALSE;
    }

    /* Check for program local variable */
    else if(uiNestingLevel == 1)
    {
        for(ucTempCount = 0; ucTempCount < psProgram->ucLocalVarCnt; ucTempCount++)
        {
            if( 0 == strcmp(psAuthToken->pcToken, psProgram->arrpsLocalVar[ucTempCount]->pcVarName) )
            {
                bRetStatus = TRUE;
                psVariable = psProgram->arrpsLocalVar[ucTempCount];
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

        for(ucTempCount = 0; ucTempCount < psProc->ucVariableCnt; ucTempCount++)
        {
            if( 0 == strcmp(psAuthToken->pcToken, psProc->arrpsVariable[ucTempCount]->pcVarName) )
            {
                bRetStatus = TRUE;
                psVariable = psProc->arrpsVariable[ucTempCount];
                break;
            }
        }
    }

    /* Check for global variable */
    for(ucTempCount = 0; (FALSE == bRetStatus) && (ucTempCount < psProgram->ucGlobalVarCnt); ucTempCount++)
    {
        if( 0 == strcmp(psAuthToken->pcToken, psProgram->arrpsGlobalVar[ucTempCount]->pcVarName) )
        {
            bRetStatus = TRUE;
            psVariable = psProgram->arrpsGlobalVar[ucTempCount];
        }
    }

    if(FALSE == bRetStatus)
    { 
        printf("Undeclared variable '%s' on line %u.\n", psAuthToken->pcToken, psAuthToken->uiLineNum);
    }

    return bRetStatus;
}

/* API: Authenticate array */
bool_t authArr( bool_t bIsArrNotDesired )
{
    if(!psVariable)
    {
        printf("1.This error should not occur.\n");
        return FALSE;
    }
    if( !psVariable->pcArrSize )
    {
        if( FALSE == bIsArrNotDesired )
        {
            printf("Variable '%s' on line %u is not an array.\n", psAuthToken->pcToken, psAuthToken->uiLineNum);
        }
        return FALSE;
    }
    if( TRUE == bIsArrNotDesired )
    {
        printf("Variable '%s' on line %u missing array syntax.\n", psAuthToken->pcToken, psAuthToken->uiLineNum);
    }
    return TRUE;
}

/* API: Authenticate data type */
bool_t authDataType()
{
    if(!psVariable)
    {
        printf("2.This error should not occur.\n");
        return FALSE;
    }
    if( UNDEFINED_TYPE == psVariable->eDataType )
    {
        printf("Data type of variable '%s' on line %u is undefined.\n", psAuthToken->pcToken, psAuthToken->uiLineNum);
        return FALSE;
    }
    return TRUE;
}

/* API: Fetch procedure name */
char *fetchProcName()
{
    if(!psProcedure)
    {
        return NULL;
    }
    return (psProcedure->pcProcName);
}

/* API: Fetch data type */
dataType_t fetchDataType()
{
    if(!psVariable)
    {
        printf("3.This error should not occur.\n");
        return FALSE;
    }
    return psVariable->eDataType;
}

/* API: Fetch parameter count */
unsigned char fetchParamCnt()
{
    unsigned char ucIndex = 0;
    if(!psProcedure)
    {
        printf("8.This error should not occur.\n");
        return -1;
    }
    for( ucIndex = 0; (ucIndex < psProcedure->ucVariableCnt) &&
                        (TRUE == psProcedure->arrpsVariable[ucIndex]->bIsParam); ucIndex++);
    return ucIndex;
}

/* API: Fetch parameter data type */
dataType_t fetchParamDataType( unsigned char ucParamNum )
{
    unsigned char ucIndex = 0;
    if(!psProcedure)
    {
        printf("4.This error should not occur.\n");
        return UNDEFINED_TYPE;
    }
    if( !(psProcedure->ucVariableCnt) )
    {
        printf("Procedure has not arguments.\n");
        return UNDEFINED_TYPE;
    }

    for( ucIndex = 0; (ucIndex < psProcedure->ucVariableCnt) &&
                        (TRUE == psProcedure->arrpsVariable[ucIndex]->bIsParam); ucIndex++)
    {
        if( ucParamNum-1 == ucIndex )
        {
            if(EXPR_DEBUG_FLAG)
            {
                printf( "Argument = '%s'(%d)\n", psProcedure->arrpsVariable[ucIndex]->pcVarName, 
                                                    psProcedure->arrpsVariable[ucIndex]->eDataType );
            }
            return (psProcedure->arrpsVariable[ucIndex]->eDataType);
        }
    }
    if( (ucParamNum == 0) || (ucParamNum > ucIndex) )
    {
        printf("Invalid paramter number requested.\n");
    }
    return UNDEFINED_TYPE;
}

/* API: Generate the runtime procedure code */
bool_t genRuntimeProcCode()
{
    unsigned char ucIndex             = 0;
    procedure_t *psTempProc           = NULL;
    char arrcStr[LENGTH_OF_EACH_LINE] = {0};

    for(ucIndex = 0; ucIndex < psProgram->ucGlobalProcCnt; ucIndex++)
    {
        if( !(psTempProc = psProgram->arrpsGlobalProc[ucIndex]) )
        {
            return FALSE;
        }
        sprintf(arrcStr, "_%p_%s_ :\n", psTempProc, psTempProc->pcProcName);
        if( TRUE != genCodeInputString(arrcStr) )
        {
            return FALSE;
        }
        if( TRUE != genCodeAddRuntimeDef(psTempProc->pcProcName) )
        {
            return FALSE;
        }
    }
    return TRUE;
}

/* API: Authenticate procedure scope */
bool_t authProc()
{
    procedure_t   *psProc     = NULL;
    bool_t        bRetStatus  = FALSE;
    unsigned char ucTempCount = 0;
    unsigned int  uiNestCount = 0;
    psProcedure               = NULL;

    if(uiNestingLevel < 1)
    {
        printf("Incorrect nesting scenario for procedure. Needs to be tested-2.\n");
        return FALSE;
    }

    /* Check for program level procedure */
    else if(uiNestingLevel == 1)
    {
        for(ucTempCount = 0; ucTempCount < psProgram->ucLocalProcCnt; ucTempCount++)
        {
            if( 0 == strcmp(psAuthToken->pcToken, psProgram->arrpsLocalProc[ucTempCount]->pcProcName) )
            {
                bRetStatus = TRUE;
                psProcedure = psProgram->arrpsLocalProc[ucTempCount];
                break;
            }
        }
    }

    /* Check for internal procedures */
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
            if( 0 == strcmp(psAuthToken->pcToken, psProc->arrpsIntrnlProc[ucTempCount]->pcProcName) )
            {
                bRetStatus = TRUE;
                psProcedure = psProc->arrpsIntrnlProc[ucTempCount];
                break;
            }
        }

        /* Check for recursion */
        if( (TRUE != bRetStatus) && (0 == strcmp(psAuthToken->pcToken, psProc->pcProcName)) )
        {
            bRetStatus = TRUE;
            psProcedure = psProc;
        }
    }

    /* Check for global procedure */
    for(ucTempCount = 0; (FALSE == bRetStatus) && (ucTempCount < psProgram->ucGlobalProcCnt); ucTempCount++)
    {
        if( 0 == strcmp(psAuthToken->pcToken, psProgram->arrpsGlobalProc[ucTempCount]->pcProcName) )
        {
            bRetStatus = TRUE;
            psProcedure = psProgram->arrpsGlobalProc[ucTempCount];
        }
    }

    if(FALSE == bRetStatus)
    { 
        printf("Undeclared procedure '%s' on line %u.\n", psAuthToken->pcToken, psAuthToken->uiLineNum);
    }

    return bRetStatus;
}

/* API: Create the expression tree */
bool_t createExprTree()
{
    exprTree_t *eTree = NULL;
    if( !(eTree = (exprTree_t *) malloc( sizeof(exprTree_t))) )
    {
        printf("Failed to create expression tree.\n");
        return FALSE;
    }
    eTree->ucOperandStkTop  = 0;
    eTree->ucOperatorStkTop = 0;

    if(MAX_EXPR_TREE_ARR_LEN <= ucExpressionTreeCnt)
    {
        printf("Exceeded the max no. of nested expression trees supported.\n");
        return FALSE;
    }
    expressionTree[ucExpressionTreeCnt++] = eTree;

    return TRUE;
}

/* API: Destroy the expression tree */
bool_t destroyExprTree()
{
    if( !ucExpressionTreeCnt )
    {
        printf("No expression tree exists.\n");
        return FALSE;
    }
    free(expressionTree[--ucExpressionTreeCnt]);
    return TRUE;
}

/* API: Populate the expression tree operand */
bool_t popuExprTreeOperand( dataType_t eOperand )
{
    exprTree_t *eTree = expressionTree[ucExpressionTreeCnt-1];
    if( MAX_EXPR_OPERAND_CNT <= eTree->ucOperandStkTop )
    {
        printf("Exceeded the max no. of operands supported per expression.\n");
        return FALSE;
    }
    eTree->arreOperandStk[(eTree->ucOperandStkTop)++] = eOperand;

    /* Start evaluation */
    while(eTree->ucOperatorStkTop)
    {
        /* Check for unary operator */
        if( TRUE == eTree->arrbOperatorType[eTree->ucOperatorStkTop-1] )
        {
            if( 0 == strcmp(eTree->arrpcOperatorStk[eTree->ucOperatorStkTop-1], "-") )
            {
                if( !(eOperand & (INTEGER_TYPE+FLOAT_TYPE)) )
                {
                    printf( "Error: Unary operator '-' supports integer or float only.\n");
                    return FALSE;
                }
            }
            else if( 0 == strcmp(eTree->arrpcOperatorStk[eTree->ucOperatorStkTop-1], "not") )
            {
                if( !(eOperand & (INTEGER_TYPE+BOOL_TYPE)) )
                {
                    printf( "Error: Unary operator 'not' supports integer or boolean only.\n");
                    return FALSE;
                }
            }
            else
            {
                printf( "Error: Invalid unary operator '%s'.\n", 
                        eTree->arrpcOperatorStk[eTree->ucOperatorStkTop-1] );
                return FALSE;
            }
            (eTree->ucOperatorStkTop)--;
        }
        else /* binary operator */
        {
            if( (0 == strcmp(eTree->arrpcOperatorStk[eTree->ucOperatorStkTop-1], "*")) ||
                (0 == strcmp(eTree->arrpcOperatorStk[eTree->ucOperatorStkTop-1], "/")) ||
                (0 == strcmp(eTree->arrpcOperatorStk[eTree->ucOperatorStkTop-1], "+")) ||
                (0 == strcmp(eTree->arrpcOperatorStk[eTree->ucOperatorStkTop-1], "-"))   )
            {
                if( (!(eTree->arreOperandStk[eTree->ucOperandStkTop-1] & (INTEGER_TYPE+FLOAT_TYPE))) ||
                    (!(eTree->arreOperandStk[eTree->ucOperandStkTop-2] & (INTEGER_TYPE+FLOAT_TYPE)))   )
                {
                    printf( "Error: Arithmetic operator '%s' supports integer or float only.\n",
                                        eTree->arrpcOperatorStk[eTree->ucOperatorStkTop-1] );
                    return FALSE;
                }
                (eTree->ucOperandStkTop)--;
                eTree->arreOperandStk[eTree->ucOperandStkTop-1] = 
                        ( eTree->arreOperandStk[eTree->ucOperandStkTop-1] | 
                          eTree->arreOperandStk[eTree->ucOperandStkTop-2]  ) & 
                        (INTEGER_TYPE+FLOAT_TYPE);
                (eTree->ucOperatorStkTop)--;
            }
            else if( (0 == strcmp(eTree->arrpcOperatorStk[eTree->ucOperatorStkTop-1], "&")) ||
                     (0 == strcmp(eTree->arrpcOperatorStk[eTree->ucOperatorStkTop-1], "|"))   )
            {
                if( (!(eTree->arreOperandStk[eTree->ucOperandStkTop-1] & (INTEGER_TYPE+BOOL_TYPE))) ||
                    (!(eTree->arreOperandStk[eTree->ucOperandStkTop-2] & (INTEGER_TYPE+BOOL_TYPE)))   )
                {
                    printf( "Error: Bitwise/logical operator '%s' supports integer or float only.\n",
                                                    eTree->arrpcOperatorStk[eTree->ucOperatorStkTop-1] );
                    return FALSE;
                }
                (eTree->ucOperandStkTop)--;
                eTree->arreOperandStk[eTree->ucOperandStkTop-1] = 
                        eTree->arreOperandStk[eTree->ucOperandStkTop-1] | 
                        eTree->arreOperandStk[eTree->ucOperandStkTop-2];
                (eTree->ucOperatorStkTop)--;
            }
            else
            {
                break;
            }
        }
    }

    return TRUE;
}

/* API: Populate the expression tree operator */
bool_t popuExprTreeOperator( char *pcOperator, bool_t bIsUnaryOperator )
{
    exprTree_t *eTree = expressionTree[ucExpressionTreeCnt-1];
    if( MAX_EXPR_OPERATOR_CNT <= eTree->ucOperatorStkTop )
    {
        printf("Exceeded the max no. of operators supported per expression.\n");
        return FALSE;
    }
    eTree->arrpcOperatorStk[eTree->ucOperatorStkTop] = pcOperator;
    eTree->arrbOperatorType[eTree->ucOperatorStkTop] = bIsUnaryOperator;
    (eTree->ucOperatorStkTop)++;
    return TRUE;
}

/* API: Evaluate the expression tree */
dataType_t evalExprTree()
{
    dataType_t eRetStatus = UNDEFINED_TYPE;
    exprTree_t *eTree = expressionTree[ucExpressionTreeCnt-1];
    unsigned char ucIndex = 0;

    /* If no computation is required */
    if( !eTree->ucOperatorStkTop )
    {
        if( 1 == eTree->ucOperandStkTop )
        {
            eTree->ucOperandStkTop = 0;
            eRetStatus = eTree->arreOperandStk[0];
        }
        else
        {
            printf("5.This error should not occur.\n");
        }
    }
    else /* computation is required */
    {
        /* No. of operands should be 1 more than No. of operators */
        if( 1 == eTree->ucOperandStkTop - eTree->ucOperatorStkTop )
        {
            for( ucIndex = eTree->ucOperatorStkTop-1; 
                    ucIndex < eTree->ucOperatorStkTop; ucIndex-- )
            {
                if( (!strcmp(eTree->arrpcOperatorStk[ucIndex],  "<")) &&
                    (!strcmp(eTree->arrpcOperatorStk[ucIndex], "<=")) &&
                    (!strcmp(eTree->arrpcOperatorStk[ucIndex], "==")) &&
                    (!strcmp(eTree->arrpcOperatorStk[ucIndex], "!=")) &&
                    (!strcmp(eTree->arrpcOperatorStk[ucIndex], ">=")) &&
                    (!strcmp(eTree->arrpcOperatorStk[ucIndex],  ">"))   )
                {
                    printf("6.This error should not occur.\n");
                    break;
                }

                if( (!(eTree->arreOperandStk[ucIndex]   & (INTEGER_TYPE+FLOAT_TYPE+BOOL_TYPE))) ||
                    (!(eTree->arreOperandStk[ucIndex+1] & (INTEGER_TYPE+FLOAT_TYPE+BOOL_TYPE)))   )
                {
                    printf( "Error: Relational operator '%s' supports integer or bool only.\n",
                                                                eTree->arrpcOperatorStk[ucIndex] );
                    break;
                }
                eRetStatus = BOOL_TYPE;
                (eTree->ucOperandStkTop)--;
            }
            eTree->ucOperatorStkTop = 0;
        }
        else /* Operator to operand matching gone wrong */
        {
            printf("7.This error should not occur.\n");
        }
    }

    if( UNDEFINED_TYPE == eRetStatus )
    {
        printf("Error: Could not evaluate the expression.\n");
    }
    return eRetStatus;
}

/* API: Generate the code for procedure label */
bool_t writeProcLabel()
{
    char arrcStr[LENGTH_OF_EACH_LINE] = {0};
    unsigned char ucIndex = 1;

    if( (!ucGotoTagCount) || (TRUE != genCodeInputString(arrcGotoTag[ucGotoTagCount-1])) )
    {
        bCodeGenErr = TRUE;
        return FALSE;
    }
    /* Re-populate the procedure location for use */
    while( '_' != arrcGotoTag[ucGotoTagCount-1][ucIndex] )
    {
        arrcStr[ucIndex-1] = arrcGotoTag[ucGotoTagCount-1][ucIndex];
        ucIndex++;
    }
    arrcStr[ucIndex-1] = 0;
    psProcedure = (procedure_t *) strtol(arrcStr, NULL, 0);

    ucGotoTagCount--;

    return TRUE;
}

/* API: Generate the code for procedure return */
bool_t writeProcReturn()
{
    char arrcStr[2*LENGTH_OF_EACH_LINE] = {0};

    /* Generate the code */
    sprintf(arrcStr, "    R[0] = MM[SP+%u];\n    goto *(void *)R[0];\n\n",
                                                psProcedure->uiReturnAddrDisp);
    if( TRUE != genCodeInputString(arrcStr) )
    {
        bCodeGenErr = TRUE;
        return FALSE;
    }
    return TRUE;
}

/* API: Generate the code for procedure call */
bool_t writeProcCall( tokenListEntry_t *psToken )
{
    char arrcStr[LENGTH_OF_EACH_LINE] = {0};

    /* Generate the code */
    sprintf(arrcStr, "    MM[SP + %u] = (int)&&_return_from_%s_%p_;\n", 
            psProcedure->uiReturnAddrDisp, psProcedure->pcProcName, psToken);
    if( TRUE != genCodeInputString(arrcStr) )
    {
        bCodeGenErr = TRUE;
        return FALSE;
    }
    arrcStr[0] = 0;
    sprintf(arrcStr, "    goto _%p_%s_;\n\n", psProcedure, psProcedure->pcProcName);
    if( TRUE != genCodeInputString(arrcStr) )
    {
        bCodeGenErr = TRUE;
        return FALSE;
    }
    arrcStr[0] = 0;
    sprintf(arrcStr, "_return_from_%s_%p_ :\n", psProcedure->pcProcName, psToken);
    if( TRUE != genCodeInputString(arrcStr) )
    {
        bCodeGenErr = TRUE;
        return FALSE;
    }
    return TRUE;
}
