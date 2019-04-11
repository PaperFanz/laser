#define USES_FLAG
#define USES_FILE
#define USES_NOTIFY
#define USES_OPERAND
#include "laser.h"

static int8_t NO_WARNINGS = 0;
static int8_t NO_ERRORS = 0;
static char *CURRENT_FILE = "";
static FILE *CURRENT_LOG = NULL;

void setVerbosity (int8_t q)
{
    if (q == noErrs) {
        NO_ERRORS = 1;
        NO_WARNINGS = 1;
    } else if (q == noWarn) {
        NO_ERRORS = 0;
        NO_WARNINGS = 1;
    } else {
        NO_ERRORS = 0;
        NO_WARNINGS = 0;
    }
}

uint8_t setcurrentlog (char *file)
{
    if (file) {
        CURRENT_LOG = fopen (file, "w");
        if (CURRENT_LOG) return 0;
    }
    return 1;
}

void endlog (void)
{
    if (CURRENT_LOG) fclose (CURRENT_LOG);
}

void setcurrentfile (char *file)
{
    // strip directories from filename string
    char *ret = NULL;
    #ifdef _WIN32
    ret = strrchr (file, '\\') + 1;
    #endif
    #ifdef _WIN64
    ret = strrchr (file, '\\') + 1;
    #endif
    #ifdef linux
    ret = strrchr (file, '/') + 1;
    #endif
    if (ret) file = ret;
    CURRENT_FILE = file;
}

void notify (const char *format, ...)
{
    va_list strs, strscpy;
    va_copy (strscpy, strs);

    va_start (strs, format);
    if (!NO_ERRORS) {
        vprintf (format, strs);
    }
    va_end (strs);

    va_start (strscpy, format);
    if (islogging ()) {
        vfprintf (CURRENT_LOG, format, strscpy);
    }
    va_end (strscpy);
}

static uint32_t ALERT_WARN;
static uint32_t ALERT_ERRS;

void initnotify (void)
{
    ALERT_WARN = 0;
    ALERT_ERRS = 0;
}

uint32_t getwarnings (void)
{
    return ALERT_WARN;
}

uint32_t geterrors (void)
{
    return ALERT_ERRS;
}

void warning (uint32_t ln, const char *format, ...)
{
    va_list strs, strscpy;
    va_copy (strscpy, strs);

    ALERT_WARN++;

    if (!NO_WARNINGS) {
        va_start (strs, format);
        printf ("\033[01;33mWarning (%s line %d):\033[0m ", CURRENT_FILE, ln);
        vprintf (format, strs);
        printf("\n");
        va_end (strs);
    }

    if (islogging ()) {
        va_start (strscpy, format);
        fprintf (CURRENT_LOG, "Warning (%s line %d): ", CURRENT_FILE, ln);
        vfprintf (CURRENT_LOG, format, strscpy);
        fprintf(CURRENT_LOG, "\n");
        va_end (strscpy);
    }
}

void error (uint32_t ln, const char *format, ...)
{
    va_list strs, strscpy;
    va_copy (strscpy, strs);

    ALERT_ERRS++;

    if (!NO_ERRORS) {
        va_start (strs, format);
        printf ("\033[1;31mError (%s line %d):\033[0m ", CURRENT_FILE, ln);
        vprintf (format, strs);
        printf("\n");
        va_end (strs);
    }

    if (islogging ()) {
        va_start (strscpy, format);
        fprintf (CURRENT_LOG, "Error (%s line %d): ", CURRENT_FILE, ln);
        vfprintf (CURRENT_LOG, format, strscpy);
        fprintf(CURRENT_LOG, "\n");
        va_end (strscpy);
    }
}