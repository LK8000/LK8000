/*
 * lkrun 1.0
 * P.Ventafridda 2/2016
 */
#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <errno.h>

#define VERSION "1.0 linux"
#define LOCKFILE "/tmp/pid_lk8000"
#define LKBIN    "/v/lk8000/LK8000/LK8000-LINUX"

// Return codes
#define RERR -1
#define ROK  0
#define RETDELAY  3 // seconds before exit on error


main(int argc, char *argv) {

  fprintf(stderr,"lkrun v%s\n",VERSION);

  FILE *fp, *fopen();
  char stlock[21];
  int	lpid;
  unsigned short crashrecover=0;

_nextrun:
  if ( (fp=fopen(LOCKFILE,"r")) != NULL) {
     fgets(stlock,20,fp);
     fprintf(stderr,"existing lock string:%s",stlock);
     lpid=atoi(stlock);
     fclose(fp);
     if ( lpid<50 || lpid>32767 ) {
        fprintf(stderr,"*** invalid lock pid: %d\n",lpid);
        lpid=32766; /* set an almost impossible one */
     }
     if ( kill(lpid,0) < 0) {
        fprintf(stderr,"nonexistent pid, fake lock\n");
     } else {
        fprintf(stderr,"LK already running!!\n");
        return(RERR);
     }
  } 

  char spid[10];
  int fpid;
  static char *proc, *varg[5];

  unlink(LOCKFILE);
  if ( (fp=fopen(LOCKFILE,"w")) == NULL) {
     fprintf(stderr,"cannot open lockfile, errno=%d %s\n", errno,sys_errlist[errno]);
     return(RERR);
  }
  fclose(fp);

  if ( (fpid=fork()) <0) {
     fprintf(stderr,"*** fork failed, errno=%d. %s\n", errno,sys_errlist[errno]);
     unlink(LOCKFILE);
     return(RERR);
  }

  //
  // PARENT LOOP
  //

  if ( fpid>0 ) {
     int status=0;
     int retval=0;
     wait(&status);
     fprintf(stderr,"Child exit, status=%d\n",status);
     if (WIFEXITED(status)) {
        retval=WEXITSTATUS(status);
        fprintf(stderr,"WIFEXIT OK, code=%d\n", retval);
        unlink(LOCKFILE);

        if (retval==111) {
           fprintf(stderr,"restarting lk\n");
           goto _nextrun;
        }
        if (retval==222) {
           fprintf(stderr,"exiting lkrun\n");
           return(ROK);
        }

        goto _abnormal;
     } else {
_abnormal:
        if (crashrecover++ <=10) {
           fprintf(stderr,"lk8000 abnormal termination, try again lk8000\n");
           goto _nextrun;
        }
        fprintf(stderr,"lk8000 abnormal termination, end lkrun\n");
        return(RERR);
     }
     fprintf(stderr,"abnormal lkrun termination\n");
     return(RERR);
  }


  //
  // CHILD. We are in the background
  //

  if ( (fp=fopen(LOCKFILE,"w")) == NULL) {
     fprintf(stderr,"*** cannot create lockfile (2), errno=%d. %s\n", errno,sys_errlist[errno]);
     Sleep(RETDELAY);
     return(RERR);
  }
  sprintf(spid,"%d",getpid());
  fputs(spid,fp);
  fputs("\nLKRUN PLEASE DO NOT EDIT OR REMOVE THIS FILE\n",fp);
  fprintf(stderr,"new lockfile, pid=%s\n",spid);
  fclose(fp);

  proc=LKBIN;
  varg[0]="LK8000";
  varg[1]= (char *) NULL;

  if ( (execv(proc,varg)) <0) {
     fprintf(stderr,"*** exec failed, errno=%d. %s\n",errno,sys_errlist[errno]);
     Sleep(RETDELAY);
     return(RERR);
  }
  fprintf(stderr,"*** Warning, exec failed with no error! ***\n");
  Sleep(RETDELAY);
  return(RERR);

} // main

