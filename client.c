
//  client.c


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>


#define MSGSIZE 1024
#define BUFSIZE 1024

int main(int argc, char *argv[])
{
  int sock, msg_len, n;
  struct sockaddr_in sa;
  char *server_ip;
  char *operations;
  char msg[MSGSIZE], buf[BUFSIZE];
  
  if(argc != 2){
    fprintf(stderr, "Usage: D78Kai_Client SERVER_IP_ADDRESS\n");
    exit(1);
  }
  server_ip = argv[1];
  
  sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  
  memset(&sa, 0, sizeof(sa));
  sa.sin_family = AF_INET;
  sa.sin_port = htons(49152);
  sa.sin_addr.s_addr = inet_addr(server_ip);
  
  if(connect(sock, (struct sockaddr *)&sa, sizeof(sa)) < 0){
    fprintf(stderr, "connect() failure\n");
    exit(1);
  }
  
  while(1){
    printf("please input a message!\n");
    printf("足し算→add 引き算→sub かけ算→mul 割り算→div\n例：add 3 2\n");
    printf(">");
    fgets(msg, MSGSIZE, stdin);
    msg_len = strnlen(msg, sizeof(msg));
    
    
    if(send(sock, msg, msg_len, 0) != msg_len){
      fprintf(stderr, "send() failure\n");
      exit(1);
    }
    
    // "quit"を入力するとループを抜ける
    operations = strtok(msg, " ");
    if (strcmp(operations, "quit") == 0){
      break;
    }
    
    if((n = recv(sock, buf, sizeof(buf), 0)) > 0){
      buf[n] = '\0';
      printf("message: ");
      printf("%s\n", buf);

    }else if(n < 0){
      fprintf(stderr, "recv() failure\n");
      exit(1);
    }
    printf("\n");
  }
  close(sock);
  return 0;
}
