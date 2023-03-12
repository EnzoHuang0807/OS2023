#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

int dir_num, file_num;

int count(char * path, char * key){
  int occurrence = 0;
  char * tmp, * p = path;

  while((tmp = strchr(p, *key)) != 0){
    p = tmp + 1;
    occurrence ++;
  }
  return occurrence;
}

void ls(char * path, char * key){
  char buf[512], *p;
  int fd;
  struct dirent de;
  struct stat st;

  if((fd = open(path, 0)) < 0){
    fprintf(2, "mp0: cannot open %s\n", path);
    return;
  }

  if(stat(path, &st) < 0){
    fprintf(2, "mp0: cannot stat %s\n", path);
    return;
  }

  switch(st.type){
  case T_FILE:
    printf("%s %d\n", path, count(path, key));
    break;

  case T_DIR:
    if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
      printf("mp0: path too long\n");
      break;
    }
    strcpy(buf, path);
    p = buf + strlen(buf);
    *p ++ = '/';

    while(read(fd, &de, sizeof(de)) == sizeof(de)){
      if(de.inum == 0 || strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
        continue;

      memmove(p, de.name, DIRSIZ);
      p[DIRSIZ] = 0;
      if(stat(buf, &st) < 0){
        printf("mp0: cannot stat %s\n", buf);
        continue;
      }
      printf("%s %d\n", buf, count(buf, key));

      if (st.type == T_DIR){
	dir_num ++;
        ls(buf, key);
      }
      else 
	file_num ++;
    }
    break;
  }
  close(fd);
}

int main(int argc, char *argv[]){

  int fd[2];
  if (pipe(fd) < 0)

    fprintf(2, "pipe error\n");

  int pid;
  if ((pid = fork()) < 0)
    fprintf(2,"fork error\n");

  else if (pid == 0){
    close(fd[0]);

    if(open(argv[1], 0) < 0)
      printf("%s [error opening dir]\n", argv[1]);
      
    else{
      printf("%s %d\n", argv[1], count(argv[1], argv[2]));
      ls(argv[1], argv[2]);
    }
    fprintf(fd[1], "\n%d directories, %d files\n", dir_num, file_num);
  }

  else{
    close(fd[1]);
    char buf[512];
    read(fd[0], buf, 512);
    printf(buf);
  }
  exit(0);
}
