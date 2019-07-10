#include <ShlObj.h>
#include <Shlwapi.h>
#include <stdio.h>
#include "cJSON/cJSON.h"
#include "Config.h"

Config config;

static CHAR path[MAX_PATH];

void Config_init(PCSTR name)
{
    PWSTR pathToDocuments;
    if (SUCCEEDED(SHGetKnownFolderPath(&FOLDERID_Documents, 0, NULL, &pathToDocuments))) {
        sprintf(path, "%ws/%s/", pathToDocuments, name);
        if (PathFileExistsA(path)) {
            strcat(path, "*");
            WIN32_FIND_DATA foundData;
            HANDLE found = FindFirstFileA(path, &foundData);

            if (found != INVALID_HANDLE_VALUE) {
                do {
                    if (!(foundData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) { 
                        size_t fileNameLength = strlen(foundData.cFileName) + 1;
                        config.count++;

                        void* newAlloc = realloc(config.names, config.count * sizeof(PSTR));
                        if (newAlloc)
                            config.names = newAlloc;
                        
                        newAlloc = malloc(fileNameLength);
                        if (newAlloc)
                            config.names[config.count - 1] = newAlloc;
                        strcpy(config.names[config.count - 1], foundData.cFileName);
                    }
                } while (FindNextFileA(found, &foundData));
            }
            path[strlen(path) - 1] = '\0';
        } else {
            CreateDirectoryA(path, NULL);
        }
        CoTaskMemFree(pathToDocuments);
    }
}

void Config_add(PSTR name)
{
    config.count++;
    void* newAlloc = realloc(config.names, config.count * sizeof(PSTR));
    if (newAlloc)
        config.names = newAlloc;
    newAlloc = malloc(strlen(name) + 1);
    if (newAlloc)
        config.names[config.count - 1] = newAlloc;
    sprintf(config.names[config.count - 1], "%s", name);
}

void Config_save(void)
{
    cJSON* json = cJSON_CreateObject();

    cJSON* miscJson = cJSON_CreateObject();

    cJSON_AddBoolToObject(miscJson, "Auto strafe", config.misc.autostrafe);
    cJSON_AddBoolToObject(miscJson, "Bunny hop", config.misc.bunnyhop);
    cJSON_AddItemToObject(json, "Misc", miscJson);

    CHAR filename[MAX_PATH];
    sprintf(filename, "%s%s", path, "config.json");

    if (!PathFileExistsA(path))
        CreateDirectoryA(path, NULL);

    FILE* out = fopen(filename, "w");

    if (out) {
        fprintf(out, cJSON_Print(json));
        fclose(out);
    }

    cJSON_Delete(json);
}
