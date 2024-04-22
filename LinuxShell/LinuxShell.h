#pragma once

// 程序要用的头文件
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

// 程序要用的宏定义
#define MAX_HISTORY 12	// 最多保存的历史命令条数
#define BUF_SIZE 256	// 缓存区大小

// 存放历史命令的结构
typedef struct ENV_HISTORY {
	int start;
	int end;
	char history_command[MAX_HISTORY][100];
}ENV_HISTORY;

// 作业链表
typedef struct NODE {
	pid_t pid;
	char command[100];
	char state[10];
	struct NODE* link;
}NODE;

/*  程序中要用的函数声明  */
/*
 *
 */