// 程序要用的头文件
#include<stdio.h>
#include<stdlib.h>
#include<ctype.h>
#include<math.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<sys/wait.h>
#include<signal.h>


// 程序要用的宏定义
#define MAX_HISTORY 12	// 最多保存的历史命令条数
#define BUF_SIZE 256	// 缓存区大小
#define PROMPT " ysh@%s> "	// 提示符

// 存放历史命令的循环数组
typedef struct ENV_HISTORY {
	int start;
	int end;
	char history_command[MAX_HISTORY][128];
}ENV_HISTORY;

// 作业链表
typedef struct NODE {
	pid_t pid;
	char command[100];
	char state[10];
	struct NODE* link;
}NODE;

/**
 * env_path 环境路径
 * buf 缓存
 * _pid 进程id
 * sig_flag 
 * sig_z 
 * env_history 历史命令
 * head 作业链表头
 * end 作业链表尾
 */
char *env_path[10], buf[BUF_SIZE];
pid_t _pid;  // 前台作业的pid，没有前台作业时为0
ENV_HISTORY env_history;
NODE *head, *end;



// 程序要用的函数声明

/**
 * @brief 初始化查找路径.
 * @param str profile文件中的字段
 */
void init_path(char* str);

/**
 * @brief 初始化环境.
 */
void init_environment();

/**
 * @brief 向ENV_HISTORY中记录命令.
 */
void add_history_command(char* input);

/**
 * @brief 显示历史命令.
 */
void show_history_command();

/**
 * @brief 查找命令.
 * @param exec 外部可执行程序的名称
 */
int is_founded(char* exec);

/**
 * @brief 重定向命令的处理.
 * @param in 输入的命令
 * @param len 命令的长度
 */
int redirect(char* in, int len);

/**
 * @brief 管道命令的处理.
 */
int pipel();

/**
 * @brief 处理cd命令.
 */
void cd(char* route);

/**
 * @brief 处理jobs命令.
 */
void jobs();

/**
 * @brief 向jobs命令的任务链表中增加节点.
 * @param command 命令名称
 * @param pid 命令的pid
 */
void add_job_node(char* command, pid_t pid);

/**
 * @brief 从jobs命令的任务链表中删除节点.
 */
void del_job_node(int sig, siginfo_t sig_info);

/**
 * @brief 处理 CTRL + Z 快捷键, 将前台程序挂起.
 */
void ctrl_z();


/**
 * @breif 处理 CTRL + C 快捷键, 将前台程序强制杀死.
 */
void ctrl_c();

/**
 * @brief 处理bg命令, 将指定命令放到后台执行.
 */
void bg(pid_t pid);

/**
 * @brief 处理fg命令, 将指定命令放到前台执行.
 */
void fg(pid_t pid);