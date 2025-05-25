#include "../header/Header.h"

int clear() {
	printf("\033[2J\033[H");//ANSI 시퀀스 이용. \033[2j : 화면 지우기, \033[H : 커서 왼쪽 위로로
	return 0;
}