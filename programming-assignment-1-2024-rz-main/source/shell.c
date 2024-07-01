// Include the shell header file for necessary constants and function declarations
#include "shell.h"
#include <pwd.h>
#include <time.h>
//#include <sys/time.h>



char *color_code = NULL; // Global variable to store the current color

/*
// Function to get memory usage in KB
long get_memory_usage_kb() {
    FILE* fp = fopen("/proc/self/status", "r");
    if (fp == NULL) {
        fprintf(stderr, "Failed to open /proc/self/status\n");
        return -1;
    }

    char line[128];
    long memory_kb = -1;
    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, "VmRSS:", 6) == 0) {
            sscanf(line, "VmRSS: %ld", &memory_kb);
            break;
        }
    }
    fclose(fp);
    return memory_kb;
}

// Function to get disk I/O statistics (reads and writes in KB/s)
int get_disk_io_stats_kb_per_sec() {
    FILE* fp = popen("iostat -d -k 1 2 | awk 'NR==7 {print $2, $3}'", "r");
    if (fp == NULL) {
        fprintf(stderr, "Failed to execute iostat\n");
        return -1;
    }

    int reads_kb_per_sec = -1, writes_kb_per_sec = -1;
    fscanf(fp, "%d %d", &reads_kb_per_sec, &writes_kb_per_sec);
    pclose(fp);

    return reads_kb_per_sec + writes_kb_per_sec;
} 
*/

// Set shell theme (color scheme)
void set_shell_theme(char *theme) {
    if (strcmp(theme, "brightwhite") == 0) {
        color_code = ("\033[38;5;15m"); // Set text to bright white for dark theme
    } else if (strcmp(theme, "black") == 0) {
        color_code = ("\033[38;5;0m"); // Set text to black for light theme
    } else if (strcmp(theme, "blue") == 0) {
        color_code = ("\033[38;5;4m"); // Set text to blue 
    } else if (strcmp(theme, "cyan") == 0) {
        color_code = ("\033[38;5;6m"); // Set text to cyan 
    } else if (strcmp(theme, "green") == 0) {
        color_code = ("\033[38;5;3m"); // Set text to green 
    } else if (strcmp(theme, "red") == 0) {
        color_code = ("\033[38;5;1m"); // Set text to red
    } else if (strcmp(theme, "magenta") == 0) {
        color_code = ("\033[38;5;5m"); // Set text to magenta 
    } else if (strcmp(theme, "yellow") == 0) {
        color_code = ("\033[38;5;3m"); // Set text to yellow 
    }     
  
     else {
        color_code = ("\033[0m"); // Reset text to default color
    }
    //printf("%s%s\033[0m", color_code, theme);
    // Optionally, set the SHELL_THEME environment variable to remember the setting
    setenv("SHELL_THEME", theme, 1);
}

int shell_set_theme(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "CSEShell: expected argument to \"settheme\"\n");
    } else {
        set_shell_theme(args[1]);
    }
    return 1;
}


// Function to get the current time as a string
void get_current_time(char *buffer, size_t size) {
    time_t rawtime;
    struct tm *timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", timeinfo);
}

/* Xavier's Portion: Third Task */
#define MAX_LINE_LENGTH 1024

void process_rc_file(const char *filepath) {
    FILE *file = fopen(filepath, "r");
    if (file == NULL) {
        perror("Failed to open .rc file");
        return;
    }

    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file)) {
        // Remove the newline character at the end
        line[strcspn(line, "\n")] = 0;

        if (strncmp(line, "PATH=", 5) == 0) {
            // Set the PATH environment variable
            if (putenv(line) != 0) {
                perror("Failed to set PATH");
            }
        } else if (strlen(line) > 0) {
            // Execute the command
            pid_t pid = fork();
            if (pid == -1) {
                perror("fork failed");
                exit(EXIT_FAILURE);
            } else if (pid == 0) {
                // Child process
                char *args[] = {"/bin/sh", "-c", line, NULL};
                execvp(args[0], args);
                // If execvp fails
                perror("execvp failed");
                _exit(EXIT_FAILURE);
            } else {
                // Parent process
                int status;
                waitpid(pid, &status, 0);
            }
        }
    }

    fclose(file);
}
/* Xavier's Portion: End of third task*/



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
      printf("%sCommand is too long, unable to process\n", color_code);
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
    char cwd[8192];
    char prompt[1024];
    char current_time[64];
    struct passwd *pw;
    char user[64];
    char *color_env;
    int color_code = 32; // Default color code (green)

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
  //printf("$$ ");  // Print the shell prompt

  // Get the current time
  get_current_time(current_time, sizeof(current_time));

  // Get the current user
  pw = getpwuid(getuid());
  if (pw == NULL) {
    perror("getpwuid");
    exit(EXIT_FAILURE);}

  strncpy(user, pw->pw_name, sizeof(user) - 1);
  user[sizeof(user) - 1] = '\0'; // Ensure null termination

  if (getcwd(cwd, sizeof(cwd)) != NULL) {
    // Construct the prompt string
    snprintf(prompt, sizeof(prompt), "\033[1;34m%s\033[0m \033[1;32m%s@%s\033[0m \033[1;31m%s\033[0m> ", current_time, user, cwd, "CSEshell");
  } else {
      perror("getcwd");
      exit(EXIT_FAILURE);
  }

   // Print the prompt
    printf("%s", prompt);
}

int shell_launch(char **args) {
  // Define an array to hold the command and its arguments
  char *cmd[MAX_ARGS];
  int child_status;
  pid_t pid;

  // Formulate the full path of the command to be executed
    char full_path[PATH_MAX];
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != NULL)
    {
      snprintf(full_path, sizeof(full_path), "%s/bin/%s", cwd, args[0]);
    }
    else
    {
      printf("%sFailed to get current working directory.", color_code);
      exit(1);
    }
  
    //fork a new process
    pid = fork();
  
    //check if fork is successful
    if (pid <0) {
      perror("Fork failed"); //print detailed error message
      exit(1); //exit the program with a non-zero status indicating failure
    }

    else if(pid==0) {
      //child successful
      execv(full_path, args);
  
    //----------rz added stuff to execute cd cmds so that path can be found---------------------
  
      //since execv returned, function likely was not found, check root
      snprintf(full_path, sizeof(full_path), "%s/bin/%s", getenv("PWD"), args[0]);
      if (execv(full_path, args) == -1) {
          // If execv fails, try execvp
          execvp(args[0], args);
    
    //----------rz added stuff to execute cd cmds so that path can be found---------------------
      }
      //if execvp returns, command execution has failed    
      if (execv(full_path, args) == -1 && execvp(args[0], args) == -1) {
      printf("%sCommand %s not found\n", color_code, args[0]);
      exit(0);
      }

        // Free the allocated memory for the command arguments before exiting
        for (int i = 0; args[i] != NULL; i++)
        {
          free(args[i]);
        }
        memset(cwd, '\0', sizeof(cwd)); // clear the cwd array

        return 0;
      }

      else {
        // Parent process
        // Wait for child process to complete
        wait(&child_status);
        // Display feedback
        /*if (WIFEXITED(child_status) && WEXITSTATUS(child_status) == 0) {
            printf("Command executed successfully\n");

            // Capture and display memory usage
            long memory_usage_kb = get_memory_usage_kb();
            if (memory_usage_kb != -1) {
                printf("Memory Usage: %ld KB\n", memory_usage_kb);
            }

            // Capture and display disk I/O
            int disk_io_kb_per_sec = get_disk_io_stats_kb_per_sec();
            if (disk_io_kb_per_sec != -1) {
                printf("Disk I/O: %d KB/s\n", disk_io_kb_per_sec);
            }*/
      } 


      
      return 1;

}



//------------------rz builtin cmds------------------// 

int shell_execute(char **args) {
    //----------rz added stuff to execute builtin cmds---------------------
    if (args[0] == NULL) {
    return 1;
    }
    // Check for built-in commands
    for (int i = 0; i< numOfBuiltinFunctions(); i++) {
      if (strcmp(args[0], builtin_commands[i]) == 0) {
        return (*builtin_command_func[i])(args);
       
      }

    }
 
    //----------added stuff to execute builtin cmds---------------------
    // If the command is not built-in, attempt to launch it
    return shell_launch(args);
}


int shell_cd(char **args)
{
  if (args[1] == NULL)
  {
    fprintf(stderr, "%sCommand not given. Type cd <command> \n", color_code);
  }
  else
  {
    // chdir() changes the current working directory of the calling process
    // to the directory specified in path.
    if (chdir(args[1]) != 0)
    { 
      perror("CSEShell");
    }
  }

  return 1;
}


/*  Prints out the usage and list of commands implemented */

int shell_help(char **args) {
  int i;

  printf("%sshellHelp is called! \n", color_code);
  printf("%sCSE Shell Interface\n", color_code);
  printf("%sType program names and arguments, and hit enter.\n", color_code);
  printf("%sThe following are built in:\n", color_code);

  for (i = 0; i < numOfBuiltinFunctions(); i++)
  {
    printf("%s  %s\n", color_code, builtin_commands[i]); //print all the commands that have been implemented in the shell program
  }

  return 1;
}


int shell_exit(char **args) {
// If the command is "exit", break out of the loop to terminate the shell
    exit(0);
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
      printf("%sType: cd directory_name to change directory to current directory of the shell\n", color_code);
      break;
    case 1:
      printf("%sType: help for supported commands\n", color_code);
      break;
    case 2:
      printf("%sType: exit to terminate the shell gracefully\n", color_code);
      break;
    case 3:
      printf("%sType: usage cd/help/exit \n", color_code);
      break;
    case 4:
      printf("%sType: env to  lists all registered env variables\n", color_code);
      break;
    case 5:
      printf("%sType: setenv ENV=VALUE to set a new env variable\n", color_code);
      break;
    case 6:
      printf("%sType: unsetenv ENV to remove this env from the list of env variables\n", color_code);
      break;
    case 7:
      printf("%sType: settheme to change the color theme of the shell\n", color_code);
      break;
  }

  return 1;

}


extern char **environ; 
int list_env(char **args) {
  //external var that holds all the env variable strings in the format 'char *environ[]'
  extern char **environ; 
  // Pointer to traverse through the environment variables
  char **env = environ;

  // Loop through all the environment variables until NULL is encountered
  while (*env) {
      printf("%s%s\n",color_code, *env); // *env to dereference the pointer to get the curr env var string
      env++;
  }

    return 1;
}

int set_env_var(char **args) {
  if (args[1] == NULL || args[2] == NULL) {
    fprintf(stderr, "please input setenv [KEY] [VALUE] \n");
  } else {
    if (setenv(args[1], args[2], 1) != 0) {
      perror("setenv");
    } else {
      printf("%sPATH set\n", color_code);
    }
  }
  return 1;
}


int unset_env_var(char **args){
  if (args[1] == NULL) {
  fprintf(stderr, "unsetenv: expected argument\n");
  } else {
    if ( unsetenv(args[1]) != 0 ) {
      perror("unsetenv");
      return 1;
  }
  }
  // Print confirmation
  printf("%sEnvironment variable '%s' removed\n", color_code, args[1] );

  return 1; 
}
  //------------------rz builtin cmds------------------//

  
// The main function where the shell's execution begins
int main(int argc, char **argv)
{
  /*struct timespec start, end;
  double cpu_time_used;

  //Record start time
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);

  // Record end time
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);

  // Calculate CPU time in milliseconds
  cpu_time_used = (end.tv_sec - start.tv_sec) * 1000.0; // sec to ms
  cpu_time_used += (end.tv_nsec - start.tv_nsec) / 1000000.0; // ns to ms

  // Display CPU time used
  printf("CPU time used: %.2f ms\n", cpu_time_used);*/

  // Execute .rc file
  process_rc_file(".cseshellrc");
  char *cmd[MAX_ARGS];
  int status = 3;
  //apply_efficiency_settings();

  //initialize();
  //handle_keyboard_shortcuts();
  //endwin(); // End ncurses mode

  // Set shell theme based on the SHELL_THEME environment variable
  char *theme = getenv("SHELL_THEME");
  if (theme != NULL)
  {
    set_shell_theme(theme);
  } else {
    set_shell_theme("default");
  }

  for (;;) {
  //Code to execute indefinitely

  // Code to execute indefinitely
  type_prompt();     // Display the prompt
  read_command(cmd); // Read a command from the user
  // Check if the command is empty
  if (cmd[0] == NULL || strcmp(cmd[0], "") == 0) {
      continue; // Skip to the next iteration of the loop
  }   

  if (strcmp(cmd[0], "exit") == 0)
  break;
 
  shell_execute(cmd);
 
  for (int i = 0; cmd[i] != NULL; i++) {
  free(cmd[i]); }
  } 
  
  return 0;
}
