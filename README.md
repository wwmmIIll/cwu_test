팩맨 (맥용) + 윈도우 버전은 추가 예정
<프로그램 실행 방법>
1. Terminal에 'brew install ncurses'입력, ncurses 라이브러리 다운로드
2. Terminal에 아래와 같이 입력하면 실행 가능
cd "프로젝트 경로" && gcc pacman.c -o pacman -lncurses && "프로젝트 경로/pacman" 

<현재 구현되어 있는 기능>
1. 맵
2. 팩맨 캐릭터 조작
3. 고스트 AI
4. 맵 충돌(팩맨이 맵을 통과하지 않음) 
