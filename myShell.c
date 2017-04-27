#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<pwd.h>
#include<sys/types.h>
#include <readline/readline.h>
#include <readline/history.h>


//define the printf color
#define L_GREEN "\e[1;32m"
#define L_BLUE  "\e[1;34m"
#define L_RED   "\e[1;31m"
#define WHITE   "\e[0m"

char lastdir[100];

//print the prompt
void set_prompt(char *prompt);
//read the command that user input
void read_command();
void init_lastdir();
void history_setup();
void history_finish();
void display_history_list();

int main(){
	char prompt[BUFSIZ];
	char *line;
	char buf[BUFSIZ];
	init_lastdir();
	history_setup();	
	while(1) {
		set_prompt(prompt);
		if(!(line = readline(prompt))) 
			break;
		if(*line)
			add_history(line);

		strcpy(buf, line);
		strcat(buf, "\n");
		
		//todo deal with the buff

	}

	history_finish();

	return 0;
}

//print the prompt
void set_prompt(char *prompt){
	char hostname[100];
	char cwd[100];
	char super = '#';
	//to cut the cwd	
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
		int shift = strlen(first) + strlen(second)+2;
		char newcwd[100];
		char *p = cwd;
		char *q = newcwd;

		p += shift;
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
//read the command that user input
void read_command(){
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

