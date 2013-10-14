/*-------------------------*
 |     RUNTIME LIBRARY     |
 *-------------------------*/

#ifndef RUN_TIME_H_
#define RUN_TIME_H_


/* Header section */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* Procedure */

/* Check boolean operation at runtime */
void boolOpRuntimeChk( int iLeft, int iRight, unsigned int uiLineNum )
{
    if( (0 != iLeft) && (1 != iLeft) )
    {
        printf("Error: Line %u: Illegal boolean value %d.\n", uiLineNum, iLeft);
        exit(EXIT_FAILURE);
    }
    if( (0 != iRight) && (1 != iRight) )
    {
        printf("Error: Line %u: Illegal boolean value %d.\n", uiLineNum, iRight);
        exit(EXIT_FAILURE);
    }
};

/* Get Integer */
int getInteger()
{
    int iValue = 0;
    scanf("%d", &iValue);
    return iValue;
};

/* Get Bool */
int getBool()
{
    int iValue = 0;
    scanf("%d", &iValue);
    while( (0 != iValue) && (1 != iValue) )
    {
        printf("You entered illegal boolean value %d. Retry.\n", iValue);
        scanf("%d", &iValue);
    }
    return iValue;
};

/* Get Float */
float getFloat()
{
    float fValue = 0.0;
    scanf("%f", &fValue);
    return fValue;
};

/* Get String */
int getString( char *pcStr )
{
    scanf("%[^\n]s", pcStr);
    return (strlen(pcStr)+1);
};

/* Put Integer */
void putInteger( int iValue )
{
    printf("%d\n", iValue);
};

/* Put Bool */
void putBool( int iValue )
{
    if (iValue) printf("true\n");
    else        printf("false\n");
};

/* Put Float */
void putFloat( float fValue )
{
    printf("%f\n", fValue);
};

/* Put String */
void putString( char *pcStr )
{
    printf("%s\n", pcStr);
};


#endif /* RUN_TIME_H_ */

