# Simple FTP Server
>FTP server(RFC959) in C language.
RFC959 표준을 지키는 FTP 서버 입니다.  
현재 Passive 모드만 지원합니다. 향후 Active 모드를 지원할 예정입니다.  

![](../header.png)


## 필요 라이브러리  
libpcap-dev     : 패킷관련 라이브러리 (사용은 안하는데, 없으면 이상하게 오류가남...)  
libssl-dev      : 암호화를 위해 필요한 라이브러리  
libsqlite3-dev  : FTP 계정 인증을 위한 라이브러리  
```sh
sudo apt-get install libpcap-dev libssl-dev
sudo apt-get install libsqlite3-dev
```

## 컴파일 방법  
```sh
gcc server.c -o server -pthread -lssl -lcrypto -lsqlite3
gcc client.c -o client2 -lssl -lcrypto
```

## 실행방법  
```sh
sudo ./server
sudo ./client2 127.0.0.1 21
```
서버에 별 다른 인자값을 주지 않고 실행 할 경우 21번 포트를 디폴트로 사용합니다.  
이때, 리눅스 환경에서는 1024번 이하의 포트는 관리자권한(su)을 요구하므로 sudo 명령을 필요로 합니다.

## 프로그램 문서
https://www.notion.so/FTP-Server-af8e30094e7a47f0b5b1b756f4333c6c

## 업데이트 내역

* 0.0.1  
    * 작업 진행 중  

## 정보

배명건 – talk@kakao.one  

GPL 3.0 라이센스를 준수하며 ``LICENSE``에서 자세한 정보를 확인할 수 있습니다.  
[https://opensource.org/licenses/gpl-3.0.html]

## 기여 방법

1. (<https://github.com/bsy0317/Simple-FTP-Server/fork>)을 포크합니다.
2. (`git checkout -b feature/fooBar`) 명령어로 새 브랜치를 만드세요.
3. (`git commit -am 'Add some fooBar'`) 명령어로 커밋하세요.
4. (`git push origin feature/fooBar`) 명령어로 브랜치에 푸시하세요. 
5. 풀리퀘스트를 보내주세요.
