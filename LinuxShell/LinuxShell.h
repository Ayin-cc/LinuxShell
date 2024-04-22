#pragma once

// ����Ҫ�õ�ͷ�ļ�
#include<stdio.h>
#include<ctype.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<sys/wait.h>
#include<math.h>
#include<signal.h>
#include<stdlib.h>

// ����Ҫ�õĺ궨��
#define MAX_HISTORY 12	// ��ౣ�����ʷ��������
#define BUF_SIZE 256	// ��������С

// �����ʷ����Ľṹ
typedef struct ENV_HISTORY {
	int start;
	int end;
	char history_command[MAX_HISTORY][100];
}ENV_HISTORY;

// ��ҵ����
typedef struct NODE {
	pid_t pid;
	char command[100];
	char state[10];
	struct NODE* link;
}NODE;

/*  ������Ҫ�õĺ�������  */
/*
 *
 */