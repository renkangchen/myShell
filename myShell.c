#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<dirent.h>
#include<pwd.h>
#include<wait.h>
#include<sys/types.h>
#include <readline/readline.h>
#include <readline/history.h>


//define the printf color
#define L_GREEN "\e[1;32m"
#define L_BLUE  "\e[1;34m"
#define L_RED   "\e[1;31m"
#define WHITE   "\e[0m"

#define TRUE 1
#define FALSE 0
#define DEBUG

char lastdir[100];
char command[BUFSIZ];
char argv[100][100];
char **argvtmp1;
char **argvtmp2;
int  argc;
int BUILTIN_COMMAND = 0;
int PIPE_COMMAND = 0;
//set the prompt
void set_prompt(char *prompt);
//analysis the command that user input
int analysis_command();
void builtin_command();
int do_command();
//print help information
void help();
void initial();
void init_lastdir();
void history_setup();
void history_finish();
void display_history_list();

int main(){
	char prompt[BUFSIZ];
	char *line;
	
	init_lastdir();
	history_setup();	
	while(1) {
		set_prompt(prompt);
		if(!(line = readline(prompt))) 
			break;
		if(*line)
			add_history(line);

		strcpy(command, line);
		//strcat(command, "\n");
		if(!(analysis_command())){
			//todo deal with the buff
			if(BUILTIN_COMMAND){
				builtin_command();		
			}//if
			else{
				do_command();
			}//else		
		}//if analysis_command
		initial();//initial 
	}//while
	history_finish();
	
	return 0;
}

//set the prompt
void set_prompt(char *prompt){
	char hostname[100];
	char cwd[100];
	char super = '#';
	//to cut the cwd by "/"	
	char delims[] = "/";	
	struct passwd* pwp;
	
	if(gethostname(hostname,sizeof(hostname)) == -1){
		//get hostname failed
		strcpy(hostname,"unknown");
	}//if
	//getuid() get user id ,then getpwuid get the user information by user id 
	pwp = getpwuid(getuid());	
	if(!(getcwd(cwd,sizeof(cwd)))){
		//get cwd failed
		strcpy(cwd,"unknown");	
	}//if
	char cwdcopy[100];
	strcpy(cwdcopy,cwd);
	char *first = strtok(cwdcopy,delims);
	char *second = strtok(NULL,delims);
	//if at home 
	if(!(strcmp(first,"home")) && !(strcmp(second,pwp->pw_name))){
		int offset = strlen(first) + strlen(second)+2;
		char newcwd[100];
		char *p = cwd;
		char *q = newcwd;

		p += offset;
		while(*(q++) = *(p++));
		char tmp[100];
		strcpy(tmp,"~");
		strcat(tmp,newcwd);
		strcpy(cwd,tmp);			
	}	
	
	if(getuid() == 0)//if super
		super = '#';
	else
		super = '$';
	sprintf(prompt, "\001\e[1;32m\002%s@%s\001\e[0m\002:\001\e[1;31m\002%s\001\e[0m\002%c",pwp->pw_name,hostname,cwd,super);	
	
}

//analysis command that user input
int analysis_command(){    
	int i = 1;
	char *p;
	//to cut the cwd by " "	
	char delims[] = " ";
	argc = 1;
	
	strcpy(argv[0],strtok(command,delims));
	while(p = strtok(NULL,delims)){
		strcpy(argv[i++],p);
		argc++;
	}//while
	
	if(!(strcmp(argv[0],"exit"))||!(strcmp(argv[0],"help"))|| !(strcmp(argv[0],"cd"))){
		BUILTIN_COMMAND = 1;	
	}
	int j;
	int pipe_location;
	for(j = 0;j < argc;j++){
		if(strcmp(argv[j],"|") == 0){
			PIPE_COMMAND = 1;
			pipe_location = j;				
			break;
		}	
	}//for
	
	if(PIPE_COMMAND){
		//command 1
		argvtmp1 = malloc(sizeof(char *)*pipe_location+1);
		int i;	
		for(i = 0;i < pipe_location+1;i++){
			argvtmp1[i] = malloc(sizeof(char)*100);
			if(i <= pipe_location)
				strcpy(argvtmp1[i],argv[i]);	
		}//for
		argvtmp1[pipe_location] = NULL;
		
		//command 2
		argvtmp2 = malloc(sizeof(char *)*(argc - pipe_location));
		int j;	
		for(j = 0;j < argc - pipe_location;j++){
			argvtmp2[j] = malloc(sizeof(char)*100);
			if(j <= pipe_location)
				strcpy(argvtmp2[j],argv[pipe_location+1+j]);	
		}//for
		argvtmp2[argc - pipe_location - 1] = NULL;
		
	}//if pipe_command
	else{
		argvtmp1 = malloc(sizeof(char *)*argc+1);
		int i;	
		for(i = 0;i < argc+1;i++){
			argvtmp1[i] = malloc(sizeof(char)*100);
			if(i < argc)
				strcpy(argvtmp1[i],argv[i]);	
		}//for
		argvtmp1[argc] = NULL;
	}
	

#ifdef DEBUG
	//test the analysis
	printf("\n==>the command is:%s with %d parameter(s):\n",argv[0],argc);
	printf("0(command): %s\n",argv[0]);	
	int k;	
	for(k = 1;k < argc;k++){
		printf("%d: %s\n",k,argv[k]);	
	}//for
			
	if(BUILTIN_COMMAND){
		printf("\tthis is a builtin command\n");
	}
	else if(PIPE_COMMAND){
		printf("\tthis is a pipe command:\n");
		printf("\t==command 1:\n");		
		int k;	
		for(k = 0;k < pipe_location + 1;k++){
			printf("\t%d: %s\n",k,argvtmp1[k]);	
		}//for
		printf("\t==command 2:\n");
			for(k = 0;k < argc - pipe_location;k++){
			printf("\t%d: %s\n",k,argvtmp2[k]);	
		}//for	
	}
			
#endif	

	return 0;
}

void builtin_command(){
	struct passwd* pwp;
	//exit when command is exit	
	if(strcmp(argv[0],"exit") == 0){
		exit(EXIT_SUCCESS);
	}
	else if(strcmp(argv[0],"help") == 0){
		help();
	}//else if
	
	else if(strcmp(argv[0],"cd") == 0){
		char cd_path[100];
		if((strlen(argv[1])) == 0 ){
			pwp = getpwuid(getuid());
			sprintf(cd_path,"/home/%s",pwp->pw_name);
			strcpy(argv[1],cd_path);
			argc++;			
		}
		else if((strcmp(argv[1],"~") == 0) ){
			pwp = getpwuid(getuid());
			sprintf(cd_path,"/home/%s",pwp->pw_name);
			strcpy(argv[1],cd_path);			
		}
	
		//do cd
#ifdef DEBUG	
		printf("cdpath = %s \n",argv[1]);	
#endif		
		if((chdir(argv[1]))< 0){
			printf("cd failed in builtin_command()\n");
		}
	}//else if cd
}

int do_command(){
	//do_command
	
	if(PIPE_COMMAND){
		int fd[2],res;
		res = pipe(fd);
	
		if(res == -1)
			printf("pipe failed in do_command()\n");
		pid_t pid = fork();
		if(pid == -1){
			printf("fork failed in do_command()\n");		
		}//if
		else if(pid == 0){
			dup2(fd[1],1);//dup the stdout
			close(fd[0]);//close the read edge
			if(execvp(argvtmp1[0],argvtmp1) < 0){
				#ifdef DEBUG
				printf("execvp failed in do_command() !\n");
				#endif
				printf("%s:command not found\n",argvtmp1[0]);		
				return 1;	
			}//if		
		}//else if 
		else{
			close(fd[1]);//close write edge
			dup2(fd[0],0);//dup the stdin
			if(execvp(argvtmp2[0],argvtmp2) < 0){
				#ifdef DEBUG
				printf("execvp failed in do_command() !\n");
				#endif
				printf("%s:command not found\n",argvtmp2[0]);		
				return 1;	
			}//if	
			int pidReturn = wait(NULL);
		}//else
	}//if pipe command

	else{
		pid_t pid = fork();	
		if(pid == -1){
			printf("fork failed in do_command()\n");		
		}//if
		else if(pid == 0){
			if(execvp(argvtmp1[0],argvtmp1) < 0){
				#ifdef DEBUG
				printf("execvp failed in do_command() !\n");
				#endif
				printf("%s:command not found\n",argvtmp1[0]);		
				return 1;	
			}//if	
		}//else if 
		else{
			int pidReturn = wait(NULL);	
		}//else  
	}
        free(argvtmp1);
	free(argvtmp2);

	return 0;
}

void help(){
		char message[50] = "Hi,welcome to myShell!";
		printf(
"< %s >\n"
"\t\t\\\n"
"\t\t \\   \\_\\_    _/_/\n"
"\t\t  \\      \\__/\n"
"\t\t   \\     (oo)\\_______\n"
"\t\t    \\    (__)\\       )\\/\\\n"
"\t\t             ||----w |\n"
"\t\t             ||     ||\n",message);
}

void initial(){
	int i = 0;	
	for(i = 0;i < argc;i++){
		strcpy(argv[i],"\0");	
	}
	argc = 0;
	BUILTIN_COMMAND = 0;
	PIPE_COMMAND = 0;
}

void init_lastdir(){
	getcwd(lastdir, sizeof(lastdir));
}

void history_setup(){
	using_history();
	stifle_history(50);
	read_history("/tmp/msh_history");	
}

void history_finish(){
	append_history(history_length, "/tmp/msh_history");
	history_truncate_file("/tmp/msh_history", history_max_entries);
}

void display_history_list(){
	HIST_ENTRY** h = history_list();
	if(h) {
		int i = 0;
		while(h[i]) {
			printf("%d: %s\n", i, h[i]->line);
			i++;
		}
	}
}

