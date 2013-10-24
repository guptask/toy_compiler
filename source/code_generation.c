/* Include section */
#include "code_generation.h"


/* Macro section */
#define NO_OF_HEADER_LINES        17
#define NO_OF_TERMINATION_LINES   2

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
    "#include <stdio.h>\n",
    "#include \"run_time.h\"\n\n",

    "#define NUM_OF_REGISTERS 0xFF\n",
    "#define MM_SIZE          0xFFFF\n",
    "#define CHAR_BUFFER_SIZE 0xFFF\n",
    "#define true             1\n",
    "#define false            0\n\n",

    "int   R[NUM_OF_REGISTERS]           = {0};\n",
    "int   MM[MM_SIZE]                   = {0};\n",
    "char  CHAR_BUFFER[CHAR_BUFFER_SIZE] = {0};\n",
    "float FLOAT_VAR                     = 0.0;\n\n",

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


/* Definition section */

/* API: Write the header */
bool_t genCodeHeader( FILE *fpCode )
{
    unsigned int uiIndex = 0;
    if( !fpCode )
    {
        return FALSE;
    }
    for(uiIndex = 0; uiIndex < NO_OF_HEADER_LINES; uiIndex++)
    {
        if( EOF == fputs(arrcHeader[uiIndex], fpCode) )
        {
            return FALSE;
        }
    }
    return TRUE;
}

/* API: Write the termination */
bool_t genCodeTermination( FILE *fpCode )
{
    unsigned int uiIndex = 0;
    if( !fpCode )
    {
        return FALSE;
    }
    for(uiIndex = 0; uiIndex < NO_OF_TERMINATION_LINES; uiIndex++)
    {
        if( EOF == fputs(arrcTermination[uiIndex], fpCode) )
        {
            return FALSE;
        }
    }
    return TRUE;
}

/* API: Write the string sent as input */
bool_t genCodeInputString( char *arrcStr, FILE *fpCode )
{
    if( !fpCode )
    {
        return FALSE;
    }
    if( EOF == fputs(arrcStr, fpCode) )
    {
        return FALSE;
    }
    return TRUE;
}

