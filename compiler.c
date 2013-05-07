/* Include section */
#include "scanner.h"
#include "parser.h"

/* Main body of compiler */
int main( int argc, char *argv[] )
{
    /* Local variables */
    FILE *fpCode = NULL;
    tokenListEntry_t *psTokenStream = NULL;

    /* If no code or log file is supplied */
    if(argc < 2)
    {
        printf("Code file to be compiled not supplied.\n");
        return 0;
    }

    /* Check for unwanted arguements */
    if(argc > 2)
    {
        printf("Extra unwanted arguements \"%s\",...\n", argv[2]);
        return 0;
    }

    /* Get the code file details */
    if( !( fpCode = fopen(argv[1], "r") ) )
    {
        printf("\"%s\" code file not found.\n", argv[1]);
        return 0;
    }

    /* If code scan fails */
    if( !(psTokenStream = scan(fpCode)) )
    {
        printf("Scan failure. Check error report.\n");
        return 0;
    }

    /* Close the code file */
    fclose( fpCode );

    /* Print the tokens generated */
    if(DEBUG_FLAG)
    {
        tokenListEntry_t *psTempStream = psTokenStream;
        while(psTempStream)
        {
            printf("Line %u: \"%s\"\n", psTempStream->uiLineNum, psTempStream->pcToken);
            psTempStream = psTempStream->psNextToken;
        }
    }

    /* Parse the code */
    if( FALSE == parse(psTokenStream) )
    {
        printf("Parse or type check failure. Check error report.\n");
        return 0;
    }

    return 0;
}

