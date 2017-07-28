
//  server.c


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <pthread.h>
#include <stdbool.h>

#define BUFSIZE 1024
#define LISTENQ 1024

void *thread(void *arg);

struct ThreadArgs {
  int sock;
  int cnt;
  struct sockaddr_in sa;
};

int main(int argc, char *argv[])
{
  int listen_sock, comm_sock, sa_len, cnt;
  struct sockaddr_in sa, new_sa;
  pthread_t thread_id;
  struct ThreadArgs *args;
  
  listen_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  
  memset(&sa, 0, sizeof(sa));
  sa.sin_family = AF_INET;
  sa.sin_port = htons(49152);
  sa.sin_addr.s_addr = htonl(INADDR_ANY);
  
  if(bind(listen_sock, (struct sockaddr *)&sa, sizeof(sa)) < 0){
    fprintf(stderr, "bind() failure \n");
    exit(1);
  }
  
  if(listen(listen_sock, LISTENQ) < 0){
    fprintf(stderr, "listen() failure\n");
    exit(1);
  }
  
  cnt = 0;
  printf("Waiting for a client\n");
  
  while(1){
    sa_len = sizeof(new_sa);
    if((comm_sock = accept(listen_sock, (struct sockaddr *)&new_sa, &sa_len)) < 0){
      fprintf(stderr, "accept() failure\n");
      exit(1);
    }
    ++cnt;
    
    args =(struct ThreadArgs *)malloc(sizeof(struct ThreadArgs));
    
    if(args == NULL){
      fprintf(stderr, "malloc() failure\n");
      exit(1);
    }
    
    args->sock = comm_sock;
    args->cnt  = cnt;
    args->sa   = new_sa;
    
    if (pthread_create(&thread_id, NULL, (void *)thread, (void *)args) != 0) {
      fprintf(stderr, "pthread_create() failure\n");
      exit(1);
    }
  }
  return 0;
}


void *thread(void *arg)
{
  int sock, cnt, n, msg_len;
  float a, b, total = 0.0; // 計算用
  char *operations, *value1, *value2;
  struct sockaddr_in sa;
  char buf[BUFSIZE];
  bool check = true; // 引数があっているかチェック
  
  check = true;
  pthread_detach(pthread_self());
  
  sock = ((struct ThreadArgs *)arg)->sock;
  cnt = ((struct ThreadArgs *)arg)->cnt;
  sa = ((struct ThreadArgs *)arg)->sa;
  
  printf("Client %d (%s) connect\n", cnt, inet_ntoa(sa.sin_addr));
  
  while(1){
    if((n= recv(sock, buf, sizeof(buf), 0)) > 0){
      check = true;
      buf[n] = '\0';
      // 空白ごとに文字を取得
      // 四則演算（add, sub, mul, div）
      operations = strtok(buf, " ");
      // 数値
      value1 = (strtok(NULL, " "));
      value2 = (strtok(NULL, " "));
      
      // string型からfloat型に変換
      a = atof(value1);
      b = atof(value2);
      
      // 文字長さを取得
      msg_len = strlen(buf);
      
      // 文字比較（四則演算を行う）
      if(strcmp(operations, "add") == 0) {
        total = a + b;
      }else if(strcmp(operations, "sub") == 0){
        total = a - b;
      }else if(strcmp(operations, "mul") == 0){
        total = a * b;
      }else if(strcmp(operations, "div") == 0){
        total = a / b;
      }
      // "quit"が入力されるとループを抜ける
      else if(strcmp(operations, "quit") == 0){
        break;
      }
      else{
        // 第一引数の文字列が異なるとき
        check = false;
        memset(buf, '\0', msg_len);
        strcat(buf, "Argument is different!");
        msg_len = strlen(buf);
      }
      
      // 指定通りの引数が送られてきたとき
      if (check == true) {
        printf("Client %d: %s %s %s %f\n", cnt, operations, value1, value2, total);
        // float型からstring型に変換
        snprintf(buf, BUFSIZE, "%f", total);
      }
      
      if(send(sock, buf, msg_len, 0) != msg_len){
        fprintf(stderr, "send() failure\n");
        break;
      }

    }else{
      break;
    }
  }
  close(sock);
  printf("Client %d disconnected\n", cnt);
  free(arg);
  return 0;
}
