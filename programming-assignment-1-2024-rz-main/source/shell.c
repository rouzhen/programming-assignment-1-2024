// Include the shell header file for necessary constants and function declarations
#include "shell.h"

// Function to read a command from the user input
void read_command(char **cmd)
{
  // Define a character array to store the command line input
  char line[MAX_LINE];
  // Initialize count to keep track of the number of characters read
  int count = 0, i = 0;
  // Array to hold pointers to the parsed command arguments
  char *array[MAX_ARGS], *command_token;

  // Infinite loop to read characters until a newline or maximum line length is reached
  for (;;)
  {
    // Read a single character from standard input
    int current_char = fgetc(stdin);
    // Store the character in the line array and increment count
    line[count++] = (char)current_char;
    // If a newline character is encountered, break out of the loop
    if (current_char == '\n')
      break;
    // If the command exceeds the maximum length, print an error and exit
    if (count >= MAX_LINE)
    {
      printf("Command is too long, unable to process\n");
      exit(1);
    }
  }
  // Null-terminate the command line string
  line[count] = '\0';

  // If only the newline character was entered, return without processing
  if (count == 1)
    return;

  // Use strtok to parse the first token (word) of the command
  command_token = strtok(line, " \n");

  // Continue parsing the line into words and store them in the array
  while (command_token != NULL)
  {
    array[i++] = strdup(command_token);  // Duplicate the token and store it
    command_token = strtok(NULL, " \n"); // Get the next token
  }

  // Copy the parsed command and its parameters to the cmd array
  for (int j = 0; j < i; j++)
  {
    cmd[j] = array[j];
  }
  // Null-terminate the cmd array to mark the end of arguments
  cmd[i] = NULL;
  
}

// Function to display the shell prompt
void type_prompt()
{
  // Use a static variable to check if this is the first call to the function
  static int first_time = 1;
  if (first_time)
  {
    // Clear the screen on the first call
#ifdef _WIN32
    system("cls"); // Windows command to clear screen
#else
    system("clear"); // UNIX/Linux command to clear screen
#endif
    first_time = 0;
  }
  fflush(stdout); // Flush the output buffer
  printf("$$ ");  // Print the shell prompt
}



// The main function where the shell's execution begins
int main(void)
{
  // Define an array to hold the command and its arguments
  char *cmd[MAX_ARGS];
  // to tell shell whether to terminate
  //int status;
  int child_status;
  pid_t pid;

  

for (;;) {
  type_prompt();     // Display the prompt
  read_command(cmd); // Read a command from the user
  
  //check if command is empty
  if(cmd[0] == NULL || strcmp(cmd[0], "" )== 0) {
    continue; //skip to the next iteration of loop
  } 
  
  // If the command is "exit", break out of the loop to terminate the shell
  if (strcmp(cmd[0], "exit") == 0)
    // break;
    return 0;

  //fork a new process
  pid = fork();

  //check if fork is successful
  if (pid <0) {
    perror("Fork failed"); //print detailed error message
    exit(1); //exit the program with a non-zero status indicating failure
  }
  else if(pid==0) {
    //child successful
    // Formulate the full path of the command to be executed
    char full_path[PATH_MAX];
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL)
    {
      snprintf(full_path, sizeof(full_path), "%s/bin/%s", cwd, cmd[0]);
    }
    else
    {
      printf("Failed to get current working directory.");
      exit(1);
    }

    execv(full_path, cmd);
     
    
    //----------added stuff to execute builtin cmds---------------------

    // Check for built-in commands
    for (int i = 0; i< numOfBuiltinFunctions(); i++) {
      if (strcmp(cmd[0], builtin_commands[i]) == 0) {
        (*builtin_command_func[i])(cmd);
        continue;
      }
    }
    //----------added stuff to execute builtin cmds---------------------
    
    // If execv returns, command execution has failed
    printf("Command %s not found\n", cmd[0]);
    exit(0);

  
    // Free the allocated memory for the command arguments before exiting
    for (int i = 0; cmd[i] != NULL; i++)
    {
      free(cmd[i]);
    }
    memset(cwd, '\0', sizeof(cwd)); // clear the cwd array

    return 0;    
  }

  else {
    //parent process
    //wait for child to complete
    int child_exit_status;
    //wait(&child_status);
    waitpid(pid, &child_status, WUNTRACED);
    if (WIFEXITED(child_status)) {
      // Retrieve the exit status of the child
      child_exit_status = WEXITSTATUS(child_status);
      //printf("Child exited with status: %d\n", child_exit_status);
      }
  }

} 
}

 
int shell_cd(char **args)
{
  printf("shellCD is called! \n");
  if (args[1] == NULL)
  {
    fprintf(stderr, "CSEShell: expected argument to \"cd\"\n");
  }
  else
  {
    // chdir() changes the current working directory of the calling process
    // to the directory specified in path.
    if (chdir(args[1]) != 0)
    { 
      perror("$$:");
    }
  }

  return 1;
}


/*  Prints out the usage and list of commands implemented */

int shell_help(char **args) {
  int i;

  printf("shellHelp is called! \n");
  printf("CSE Shell Interface\n");
  printf("Type program names and arguments, and hit enter.\n");
  printf("The following are built in:\n");

  for (i = 0; i < numOfBuiltinFunctions(); i++)
  {
    printf("  %s\n", builtin_commands[i]); //print all the commands that have been implemented in the shell program
  }

  return 1;
}

/* Returns 0, to terminate execution */

int shell_exit(char **args) {
// If the command is "exit", break out of the loop to terminate the shell
  if (strcmp(args[0], "exit") == 0)
    return 0;
}      

/* Provides a brief usage guide for the shell and its built-in command */
int shell_usage(char **args) {
  int functionIndex = -1;
  //check if there is an arg
  if(args[1] == NULL) {
    fprintf(stderr, "Command not given. Type usage <command>.\n");
    return 1;
  }

  for (int i=0; i< numOfBuiltinFunctions(); i++) {
    if (strcmp(args[1], builtin_commands[i]) == 0) {
      functionIndex = i;
      break;
    }
  }

  switch(functionIndex) {
    case 0:
      printf("Type: cd directory_name to change directory to current directory of the shell\n");
      break;
    case 1:
      printf("Type: help for supported commands\n");
      break;
    case 2:
      printf("Type: exit to terminate the shell gracefully\n");
      break;
    case 3:
      printf("Type: usage cd/help/exit \n");
      break;
    case 4:
      printf("Type: env to  lists all registered env variables\n");
      break;
    case 5:
      printf("Type: setenv ENV=VALUE to set a new env variable\n");
      break;
    case 6:
      printf("Type: unsetenv ENV to remove this env from the list of env variables\n");
      break;
  }

  return 1;

}


extern char **environ; //external var that holds all the env variable strings in the format 'char *environ[]'
int list_env(char **args) {
    // Pointer to traverse through the environment variables
    char **env = environ;

    // Loop through all the environment variables until NULL is encountered
    while (*env) {
        printf("%s\n", *env); // *env to dereference the pointer to get the curr env var string
        env++;
    }

    return 1;
}

// Sets or modifies an environment variable for this shell session
int set_env_var(char **args) {
  int overwrite = 1;  
  //ensure the correct number of args is provided
  if (args[1] == NULL) {
    fprintf(stderr, "CSEShell: expected an arg for the \"setenv\" command: KEY=VALUE\n");
    return 1;
  }

  //find the "=" character in the argument
  char *delimiter = strchr(args[1], '=');
  if (delimiter == NULL){
    fprintf(stderr, "CSEShell: Invalid format for the \"setenv\" command. Expected format: KEY=VALUE\n");
    return 1;
  }

  //Split the arg into key and value 
  *delimiter = '\0'; // Replace '=' with null terminator to split the string
  char *key = args[1];
  char *value = delimiter + 1;

  if (setenv(key, value, overwrite) != 0) { 
    //setenv return 0 on success and -1 on failure
    perror("setenv");
    return 1;
  }
    // Print confirmation, getenv retrieve modified value of env var
    printf("Environment variable '%s' set to '%s'\n ", args[1], getenv(args[1]));

    return 0;
}  

int unset_env_var(char **args){
  if ( unsetenv(args[1]) != 0 ) {
    perror("unsetenv");
    return 1;
  }
  // Print confirmation
  printf("Environment variable '%s' removed\n", args[1] );

  return 0;  
}

