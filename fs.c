#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>

#include "globals.h"


void expandMemory(char **str, int newSize)
{
    *str = (char *)realloc(*str, newSize * sizeof(char));
    if (!*str)
    {
        fprintf(stderr, "Memory reallocation error\n");
        exit(1);
    }
}

/*
    File operations
*/

char *readLine(FILE *fp)
{
    int bufferSize = 512; // Initial buffer size
    char *line = (char *)malloc(bufferSize * sizeof(char));
    if (!line)
    {
        fprintf(stderr, "Memory allocation error\n");
        return NULL;
    }

    int index = 0;
    char c;

    while (True)
    {
        c = fgetc(fp);

        if (c == EOF)
        {
            if (index == 0)
            {
                free(line);
                return NULL;
            }
            else
            {
                line[index] = '\0';
                break;
            }
        }
        else
        {
            line[index] = c;
            index++;

            // Check if the buffer is full and reallocate if necessary
            if (index == bufferSize - 1)
            {
                bufferSize *= 2; // Double the buffer size
                line = (char *)realloc(line, bufferSize * sizeof(char));
                if (!line)
                {
                    fprintf(stderr, "Memory reallocation error\n");
                    return NULL;
                }
            }
            if (c == '\n' || c == '\r')
            {
                line[index] = '\0';
                break;
            }
        }
    }
    return line;
}

/*
    Directory operations
*/

int dirExists(const char *path)
{
    DIR *dir = opendir(path);
    if (dir)
    {
        closedir(dir);
        return 1;
    }
    else if (ENOENT == errno)
    {
        return 0;
    }
    return 0;
}

int createDir(const char *path)
{
    if (dirExists(path))
    {
        fprintf(stderr, "Directory %s already exists\n", path);
        return 0;
    }

    int status = mkdir(path, 0777);
    if (status)
    {
        fprintf(stderr, "Could not create directory %s\n", path);
        return 0;
    }
    return 1;
}

int createDirPath(const char *path)
{
    // Absolute folder path
    char *absoluteDirPath = (char *)malloc(strlen(path) * sizeof(char));

    char *dirPos = (char *)path;
    while (True)
    {
        int bytesToCopy = 0;
        if (strchr(dirPos + 1, '/') == NULL)
        {
            dirPos = ((char *)path) + strlen(path);
            bytesToCopy = strlen(path);
            strncpy(absoluteDirPath, path, bytesToCopy);
        }
        else
        {
            // Move the pointer to the next directory position
            dirPos = strchr(dirPos + 1, '/');

            // Copy the absolute directory path
            bytesToCopy = strlen(path) - strlen(dirPos);
            strncpy(absoluteDirPath, path, bytesToCopy);
        }
        absoluteDirPath[bytesToCopy] = '\0';

        if (!dirExists(absoluteDirPath))
        {
            if (!createDir(absoluteDirPath))
            {
                return 0;
            }
        }

        if (dirPos == ((char *)path) + strlen(path))
        {
            break;
        }
    }
    dealloc(1, absoluteDirPath);

    return 1;
}

/*
    File operations
*/

int fileExists(const char *path)
{
    FILE *fp = fopen(path, "r");
    if (fp)
    {
        fclose(fp);
        return 1;
    }
    return 0;
}

int createFile(const char *path, const char *content, int force)
{
    if (!force && fileExists(path))
    {
        fprintf(stderr, "File %s already exists\n", path);
        return 0;
    }

    FILE *fp = fopen(path, "w+");
    if (!fp)
    {
        fprintf(stderr, "Could not create file %s\n", path);
        return 0;
    }

    fputs(content, fp);
    fclose(fp);
    return 1;
}

char *readFile(const char *path)
{
    FILE *fp = fopen(path, "r");
    if (!fp)
    {
        fprintf(stderr, "Could not open fp %s\n", path);
        return NULL;
    }
    // read the file contents and store them to the content variable
    int bufferSize = 1024; // Initial buffer size
    int bytesRead = 0;
    int contentMemSize = bufferSize;
    char *buffer = (char *)malloc(bufferSize * sizeof(char));
    char *content = (char *)malloc(bufferSize * sizeof(char));
    // Set the first character to null so that strcat can append to it (wow!)
    content[0] = '\0';

    if (!content || !buffer)
    {
        fprintf(stderr, "Memory allocation error\n");
        return NULL;
    }

    while (True)
    {
        memset(buffer, 0, bufferSize);
        fread(buffer, bufferSize, sizeof(char), fp);
        bytesRead += strlen(buffer);

        if (buffer == NULL)
        {
            break;
        }

        strcat(content, buffer);
        if (strlen(buffer) < bufferSize)
        {
            break;
        }

        if (bytesRead + bufferSize >= contentMemSize)
        {
            int newBufferSize = strlen(content) * 2; // 2x the current size
            expandMemory(&content, newBufferSize);
            contentMemSize = newBufferSize;
        }
    }
    fclose(fp);
    dealloc(1, buffer);

    return content;
}

/*
    Settings file operations
*/

void *createSettingsFile()
{
    char *globalSettings = readFile(GLOBAL_SETTINGS_PATH);
    if (globalSettings == NULL)
    {
        printf("Global settings file does not exist\n");
        exit(1);
    }

    // copy "workbench.colorCustomizations" field
    char *colorFieldStart = strstr(globalSettings, "\"workbench.colorCustomizations");
    if (colorFieldStart == NULL)
    {
        printf("Color field not found\n");
        return NULL;
    }

    // find the last bracket and copy the content
    int bracketCounter = 0;
    int insideColorField = 0;
    int i = 0;
    while (colorFieldStart[i] != '\0')
    {
        if (colorFieldStart[i] == '{')
        {
            bracketCounter++;
            insideColorField = 1;
        }
        else if (colorFieldStart[i] == '}')
        {
            bracketCounter--;
        }
        if (insideColorField == 1 && bracketCounter == 0)
        {
            break;
        }
        i++;
    }
    char *colorFieldContent = (char *)malloc((i + 7) * sizeof(char));
    colorFieldContent[0] = '\0';

    char *workspaceSettings = !fileExists(WORKSPACE_SETTINGS_PATH) ? NULL : readFile(WORKSPACE_SETTINGS_PATH);

    if (workspaceSettings == NULL || strlen(workspaceSettings) < 4 || strstr(workspaceSettings, "\"workbench.colorCustomizations") != NULL)
    {
        strcat(colorFieldContent, "{\n\t");
        strncpy(colorFieldContent + 3, colorFieldStart, i);
        strcat(colorFieldContent, "}\n}\0");

        // Create the dir if it does not exist
        char *workspaceSettingsDir = (char *)malloc(strlen(WORKSPACE_SETTINGS_PATH) * sizeof(char));
        int bytesToCopy = (int)strlen(WORKSPACE_SETTINGS_PATH) - (int)strlen(strrchr(WORKSPACE_SETTINGS_PATH, '/'));
        strncpy(workspaceSettingsDir, WORKSPACE_SETTINGS_PATH, bytesToCopy);
        workspaceSettingsDir[bytesToCopy] = '\0';

        if (!dirExists(workspaceSettingsDir))
        {
            createDirPath(workspaceSettingsDir);
        }

        createFile(WORKSPACE_SETTINGS_PATH, colorFieldContent, 1);
    }
    else
    {
        // copy the color field content
        strncpy(colorFieldContent, colorFieldStart, i);
        colorFieldContent[i] = '\0';

        // find the last bracket to copy up to
        int bracketPos = strrchr(workspaceSettings, '}') - workspaceSettings;

        bool hasComma = False;
        int commaPos = 1;
        while (True)
        {
            if (workspaceSettings[bracketPos - commaPos] == ',')
            {
                hasComma = True;
                break;
            }
            if (workspaceSettings[bracketPos - commaPos] == '}' ||
                workspaceSettings[bracketPos - commaPos] == '\"' ||
                workspaceSettings[bracketPos - commaPos] == '\'')
            {
                break;
            }
            commaPos++;
        }

        // New settings string
        char *newSettings = (char *)malloc((strlen(workspaceSettings) + strlen(colorFieldContent) + 10) * sizeof(char));
        newSettings[0] = '\0';
        if (hasComma)
        {
            strncat(newSettings, workspaceSettings, bracketPos);
            strcat(newSettings, "\t");
        }
        else
        {
            strncat(newSettings, workspaceSettings, bracketPos - commaPos + 1);
            strcat(newSettings, ",\n\t");
        }
        strcat(newSettings, colorFieldContent);
        strcat(newSettings, "}, \n}");

        // Create (w+) the file with the new settings
        createFile(WORKSPACE_SETTINGS_PATH, newSettings, 1);

        dealloc(1, newSettings);
    }
    dealloc(3, globalSettings, colorFieldContent, workspaceSettings);

    return NULL;
}
