/* $begin tinymain */
/*
 * tiny.c - A simple, iterative HTTP/1.0 Web server that uses the
 *     GET method to serve static and dynamic content.
 *
 * Updated 11/2019 droh
 *   - Fixed sprintf() aliasing issue in serve_static(), and clienterror().
 */
#include "csapp.h"

void doit(int fd);
void read_requesthdrs(rio_t *rp);
int parse_uri(char *uri, char *filename, char *cgiargs);
void serve_static(int fd, char *filename, int filesize, char *method);
void get_filetype(char *filename, char *filetype);
void serve_dynamic(int fd, char *filename, char *cgiargs, char *method);
void clienterror(int fd, char *cause, char *errnum, char *shortmsg,
                 char *longmsg);

int main(int argc, char **argv)
{
  int listenfd, connfd; // 리스닝 소켓 fd, 연결된 클라이언트 소켓 fd
  char hostname[MAXLINE], port[MAXLINE];
  socklen_t clientlen;
  struct sockaddr_storage clientaddr; // 클라이언트 소켓 주소 정보

  /* Check command line args */
  if (argc != 2)
  { // 명령줄 인자 검사
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }
  // 리스닝 소켓 초기화, argv[1]에서 받은 포트 번호를 사용하여 리스닝 소켓 초기화, 이 소켓 fd 할당
  listenfd = Open_listenfd(argv[1]);
  while (1)
  { // 클라이언트의 연결 요청 기다림
    clientlen = sizeof(clientaddr);
    // listenfd 를 통해 들어오는 연결을 수락하고, 연결된 클라이언트 주소정보를 clientaddr에 저장
    // 연결된 소켓의 fd를 connfd에 할당
    connfd = Accept(listenfd, (SA *)&clientaddr,
                    &clientlen); // line:netp:tiny:accept
    // 클라이언트 소켓 주소로부터 호스트 이름, 포트 번호 추출
    Getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE,
                0);
    printf("Accepted connection from (%s, %s)\n", hostname, port);
    // 클라이언트와의 연결을 통해 특정 작업 수행
    doit(connfd);  // line:netp:tiny:doit
    Close(connfd); // line:netp:tiny:close
  }
}

// 클라이언트의 요청 라인을 확인해 정적, 동적 컨텐츠를 확인하고 돌려줌
void doit(int fd) // fd는 connfd
{
  int is_static;
  struct stat sbuf;
  char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
  char filename[MAXLINE], cgiargs[MAXLINE]; // cgiargs : 클라이언트 요청에 포함한 쿼리 스트링 또는 동적 컨텐츠 생성을 위해 필요한 인자 저장
  rio_t rio;

  /* Read request line and headers */
  Rio_readinitb(&rio, fd);           // 클라이언트 연결 fd를 사용하여 rio 초기화
  Rio_readlineb(&rio, buf, MAXLINE); // 클라이언트로부터 HTTP 요청 헤더 첫 줄을 읽는다
  printf("Request headers:\n");
  printf("%s", buf);
  sscanf(buf, "%s %s %s", method, uri, version);               // buf의 내용을 method, uri, version이라는 문자열에 저장
  if (strcasecmp(method, "GET") && strcasecmp(method, "HEAD")) // 두 문자열이 동일한 경우 0을 반환 -> GET이면 0
  {
    clienterror(fd, method, "501", "Not implemented", "Tiny does not implement this method");
    return;
  }
  read_requesthdrs(&rio); // 나머지 HTTP 요청 헤더를 읽고 처리

  /* Parse URI form GET request */
  // URI를 filename과 CGI argument string으로 parse하고
  // request가 static인지 dynamic인지 확인하는 flag return(1이면 static)
  is_static = parse_uri(uri, filename, cgiargs); // uri 바탕으로 filename, cgiargs 채워짐
  if (stat(filename, &sbuf) < 0)                 // disk에 파일이 없으면 filename을 sbuf에 넣는다. 종류 크기등이 sbuf에 저장됨, 성공하면 0 실패하면 -1
  {
    clienterror(fd, filename, "404", "Not found", "Tiny couldn't find this file");
    return;
  }

  if (is_static)
  { // Serve static content
    // S_ISREG -> 파일 종류 확인: 일반(regular) 파일인지 판별
    // 읽기 권한(S_IRUSR)을 가지고 있는지 판별
    if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode))
    {
      clienterror(fd, filename, "403", "Forbidden", // 읽기 권한이 없거나 정규파일 아니면
                  "Tiny couldn't read the file");   // 읽을 수 없다.
      return;
    }
    serve_static(fd, filename, sbuf.st_size, method); // fd: connfd 정적(static) 컨텐츠를 클라이언트에게 제공
  }
  else
  {
    if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode))
    {
      clienterror(fd, filename, "403", "Forbidden", "Tiny couldn't run the CGI program");
      return;
    }
    serve_dynamic(fd, filename, cgiargs, method); // 실행 가능하면 동적 컨텐츠를 클라이언트에게 제공
  }
}

// 웹 서버가 클라이언트에게 오류 메시지를 전송하는 역할
// 이 함수는 HTTP 오류 응답을 구성하고, 해당 응답을 클라이언트에게 보내는 과정을 담당
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg)
{
  char buf[MAXLINE], body[MAXBUF]; // 헤더 저장 배열, 응답 본문 저장 배열

  // Build the HTTP response body
  // sprintf는 출력하는 결과 값을 변수에 저장하게 해주는 기능있음
  sprintf(body, "<html><title>Tiny Error</title>");
  sprintf(body, "%s<body bgcolor="
                "ffffff"
                ">\r\n",
          body);
  sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
  sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
  sprintf(body, "%s<hr><em>The Tiny Web server</em>\r\n", body); // 마지막에는 서버의 이름을 나타내는 <em> 태그

  // Print the HTTP response
  sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
  Rio_writen(fd, buf, strlen(buf)); // 구성된 상태 라인을 클라이언트에게 전송
  sprintf(buf, "Content-type: text/html\r\n");
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
  Rio_writen(fd, buf, strlen(buf));
  Rio_writen(fd, body, strlen(body));
}

// Tiny는 request header 정보를 하나도 사용하지 않는다
//  요청 라인 한줄, 요청 헤더 여러줄 받는다
//  요청 라인은 저장(tiny에서 필요한 것), 나머지는 그냥 출력
void read_requesthdrs(rio_t *rp)
{
  char buf[MAXLINE];

  Rio_readlineb(rp, buf, MAXLINE); // 한줄 읽어 들인다
  while (strcmp(buf, "\r\n"))      // 마지막 줄 만나면 탈출
  {
    Rio_readlineb(rp, buf, MAXLINE);
    printf("%s", buf);
  }
  return;
}

// Tiny는 정적 컨텐츠를 위한 홈 디렉토리가 자신의 현재 디렉토리이고,
// 실행 파일의 홈 디렉토리는 /cgi-bin이라고 가정한다
int parse_uri(char *uri, char *filename, char *cgiargs)
{
  char *ptr;
  // strstf(대상 문자열, 검색할 문자열) -> 검색된 문자열(대상 문자열) 뒤에 모든 문자열이 나온다
  // uri에 cgi-bin이라는 문자열이 없으면 정적 컨텐츠
  if (!strstr(uri, "cgi-bin"))
  {
    strcpy(cgiargs, "");             // cgiargs 인자 스트링을 지운다
    strcpy(filename, ".");           // 상대 리눅스 경로이름으로 변환(현재 작업중인 디렉토리를 기준으로 파일이나 디렉토리의 위치를 나타낸다)
    strcat(filename, uri);           // 상대 리눅스 경로이름으로 변환
    if (uri[strlen(uri) - 1] == '/') // uri가 /로 끝난다면
    {
      strcat(filename, "home.html"); // 마지막에 home.html 추가
    }
    return 1;
  }
  else // uri에 포함되어 있으면 동적 컨텐츠
  {
    ptr = index(uri, '?'); // ?문자를 찾아 CGI인자 시작위치를 찾는다
    if (ptr)
    {
      // ? 다음부터 끝까지 cgiargs에 복사
      // cgiargs는 CGI 스크립트에 전달될 인자
      strcpy(cgiargs, ptr + 1);
      *ptr = '\0'; // ?위치에 NULL문자 삽입하여 URI를 두부분으로 나눈다. 파일이름 부분이 종료
    }
    else // ?가 없으면
    {
      strcpy(cgiargs, ""); // cgi가 없으므로 빈 문자열
    }
    strcpy(filename, "."); // 파일 이름의 기본 경로를 현재 디렉토리로
    strcat(filename, uri); // 변경된 URI(CGI 스크립트 경로)를 기본 경로에 추가
    return 0;              // 동적 컨텐츠를 처리하고 있음 : 0
  }
}

// 웹 서버에서 정적 컨텐츠를 클라이언트에게 제공하는 기능
// static content를 요청하면 서버가 disk에서 파일을 찾아서 메모리 영역으로
// 복사하고, 복사한 것을 client fd로 복사
void serve_static(int fd, char *filename, int filesize, char *method)
{
  int srcfd;
  char *srcp, filetype[MAXLINE], buf[MAXBUF];

  /* Send response headers to client */
  get_filetype(filename, filetype); // 주어진 파일 이름에 따라 컨텐츠 유형 결정
  sprintf(buf, "HTTP/1.0 200 OK\r\n");
  sprintf(buf, "%sServer: Tiny Web Server\r\n", buf);
  sprintf(buf, "%sConnection: close\r\n", buf); // while을 한번돌면 close가 되고, 새로 연결하더라도 새로 connect하므로 close가 default가됨
  sprintf(buf, "%sContent-length: %d\r\n", buf, filesize);
  sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype); // 여기 \r\n 빈줄 하나가 헤더종료표시
  Rio_writen(fd, buf, strlen(buf));                          // buf에서 strlen(buf) 바이트만큼 fd로 전송한다. buf는 가만히 있고 그 함수안에서 sbuf같은걸 설정해서~.~
  printf("Response headers: \n");
  printf("%s", buf);

  /* Send response body to client */
  if (strcasecmp(method, "GET") == 0)
  {
    srcfd = Open(filename, O_RDONLY, 0); // 읽기 전용으로 연다

    // 파일의 내용을 메모리에 매핑, 파일을 효율적으로 읽어서 직접 메모리에 클라이언트로 전송할 수 있게 함
    // void *addr : 0 또는 NULL을 지정하면 커널이 적절한 주소 자동으로 선택
    // 매핑한 파일의 크기
    // 매핑된 메모리 보호수준 -> 읽기 가능함
    // 매핑된 메모리의 특성 -> 프라이빗 매핑
    // 매핑한 파일의 디스크립터
    // 시작할 오프셋
    // Mmap을 이용한 정적 컨텐츠 처리
    srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
    Close(srcfd);                   // 이미 메모리에 매핑되었기 때문에 파일 디스크립터 닫는다
    Rio_writen(fd, srcp, filesize); // 메모리에 매핑된 파일 내용을 클라이언트에 전송
    Munmap(srcp, filesize);         // 사용이 완료된 메모리 매핑을 해제
  }

  // srcp = (char *)malloc(filesize);
  // if (srcp == NULL)
  // {
  //   perror("Error : Memory allocation failed");
  //   return;
  // }
  // Rio_readn(srcfd, srcp, filesize); // 지정된 바이트 수가 완전히 읽힐 때까지 반복적으로 fd로부터 데이터를 읽는다
  // Close(srcfd);
  // Rio_writen(fd, srcp, filesize);
  // free(srcp);
}

void get_filetype(char *filename, char *filetype)
{
  if (strstr(filename, ".html"))   // filename 문자열 안에 ".html"이 있는지 검사
    strcpy(filetype, "text/html"); // MIME타입 문자열을 filetype 변수에 복사
  else if (strstr(filename, ".gif"))
    strcpy(filetype, "image/gif");
  else if (strstr(filename, ".png"))
    strcpy(filetype, "image/png");
  else if (strstr(filename, ".jpg"))
    strcpy(filetype, "image/jpeg");
  else if (strstr(filename, ".mp4"))
    strcpy(filetype, "video/mp4");
  else if (strstr(filename, ".mpeg"))
    strcpy(filetype, "video/mpeg");
  else
    strcpy(filetype, "text/plain");
}

// 웹 서버에서 동적 컨텐츠를 처리하기 위한 함수
// CGI프로그램을 실행하여 클라이언트 요청에 응답.
// 이 함수는 CGI 프로그램을 실행하기 전에 필요한 HTTP 응답 헤더를 전송하고,
// CGI 프로그램의 실행 결과를 클라이언트에게 직접 출력
void serve_dynamic(int fd, char *filename, char *cgiargs, char *method)
{
  char buf[MAXLINE], *emptylist[] = {NULL};

  /* Return first part of HTTP response */
  // 응답이 성공적임을 알리는 헤더를 버퍼에 저장
  sprintf(buf, "HTTP/1.0 200 OK\r\n");
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Server : Tiny Web Server\r\n");
  Rio_writen(fd, buf, strlen(buf));

  if (Fork() == 0) // Fork가 0을 반환하면 현재 코드 블록은 자식 프로세스에서 실행
  {
    /* Real server would set all CGI vars here */
    setenv("QUERY_STRING", cgiargs, 1);   // 환경 변수 설정. CGI프로그램이 클라이언트로부터 전달 받은 인자를 처리하는 표준 방법
    setenv("REQUEST_METHOD", method, 1);  // method를 cgi-bin/adder.c에 넘겨주기 위해 환경변수 set
    Dup2(fd, STDOUT_FILENO);              // 표준 출력 리다이렉션. CGI프로그램의 출력이 직접 클라이언트에게 전송
    Execve(filename, emptylist, environ); // 프로그램 실행
  }
  Wait(NULL); // 부모 프로세스는  Wait을 호출해 자식 프로세스의 종료를 대기, 종료 시 자원 회수
}