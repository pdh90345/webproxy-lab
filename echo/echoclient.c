#include "../csapp.h"

int main(int argc, char **argv)
{
  int clientfd;                    // 서버에 연결된 소켓의 파일 디스크립터
  char *host, *port, buf[MAXLINE]; // 호스트 이름, 포트 번호, 데이터 저장할 버퍼
  rio_t rio;                       // Robert I/O 구조체

  if (argc != 3) // 인자의 수가 올바르지 않으면
  {
    fprintf(stderr, "usage: %s <host> <port>\n", argv[0]);
    exit(0);
  }
  host = argv[1];
  port = argv[2];

  clientfd = Open_clientfd(host, port); // 지정된 이름과 포트에 대한 네트워크 연결, 성공하면 fd반환
  Rio_readinitb(&rio, clientfd);        // 네트워크 통신을 위한 버퍼 관리 간소화

  while (Fgets(buf, MAXLINE, stdin) != NULL) // 표준 입력으로부터 데이터를 읽고
  {
    Rio_writen(clientfd, buf, strlen(buf)); // 읽은 데이터를 서버로 전송한 후
    Rio_readlineb(&rio, buf, MAXLINE);      // 서버로부터 한 줄의 응답을 받아 buf에 저장
    Fputs(buf, stdout);                     // buf에 저장된 내용을 표준 출력에 출력
  }
  Close(clientfd);
  exit(0);
}