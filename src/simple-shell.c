// ############################## INCLUDE SECTION ######################################
#include <stdio.h>  // printf(), fgets()
#include <string.h> // strtok(), strcmp(), strdup()
#include <stdlib.h> // free()
#include <unistd.h> // fork()
#include <sys/types.h>
#include <sys/wait.h> // waitpid()
#include <sys/stat.h>
#include <fcntl.h> // open(), creat(), close()
#include <time.h>
#include <errno.h>
#include <limits.h> // FILENAME_MAX
#include <libgen.h> 
#include <linux/limits.h>
// ######################################################################################

// ############################## DEFINE SECTION ########################################
#define MAX_LINE_LENGTH 1024
#define BUFFER_SIZE 64
#define REDIR_SIZE 2
#define PIPE_SIZE 3
#define MAX_HISTORY_SIZE 128
#define MAX_COMMAND_NAME_LENGTH 128

#define PROMPT_FORMAT "%F %T "
#define PROMPT_MAX_LENGTH 30

#define TOFILE_DIRECT ">"
#define APPEND_TOFILE_DIRECT ">>"
#define FROMFILE "<"
#define PIPE_OPT "|"
// ######################################################################################


// ############################## GLOBAL VARIABLES SECTION ##############################
int running = 1;
char API_KEY[] = ""; // Perplexity API key
// ######################################################################################


/**
 * @param None
 * @return None
 */
void init_shell() {
    printf("  ____              _     ___  ____ \n"); 
    printf(" / ___|  __ _ _   _| | __/ _ \\/ ___| \n");
    printf(" \\___ \\ / _` | | | | |/ / | | \\___ \\ \n");
    printf("  ___) | (_| | |_| |   <| |_| |___) |\n");
    printf(" |____/ \\__,_|\\__,_|_|\\_\\___/|____/  \n");
    printf("                                     \n");
    char *username = getenv("USER");
    printf("\n\n\nCurrent user: @%s", username);
    printf("\n");
}

char *get_current_dir() {
    char cwd[FILENAME_MAX];
    char*result = getcwd(cwd, sizeof(cwd));
    return result;
}

/**
 * @param None
 * @return a prompt string
 */
char *prompt() {
    static char *_prompt = NULL;
    time_t now;
    struct tm *tmp;
    size_t size;

    if (_prompt == NULL) {
        _prompt = malloc(PROMPT_MAX_LENGTH * sizeof(char));
        if (_prompt == NULL) {
            perror("Error: Unable to locate memory");
            exit(EXIT_FAILURE);
        }
    }

    
    now = time(NULL);
    if (now == -1) {
        fprintf(stderr, "Error: Cannot get current timestamp");
        exit(EXIT_FAILURE);
    }

    
    tmp = localtime(&now);
    if (tmp == NULL) {
        fprintf(stderr, "Error: Cannot identify timestamp");
        exit(EXIT_FAILURE);
    }

    
    size = strftime(_prompt, PROMPT_MAX_LENGTH, PROMPT_FORMAT, tmp);
    if (size == 0) {
        fprintf(stderr, "Error: Cannot convert time to string");
        exit(EXIT_FAILURE);
    }
    
    char* username = getenv("USER");
    strncat(_prompt, username, strlen(username));
    return _prompt;
}

/**
 
 * @param None
 * @return None
 */
void error_alert(char *msg) {
    printf("%s %s\n", prompt(), msg);
}

void remove_end_of_line(char *line) {
    int i = 0;
    while (line[i] != '\n') {
        i++;
    }

    line[i] = '\0';
}


void read_line(char *line) {
    char *ret = fgets(line, MAX_LINE_LENGTH, stdin);

    
    remove_end_of_line(line);

    
    if (strcmp(line, "exit") == 0 || ret == NULL || strcmp(line, "quit") == 0) {
        exit(EXIT_SUCCESS);
    }
}

// Parser


void parse_command(char *input_string, char **argv, int *wait) {
    int i = 0;

    while (i < BUFFER_SIZE) {
        argv[i] = NULL;
        i++;
    }

    // If - else cho gọn tí
    *wait = (input_string[strlen(input_string) - 1] == '&') ? 0 : 1; // Nếu có & thì wait = 0, ngược lại wait = 1
    input_string[strlen(input_string) - 1] = (*wait == 0) ? input_string[strlen(input_string) - 1] = '\0' : input_string[strlen(input_string) - 1];
    i = 0;
    argv[i] = strtok(input_string, " ");

    if (argv[i] == NULL) return;

    while (argv[i] != NULL) {
        i++;
        argv[i] = strtok(NULL, " ");
    }

    argv[i] = NULL;
}

/**
 * @description:
 * @param  argv 
 * @return 
 */
int is_redirect(char **argv) {
    int i = 0;
    while (argv[i] != NULL) {
        if (strcmp(argv[i], TOFILE_DIRECT) == 0 || strcmp(argv[i], APPEND_TOFILE_DIRECT) == 0 || strcmp(argv[i], FROMFILE) == 0) {
            return i;
        }
        i = -~i; 
    }
    return 0; 
}

/**
 * @description:
 * @param argv 
 * @return
 */
int is_pipe(char **argv) {
    int i = 0;
    while (argv[i] != NULL) {
        if (strcmp(argv[i], PIPE_OPT) == 0) {
            return i;
        }
        i = -~i; 
    }
    return 0; 
}

void parse_redirect(char **argv, char **redirect_argv, int redirect_index) {
    redirect_argv[0] = strdup(argv[redirect_index]);
    redirect_argv[1] = strdup(argv[redirect_index + 1]);
    argv[redirect_index] = NULL;
    argv[redirect_index + 1] = NULL;
}
void parse_pipe(char **argv, char **child01_argv, char **child02_argv, int pipe_index) {
    int i = 0;
    for (i = 0; i < pipe_index; i++) {
        child01_argv[i] = strdup(argv[i]);
    }
    child01_argv[i++] = NULL;

    while (argv[i] != NULL) {
        child02_argv[i - pipe_index - 1] = strdup(argv[i]);
        i++;
    }
    child02_argv[i - pipe_index - 1] = NULL;
}

// Execution

/**
 * @description: 
 * @param argv 
 * @return none
 */
void exec_child(char **argv) {
    if (execvp(argv[0], argv) < 0) {
        fprintf(stderr, "Error: Failed to execte command.\n");
        exit(EXIT_FAILURE);
    }
}

/**
 * @description 
 * @param argv 
 * @return none
 */
void exec_child_overwrite_from_file(char **argv, char **dir) {
    // osh>ls < out.txt
    int fd_in = open(dir[1], O_RDONLY);
    if (fd_in == -1) {
        perror("Error: Redirect input failed");
        exit(EXIT_FAILURE);
    }

    dup2(fd_in, STDIN_FILENO);

    if (close(fd_in) == -1) {
        perror("Error: Closing input failed");
        exit(EXIT_FAILURE);
    }
    exec_child(argv);
}

/**
 * @description 
 * @param argv 
 * @return none
 */
void exec_child_overwrite_to_file(char **argv, char **dir) {
    // osh>ls > out.txt

    int fd_out;
    fd_out = creat(dir[1], S_IRWXU);
    if (fd_out == -1) {
        perror("Error: Redirect output failed");
        exit(EXIT_FAILURE);
    }
    dup2(fd_out, STDOUT_FILENO);
    if (close(fd_out) == -1) {
        perror("Error: Closing output failed");
        exit(EXIT_FAILURE);
    }

    exec_child(argv);
}

/**
 * @description 
 * @param argv 
 * @return none
 */
void exec_child_append_to_file(char **argv, char **dir) {
    // osh>ls >> out.txt
    int fd_out;
    if (access(dir[0], F_OK) != -1) {
        fd_out = open(dir[0], O_WRONLY | O_APPEND);
    }
    if (fd_out == -1) {
        perror("Error: Redirect output failed");
        exit(EXIT_FAILURE);
    }
    dup2(fd_out, STDOUT_FILENO);
    if (close(fd_out) == -1) {
        perror("Error: Closing output failed");
        exit(EXIT_FAILURE);
    }
    exec_child(argv);
}

/**
 * @description 
 * @param argv_i
 * @return none
 */
void exec_child_pipe(char **argv_in, char **argv_out) {
    int fd[2];
    // p[0]: read end
    // p[1]: write end
    if (pipe(fd) == -1) {
        perror("Error: Pipe failed");
        exit(EXIT_FAILURE);
    }

    //child 1 exec input from main process
    //write to child 2
    if (fork() == 0) {
        dup2(fd[1], STDOUT_FILENO);
        close(fd[0]);
        close(fd[1]);
        exec_child(argv_in);
        exit(EXIT_SUCCESS);
    }

    //child 2 exec output from child 1
    //read from child 1
    if (fork() == 0) {
        dup2(fd[0], STDIN_FILENO);
        close(fd[1]);
        close(fd[0]);
        exec_child(argv_out);
        exit(EXIT_SUCCESS);
    }

    close(fd[0]);
    close(fd[1]);
    wait(0);
    wait(0);    
}

/**
 * @description 
 * @param 
 * @return
 */
void exec_parent(pid_t child_pid, int *bg) {}

// History

void set_prev_command(char *history, char *line) {
    strcpy(history, line);
}


char *get_prev_command(char *history) {
    if (history[0] == '\0') {
        fprintf(stderr, "No commands in history\n");
        return NULL;
    }
    return history;
}


/*
  Function Declarations for builtin shell commands:
 */
int simple_shell_cd(char **args);
int simple_shell_help(char **args);
int simple_shell_exit(char **args);
int simple_shell_sauko(char **args);
int simple_shell_saukobot(char **args);
void exec_command(char **args, char **redir_argv, int wait, int res);

// List of builtin commands
char *builtin_str[] = {
    "cd",
    "help",
    "exit",
    "sauko",
    "saukobot"
};

// Corresponding functions.
int (*builtin_func[])(char **) = {
    &simple_shell_cd,
    &simple_shell_help,
    &simple_shell_exit,
    &simple_shell_sauko,
    &simple_shell_saukobot
};

int simple_shell_num_builtins() {
    return sizeof(builtin_str) / sizeof(char *);
}


/**
 * @description cd (change directory) 
 * @param argv 
 * @return 0 
 */
int simple_shell_cd(char **argv) {
    if (argv[1] == NULL) {
        fprintf(stderr, "Error: Expected argument to \"cd\"\n");
    } else {
        // Change the process's working directory to PATH.
        if (chdir(argv[1]) != 0) {
            perror("Error: Error when change the process's working directory to PATH.");
        }
    }
    return 1;
}


/** -----------------------------------------------------------------------------------------
 * Función para ejecutar aplicaciones bajo el comando sauko
 * @param args: [sauko, aplicación, ...]
 * @return 1 si se ejecutó correctamente
 */
int simple_shell_sauko(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "Uso: sauko <aplicación> [parámetros]\n");
        return 1;
    }

    char *app_name = args[1];
    char app_path[PATH_MAX];

    if (strcmp(app_name, "tetris") == 0) {
        // Obtener la ruta absoluta del ejecutable actual
        char exe_path[PATH_MAX];
        ssize_t len = readlink("/proc/self/exe", exe_path, sizeof(exe_path)-1);
        if (len == -1) {
            perror("No se pudo obtener la ruta del ejecutable");
            return 1;
        }
        exe_path[len] = '\0';
        // Obtener el directorio donde está el ejecutable (src)
        char *dir = dirname(exe_path);
        // Construir la ruta absoluta al script tetris.sh
        snprintf(app_path, sizeof(app_path), "%s/Tetris/tetris.sh", dir);
    } else {
        fprintf(stderr, "Aplicación no reconocida: %s\n", app_name);
        return 1;
    }

    // Ejecutar la aplicación
    pid_t pid = fork();
    if (pid == 0) {
        // Proceso hijo: ejecuta el script con los argumentos (empezando desde app_path)
        char *argv_exec[] = {app_path, NULL};
        if (execv(app_path, argv_exec) == -1) {
            perror("sauko");
        }
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
    } else {
        perror("Error en fork");
    }
    return 1;
}


/**
 * Ejecuta el comando saukobot [prompt] usando la API de Perplexity.
 * @param args: [saukobot, prompt, ...]
 * @return 1 si se ejecutó correctamente
 */
int simple_shell_saukobot(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "Uso: saukobot <prompt>\n");
        return 1;
    }

    char prompt[1024] = "";
    for (int i = 1; args[i] != NULL; i++) {
        strcat(prompt, args[i]);
        if (args[i+1] != NULL) strcat(prompt, " ");
    }

    char json_data[2048];
    snprintf(
        json_data, sizeof(json_data),
        "{"
            "\"model\": \"sonar-pro\","
            "\"messages\": ["
                "{\"role\": \"system\", \"content\": \"Eres un asistente experto en el shell SaukOS. Solo responde preguntas sobre el funcionamiento del shell, sus comandos internos (cd, help, exit, sauko, saukobot) y las aplicaciones disponibles a través de sauko. Si la pregunta no está relacionada con el shell SaukOS o sus comandos, responde: 'Solo puedo responder preguntas sobre el shell SaukOS y sus comandos.'\"},"
                "{\"role\": \"user\", \"content\": \"%s\"}"
            "]"
        "}", prompt
    );

    FILE *tmp = fopen("/tmp/saukobot.json", "w");
    if (!tmp) {
        fprintf(stderr, "No se pudo crear archivo temporal\n");
        return 1;
    }
    fputs(json_data, tmp);
    fclose(tmp);

    char curl_cmd[4096];
    snprintf(
        curl_cmd, sizeof(curl_cmd),
        "curl --silent --location 'https://api.perplexity.ai/chat/completions' "
        "--header 'accept: application/json' "
        "--header 'content-type: application/json' "
        "--header 'Authorization: Bearer %s' "
        "--data-binary '@/tmp/saukobot.json' | jq -r '.choices[0].message.content'",
        API_KEY
    );

    FILE *fp = popen(curl_cmd, "r");
    if (fp == NULL) {
        fprintf(stderr, "Error al ejecutar curl\n");
        return 1;
    }

    char result[4096];
    int printed = 0;
    while (fgets(result, sizeof(result), fp) != NULL) {
        if (strcmp(result, "null\n") != 0) {
            printf("%s", result);
            printed = 1;
        }
    }
    pclose(fp);

    if (!printed) {
        printf("No se obtuvo respuesta válida de la API.\n");
    }

    remove("/tmp/saukobot.json");
    return 1;
}



//-----------------------------------------------------------------------------------------

int simple_shell_help(char **argv) {
    static char help_team_information[] =
        "OPERATING SYSTEMS PROJECT - SaukOS\n"
        "λ Team member λ\n"
        "2023241041 \t\tAndry Caceres\n"
        "2023803802 \t\tSlavitza Zvietcovich\n"
        "λ Descripción λ\n"
        "El shell SaukOS propuesto desarrolado por Slavitza es parte del proyecto final de Sistemas Operativos.\n"
        "This program was written entirely in C as assignment"
        "\n"
        "\nUsage help command. Type help [command name] for help/ more information.\n"
        "Options for [command name]:\n"
        "cd <directory name>\t\t\tDescription: Change the current working directory.\n"
        "sauko <application name> [args]\tDescription: Run a SaukOS application.\n"
        "help <command name> \t\t\tDescription: Show help information for a specific command.\n"

        "exit              \t\t\tDescription: Exit to SaukOS.\n";
    static char help_cd_command[] = "HELP CD COMMAND\n";
    static char help_exit_command[] = "HELP EXIT COMMAND\n";

    if (strcmp(argv[0], "help") == 0 && argv[1] == NULL) {
        printf("%s", help_team_information);
        return 0;
    }

    if (strcmp(argv[1], "cd") == 0) {
        printf("%s", help_cd_command);
    } else if (strcmp(argv[1], "exit") == 0) {
        printf("%s", help_exit_command);
    } else {
        printf("%s", "Error: Too much arguments.");
        return 1;
    }
    return 0;
}


int simple_shell_exit(char **args) {
    running = 0;
    return running;
}

/**
 * @description Hàm thoát
 * @param 
 * @return
 */
int simple_shell_history(char *history, char **redir_args) {
    char *cur_args[BUFFER_SIZE];
    char cur_command[MAX_LINE_LENGTH];
    int t_wait;

    if (history[0] == '\0') {
        fprintf(stderr, "No commands in history\n");
        return 1;
    }
    strcpy(cur_command, history);
    printf("%s\n", cur_command);
    parse_command(cur_command, cur_args, &t_wait);
    int res = 0;
    exec_command(cur_args, redir_args, t_wait, res);
    return res;
}



int simple_shell_redirect(char **args, char **redir_argv) {
    // printf("%s", "Executing redirect\n");
    int redir_op_index = is_redirect(args);
    // printf("%d", redir_op_index);
    if (redir_op_index != 0) {
        parse_redirect(args, redir_argv, redir_op_index);
        if (strcmp(redir_argv[0], ">") == 0) {
            exec_child_overwrite_to_file(args, redir_argv);
        } else if (strcmp(redir_argv[0], "<") == 0) {
            exec_child_overwrite_from_file(args, redir_argv);
        } else if (strcmp(redir_argv[0], ">>") == 0) {
            exec_child_append_to_file(args, redir_argv);
        }
        return 1;
    }
    return 0;
}


int simple_shell_pipe(char **args) {
    int pipe_op_index = is_pipe(args);
    if (pipe_op_index != 0) {  
        // printf("%s", "Exec Pipe");
        char *child01_arg[PIPE_SIZE];
        char *child02_arg[PIPE_SIZE];   
        parse_pipe(args, child01_arg, child02_arg, pipe_op_index);
        exec_child_pipe(child01_arg, child02_arg);
        return 1;
    }
    return 0;
}

/**
 * @description Hàm thực thi lệnh
 * @param 
 * @return
 */
void exec_command(char **args, char **redir_argv, int wait, int res) {
    
    for (int i = 0; i < simple_shell_num_builtins(); i++) {
        if (strcmp(args[0], builtin_str[i]) == 0) {
            (*builtin_func[i])(args);
            res = 1;
        }
    }

    // Chưa thực thi builtin commands
    if (res == 0) {
        int status;

        // Tạo tiến trình con
        pid_t pid = fork();
        if (pid == 0) {
            // Child process
            if (res == 0) res = simple_shell_redirect(args, redir_argv);
            if (res == 0) res = simple_shell_pipe(args);
            if (res == 0) execvp(args[0], args);
            exit(EXIT_SUCCESS);

        } else if (pid < 0) {
            perror("Error: Error forking");
            exit(EXIT_FAILURE);
        } else { // Thực thi chạy nền
            // Parent process
            // printf("[LOGGING] Parent pid = <%d> spawned a child pid = <%d>.\n", getpid(), pid);
            if (wait == 1) {
                waitpid(pid, &status, WUNTRACED); // 
            }
        }
    }
}

int main(void) {
    char *args[BUFFER_SIZE];


    char line[MAX_LINE_LENGTH];


    char t_line[MAX_LINE_LENGTH];


    char history[MAX_LINE_LENGTH] = "No commands in history";


    char *redir_argv[REDIR_SIZE];


    int wait;


    init_shell();
    int res = 0;

    while (running) {
        printf("%s:%s> ", prompt(), get_current_dir());
        fflush(stdout);


        read_line(line);


        strcpy(t_line, line);


        parse_command(line, args, &wait);


        if (strcmp(args[0], "!!") == 0) {
            res = simple_shell_history(history, redir_argv);
        } else {
            set_prev_command(history, t_line);
            exec_command(args, redir_argv, wait, res);
        }
        res = 0;
    }
    return 0;
}