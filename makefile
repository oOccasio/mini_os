# 컴파일러 및 옵션
CC = gcc
CFLAGS = -fcommon -w -D _GNU_SOURCE -Iheader -pthread

# 디렉토리 설정
SRCDIR = src
OBJDIR = obj

# 소스 파일 목록 (src 하위 모든 .c 파일)
SRC = $(shell find $(SRCDIR) -name '*.c')

# 오브젝트 파일 목록 (obj 디렉토리에 .o로 생성)
OBJ = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRC))

# 실행파일 이름 (bin 폴더 없이 최상위에 생성)
TARGET = mini_os

# 기본 타겟: 빌드
all: $(TARGET)

# 링크 단계: 오브젝트 파일들을 연결해서 실행파일 생성
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@

# 오브젝트 파일 컴파일
$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# 디렉토리 없으면 생성
$(OBJDIR):
	mkdir -p $(OBJDIR)

# 클린 (빌드 결과물 삭제)
clean:
	rm -rf $(OBJDIR) $(TARGET)

.PHONY: all clean
