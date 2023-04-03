# costeanShell
This is a custom shell implementation that provides several built-in commands such as "dir", "rm", and "uniq". These commands have been implemented specifically for this shell and are not standard Unix commands.

The "dir" command is used to list the contents of a directory, while "rm" is used to remove files or directories. The "uniq" command removes duplicate lines from a text file and can also display only the lines that are duplicated or the unique lines.

In addition to the built-in commands, this shell can also execute external commands. Simply enter the name of the command followed by any necessary arguments. For example, if you want to execute the "ls" command, simply type "ls" followed by any arguments.

To use this shell, simply compile the code using the provided command: "gcc main.c -lreadline -o terminal.exe". Then run the compiled executable, and you will be greeted with a shell prompt. From there, you can enter commands and execute them.

Note that this shell has been designed for Unix-like systems and may not work properly on Windows systems.
