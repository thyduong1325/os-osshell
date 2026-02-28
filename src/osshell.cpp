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

    while(true){
        std::cout << "osshell> ";
        std::getline(std::cin, user_command);

        //Special commands
        if(user_command == "exit"){
            break;
        } 
        
        if(user_command == "history"){
            for(int i = 0; i < history.size(); i++){
                printf("%2d: %s\n", i + 1, history[i].c_str());   
        }
        continue;
        }

        if(user_command.empty()){
            continue;
        }

        //store in history
        history.push_back(user_command);

        //check size
        if(history.size() > 128){
            history.erase(history.begin());
        }

        //split the user_command
        splitString(user_command, ' ', command_list);
        vectorOfStringsToArrayOfCharArrays(command_list, &command_list_exec);

        bool found = false;
        std::string found_path;

        //search in path
        for(int i = 0; i < os_path_list.size(); i++){
            //create path
            std::string command_path = os_path_list[i] + "/" +  command_list_exec[0];

            //found command
            if(access(command_path.c_str(), X_OK) == 0){
                found = true;
                found_path = command_path;
                break;
            }
        }                                                                                                                                                                                                                                                                        
        

        if(found){
            pid_t pid = fork();
            if(pid == 0){
                //execute
                execv(found_path.c_str(), command_list_exec);
            }else{
                int status;
                waitpid(pid, &status, 0);
            }
        }

        if(!found){
          std::cout << command_list_exec[0] << ": Error command not found" << std::endl;  
        }
        
        //free memory
        freeArrayOfCharArrays(command_list_exec, command_list.size() + 1);


    }


    
    

    
    /************************************************************************************
     *   Example code - remove in actual program                                        *
     ************************************************************************************/
    // Shows how to loop over the directories in the PATH environment variable
    /*
    int i;
    for (i = 0; i < os_path_list.size(); i++)
    {
        printf("PATH[%2d]: %s\n", i, os_path_list[i].c_str());
    }
    printf("------\n");
    
    // Shows how to split a command and prepare for the execv() function
    std::string example_command = "ls -lh";
    splitString(example_command, ' ', command_list);
    vectorOfStringsToArrayOfCharArrays(command_list, &command_list_exec);
    // use `command_list_exec` in the execv() function rather than looping and printing
    i = 0;
    while (command_list_exec[i] != NULL)
    {
        printf("CMD[%2d]: %s\n", i, command_list_exec[i]);
        i++;
    }
    // free memory for `command_list_exec`
    freeArrayOfCharArrays(command_list_exec, command_list.size() + 1);
    printf("------\n");

    // Second example command - reuse the `command_list` and `command_list_exec` variables
    example_command = "echo \"Hello world\" I am alive!";
    splitString(example_command, ' ', command_list);
    vectorOfStringsToArrayOfCharArrays(command_list, &command_list_exec);
    // use `command_list_exec` in the execv() function rather than looping and printing
    i = 0;
    while (command_list_exec[i] != NULL)
    {
        printf("CMD[%2d]: %s\n", i, command_list_exec[i]);
        i++;
    }
    // free memory for `command_list_exec`
    freeArrayOfCharArrays(command_list_exec, command_list.size() + 1);
    printf("------\n");
    /************************************************************************************
     *   End example code                                                               *
     ************************************************************************************/


    return 0;
}

/*
   file_path: path to a file
   RETURN: true/false - whether or not that file exists and is executable
*/
bool fileExecutableExists(std::string file_path)
{
    bool exists = false;
    // check if `file_path` exists
    // if so, ensure it is not a directory and that it has executable permissions

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
