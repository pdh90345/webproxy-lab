####################################################################
# CS:APP Proxy Lab
#
# Student Source Files
####################################################################

This directory contains the files you will need for the CS:APP Proxy
Lab.

proxy.c
csapp.h
csapp.c
    These are starter files.  csapp.c and csapp.h are described in
    your textbook. 

    You may make any changes you like to these files.  And you may
    create and handin any additional files you like.

    Please use `port-for-user.pl' or 'free-port.sh' to generate
    unique ports for your proxy or tiny server. 

Makefile
    This is the makefile that builds the proxy program.  Type "make"
    to build your solution, or "make clean" followed by "make" for a
    fresh build. 

    Type "make handin" to create the tarfile that you will be handing
    in. You can modify it any way you like. Your instructor will use your
    Makefile to build your proxy from source.

port-for-user.pl
    Generates a random port for a particular user
    usage: ./port-for-user.pl <userID>

free-port.sh
    Handy script that identifies an unused TCP port that you can use
    for your proxy or tiny. 
    usage: ./free-port.sh

driver.sh
    The autograder for Basic, Concurrency, and Cache.        
    usage: ./driver.sh

nop-server.py
     helper for the autograder.         

tiny
    Tiny Web server from the CS:APP text

proxy.c: 프록시 서버의 메인 소스 파일입니다. 이 파일에는 프록시 서버의 기본적인 로직이 구현됩니다.
csapp.h, csapp.c: 이 두 파일은 교재에서 사용되는 유틸리티 함수들을 포함하고 있습니다. 예를 들어, 네트워크 I/O, 프로세스 관리, 파일 I/O 등의 함수가 포함되어 있습니다.
Makefile: 이 파일은 프록시 프로그램을 빌드하기 위한 메이크파일입니다. make 명령어를 사용하여 프로그램을 빌드하거나, make clean 다음에 make를 사용하여 새로 빌드할 수 있습니다. 또한, make handin을 통해 제출 파일(tar 파일)을 생성할 수 있습니다.
port-for-user.pl: 특정 사용자에 대해 무작위 포트를 생성하는 스크립트입니다. 사용법은 ./port-for-user.pl <userID>로, 사용자 ID를 인자로 제공합니다.
free-port.sh: 사용되지 않는 TCP 포트를 찾아주는 스크립트입니다. 프록시 서버나 tiny 서버를 위한 포트를 확인할 때 사용할 수 있습니다.
driver.sh: 기본 기능, 동시성, 캐시에 대한 자동 평가를 수행하는 스크립트입니다.
nop-server.py: 자동 평가를 돕는 헬퍼 스크립트입니다.
tiny: CS:APP 교재에서 설명하는 Tiny 웹 서버입니다. 이 웹 서버는 프록시 랩의 일부로 사용될 수 있습니다.