# A-Simple-Shell

Designed and implemented a simple, interactive shell program that prompts the user for a command, parses the command, and then executes it with a child process. The shell supports I/O redirection, multiple commands and background execution. In the solution execv() command is used, which means that it will have to read the PATH environment, then search each directory in the PATH for the command file name that appears on the command line.
