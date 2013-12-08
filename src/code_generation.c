/* Include section */
#include "code_generation.h"


/* Macro section */
#define NO_OF_HEADER_LINES        17
#define NO_OF_TERMINATION_LINES   2
#define NO_OF_GET_BOOL_LINES      4
#define NO_OF_GET_INTEGER_LINES   4
#define NO_OF_GET_FLOAT_LINES     5
#define NO_OF_GET_STRING_LINES    7
#define NO_OF_PUT_BOOL_LINES      4
#define NO_OF_PUT_INTEGER_LINES   4
#define NO_OF_PUT_FLOAT_LINES     4
#define NO_OF_PUT_STRING_LINES    4


/** Global variables **/

/* Generated C code file parameters */
char *pcGenFileName = NULL;
FILE *fpGenCode     = NULL;
bool_t bCodeGenErr  = FALSE;

/* Code parameters */
unsigned int uiRegCount = 0;
unsigned int uiStackPtr = 0;
unsigned int uiHeapPtr  = 0;
unsigned int uiFramePtr = 0;


/** Static variables **/

/* Header details */
static char arrcHeader[NO_OF_HEADER_LINES][LENGTH_OF_EACH_LINE] = {
    "#include \"run_time.h\"\n\n",

    "#define NUM_OF_REGISTERS 0xFF\n",
    "#define MM_SIZE          0xFFFF\n",
    "#define CHAR_BUFFER_SIZE 0xFFF\n",
    "#define true             1\n",
    "#define false            0\n\n",

    "int   R[NUM_OF_REGISTERS]           = {0};\n",
    "int   MM[MM_SIZE]                   = {0};\n",
    "char  CHAR_BUFFER[CHAR_BUFFER_SIZE] = {0};\n",
    "float FLOAT_VAR                     = 0.0;\n",
    "float FLOAT_VAR1                    = 0.0;\n\n",

    "/* Stack pointer */\n",
    "int SP = 0;\n",
    "/* Heap pointer */\n",
    "int HP = MM_SIZE-1;\n\n",

    "int main() {\n",
    "    goto _program_body_;\n\n"
};

/* Termination details */
static char arrcTermination[NO_OF_TERMINATION_LINES][LENGTH_OF_EACH_LINE] = {
    "\n    return 0;\n",
    "}\n"
};

/* getBool() definition */
static char arrcGetBool[NO_OF_GET_BOOL_LINES][LENGTH_OF_EACH_LINE] = {
    "    R[0] = getBool();\n",
    "    MM[SP+0] = R[0];\n",
    "    R[0] = MM[SP+1];\n",
    "    goto *(void *)R[0];\n\n"
};

/* putBool() definition */
static char arrcPutBool[NO_OF_PUT_BOOL_LINES][LENGTH_OF_EACH_LINE] = {
    "    R[0] = MM[SP+0];\n",
    "    putBool(R[0]);\n",
    "    R[0] = MM[SP+1];\n",
    "    goto *(void *)R[0];\n\n"
};

/* getInteger() definition */
static char arrcGetInteger[NO_OF_GET_INTEGER_LINES][LENGTH_OF_EACH_LINE] = {
    "    R[0] = getInteger();\n",
    "    MM[SP+0] = R[0];\n",
    "    R[0] = MM[SP+1];\n",
    "    goto *(void *)R[0];\n\n"
};

/* putInteger() definition */
static char arrcPutInteger[NO_OF_PUT_INTEGER_LINES][LENGTH_OF_EACH_LINE] = {
    "    R[0] = MM[SP+0];\n",
    "    putInteger(R[0]);\n",
    "    R[0] = MM[SP+1];\n",
    "    goto *(void *)R[0];\n\n"
};

/* getFloat() definition */
static char arrcGetFloat[NO_OF_GET_FLOAT_LINES][LENGTH_OF_EACH_LINE] = {
    "    FLOAT_VAR = getFloat();\n",
    "    memcpy( &R[0], &FLOAT_VAR, sizeof(float) );\n",
    "    MM[SP+0] = R[0];\n",
    "    R[0] = MM[SP+1];\n",
    "    goto *(void *)R[0];\n\n"
};

/* putFloat() definition */
static char arrcPutFloat[NO_OF_PUT_FLOAT_LINES][LENGTH_OF_EACH_LINE] = {
    "    memcpy( &FLOAT_VAR, &MM[SP+0], sizeof(float) );\n",
    "    putFloat(FLOAT_VAR);\n",
    "    R[0] = MM[SP+1];\n",
    "    goto *(void *)R[0];\n\n"
};

/* getString() definition */
static char arrcGetString[NO_OF_GET_STRING_LINES][LENGTH_OF_EACH_LINE] = {
    "    R[0] = getString(CHAR_BUFFER);\n",
    "    HP = HP - R[0];\n",
    "    memcpy( &MM[HP], &CHAR_BUFFER, R[0]*sizeof(char) );\n",
    "    MM[SP+0] = (int)((char *)&MM[HP]);\n",
    "    MM[SP+1] = R[0];\n",
    "    R[0] = MM[SP+2];\n",
    "    goto *(void *)R[0];\n\n"
};

/* putString() definition */
static char arrcPutString[NO_OF_PUT_STRING_LINES][LENGTH_OF_EACH_LINE] = {
    "    R[0] = MM[SP+0];\n",
    "    putString( (char *)R[0]);\n",
    "    R[0] = MM[SP+1];\n",
    "    goto *(void *)R[0];\n\n"
};


/* Definition section */

/* API: Write the header */
bool_t genCodeHeader()
{
    unsigned int uiIndex = 0;
    if( !fpGenCode )
    {
        return FALSE;
    }
    for(uiIndex = 0; uiIndex < NO_OF_HEADER_LINES; uiIndex++)
    {
        if( EOF == fputs(arrcHeader[uiIndex], fpGenCode) )
        {
            return FALSE;
        }
    }
    return TRUE;
}

/* API: Write the termination */
bool_t genCodeTermination()
{
    unsigned int uiIndex = 0;
    if( !fpGenCode )
    {
        return FALSE;
    }
    for(uiIndex = 0; uiIndex < NO_OF_TERMINATION_LINES; uiIndex++)
    {
        if( EOF == fputs(arrcTermination[uiIndex], fpGenCode) )
        {
            return FALSE;
        }
    }
    return TRUE;
}

/* API: Write the string sent as input */
bool_t genCodeInputString( char *pcStr )
{
    if( !fpGenCode )
    {
        return FALSE;
    }
    if( EOF == fputs(pcStr, fpGenCode) )
    {
        return FALSE;
    }
    return TRUE;
}

/* API: Add the runtime definitions */
bool_t genCodeAddRuntimeDef( char *pcProcName )
{
    unsigned int uiIndex = 0;
    if( (!fpGenCode) || (!pcProcName) )
    {
        return FALSE;
    }

    if( 0 == strcmp(pcProcName, "getInteger") )
    {
        for(uiIndex = 0; uiIndex < NO_OF_GET_INTEGER_LINES; uiIndex++)
        {
            if( EOF == fputs(arrcGetInteger[uiIndex], fpGenCode) )
            {
                return FALSE;
            }
        }
    }
    else if( 0 == strcmp(pcProcName, "putInteger") )
    {
        for(uiIndex = 0; uiIndex < NO_OF_PUT_INTEGER_LINES; uiIndex++)
        {
            if( EOF == fputs(arrcPutInteger[uiIndex], fpGenCode) )
            {
                return FALSE;
            }
        }
    } 
    else if( 0 == strcmp(pcProcName, "getBool") )
    {
        for(uiIndex = 0; uiIndex < NO_OF_GET_BOOL_LINES; uiIndex++)
        {
            if( EOF == fputs(arrcGetBool[uiIndex], fpGenCode) )
            {
                return FALSE;
            }
        }
    }
    else if( 0 == strcmp(pcProcName, "putBool") )
    {
        for(uiIndex = 0; uiIndex < NO_OF_PUT_BOOL_LINES; uiIndex++)
        {
            if( EOF == fputs(arrcPutBool[uiIndex], fpGenCode) )
            {
                return FALSE;
            }
        }
    }
    else if( 0 == strcmp(pcProcName, "getFloat") )
    {
        for(uiIndex = 0; uiIndex < NO_OF_GET_FLOAT_LINES; uiIndex++)
        {
            if( EOF == fputs(arrcGetFloat[uiIndex], fpGenCode) )
            {
                return FALSE;
            }
        }
    }
    else if( 0 == strcmp(pcProcName, "putFloat") )
    {
        for(uiIndex = 0; uiIndex < NO_OF_PUT_FLOAT_LINES; uiIndex++)
        {
            if( EOF == fputs(arrcPutFloat[uiIndex], fpGenCode) )
            {
                return FALSE;
            }
        }
    }
    else if( 0 == strcmp(pcProcName, "getString") )
    {
        for(uiIndex = 0; uiIndex < NO_OF_GET_STRING_LINES; uiIndex++)
        {
            if( EOF == fputs(arrcGetString[uiIndex], fpGenCode) )
            {
                return FALSE;
            }
        }
    }
    else if( 0 == strcmp(pcProcName, "putString") )
    {
        for(uiIndex = 0; uiIndex < NO_OF_PUT_STRING_LINES; uiIndex++)
        {
            if( EOF == fputs(arrcPutString[uiIndex], fpGenCode) )
            {
                return FALSE;
            }
        }
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}


