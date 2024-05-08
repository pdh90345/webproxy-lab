/*
 * adder.c - a minimal CGI program that adds two numbers together
 */
/* $begin adder */
#include "csapp.h"

int main(void)
{
  char *buf, *p, *method, *arg1_p, *arg2_p;
  char arg1[MAXLINE], arg2[MAXLINE], content[MAXLINE], val1[MAXLINE], val2[MAXLINE];
  int n1 = 0, n2 = 0;

  /* Extract the two arguments */
  if ((buf = getenv("QUERY_STRING")) != NULL) // 환경 변수  QUERY_STRING에서 쿼리 문자열 가져옴
  {
    p = strchr(buf, '&'); // & 문자 찾아 문자열 분리
    *p = '\0';
    strcpy(arg1, buf);
    strcpy(arg2, p + 1);

    // n1 = atoi(arg1); // 문자열 형태의 숫자를 정수로 변경
    // n2 = atoi(arg2);

    arg1_p = strchr(arg1, '=');
    *arg1_p = '\0';
    strcpy(val1, arg1_p + 1);

    arg2_p = strchr(arg2, '=');
    *arg2_p = '\0';
    strcpy(val2, arg2_p + 1);

    n1 = atoi(val1); // 문자열 형태의 숫자를 정수로 변경
    n2 = atoi(val2);
  }
  /* Make the response body */
  // 응답 본문 생성
  method = getenv("REQUEST_METHOD");
  sprintf(content, "QUERY_STRING=%s", buf);
  sprintf(content, "Welcome to add.com: ");
  sprintf(content, "%sTHE Internet addition portal.\r\n<p>", content);
  sprintf(content, "%sThe answer is: %d + %d = %d\r\n<p>", content, n1, n2, n1 + n2); // 두 수의 합
  sprintf(content, "%sThanks for visiting!\r\n", content);
  /* Generate the HTTP response */
  printf("Connection: close\r\n");
  printf("Content-length: %d\r\n", (int)strlen(content)); // 컨텐츠 길이
  printf("Content-type: text/html\r\n\r\n");              // 응답의 MIME 타입
  // printf("%s", content);

  // // method가 GET일 경우에만 response body 보냄
  if (strcasecmp(method, "GET") == 0)
  {
    printf("%s", content); // 최종적으로 구성된 응답 본문 출력
    printf("Method: %s\r\n", method);
  }
  fflush(stdout); // 표준 출력 버퍼 강제로 비운다

  exit(0);
}
/* $end adder */
