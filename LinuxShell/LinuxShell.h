// ����Ҫ�õ�ͷ�ļ�
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


// ����Ҫ�õĺ궨��
#define MAX_HISTORY 12	// ��ౣ�����ʷ��������
#define BUF_SIZE 256	// ��������С
#define PROMPT " ysh@%s> "	// ��ʾ��

// �����ʷ�����ѭ������
typedef struct ENV_HISTORY {
	int start;
	int end;
	char history_command[MAX_HISTORY][128];
}ENV_HISTORY;

// ��ҵ����
typedef struct NODE {
	pid_t pid;
	char command[100];
	char state[10];
	struct NODE* link;
}NODE;

/**
 * env_path ����·��
 * buf ����
 * _pid ����id
 * sig_flag 
 * sig_z 
 * env_history ��ʷ����
 * head ��ҵ����ͷ
 * end ��ҵ����β
 */
char *env_path[10], buf[BUF_SIZE];
pid_t _pid;  // ǰ̨��ҵ��pid��û��ǰ̨��ҵʱΪ0
ENV_HISTORY env_history;
NODE *head, *end;



// ����Ҫ�õĺ�������

/**
 * @brief ��ʼ������·��.
 * @param str profile�ļ��е��ֶ�
 */
void init_path(char* str);

/**
 * @brief ��ʼ������.
 */
void init_environment();

/**
 * @brief ��ENV_HISTORY�м�¼����.
 */
void add_history_command(char* input);

/**
 * @brief ��ʾ��ʷ����.
 */
void show_history_command();

/**
 * @brief ��������.
 * @param exec �ⲿ��ִ�г��������
 */
int is_founded(char* exec);

/**
 * @brief �ض�������Ĵ���.
 * @param in ���������
 * @param len ����ĳ���
 */
int redirect(char* in, int len);

/**
 * @brief �ܵ�����Ĵ���.
 */
int pipel();

/**
 * @brief ����cd����.
 */
void cd(char* route);

/**
 * @brief ����jobs����.
 */
void jobs();

/**
 * @brief ��jobs������������������ӽڵ�.
 * @param command ��������
 * @param pid �����pid
 */
void add_job_node(char* command, pid_t pid);

/**
 * @brief ��jobs���������������ɾ���ڵ�.
 */
void del_job_node(int sig, siginfo_t sig_info);

/**
 * @brief ���� CTRL + Z ��ݼ�, ��ǰ̨�������.
 */
void ctrl_z();


/**
 * @breif ���� CTRL + C ��ݼ�, ��ǰ̨����ǿ��ɱ��.
 */
void ctrl_c();

/**
 * @brief ����bg����, ��ָ������ŵ���ִ̨��.
 */
void bg(pid_t pid);

/**
 * @brief ����fg����, ��ָ������ŵ�ǰִ̨��.
 */
void fg(pid_t pid);