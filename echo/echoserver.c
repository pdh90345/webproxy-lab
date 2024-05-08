#include "../csapp.h"

void echo(int connfd)
{
  size_t n;
  char buf[MAXLINE];
  rio_t rio;

  Rio_readinitb(&rio, connfd); // connfd를 사용하여 데이터를 읽을 준비
  while ((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0)
  {
    printf("server received %d bytes\n", (int)n);
    Rio_writen(connfd, buf, n); // 읽은 데이터를 클라이언트에게 다시 전송
  }
}

int main(int argc, char **argv)
{
  int listenfd, connfd; // 서버의 리스닝 소켓과 클라이언트의 연결 소켓
  socklen_t clientlen;
  struct sockaddr_storage clientaddr;                  // 클라이언트의 주소 정보 저장
  char client_hostname[MAXLINE], client_port[MAXLINE]; // 호스트이름, 포트번호

  if (argc != 2) // 포트번호 입력
  {
    fprintf(stderr, "usage : %s <port>\n", argv[0]);
    exit(0);
  }
  listenfd = Open_listenfd(argv[1]); // 주어진 포트번호로 리스닝 소켓을 열어 클라이언트의 연결 요청 기다림
  while (1)
  {
    clientlen = sizeof(struct sockaddr_storage);
    connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen); // 클라이언트의 연결 수락
    // 클라이언트 주소정보에서 호스트 이름과 포트 번호 추출
    Getnameinfo((SA *)&clientaddr, clientlen, client_hostname, MAXLINE, client_port, MAXLINE, 0);
    printf("Connected to (%s, %s)\n", client_hostname, client_port);
    echo(connfd);
    Close(connfd);
  }
  exit(0);
}