# Operating-Systems
This repository contains the final assignment for the Academic Courses "Operating Systems" taught in the Fall of 2019-2020 in Aristotle University of Thessaloniki - Electrical and Computer Engineering. 

The purpose of this assignment is the familiarity and the better understanding of the **Linux Shell**, **Processes** in Linux, **bash**, **Makefile** and ultimately **C programming**. The assignment's goal was to create our own **Shell** in C language. The shell that we had to create should be able to run multiple types of commands, handle **redirection** and also **piping**.  In general,

* Improper space handle.
* Redirecting input with '<' handle.
* Redirecting output with '>' handle.
* Pipelining with '|' handle.
* Execute a series of commands with respect to their _exit status_ with '&&' and ';' handle.

##### Supported modes

* **Interactive**, when the program is executed without further arguments.

  * > ./myshell

* **Batch**, when only 1 argument-(filename) is given.

  * > ./myshell [batchfile_name]

##### Valid Instructions

* pwd ; ls -l ; echo "Hello World" ; ps -a
* pwd && ls -l && echo "Hello World" ; ps -a
* pwd ; ls -l && echo "Hello World" ; ps -a
* pwd && ls -l && echo "Hello World" && ps -a
* cat file1.txt > file2.txt
* cat < file1.txt > file2.txt
* cat file.txt | wc -l > file2.txt && cat file2.txt && rm -f file2.txt

##### Invalid Instructions

* Instruction(s) with 1, 3 or more sequential ampersands. (i.e _pwd & ls_ or _pwd &&& ls_)
* Instruction(s) with 2 or more sequential semicolons. (i.e _pwd ;; ls_)
* Instruction(s) with more than 512 characters.
* Instructions which their total number is more than 256.
* Built-in Instructions.
* **NULL** commands.

***

## Execution

In order to execute the code, the following commands are necessary:

```bash
make all
```

and then you can run it using:

```bash
./bin/myshell [batchfile_name]
```

where the [] means that the parameter is optional.

---

## Further Work

While the project was really enjoying and I learnt so many things about **C programming** and **Linux**, the project will not be maintained after it's completion due to the lack of time. It may contain some bugs that I was not able to find out so if anyone ever gets help from my project please inform me about errors that could be fixed!

