//Importing libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dirent.h"
#include <yara.h>
#include <Windows.h>

#define PATH_SEPARATOR '\\'
#define BUFFER_SIZE 1024

//Function to Display error message by formatting it and printing it if not null
void displayErrorMessage(DWORD errorCode) {
    LPSTR messageBuffer = NULL;
    FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        errorCode,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR)&messageBuffer,
        0,
        NULL);

    if (messageBuffer != NULL) {
        printf("ERROR! %s\n", messageBuffer);
        LocalFree(messageBuffer);
    }
    else {
        printf("Unable to display the error message for your code ...%d\n", errorCode);
    }
}

// Function to return info based on rule matching from filepath data
int scanCallback(
    YR_SCAN_CONTEXT* context,
    int message,
    void* message_data,
    void* user_data) {
    switch (message) {
    case CALLBACK_MSG_RULE_MATCHING:
        printf("Matched rule: %s\n", ((YR_RULE*)message_data)->identifier);
        break;
    case CALLBACK_MSG_RULE_NOT_MATCHING:
        printf("Did not match rule: %s\n", ((YR_RULE*)message_data)->identifier);
        break;
    case CALLBACK_MSG_SCAN_FINISHED:
        printf("Scan finished\n");
        break;
    case CALLBACK_MSG_TOO_MANY_MATCHES:
        printf("Too many matches\n");
        break;
    case CALLBACK_MSG_CONSOLE_LOG:
        printf("Console log: %s\n", (char*)message_data);
        break;
    default:
        break;
    }

    return CALLBACK_CONTINUE;
}

// Function to scan the filepath with given YARA rules
void scanFile(const char* filePath, YR_RULES* rules) {

    yr_rules_scan_file(rules, filePath, SCAN_FLAGS_REPORT_RULES_MATCHING, scanCallback, NULL, NULL);

}

// Function to scan the directory
void scanDirectory(const char* dirPath, YR_RULES* rules){
    DIR* dir;
    struct dirent* entry;

    if (!(dir == opendir(dirPath))){
        printf("ERROR! (while opening the directory)\n");
        return;
    }

    // Reading the directory contents
    while ((entry = readdir(dir)) != NULL){
        if (entry->d_type == DT_DIR) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }

            char path[BUFFER_SIZE];
            snprintf(path, sizeof(path), "%s%c%s", dirPath, PATH_SEPARATOR, entry->d_name);
            scanDirectory(path, rules);
        }
        else {
            char filePath[BUFFER_SIZE];
            snprintf(filePath, sizeof(filePath), "%s%c%s", dirPath, PATH_SEPARATOR, entry->d_name);
            scanFile(filePath, rules);
        }
    }

    closedir(dirPath);
}

// Function to check if file path is a regular file or a directory
void checkType(const char* path, const char* rules){
    struct stat path_stat;
    if (stat(path, &path_stat) == 0){
        if (S_ISREG(path_stat.st_mode)){
            scanFile(path,rules);
        }
        else if (S_ISDIR(path_stat.st_mode)){
            scanDirectory(path,rules); 
        }
        else {
            printf("Unknown File Type\n");
        }
    }
    else{
        perror("ERROR (while getting file status) \n");
    }
}

int main(int argc, char* argv[]){
    if (argc != 2){
        printf("[-] Parameters Incorrect!");
        return 1;
    }

    char* file_path = argv[1];
    const char directory_path[] = "C:\Users\ssric\OneDrive\Desktop\Edu stuff\Coding\Antivirus_Project\rules";    // Stores the YARA rules

    int Initresult = yr_initialize();
    if (Initresult != 0){
        printf("[-] YARA initialization error!\n");
        return 1;
    }

    printf("[+] Successfully Initialized YARA ...\n");

    YR_COMPILER* compiler = NULL;
    int Compilerres = yr_compiler_Create(&compiler);

    if (Compilerres != ERROR_SUCCESS){
        printf("[-] YARA initialization error!\n");
        return 1;
    }

    printf("[+] COMPILER creation successful\n");

    // Loading all the YARA rules
    DIR* directory = opendir(directory_path);
    if (directory == NULL){
        printf("[-] Failed to Open Directory : %s\n",directory_path);
        yr_finalize();
        return 1;
    }

    printf("[+] DIRECTORY opened successfully ...\n");

    struct dirent* entry;

    //Going through the rules and checking for errors
    struct dirent* entry;
    while ((entry = readdir(directory)) != NULL) {
        if (entry->d_type == DT_REG && strstr(entry->d_name, ".yar") != NULL) {
            char rule_file_path[PATH_MAX];
            snprintf(rule_file_path, sizeof(rule_file_path), "%s/%s", directory_path, entry->d_name);
            FILE* rule_file = fopen(rule_file_path, "rb");
                int Addresult = yr_compiler_add_file(compiler, rule_file, NULL, NULL); 
                if (Addresult > 0) {
                    printf("[-] Failed to compile YARA rule %s, number of errors found: %d\n", rule_file_path, Addresult);
                    displayErrorMessage(GetLastError());
                }
                else {
                    printf("[+] Compiled rules %s\n", rule_file_path);
                }
               fclose(rule_file);
        }         
    }
  
    closedir(directory);


    YR_RULES* rules = NULL;
    yr_compiler_get_rules(compiler, &rules);

    checkType(file_path, rules);


    // Cleaning up the rules
    yr_rules_destroy(rules);
    yr_finalize();

    return 0;
}