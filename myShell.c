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

char lastdir[100];
char command[BUFSIZ];
char argv[100][100];
int  argc;
int BUILTIN_COMMAND = 0;
//set the prompt
void set_prompt(char *prompt);
//analysis the command that user input
int analysis_command();
void builtin_command();
int is_valid_command(char *command);
int do_command();
//print help information
void help();
void clean_argv();
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
			}
			else{
			int pid = fork();
				if(pid == -1){
					printf("fork failed!\n");		
				}//if
				else if(pid == 0){
					do_command();		
				}//else if 
				else{
					int pidReturn = wait(NULL);
				}//else
			}//else
		}//if analysis_command
		clean_argv();
		BUILTIN_COMMAND = 0;
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

	if(!is_valid_command(argv[0])){
		printf("%s: command not found\n",argv[0]);
		return 1;	
	}

	//test the analysis
	printf("\n==>the command is:%s with %d parameter(s):\n",argv[0],argc);
	printf("0(command): %s\n",argv[0]);	
	int j;	
	for(j = 1;j < argc;j++){
		printf("%d: %s\n",j,argv[j]);	
	}//for
	printf("BUILTIN_COMMAND = %d\n",BUILTIN_COMMAND);
	return 0;
}

void builtin_command(){
	struct passwd* pwp;
	//exit when command is exit	
	if(strcmp(argv[0],"exit") == 0){
		printf("exit====\n");
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
		else if( !(strcmp(argv[1],"~")) ){
			pwp = getpwuid(getuid());
			sprintf(cd_path,"/home/%s",pwp->pw_name);
			strcpy(argv[1],cd_path);			
		}
	
		//do cd	
		printf("cdpath = %s \n",argv[1]);	
		if((chdir(argv[1]))< 0){
			printf("cd failed!\n");
		}
	}//else if cd
}

int is_valid_command(char *command){
/*	
	DIR *dir;
	struct dirent *ptr;
	//must have enough space
	char tmp[BUFSIZ];
	char *pathdir;
	char *path = getenv("PATH");
	strcpy(tmp,path);
	//printf("\npath == %s\n",path);
	pathdir = strtok(tmp,":");

	while(pathdir){
		dir = opendir(pathdir);
		while((ptr = readdir(dir)) != NULL){
		//	printf("d_name:%s\n",ptr->d_name);

			if(strcmp(ptr->d_name,command) == 0){
				closedir(dir);
				return TRUE;
			}//if		
		}//while
		closedir(dir);
		//next pathdir 
		pathdir =strtok(NULL,":");
	}//while

	return FALSE;
*/
//	if( !(strcmp(argv[0],"cd")) || !(strcmp(argv[0],"help")) || !(strcmp(argv[0],"exit")))
//		return TRUE;
//	else if()
		return TRUE;
}

int do_command(){
	//do_command
	printf("do commmand...\n");
	char **argvtmp;
	argvtmp = malloc(sizeof(char *)*argc+1);
	int i;	
	for(i = 0;i <= argc;i++){
		argvtmp[i] = malloc(sizeof(char)*100);
		if(i < argc)
			strcpy(argvtmp[i],argv[i]);	
	}//for
	argvtmp[argc] = NULL;
	printf("argvtmp====\n");
	int j;	
	for(j = 0;j <= argc;j++){
		printf("%d: %s\n",j,argvtmp[j]);	
	}//for

	if(execvp(argv[0],argvtmp) < 0){
		printf("execvp failed!\n");
		return 1;	
	}
        free(argvtmp);
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

void clean_argv(){
	int i = 0;	
	for(i = 0;i < argc;i++){
		strcpy(argv[i],"\0");	
	}
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

