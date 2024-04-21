#ifndef True
#define True 1
#endif

#ifndef False
#define False 0
#endif

#ifndef COLOR_SIZE 
// Hex string color size: #ffffff = 7
#define COLOR_SIZE 7 
#endif

#ifndef GLOBAL_VARS
#define GLOBAL_VARS 1
char *GLOBAL_SETTINGS_PATH;
char *WORKSPACE_SETTINGS_PATH;
char *BASE_COLOR_FIELD; // "base_color": "#ffffff"
int COLOR_ROTATION_INTERVAL;      // In seconds
float COLOR_ROTATION;           // In degrees
#endif

#ifndef RGB_STRUCT
#define RGB_STRUCT
typedef struct
{
    double r; // percent
    double g; // percent
    double b; // percent
} rgb;
#endif

#ifndef HSL_STRUCT
#define HSL_STRUCT
typedef struct
{
    double h; // angle in degrees
    double s; // percent
    double l; // percent
} hsl;
#endif


#ifndef BOOLEAN_STRUCT
#define BOOLEAN_STRUCT
typedef char bool;
#endif

#ifndef DEALLOC
#include <stdarg.h>
#include <stdlib.h>
#define DEALLOC
void dealloc(int args, ...)
{    
    va_list ptr_list;
    va_start(ptr_list, args);
    void *p;
    int i = args;
    while (i--)
    {
        p = va_arg(ptr_list, void *);
        if (p == NULL)
        {
            break;
        }
        // printf("Deallocating string %s\n", (char *)p);
        free(p);
    }
    va_end(ptr_list);
}
#endif