/* COMP 530: Tar Heel SHell */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>

// Assume no input line will be longer than 1024 bytes
void parsecmd(char* cmd, char** args);
int execute(char** args);
int redirectIO(char**args);
int pipeCreator(char **tempArgs, int pipe_rw[2]);
void redirectHQ (char **args,char **tempArgs);

#define MAX_INPUT 1024
#define MAX_ARGS 10


int main (int argc, char ** argv, char **envp) {
  //make the first prompt
  char* cwd;
  char* temp;
  char buffer[MAX_INPUT + 1];
  cwd = getcwd( buffer, MAX_INPUT + 1 );
  char prompt[MAX_INPUT]; 
  char *tempArgs[MAX_INPUT+1];
  strcpy(prompt,"[");
  strcat(prompt, cwd);
  strcat(prompt, "] thsh> ");

  char* args[MAX_INPUT + 1];
  char cmd[MAX_INPUT+1];
  int finished = 0;
  int status, i;
  int cmdCount = 0;
  int debug = 0; //0 means debug status is off, 1 for on
  



  //initialize string to hold the last working directory (will be used for "cd -")
    char* last_cwd = NULL;
    char buff[MAX_INPUT + 1];
    last_cwd = cwd;
    char *home = getenv("HOME");


  while (!finished) {
  cmdCount++;
   //make the new prompt 
  cwd = getcwd( buffer, MAX_INPUT + 1 );
  
  char prompt[MAX_INPUT]; 
  strcpy(prompt,"[");
  strcat(prompt, cwd);
  strcat(prompt, "] thsh> "); 


    char *cursor;
    char last_char;
    int rv;
    int count;
    
    int i;

    // Print the promptx
    rv = write(1, prompt, strlen(prompt));
    if (!rv) { 
      finished = 1;
      break;
    }
    
    // read and parse the input
    for(rv = 1, count = 0, 
    cursor = cmd, last_char = 1;
       rv 
    && (++count < (MAX_INPUT-1))
    && (last_char != '\n');
  cursor++) { 

      rv = read(0, cursor, 1);
      last_char = *cursor;
    } 
    *cursor = '\0';

    if (!rv) { 
      finished = 1;
      break;
    }

     // Remove trailing newline character, if any
        if(cmd[strlen(cmd)-1] == '\n') {
            cmd[strlen(cmd)-1] = '\0';
        }
  

    //break code up into array of arguments
    parsecmd(cmd, args);


    //turn on debug status?
    if(cmdCount == 1 && strcmp(args[0],"-d") == 0){
      debug =  1;
    }

    if (debug ==1){
      
      printf("RUNNING: %s\n", cmd);

    }


    //check to see if need to redirect or pipe
    int finished=0;
    while (args[i]!=NULL){
    
      if ((strcmp(args[i],">")==0 )|| (strcmp(args[i],"<")==0) || (strcmp(args[i],"|")==0)){

        /* Go to pipe*/

        redirectHQ(args, tempArgs);
        printf ("back to MAIN and EXITING\n");
        finished=1;
      
       }
       if (finished == 1){
        exit(0);
       }

       i++;

    }

    
    
  
    if(strcmp(args[0], "exit")==0) break;  //handle exit built in command

     else if(strcmp(args[0], "cd") == 0) {

         if(args[1] == NULL){ //change directory to home if user inputs "cd"
            last_cwd = getcwd( buff, MAX_INPUT + 1 );
            //printf("ENDED %s\n", args[0]);
            chdir(home);
            


         }
         else if(strcmp(args[1], "-") == 0){ //go back to last directory if cmd is "cd -"
            if(last_cwd)
              {
                char* temp;
                char buffer[MAX_INPUT + 1];
                temp = getcwd( buffer, MAX_INPUT + 1 );

               // printf("ENDED %cd%cd",args[0],cwd);
                chdir(last_cwd);
                last_cwd = strdup(temp);
              }
            else
            {
              char* error = strerror(errno);
              
            }
         }
         else    //otherwise, go to user specified directory
         {
           last_cwd = getcwd( buff, MAX_INPUT + 1 );
           //printf("ENDED %s\n", args);
           chdir(args[1]);


         }
     }

    else {   //handle all other commands
      if(execute(args)==0) break;

    }


  }

  return 0;
}

//parse user input and store into an array of args to be executed
void parsecmd(char* cmd, char** args){   

   char *token;
   int i = 0;
   //first token
   token = strtok(cmd, " ");
   
   //get other tokens
   for(i = 0; i < MAX_INPUT; i++) {
      if(token != NULL){
      char* temp = strdup(token);
      args[i] = temp;
      token = strtok(NULL, " ");
      }
      else{
        args[i] = NULL;
      }
   }
      

}

int execute(char** args){
  int status;
  int debug;

    id_t pid = fork(); //child

    // Error
    if (pid == -1) {
        char* error = strerror(errno);
        printf("fork: %s\n", error);
        return 1;
    }

    else if(pid == 0){  //it is child        
        execvp(args[0], args);
        //char* error = strerror(errno);
        //printf("shell:%s: %s\n", args[0], error);
        return 0;

          
    }    
    else {
        int status;
        wait(0);

          printf("ENDED: %s EXIT STATUS: %d\n", args[0], status);
         


        return 1;

    }

}


void redirectHQ (char **args, char **tempArgs){
  printf("%s\n", "redirectHQ");
 fflush(stdout);
  int i=0;
  int j=0; 
  int num_pipes =0; 
  int pipe_rw[2];
  int fd_in =0;
  int fd_out =0;

// The purpose of  this function is to serve as the I/O port for all
//piping and redirecting ops     
  
  //count up the pipes

  while (args[i]!=NULL){
      //printf("i: %d\n",i);
     if (strcmp(args[i], "|")==0){

        num_pipes++;
        
     }
     i++;
   }
   printf("args[2]:\t%s\n",args[2]);
   fflush(stdout);
   while (args[j]!=NULL){

     //tempArgs[j] = malloc(sizeof(char) * strlen(args[j]));
     //strcpy(tempArgs[j], args[j]);
    
     if (strcmp(args[j], "|")==0){
      
      args[j]=NULL;

      printf("got |\n");
      fflush(stdout);

      //printf("going to pipeCreator\n");

      // tempArgs[j]=args[j];
     
     
      //args[i]=NULL;


      pipeCreator(tempArgs, pipe_rw);
      
      fflush(stdout);
     
      id_t pid= fork();



      // fork for second child
    
        if (pid==0){

          
          if (dup2(pipe_rw[0], STDIN_FILENO)==-1){
              perror("dup2 out of pipecreaor\n");
            }

          
        /*
        
        char buffer[100];
        int rv;
        if((rv=read(pipe_rw[0], buffer, 99))==-1){perror("ERROR");}
        printf("read:%d\n", rv);





        buffer[99]= '\0';

        printf("****%s****", buffer);*/
        fflush(stdout);

  
        close(pipe_rw[1]);
        tempArgs[3] = NULL;        
        printf("tempArg[2]:\t%s\n", tempArgs[2]);
        if(execvp(tempArgs[2], tempArgs+2)==-1){perror("exec fail\n");}

        printf("failed");
        fflush(stdout);
        exit(0);
      }

      else{
        wait (0);
        wait (0);

      }


      
    }
    /*
    else if (strcmp(args[i], ">")==0){
      printf("got >\n");
      redirectIO(args);
      i++;
    }

    else if (strcmp(args[i], "<")==0){
      printf("got <\n");
      redirectIO(args);
      i++;
    }
*/
    j++;

  }  


}

// pipe functions

int pipeCreator(char **tempArgs, int pipe_rw[2]){

  int status;
  int debug;
  int i=0;
  int done=0;
  int fd_in;
  int fd_out;

  

  pipe(pipe_rw);



  // pipe_rw[0] is read end of arg[0] to arg[2]
  printf("we are in pipeCreator\n");
  fflush(stdout);
  


  id_t pid = fork(); 



  if (pid==0){

    close(pipe_rw[0]);

    if (dup2(pipe_rw[1], STDOUT_FILENO)==-1){perror("ERROR DUP\n");}

    


  

    execvp(tempArgs[0], tempArgs);
    //printf("%s\n", tempArgs[0]);
    printf("failed\n");
    exit(0);
  }
 
  wait(&status); 
  return 1;

}

int redirectIO (char **args){

  int status;
  int debug;
  int fd_in;
  int fd_out;

  id_t pid = fork(); 

    // Error
    if (pid == -1) {
        char* error = strerror(errno);
        printf("fork: %s\n", error);
        return 1;
    }

    else if (pid==0){

      // Child Process 
      if (strcmp(args[1],">")==0){

         //WRITE the output of args[0] into args[2] which reads said input 
      // args[0] > args[2]

      fd_out=open(args[2], O_WRONLY, 1);
     // if (dup2(fd_out, STDOUT_FILENO)==-1){printf("ERROR DUP")} 
      close(fd_out);
      execvp(args[0], args);

    }

      else  if (strcmp(args[1],"<")==0){

        // WRITE args[2] output; args[0] READ args[2] output

        fd_in= open (args[2], O_RDONLY);
        dup2(fd_in, STDIN_FILENO);
        close(fd_in);
        execvp(args[0],args); 

      }
    }

    else {

      exit(0);
    }
    return 1;

}

