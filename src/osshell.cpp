#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <sstream>
#include <vector>
#include <filesystem>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>

bool fileExecutableExists(std::string file_path);
void splitString(std::string text, char d, std::vector<std::string>& result);
void vectorOfStringsToArrayOfCharArrays(std::vector<std::string>& list, char ***result);
void freeArrayOfCharArrays(char **array, size_t array_length);

int main (int argc, char **argv)
{
    // Get list of paths to binary executables
    std::vector<std::string> os_path_list;
    char* os_path = getenv("PATH");
    splitString(os_path, ':', os_path_list);

    // Create list to store history
    std::vector<std::string> history;

    // Create variables for storing command user types
    std::string user_command;               // to store command user types in
    std::vector<std::string> command_list;  // to store `user_command` split into its variour parameters
    char **command_list_exec;               // to store `command_list` converted to an array of character arrays

    // Welcome message
    printf("Welcome to OSShell! Please enter your commands ('exit' to quit).\n");

    // Repeat:
    //  Print prompt for user input: "osshell> " (no newline)
    //  Get user input for next command
    //  If command is `exit` exit loop / quit program
    //  If command is `history` print previous N commands
    //  For all other commands, check if an executable by that name is in one of the PATH directories
    //   If yes, execute it
    //   If no, print error statement: "<command_name>: Error command not found" (do include newline)

    std::filesystem::path history_path = "history.txt";

    if(std::filesystem::exists(history_path)){
        FILE* load_file = fopen(history_path.string().c_str(), "r");

        if(load_file != NULL){
            // The buffer
            char command[1024];

            while(fgets(command, sizeof(command), load_file) != NULL){

                // Convert char to string
                std::string line = command;

                // Remove newline characters
                if(!line.empty() && line.back() == '\n'){
                    line.pop_back();
                }

                if (!line.empty() && line.back() == '\r') {
                    line.pop_back();
                }
            
            // Store the command line in history
            history.push_back(line);
            }
        fclose(load_file);
        }
    }

    while(true){
        std::cout << "osshell> ";
        std::getline(std::cin, user_command);

        // Exit command
        if(user_command == "exit"){

            // Store in history
            history.push_back(user_command);

            // Save commands in the history file
            FILE* save_file = fopen(history_path.c_str(), "w");

            if (save_file != NULL) {
                for (int i = 0; i < history.size(); i++) {
                    fprintf(save_file, "%s\n", history[i].c_str());
                }
                
                // Close file and save
                fclose(save_file);
                
            }
            std::cout << std::endl;
            break;
        } 

        // Create a stream to "read" the command
        std::stringstream ss(user_command);
        std::string first_word;

        // If the input is only spaces, ss >> first_word will fail
        if (!(ss >> first_word)) {
            continue; // User entered only spaces or just hit enter
        }

        // Empty string
        if (user_command.empty()) {
            continue;
        }

        // Split the user_command
        splitString(user_command, ' ', command_list);
        vectorOfStringsToArrayOfCharArrays(command_list, &command_list_exec);
        
        // History handling
        if(command_list[0] == "history" && command_list.size() <=2){
            // Just history command
            if(command_list.size() == 1){
                for(int i = 0; i < history.size(); i++){
                    printf("%3d: %s\n", i + 1, history[i].c_str());
                }

            }else if(command_list.size() == 2){
                if(command_list[1] == "clear"){
                    history.clear();
                    continue; // To not store in the history
                }

                size_t length;
                int count = std::stoi(command_list[1], &length);
                if(count > 0 && length == command_list[1].size()){
                    int start = history.size() - count;
                    if(start < 0) start = 0;
                    for(int i = start; i < history.size(); i++){
                        printf("%3d: %s\n", i + 1, history[i].c_str()); 
                    }
                }else{
                    std::cout << "Error: history expects an integer > 0 (or 'clear')" << std::endl;
                }
            }
            // Store in history list and restart the main loop
            history.push_back(user_command);
            continue;
        }

        // Store in history
        history.push_back(user_command);

        // Check size
        if(history.size() > 128){
            history.erase(history.begin());
        }

        bool found = false;
        std::string found_path;
        
        // User input starts with a dot (.) or slash (/)
        if (command_list_exec[0][0] == '.' || command_list_exec[0][0] == '/'){
            // Search for the command
            if(fileExecutableExists(command_list_exec[0])){
                found = true;
                found_path = command_list_exec[0];
            }
        }else {
            // Loop through the PATH environment variable
            for(int i = 0; i < os_path_list.size(); i++){
                // Combine PATH environment variable with the first element of the enetered command
                std::string command_path = os_path_list[i] + "/" +  command_list_exec[0];

                // Search the combined command
                if(fileExecutableExists(command_path.c_str())){
                    found = true;
                    found_path = command_path;
                    break;
                }
            } 
        }

        // If found, spawn a new process to run that executable, and wait for its completion
        if(found){
            pid_t pid = fork();
            if(pid == 0){
                execv(found_path.c_str(), command_list_exec);
            }else{
                int status;
                waitpid(pid, &status, 0);
            }
        }else {
            // If not found, print the error message
            std::cout << command_list_exec[0] << ": Error command not found" << std::endl;  
        }

        // Free memory
        freeArrayOfCharArrays(command_list_exec, command_list.size() + 1);
    }

    return 0;
}

/*
   file_path: path to a file
   RETURN: true/false - whether or not that file exists and is executable
*/
bool fileExecutableExists(std::string file_path)
{
    bool exists = false;
    // Check if `file_path` exists
    // If so, ensure it is not a directory and that it has executable permissions

    // Convert string to a filesystem path object
    std::filesystem::path p(file_path);

    // Check if the path exists and ensure it is a regular file (not a directory)
    if (std::filesystem::exists(p) && std::filesystem::is_regular_file(p)) {
        
        // Check for executable permissions
        if (access(file_path.c_str(), X_OK) == 0) {
            exists = true;
        }
    }
    return exists;
}

/*
   text: string to split
   d: character delimiter to split `text` on
   result: vector of strings - result will be stored here
*/
void splitString(std::string text, char d, std::vector<std::string>& result)
{
    enum states { NONE, IN_WORD, IN_STRING } state = NONE;

    int i;
    std::string token;
    result.clear();
    for (i = 0; i < text.length(); i++)
    {
        char c = text[i];
        switch (state) {
            case NONE:
                if (c != d)
                {
                    if (c == '\"')
                    {
                        state = IN_STRING;
                        token = "";
                    }
                    else
                    {
                        state = IN_WORD;
                        token = c;
                    }
                }
                break;
            case IN_WORD:
                if (c == d)
                {
                    result.push_back(token);
                    state = NONE;
                }
                else
                {
                    token += c;
                }
                break;
            case IN_STRING:
                if (c == '\"')
                {
                    result.push_back(token);
                    state = NONE;
                }
                else
                {
                    token += c;
                }
                break;
        }
    }
    if (state != NONE)
    {
        result.push_back(token);
    }
}

/*
   list: vector of strings to convert to an array of character arrays
   result: pointer to an array of character arrays when the vector of strings is copied to
*/
void vectorOfStringsToArrayOfCharArrays(std::vector<std::string>& list, char ***result)
{
    int i;
    int result_length = list.size() + 1;
    *result = new char*[result_length];
    for (i = 0; i < list.size(); i++)
    {
        (*result)[i] = new char[list[i].length() + 1];
        strcpy((*result)[i], list[i].c_str());
    }
    (*result)[list.size()] = NULL;
}

/*
   array: list of strings (array of character arrays) to be freed
   array_length: number of strings in the list to free
*/
void freeArrayOfCharArrays(char **array, size_t array_length)
{
    int i;
    for (i = 0; i < array_length; i++)
    {
        if (array[i] != NULL)
        {
            delete[] array[i];
        }
    }
    delete[] array;
}
