/* Include section */
#include "scanner.h"
#include "parser.h"

/* Main body of compiler */
int main( int argc, char *argv[] )
{
    /* Local variables */
    FILE *fpCode = NULL, *fpGenCode = NULL;
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
        printf("Scan failure. Check errors reported.\n");
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

    /* Create the generated C code file */
    char *pcGenFileName = strtok(argv[1], ".");
    if( !strcat(pcGenFileName, ".c") )
    {
        printf("Generated file name error.\n");
    }
    if( !( fpGenCode = fopen(pcGenFileName, "w") ) )
    {
        printf("Generated code file could not be created.\n");
        return 0;
    }

    /* Parse, type check the code and generate the C code */
    if( FALSE == parse(psTokenStream, fpGenCode) )
    {
        printf("Parse or type check failure. Check errors reported.\n");
        fclose(fpGenCode);
        if( remove(pcGenFileName) )
        {
            printf("Could not remove invalid file '%s'.\n", pcGenFileName);
        }
        return 0;
    }
    fclose(fpGenCode);
    printf("Generated code file is '%s'.\n", pcGenFileName);

    return 0;
}

