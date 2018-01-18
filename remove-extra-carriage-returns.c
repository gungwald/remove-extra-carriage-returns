#include <stdio.h>
#include <stdlib.h> /* EXIT_SUCCESS, EXIT_FAILURE */
#include <stdbool.h>
#include <errno.h>
#include <string.h> /* strerror */

void printError(const char *operation, const char *object, int errorNumber);
bool overwrite(const char *destName, const char *srcName);
bool removeExtraCarriageReturns(const char *fileName);
void removeBackslashPrefix(char *outputName);

int main(int argc, char *argv[])
{
    int i;
    int status = EXIT_SUCCESS;

    for (i = 1; i < argc; i++) {
        if (! removeExtraCarriageReturns(argv[i])) {
            status = EXIT_FAILURE;
        }
    }
    return status;
}

void removeBackslashPrefix(char *s)
{
    int i;
    size_t len;

    if (s[0] != '\0' && s[0] == '\\') {
        len = strlen(s);
        for (i = 1; i <= len; i++) {
            s[i-1] = s[i];
        }
    }
}

bool overwrite(const char *destName, const char *srcName)
{
    if (_unlink(destName) != -1) {
        if (rename(srcName, destName) == 0) {
            return true;
        }
        else {
            printError("rename", srcName, errno);
        }
    }
    else {
        printError("unlink", destName, errno);
    }
    return false;
}

void printError(const char *operation, const char *object, int errorNumber)
{
    fprintf(stderr, "%s: %s: %s\n", operation, object, strerror(errorNumber));
}

bool removeExtraCarriageReturns(const char *fileName)
{
    FILE *f;
    FILE *output;
    int c;
    int last;
    char *outputName;

    f = fopen(fileName, "rb");
    if (f == NULL) {
        printError("fopen", fileName, errno);
        return false;
    }

    outputName = tmpnam(NULL);
    if (outputName == NULL) {
        printError("tmpnam", "None", errno);
        return false;
    }
    removeBackslashPrefix(outputName);

    output = fopen(outputName, "wb");
    if (output == NULL) {
        printError("fopen", outputName, errno);
        return false;
    }

    if ((last = fgetc(f)) != EOF) {
        while ((c = fgetc(f)) != EOF) { 
            printf("%d %d\n", last, c);
            if (last != '\r' || c != '\r') {
                if (fputc(last, output) == EOF) {
                    printError("fputc", outputName, errno);
                    break;
                }
                last = c;
            }
        }
        if (last != '\r') {
            if (fputc(last, output) == EOF) {
                printError("fputc", outputName, errno);
            }
        }
    }
    if (ferror(f)) {
        printError("ferror", fileName, errno);
    }
    if (fclose(output) == EOF) {
        printError("fclose", outputName, errno);
    }
    if (fclose(f) == EOF) {
        printError("fclose", fileName, errno);
    }
    overwrite(fileName, outputName);
    return true;
}

