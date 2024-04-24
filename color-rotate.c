#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#include "globals.h"
#include "fs.c"
#include "colorUtils.c"

char *GLOBAL_SETTINGS_PATH = "/home/kostas/.config/Code/User/profiles/-fb328e2/settings.json";
char *BASE_COLOR_FIELD = "base_color"; // "base_color": "#ffffff"
int COLOR_ROTATION_INTERVAL = 10;      // In seconds
float COLOR_ROTATION = 8.33;           // In degrees

static volatile bool keepRunning = True;

void intHandler(int dummy)
{
    keepRunning = False;
}

char *readColorFromFile(const char *filePath)
{
    File *fp = openFile(filePath, "r");
    if (fp == NULL) return NULL;

    char *color = malloc(COLOR_SIZE + 1);
    char *line;
    while ((line = readLine(fp)) != NULL)
    {
        char *colorPos = strstr(line, BASE_COLOR_FIELD);
        if (colorPos)
        {
            // The + 3 is for "base_color: "#ffffff" "
            strncpy(color, colorPos + strlen(BASE_COLOR_FIELD) + 4, COLOR_SIZE);
            break;
        }
        dealloc(1, line);
    }

    fclose(fp);

    return color;
}

void replaceColorInFile(const char *filePath, char *currentColor, char *newColor)
{
    File *fp = fopen(filePath, "r+");
    if (!fp)
    {
        fprintf(stderr, "Could not open fp %s\n", filePath);
        return;
    }

    char *line;
    printf("Replacing %s with %s\n", currentColor, newColor);
    while ((line = readLine(fp)) != NULL)
    {
        char *colorPos = strstr(line, currentColor);
        if (colorPos)
        {
            fseek(fp, -strlen(colorPos), SEEK_CUR);
            fputs(newColor, fp);
            fseek(fp, strlen(colorPos) - strlen(newColor), SEEK_CUR);
        }
        free(line);
    }
    free(line);

    fclose(fp);
}

void loadArgs(int argc, char *argv[])
{
    for (int i = 1; i < argc; i += 2)
    {
        printf("Arg flag: %s\n", argv[i]);
        printf("Arg value: %s\n", argv[i + 1]);
        if (strcmp(argv[i], "-p") == 0)
        {
            WORKSPACE_SETTINGS_PATH = (char *)malloc((strlen(argv[i + 1]) + 24) * (sizeof(char)));
            WORKSPACE_SETTINGS_PATH = strcpy(WORKSPACE_SETTINGS_PATH, argv[i + 1]);
            strcat(WORKSPACE_SETTINGS_PATH, "/.vscode/settings.json");
        }
        else if (strcmp(argv[i], "-r") == 0)
        {
            COLOR_ROTATION = atof(argv[i + 1]);
        }
        else if (strcmp(argv[i], "-i") == 0)
        {
            COLOR_ROTATION_INTERVAL = atoi(argv[i + 1]);
        }
    }

    if (WORKSPACE_SETTINGS_PATH == NULL)
    {
        WORKSPACE_SETTINGS_PATH = (char *)malloc(1024 * (sizeof(char)));
        getcwd(WORKSPACE_SETTINGS_PATH, 1001);
        strcat(WORKSPACE_SETTINGS_PATH, "/.vscode/settings.json");
    }
    printf("Workspace settings path: %s\n", WORKSPACE_SETTINGS_PATH);
    printf("Color rotation: %f\n", COLOR_ROTATION);
    printf("Color rotation interval: %d\n\n", COLOR_ROTATION_INTERVAL);
}

int main(int argc, char *argv[])
{
    loadArgs(argc, argv);
    createSettingsFile();

    signal(SIGINT, intHandler);

    // Read color from JSON fp
    char *color = readColorFromFile(WORKSPACE_SETTINGS_PATH);
    char *newColor = malloc(COLOR_SIZE + 1);
    while (keepRunning)
    {
        HSLToHex(rotateHue(HexToHSL(color), COLOR_ROTATION), newColor);

        replaceColorInFile(WORKSPACE_SETTINGS_PATH, color, newColor);

        memcpy(color, newColor, COLOR_SIZE);

        sleep(COLOR_ROTATION_INTERVAL);
    }

    return 0;
}