/*CT30A3370 Käyttöjärjestelmät ja systeemiohjelmointi harjoitustyö plan B
nimi: Teemu Juura
opnro: 0509164*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>

#define LOGOUT 240
#define MAXNUM 40
#define MAXLEN 160

void sighandler(int sig)
{
	switch (sig) {
		case SIGALRM:
			printf("\nautologout\n");
			exit(0);
		default:
			break;
	}
	return;
}


int main(void)
{
	char * cmds, line[MAXLEN], * args[MAXNUM], *pcmd[MAXNUM][MAXNUM], *out, *in, *cmd[MAXNUM];
	int background, pipec,rout, rin, i, j, k, l, pipe1[2],pipe2[2] , outfd, infd;
	int pid;
	char cwd[1024];

	signal(SIGALRM, sighandler);
	signal(SIGINT, sighandler);

	while (1) {
		background = 0;
    pipec = 0;
    rout = 0;
    rin = 0;
    in = NULL;
    out = NULL;

		/* Get current working directory */
		if (getcwd(cwd, sizeof(cwd)) != NULL) {

		/* print the prompt */
		printf("%s > ",cwd);
		/* set the timeout for alarm signal (autologout) */
		alarm(LOGOUT);

		/* read the users command */
		if (fgets(line,MAXLEN,stdin) == NULL) {
			printf("\nlogout\n");
			exit(0);
		}
		line[strlen(line) - 1] = '\0';

		if (strlen(line) == 0)
			continue;

		/* start to background? */
		if (line[strlen(line)-1] == '&') {
			line[strlen(line)-1]=0;
			background = 1;
		}

		/* split the command line */
		i = 0;
		cmds = line;
		while ( (args[i] = strtok(cmds, " ")) != NULL) {
			i++;
			cmds = NULL;
		}

		if (strcmp(args[0],"exit")==0) {
			exit(0);
		}

		else if (strcmp(args[0],"cd")==0) {
		/* changes directory if specified*/
			if(args[1] != NULL) {

				chdir(args[1]);
				/* Goes home*/
			} else {
				chdir(getenv("HOME"));
		}

  }
		else {
      i = 0;
      while (args[i] != NULL){

				/* count pipes*/
        if (strcmp(args[i],"|") == 0){
          pipec++;

 				/* get input file*/
      } else if (strcmp(args[i],"<") == 0){
        rin = 1;
        in = strdup(args[i+1]);

      }
			/*get output file*/
      else if (strcmp(args[i],">") == 0){
        rout = 1;
        out = strdup(args[i+1]);

      }

      i++;
		}

     /*Get commands and run*/

   if (rout > 0 || rin > 0 ) { /* if we have redirs*/
     i =0;
     j = 0;

     if(pipec > 0) { /*skip filenames and ">"or"<" for piped commands*/

       while(args[i] != NULL) {

         if(strcmp(args[i],">") == 0 || strcmp(args[i],"<") == 0) {
           i = i+2;


         }

         cmd[j] = args[i];
				 /*printf("cmd %d: %s\n", i, args[i]);*/
         i++;
         j++;

             }

     } else { /*we have only one command with input and/or output redirection*/
       while(strcmp(args[i],">") != 0 &&strcmp(args[i],"<") != 0 && args[i] != NULL) {
         cmd[i] = args[i];

         i++;

       }

     }


 } else { /* No pipes or redirects cmd = args*/
   i = 0;
     while (args[i] != NULL){
       cmd[i] = args[i];
       i++;
     }

 }
	/* we have pipes*/
   if (pipec > 0) {

     pipec++;
     j = 0;
     k = 0;
     l = 0;

     i = 0;

     while (cmd[j] != NULL){
       k = 0;

			  /*get commands seperated by '|'*/
     while (strcmp(cmd[j],"|") != 0){

         pcmd[l][k] = cmd[j];

         j++;

         if (cmd[j] == NULL){
           /* End of cmd exit loop*/
           k++;
           break;
         }

         k++;
       }
       pcmd[l][k] = NULL;
       l++;
       j++;
     }


    /* Loop to fork piped commands*/

     for(i = 0; i <= pipec; i++) {

       if(pipec > 0 && i != pipec) {
         pipe(pipe1);
       }

    switch (pid = fork()) {
      case -1:
        /* error */
        perror("fork");
        exit(1);
      case 0:
        /* child process */

				/* set output file for the last in pipe*/
         if (rout > 0 && i == pipec-1) {

           outfd = open(out, O_CREAT | O_TRUNC | O_WRONLY, 0600);

           if(outfd < 0) {
             perror("failed to open output");
             exit(1);
           }

           dup2(outfd, STDOUT_FILENO);
           close(outfd);

         }
				 	/* set input file for the first in pipe*/
         if (rin > 0 && i == 0) {

           infd = open(in, O_RDONLY, 0600);
           if(infd < 0) {
             perror("failed to open input");
            exit(1);
           }

          dup2(infd,STDIN_FILENO);
          close(infd);

         }


					 	/* First in pipe*/
           if(i == 0) {
               dup2(pipe1[1], STDOUT_FILENO);
               close(pipe1[0]);
               close(pipe1[1]);

           }
					 /*Last in pipe*/
           else if (i == pipec - 1){
                dup2(pipe2[0],STDIN_FILENO);
                close(pipe2[0]);
                close(pipe2[1]);
					/* Middle in pipe*/
         }else{

           dup2(pipe2[0],STDIN_FILENO);
           close(pipe2[0]);
           close(pipe2[1]);

           dup2(pipe1[1],STDOUT_FILENO);
           close(pipe1[0]);
           close(pipe1[1]);

         }



           if (execvp(pcmd[i][0],pcmd[i])==-1){
						/* printf("pc %d: %s\n", i, pcmd[i][0]);*/
             perror("Pipe exec");
             exit(1);
           }

      default:
        /* parent handles pipes for children*/
        if(i > 0) {
          close(pipe2[0]);
          close(pipe2[1]);
        }
        pipe2[0] = pipe1[0];
        pipe2[1] = pipe1[1];

        if (background == 1) {
          alarm(0);

        } else {
          waitpid(pid, NULL, 0);
        }
        break;

    } /* end switch*/

	}/* end loop*/

} /* end of if*/

/* no pipes*/
 else{

   /* fork to run the command */

   switch (pid = fork()) {
     case -1:
       /* error */
       perror("fork");
       exit(1);
     case 0:
       /* child process */

			 /* set output file*/
       if (rout > 0) {
         outfd = open(out, O_CREAT | O_TRUNC | O_WRONLY, 0600);

         if(outfd < 0) {
           perror("failed to open output");
          exit(1);
         }

         dup2(outfd, STDOUT_FILENO);
         close(outfd);

       }
			 /* set input file*/
       if (rin > 0) {

         infd = open(in, O_RDONLY, 0600);
         if(infd < 0) {
           perror("failed to open input");
           exit(1);
         }

         dup2(infd,STDIN_FILENO);
         close(infd);
       }
         execvp(cmd[0], cmd);
         perror("execvp");
         exit(1);



     default:
       /* parent (shell) */
       if (background == 1) {
         alarm(0);

       } else {
         waitpid(pid, NULL, 0);
       }

   }

   }
   /*Clear the command*/
   for (i = 0; i < MAXNUM; ++i) {
      cmd[i] = 0;
  }

	/*Clear the args*/
	for (i = 0; i < MAXNUM; ++i) {
		 args[i] = 0;
 }
 /*Clear the pipecommand*/
	for (i = 0; i < MAXNUM; ++i) {
		for (j = 0; j < MAXNUM; ++j) {
       pcmd[i][j] = 0;
   }
 }

  }
}

  else {
			/*failed to get current working directory*/
       			perror("getcwd() error");
						exit(1);
			}

    }

	return 0;
}
