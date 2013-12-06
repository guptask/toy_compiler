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
static unsigned char ucSPDisplacement = 0;
static bool_t        bIsSPChangeNeeded = FALSE;

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
    psVar->ucCallStkDisp     = 0;
    psProgram->arrpsGlobalProc[(psProgram->ucGlobalProcCnt)++] = psProc;

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
    psVar->ucCallStkDisp     = 0;
    psProgram->arrpsGlobalProc[(psProgram->ucGlobalProcCnt)++] = psProc;

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
    psVar->ucCallStkDisp     = 0;
    psProgram->arrpsGlobalProc[(psProgram->ucGlobalProcCnt)++] = psProc;

    /* getString(string a out, integer b out) */
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
    psVar->bIsOutParam       = TRUE;
    psVar->ucCallStkDisp     = 0;

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
    psVar->ucCallStkDisp     = 1;

    psProgram->arrpsGlobalProc[(psProgram->ucGlobalProcCnt)++] = psProc;

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
    psVar->ucCallStkDisp     = 0;
    psProgram->arrpsGlobalProc[(psProgram->ucGlobalProcCnt)++] = psProc;

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
    psVar->ucCallStkDisp     = 0;
    psProgram->arrpsGlobalProc[(psProgram->ucGlobalProcCnt)++] = psProc;

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
    psVar->ucCallStkDisp     = 0;
    psProgram->arrpsGlobalProc[(psProgram->ucGlobalProcCnt)++] = psProc;

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
    psVar->ucCallStkDisp     = 0;
    psProgram->arrpsGlobalProc[(psProgram->ucGlobalProcCnt)++] = psProc;

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
        psTemp->ucCallStkDisp = 0;

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
            psTemp->ucCallStkDisp = psProgram->ucGlobalVarCnt + psProgram->ucLocalVarCnt - 1;
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
            psTemp->ucCallStkDisp = psNode->ucVariableCnt - 1;
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
                ucSPDisplacement = psProgram->ucLocalVarCnt + psProgram->ucGlobalVarCnt + 1;
                bIsSPChangeNeeded = TRUE;
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
                ucSPDisplacement = psProc->ucVariableCnt + 1;
                bIsSPChangeNeeded = TRUE;
                break;
            }
        }

        /* Check for recursion */
        if( (TRUE != bRetStatus) && (0 == strcmp(psAuthToken->pcToken, psProc->pcProcName)) )
        {
            bRetStatus = TRUE;
            psProcedure = psProc;
            ucSPDisplacement = psProc->ucVariableCnt + 1;
            bIsSPChangeNeeded = TRUE;
        }
    }

    /* Check for global procedure */
    for(ucTempCount = 0; (FALSE == bRetStatus) && (ucTempCount < psProgram->ucGlobalProcCnt); ucTempCount++)
    {
        if( 0 == strcmp(psAuthToken->pcToken, psProgram->arrpsGlobalProc[ucTempCount]->pcProcName) )
        {
            bRetStatus = TRUE;
            psProcedure = psProgram->arrpsGlobalProc[ucTempCount];
            ucSPDisplacement = psProgram->ucLocalVarCnt + psProgram->ucGlobalVarCnt + 1;
            bIsSPChangeNeeded = TRUE;
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
    char arrcStr[LENGTH_OF_EACH_LINE] = {0};
    bool_t bIsFirstArgFloat = FALSE, bIsSecondArgFloat = FALSE;
    exprTree_t *eTree = expressionTree[ucExpressionTreeCnt-1];
    unsigned char ucIndex = 0;

    if( MAX_EXPR_OPERAND_CNT <= eTree->ucOperandStkTop )
    {
        printf("Exceeded the max no. of operands supported per expression.\n");
        return FALSE;
    }
    eTree->arreOperandStk[eTree->ucOperandStkTop] = eOperand;
    eTree->arruiOperandRegCnt[eTree->ucOperandStkTop] = uiRegCount;
    (eTree->ucOperandStkTop)++;

    if(EXPR_DEBUG_FLAG)
    {
        printf("Before prelim computation\nOperator stack:\n");
        for(ucIndex = 0; ucIndex < eTree->ucOperatorStkTop; ucIndex++)
        {
            printf("%s ,", eTree->arrpcOperatorStk[ucIndex]);
        }
        printf("\nOperand stack:\n");
        for(ucIndex = 0; ucIndex < eTree->ucOperandStkTop; ucIndex++)
        {
            printf("%d ,", eTree->arreOperandStk[ucIndex]);
        }
        printf("\n");
    }

    /* Start evaluation */
    /* Check for unary operator */
    while( (eTree->ucOperatorStkTop) && (TRUE == eTree->arrbOperatorType[eTree->ucOperatorStkTop-1]) )
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

            sprintf(arrcStr, "    R[%u] = !R[%u];\n", 
                    eTree->arruiOperandRegCnt[eTree->ucOperandStkTop-1], 
                    eTree->arruiOperandRegCnt[eTree->ucOperandStkTop-1]);
            if( TRUE != genCodeInputString(arrcStr) )
            {
                bCodeGenErr = TRUE;
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

    /* binary operators '*' and '/' */
    while( (eTree->ucOperatorStkTop) && 
           ( (0 == strcmp(eTree->arrpcOperatorStk[eTree->ucOperatorStkTop-1], "*")) || 
             (0 == strcmp(eTree->arrpcOperatorStk[eTree->ucOperatorStkTop-1], "/"))
           )
         )
    {
        if( (!(eTree->arreOperandStk[eTree->ucOperandStkTop-1] & (INTEGER_TYPE+FLOAT_TYPE))) ||
            (!(eTree->arreOperandStk[eTree->ucOperandStkTop-2] & (INTEGER_TYPE+FLOAT_TYPE)))   )
        {
            printf( "Error: Arithmetic operator '%s' supports integer or float only.\n",
                                        eTree->arrpcOperatorStk[eTree->ucOperatorStkTop-1] );
            return FALSE;
        }

        /* Generate the code */
        if( (eTree->arreOperandStk[eTree->ucOperandStkTop-2]) & FLOAT_TYPE )
        {
            bIsFirstArgFloat = TRUE;
            sprintf(arrcStr, "    memcpy( &FLOAT_VAR, &R[%u], sizeof(float) );\n", 
                    eTree->arruiOperandRegCnt[eTree->ucOperandStkTop-2]);
            if( TRUE != genCodeInputString(arrcStr) )
            {
                bCodeGenErr = TRUE;
                return FALSE;
            }
        }
        if( (eTree->arreOperandStk[eTree->ucOperandStkTop-1]) & FLOAT_TYPE )
        {
            bIsSecondArgFloat = TRUE;
            sprintf(arrcStr, "    memcpy( &FLOAT_VAR1, &R[%u], sizeof(float) );\n", 
                    eTree->arruiOperandRegCnt[eTree->ucOperandStkTop-1]);
            if( TRUE != genCodeInputString(arrcStr) )
            {
                bCodeGenErr = TRUE;
                return FALSE;
            }
        }
        if( (TRUE == bIsFirstArgFloat) && (FALSE == bIsSecondArgFloat) )
        {
            sprintf(arrcStr, "    FLOAT_VAR = FLOAT_VAR %s R[%u];\n", 
                    eTree->arrpcOperatorStk[eTree->ucOperatorStkTop-1],
                    eTree->arruiOperandRegCnt[eTree->ucOperandStkTop-1]);
        }
        else if( (FALSE == bIsFirstArgFloat) && (TRUE == bIsSecondArgFloat) )
        {
            sprintf(arrcStr, "    FLOAT_VAR = R[%u] %s FLOAT_VAR1;\n", 
                    eTree->arruiOperandRegCnt[eTree->ucOperandStkTop-2], 
                    eTree->arrpcOperatorStk[eTree->ucOperatorStkTop-1]);
        }
        else if( (TRUE == bIsFirstArgFloat) && (TRUE == bIsSecondArgFloat) )
        {
            sprintf(arrcStr, "    FLOAT_VAR = FLOAT_VAR %s FLOAT_VAR1;\n", 
                    eTree->arrpcOperatorStk[eTree->ucOperatorStkTop-1]);
        }
        else
        {
            sprintf(arrcStr, "    R[%u] = R[%u] %s R[%u];\n", 
                    eTree->arruiOperandRegCnt[eTree->ucOperandStkTop-2], 
                    eTree->arruiOperandRegCnt[eTree->ucOperandStkTop-2],
                    eTree->arrpcOperatorStk[eTree->ucOperatorStkTop-1],
                    eTree->arruiOperandRegCnt[eTree->ucOperandStkTop-1]);
        }
        if( TRUE != genCodeInputString(arrcStr) )
        {
            bCodeGenErr = TRUE;
            return FALSE;
        }

        if( (TRUE == bIsFirstArgFloat) || (TRUE == bIsSecondArgFloat) )
        {
            sprintf(arrcStr, "    memcpy( &R[%u], &FLOAT_VAR, sizeof(float) );\n", 
                    eTree->arruiOperandRegCnt[eTree->ucOperandStkTop-2]);
            if( TRUE != genCodeInputString(arrcStr) )
            {
                bCodeGenErr = TRUE;
                return FALSE;
            }
            bIsFirstArgFloat  = FALSE;
            bIsSecondArgFloat = FALSE;
        }

        eTree->arreOperandStk[eTree->ucOperandStkTop-2] = 
                        ( eTree->arreOperandStk[eTree->ucOperandStkTop-1] | 
                          eTree->arreOperandStk[eTree->ucOperandStkTop-2]  ) & 
                        (INTEGER_TYPE+FLOAT_TYPE);
        (eTree->ucOperandStkTop)--;
        (eTree->ucOperatorStkTop)--;
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
dataType_t evalExprTree( unsigned int *puiRegCount )
{
    dataType_t eRetStatus = UNDEFINED_TYPE;
    exprTree_t *eTree = expressionTree[ucExpressionTreeCnt-1];
    unsigned char ucIndex = 0, ucTempIndex = 0;
    char arrcStr[LENGTH_OF_EACH_LINE] = {0};
    bool_t bIsFirstArgFloat = FALSE, bIsSecondArgFloat = FALSE;

    /* If no computation is required */
    if( !eTree->ucOperatorStkTop )
    {
        if( 1 == eTree->ucOperandStkTop )
        {
            eTree->ucOperandStkTop = 0;
            eRetStatus = eTree->arreOperandStk[0];
            *puiRegCount = eTree->arruiOperandRegCnt[0];
        }
        else
        {
            printf("5.This error should not occur.\n");
        }
        return eRetStatus;
    }

    /* No. of operands should be 1 more than No. of operators */
    if( 1 != eTree->ucOperandStkTop - eTree->ucOperatorStkTop )
    {
        printf("6.This error should not occur.\n");
        return UNDEFINED_TYPE;
    }

    if(EXPR_DEBUG_FLAG)
    {
        printf("Before evaluation\nOperator stack:\n");
        for(ucIndex = 0; ucIndex < eTree->ucOperatorStkTop; ucIndex++)
        {
            printf("%s ,", eTree->arrpcOperatorStk[ucIndex]);
        }
        printf("\nOperand stack:\n");
        for(ucIndex = 0; ucIndex < eTree->ucOperandStkTop; ucIndex++)
        {
            printf("%d ,", eTree->arreOperandStk[ucIndex]);
        }
        printf("\n");
    }

    for( ucIndex = 0; ucIndex < eTree->ucOperatorStkTop; )
    {
        /* binary operators '+' and '-' */
        if( (0 == strcmp(eTree->arrpcOperatorStk[ucIndex], "+")) || 
            (0 == strcmp(eTree->arrpcOperatorStk[ucIndex], "-"))    )
        {
            if( (!(eTree->arreOperandStk[ucIndex]   & (INTEGER_TYPE+FLOAT_TYPE))) ||
                (!(eTree->arreOperandStk[ucIndex+1] & (INTEGER_TYPE+FLOAT_TYPE)))   )
            {
                printf( "Error: Arithmetic operator '%s' supports integer or float only.\n",
                                        eTree->arrpcOperatorStk[ucIndex] );
                return UNDEFINED_TYPE;
            }

            /* Generate the code */
            if( (eTree->arreOperandStk[ucIndex]) & FLOAT_TYPE )
            {
                bIsFirstArgFloat = TRUE;
                sprintf(arrcStr, "    memcpy( &FLOAT_VAR, &R[%u], sizeof(float) );\n", 
                                eTree->arruiOperandRegCnt[ucIndex]);
                if( TRUE != genCodeInputString(arrcStr) )
                {
                    bCodeGenErr = TRUE;
                    return FALSE;
                }
            }
            if( (eTree->arreOperandStk[ucIndex+1]) & FLOAT_TYPE )
            {
                bIsSecondArgFloat = TRUE;
                sprintf(arrcStr, "    memcpy( &FLOAT_VAR1, &R[%u], sizeof(float) );\n", 
                                eTree->arruiOperandRegCnt[ucIndex+1]);
                if( TRUE != genCodeInputString(arrcStr) )
                {
                    bCodeGenErr = TRUE;
                    return FALSE;
                }
            }
            if( (TRUE == bIsFirstArgFloat) && (FALSE == bIsSecondArgFloat) )
            {
                sprintf(arrcStr, "    FLOAT_VAR = FLOAT_VAR %s R[%u];\n", 
                                        eTree->arrpcOperatorStk[ucIndex],
                                        eTree->arruiOperandRegCnt[ucIndex+1]);
            }
            else if( (FALSE == bIsFirstArgFloat) && (TRUE == bIsSecondArgFloat) )
            {
                sprintf(arrcStr, "    FLOAT_VAR = R[%u] %s FLOAT_VAR1;\n", 
                                        eTree->arruiOperandRegCnt[ucIndex], 
                                        eTree->arrpcOperatorStk[ucIndex]);
            }
            else if( (TRUE == bIsFirstArgFloat) && (TRUE == bIsSecondArgFloat) )
            {
                sprintf(arrcStr, "    FLOAT_VAR = FLOAT_VAR %s FLOAT_VAR1;\n", 
                                        eTree->arrpcOperatorStk[ucIndex]);
            }
            else
            {
                sprintf(arrcStr, "    R[%u] = R[%u] %s R[%u];\n", 
                                        eTree->arruiOperandRegCnt[ucIndex], 
                                        eTree->arruiOperandRegCnt[ucIndex],
                                        eTree->arrpcOperatorStk[ucIndex],
                                        eTree->arruiOperandRegCnt[ucIndex+1]);
            }
            if( TRUE != genCodeInputString(arrcStr) )
            {
                bCodeGenErr = TRUE;
                return FALSE;
            }

            if( (TRUE == bIsFirstArgFloat) || (TRUE == bIsSecondArgFloat) )
            {
                sprintf(arrcStr, "    memcpy( &R[%u], &FLOAT_VAR, sizeof(float) );\n", 
                                        eTree->arruiOperandRegCnt[ucIndex]);
                if( TRUE != genCodeInputString(arrcStr) )
                {
                    bCodeGenErr = TRUE;
                    return FALSE;
                }
                bIsFirstArgFloat  = FALSE;
                bIsSecondArgFloat = FALSE;
            }

            eTree->arreOperandStk[ucIndex] = 
                        ( eTree->arreOperandStk[ucIndex] | eTree->arreOperandStk[ucIndex+1]  ) & 
                        (INTEGER_TYPE+FLOAT_TYPE);
            for(ucTempIndex = ucIndex+2; ucTempIndex < eTree->ucOperandStkTop; ucTempIndex++)
            {
                eTree->arreOperandStk[ucTempIndex-1] = eTree->arreOperandStk[ucTempIndex];
                eTree->arruiOperandRegCnt[ucTempIndex-1] = eTree->arruiOperandRegCnt[ucTempIndex];
            }
            (eTree->ucOperandStkTop)--;
            for(ucTempIndex = ucIndex+1; ucTempIndex < eTree->ucOperatorStkTop; ucTempIndex++)
            {
                eTree->arrpcOperatorStk[ucTempIndex-1] = eTree->arrpcOperatorStk[ucTempIndex];
                eTree->arrbOperatorType[ucTempIndex-1] = eTree->arrbOperatorType[ucTempIndex];
            }
            (eTree->ucOperatorStkTop)--;
        }
        else
        {
            ucIndex++;
        }
    }

    for( ucIndex = 0; ucIndex < eTree->ucOperatorStkTop; )
    {
        /* relational operators '<',..,'>' */
        if( (0 == strcmp(eTree->arrpcOperatorStk[ucIndex],  "<")) || 
            (0 == strcmp(eTree->arrpcOperatorStk[ucIndex], "<=")) ||   
            (0 == strcmp(eTree->arrpcOperatorStk[ucIndex], "==")) ||   
            (0 == strcmp(eTree->arrpcOperatorStk[ucIndex], "!=")) ||   
            (0 == strcmp(eTree->arrpcOperatorStk[ucIndex], ">=")) ||   
            (0 == strcmp(eTree->arrpcOperatorStk[ucIndex],  ">"))   )
        {
            if( (!(eTree->arreOperandStk[ucIndex]   & (INTEGER_TYPE+BOOL_TYPE))) ||
                (!(eTree->arreOperandStk[ucIndex+1] & (INTEGER_TYPE+BOOL_TYPE)))   )
            {
                printf( "Error: Relational operator '%s' supports integer or boolean only.\n",
                                        eTree->arrpcOperatorStk[ucIndex] );
                return UNDEFINED_TYPE;
            }

            /* Generate the code */
            sprintf(arrcStr, "    R[%u] = R[%u] %s R[%u];\n", 
                                    eTree->arruiOperandRegCnt[ucIndex], 
                                    eTree->arruiOperandRegCnt[ucIndex],
                                    eTree->arrpcOperatorStk[ucIndex],
                                    eTree->arruiOperandRegCnt[ucIndex+1]);
            if( TRUE != genCodeInputString(arrcStr) )
            {
                bCodeGenErr = TRUE;
                return FALSE;
            }

            eTree->arreOperandStk[ucIndex] = BOOL_TYPE;
            for(ucTempIndex = ucIndex+2; ucTempIndex < eTree->ucOperandStkTop; ucTempIndex++)
            {
                eTree->arreOperandStk[ucTempIndex-1] = eTree->arreOperandStk[ucTempIndex];
                eTree->arruiOperandRegCnt[ucTempIndex-1] = eTree->arruiOperandRegCnt[ucTempIndex];
            }
            (eTree->ucOperandStkTop)--;
            for(ucTempIndex = ucIndex+1; ucTempIndex < eTree->ucOperatorStkTop; ucTempIndex++)
            {
                eTree->arrpcOperatorStk[ucTempIndex-1] = eTree->arrpcOperatorStk[ucTempIndex];
                eTree->arrbOperatorType[ucTempIndex-1] = eTree->arrbOperatorType[ucTempIndex];
            }
            (eTree->ucOperatorStkTop)--;
        }
        else
        {
            ucIndex++;
        }
    }

    for( ucIndex = 0; ucIndex < eTree->ucOperatorStkTop; )
    {
        /* boolean operators '&' and '|' */
        if( (0 == strcmp(eTree->arrpcOperatorStk[ucIndex], "&")) || 
            (0 == strcmp(eTree->arrpcOperatorStk[ucIndex], "|"))   )
        {
            if( (!(eTree->arreOperandStk[ucIndex]   & (INTEGER_TYPE+BOOL_TYPE))) ||
                (!(eTree->arreOperandStk[ucIndex+1] & (INTEGER_TYPE+BOOL_TYPE)))   )
            {
                printf( "Error: Boolean operator '%s' supports integer or boolean only.\n",
                                        eTree->arrpcOperatorStk[ucIndex] );
                return UNDEFINED_TYPE;
            }

            /* Generate the code */
            sprintf(arrcStr, "    R[%u] = R[%u] %s R[%u];\n", 
                                    eTree->arruiOperandRegCnt[ucIndex], 
                                    eTree->arruiOperandRegCnt[ucIndex],
                                    eTree->arrpcOperatorStk[ucIndex],
                                    eTree->arruiOperandRegCnt[ucIndex+1]);
            if( TRUE != genCodeInputString(arrcStr) )
            {
                bCodeGenErr = TRUE;
                return FALSE;
            }

            eTree->arreOperandStk[ucIndex] = 
                        ( eTree->arreOperandStk[ucIndex] | eTree->arreOperandStk[ucIndex+1]  ) & 
                        (INTEGER_TYPE+BOOL_TYPE);
            for(ucTempIndex = ucIndex+2; ucTempIndex < eTree->ucOperandStkTop; ucTempIndex++)
            {
                eTree->arreOperandStk[ucTempIndex-1] = eTree->arreOperandStk[ucTempIndex];
                eTree->arruiOperandRegCnt[ucTempIndex-1] = eTree->arruiOperandRegCnt[ucTempIndex];
            }
            (eTree->ucOperandStkTop)--;
            for(ucTempIndex = ucIndex+1; ucTempIndex < eTree->ucOperatorStkTop; ucTempIndex++)
            {
                eTree->arrpcOperatorStk[ucTempIndex-1] = eTree->arrpcOperatorStk[ucTempIndex];
                eTree->arrbOperatorType[ucTempIndex-1] = eTree->arrbOperatorType[ucTempIndex];
            }
            (eTree->ucOperatorStkTop)--;
        }
        else
        {
            ucIndex++;
        }
    }

    if(EXPR_DEBUG_FLAG)
    {
        printf("After evaluation\nOperator stack:\n");
        for(ucIndex = 0; ucIndex < eTree->ucOperatorStkTop; ucIndex++)
        {
            printf("%s ,", eTree->arrpcOperatorStk[ucIndex]);
        }
        printf("\nOperand stack:\n");
        for(ucIndex = 0; ucIndex < eTree->ucOperandStkTop; ucIndex++)
        {
            printf("%d ,", eTree->arreOperandStk[ucIndex]);
        }
        printf("\n");
    }

    if( (eTree->ucOperatorStkTop) || (eTree->ucOperandStkTop != 1) )
    {
        printf("Error: Could not evaluate the expression.\n");
        return UNDEFINED_TYPE;
    }

    eRetStatus = eTree->arreOperandStk[0];
    *puiRegCount = eTree->arruiOperandRegCnt[0];
    eTree->ucOperatorStkTop = 0;

    if( UNDEFINED_TYPE == eRetStatus )
    {
        printf("Error: Incorrect evaluation of the expression.\n");
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
    sprintf(arrcStr, "    R[0] = MM[SP+%d];\n    goto *(void *)R[0];\n\n",
                                            (int)psProcedure->ucVariableCnt);
    if( TRUE != genCodeInputString(arrcStr) )
    {
        bCodeGenErr = TRUE;
        return FALSE;
    }
    return TRUE;
}

/* API: Generate the code for procedure argument */
bool_t writeProcArgs( unsigned char ucArgSPDisp )
{
    char arrcStr[LENGTH_OF_EACH_LINE] = {0};

    /* Generate the code */
    if( TRUE == bIsSPChangeNeeded )
    {
        sprintf(arrcStr, "    SP = SP + %d;\n", (int)ucSPDisplacement);
        if( TRUE != genCodeInputString(arrcStr) )
        {
            bCodeGenErr = TRUE;
            return FALSE;
        }
        bIsSPChangeNeeded = FALSE;
    }

    /* If argument is an 'in' parameter */
    if( TRUE != psProcedure->arrpsVariable[ucArgSPDisp]->bIsOutParam )
    {
        sprintf(arrcStr, "    MM[SP + %d] = R[%u];\n", (int)ucArgSPDisp, uiRegCount);
        if( TRUE != genCodeInputString(arrcStr) )
        {
            bCodeGenErr = TRUE;
            return FALSE;
        }
    }
    else /* when argument is an out paramter */
    {
    }

    return TRUE;
}

/* API: Generate the code for procedure call */
bool_t writeProcCall( tokenListEntry_t *psToken )
{
    char arrcStr[LENGTH_OF_EACH_LINE] = {0};

    /* Generate the code */
    sprintf(arrcStr, "    MM[SP + %d] = (int)&&_return_from_%s_%p_;\n", 
          (int)psProcedure->ucVariableCnt, psProcedure->pcProcName, psToken);
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
    arrcStr[0] = 0;
    sprintf(arrcStr, "    SP = SP - %d;\n", (int)ucSPDisplacement);
    if( TRUE != genCodeInputString(arrcStr) )
    {
        bCodeGenErr = TRUE;
        return FALSE;
    }
    ucSPDisplacement = 0;

    return TRUE;
}

