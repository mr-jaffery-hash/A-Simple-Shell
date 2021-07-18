#include <unistd.h>
#include <stdio.h>      /* printf*/
#include <stdlib.h>     /* getenv*/
#include <iostream>		/*cout*/
#include <cstring>		/*strlen*/
#include <string>		/*getline*/
#include <string.h>		/*strtok*/
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <bits/stdc++.h>

using namespace std;

int inputCheck, outputCheck;
char* inputfilename;
bool backgroundProcessCheck=false;
char* FindCommand(char* command) {
	char* path;
	path = getenv("PATH");
	int commandlength = strlen(command);
	int length = strlen(path);
	int index = 0;
	int size = 30;
	char* shortpath = new char[size];
	for (int i = 0; i < length; i++) {
		while (path[i] != ':' && path[i] != '\0') {
			if (index < size) {
				shortpath[index] = path[i];
				i++;
				index++;
			}
			else {
				int oldsize = size;
				size = size + size;
				char* newptr = shortpath;
				shortpath = new char[size];
				for (int i = 0; i < oldsize; i++) {
					shortpath[i] = newptr[i];
				}
				shortpath[index] = path[i];
				i++;
				index++;
			}
		}
		shortpath[index] = '/';
		index++;
		for (int j = 0; j < commandlength; j++) {
			if (index < size) {
				shortpath[index] = command[j];
				index++;
			}
			else {
				int oldsize = size;
				size = size + size;
				char* newptr = shortpath;
				shortpath = new char[size];
				for (int i = 0; i < oldsize; i++) {
					shortpath[i] = newptr[i];
				}
				shortpath[index] = command[j];
				index++;
			}
		}
		shortpath[index] = '\0';
		index++;
		if (access(shortpath, F_OK) == 0) {
			return shortpath;
		}
		index = 0;
	}
	return NULL;
}
void tokenize(char**& token, string command, char* str) {
	int size = 0;
	for (int i = 0; i < command.length(); i++) {
		if (command[i] == ' ') {
			size++;
		}
	}
	size++;
	int s = size;
	token = new char* [size + 1];
	token[size] = nullptr;
	char* temp = new char[100];
	size = 0;
	int tokenNumber = 0;
	for (int i = 0; i < command.length(); i++) {
		while (command[i] != ' ' && command[i] != '\0') {
			temp[size] = command[i];
			size++;
			i++;
		}
		token[tokenNumber] = new char[size + 1];
		for (int j = 0; j < size; j++) {
			token[tokenNumber][j] = temp[j];
		}
		token[tokenNumber][size] = '\0';
		tokenNumber++;
		size = 0;
	}
}
void execute(string commandAndParameters) {
	int size = 30;
	int index = 0;
	char* cap = new char[commandAndParameters.length() + 1];
	strcpy(cap, commandAndParameters.c_str());
	char* command = new char[size];
	for (int i = 0; cap[i] != ' ' && cap[i] != '\0'; i++) {
		if (index < size) {
			command[index] = cap[i];
			index++;
		}
		else {
			int oldsize = size;
			size = size + size;
			char* newptr = command;
			command = new char[size];
			for (int i = 0; i < oldsize; i++) {
				command[i] = newptr[i];
			}
			command[index] = cap[i];
			index++;
		}
	}
	command[index] = '\0';
	if (FindCommand(command)) {
		char* exec = FindCommand(command);
		char** myargs = nullptr;
		tokenize(myargs, commandAndParameters, exec);
		execv(exec, myargs);
	}
	else {
		cout << "Command Not Found" << endl;
		return;
	}
}
void parse(char* fullString, char**&seperatedCommands, int (&delPosition)[10], int&noOfDelCount, int& NumberOfPipes, int& NumberOfCommands) {
	char* pch;
	noOfDelCount = 0;
	NumberOfPipes = 0;
	NumberOfCommands = 0;
	inputCheck=0;
	outputCheck=0;
	for (int i = 0; i < strlen(fullString); i++) {
		if (fullString[i] == '|') {
			delPosition[noOfDelCount] = 0;
			noOfDelCount++;
			NumberOfPipes++;
		}
		else if (fullString[i] == '<') {
			delPosition[noOfDelCount] = 1;
			noOfDelCount++;
			inputCheck=1;
		}
		else if (fullString[i] == '>') {
			delPosition[noOfDelCount] = 2;
			noOfDelCount++;
			outputCheck=1;
		}
		else if (fullString[i] == '&'){
			backgroundProcessCheck=true;
		}
	}
	seperatedCommands = new char* [noOfDelCount + 1];
	int seperatedCommandsIndex = 0;
#pragma warning(suppress : 4996)
	pch = strtok(fullString, "|<>");
	while (pch != NULL){
		seperatedCommands[seperatedCommandsIndex] = new char[strlen(pch)];
		for (int i = 0; i < strlen(pch); i++) {
			seperatedCommands[seperatedCommandsIndex][i] = pch[i];
		}
		seperatedCommandsIndex++;
#pragma warning(suppress : 4996)
		pch = strtok(NULL, "|<>");
	}
	NumberOfCommands=seperatedCommandsIndex;
	int delimCheckCount = 0;
	//int readorwrite = 0;	
	if (noOfDelCount == 0) {
		int fd=fork();
		if(fd==0){
		execute(seperatedCommands[0]);
	}	
		else if(fd>0){
			wait(NULL);
		}	
	}
}
void doTheThing(char**allCommands, int NumberOfSigns,int NumberOfPipes,int NumberOfCommands){
	int signsIndex=0;
	if(NumberOfCommands==1){
		return;
	}
	int pipefds[NumberOfPipes*2];
		/* parent creates all needed pipes at the start */
	for( int i = 0; i < NumberOfPipes; i++ ){
	    if( pipe(pipefds + i*2) < 0 ){
		exit;
	    }
	}
	//commandc = 0
	char* inputFile;
	int command=0;
	bool inputTaken=true;
	int savestdout = dup(1);
	if(inputCheck==1){
		inputfilename=new char[strlen(allCommands[1])];
		for(int i=0; i<strlen(allCommands[1]); i++){	
				inputfilename[i]=allCommands[1][i];
		}
		for(int i=1; i<NumberOfCommands-1;i++){
			allCommands[i]=new char[strlen(allCommands[i+1])];
			for(int j=0; j<strlen(allCommands[i+1]); j++){
				allCommands[i][j]=allCommands[i+1][j];
			}
		}
		NumberOfCommands--;
	}
	if(outputCheck==1){
		NumberOfCommands--;
	}
	while(command!=NumberOfCommands){
	    int pid = fork();
	    if( pid == 0 ){
		//if it is first command and we have to take input from the file
		if(command==0){
			if(inputCheck==1){
				int in = open(inputfilename, O_RDONLY);
				dup2(in, 0);
				close(in);
			}
		}
		//get the input from the previous commands if it is not the first command
		if(command!=0){
		    if( dup2(pipefds[(command-1)*2], 0) < 0 ){
			perror and exit;
		    }
		}
		//direct the output to the next pipe if it is not the last command
		if(command!=NumberOfCommands){
		    if( dup2(pipefds[command*2+1], 1) < 0 ){
			perror and exit;
		    }
		}
		//if it is the last command check where to direct the output
		if(command==NumberOfCommands-1){
			if(dup2(savestdout, 1)<0)
				perror and exit;
			if(outputCheck==1){
				int out = open(allCommands[command+ 1], O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
				dup2(out, 1);
				close(out);
			}
		}
		//close all pipes
		for( int i = 0; i < 2 * NumberOfCommands; i++ ){
	    		close( pipefds[i] );
		}
		execute(allCommands[command]);
	    } 
	    else if( pid < 0 ){
		perror and exit;
	    }
	    //wait(NULL);
	    command++;
	}
	//closing all the pipes in parent
	for(int i = 0; i < 2 * NumberOfCommands; i++ ){
	    close( pipefds[i] );
	}
	//if '&' was recieved in the command the do the processing in the background
	if(backgroundProcessCheck==false)
		wait(NULL);
}
void shell_display(){
	system("clear");
		cout<<"ENTER -1 TO EXIT"<<endl;
	    cout<<"\n\n\n\n******************"
		"***************************************";
	cout<<"\n\n\t****SHELL-19L-0910****";
	cout<< "\n\n\n1) Executes commands having multiple pipes and filters (program names). \n2) Support for I/O redirection is provided. \n3) If user appends string with '&' then the command executes in the background.";
	 cout<<"\n\n\n*******************"
		"**************************************";
	    char* username = getenv("USER");
	    printf("\n\n\nsystem: @%s", username);
	    cout<<"\n";
	   sleep(1);
	   system("clear");
}
void myShell() {
	string commandAndParameters;
	int NumberOfSigns;
	int NumberOfPipes;
	int NumberOfCommands;
	char**allCommands;
	int signs[10];
	shell_display();
	cout << endl << "L190910_Assignement 2: Enter Command: ";
	getline(cin, commandAndParameters);
	char* stringToPass = new char[commandAndParameters.length()+1];
	for (int i = 0; i < commandAndParameters.length(); i++) {
		stringToPass[i] = commandAndParameters[i];
	}
	stringToPass[commandAndParameters.length()] = '\0';
	for(int i=0; i<10; i++){
		signs[i]=-1;
	}
	parse(stringToPass,allCommands,signs,NumberOfSigns, NumberOfPipes, NumberOfCommands);
	doTheThing(allCommands, NumberOfSigns, NumberOfPipes, NumberOfCommands);
}
int main() {
	//while(true)
	myShell();
}
