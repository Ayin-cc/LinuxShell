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
int sig_z;
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
void add_history_command();

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
void cd();

/**
 * @brief ����jobs����.
 */
void jobs();

/**
 * @brief ��jobs������������������ӽڵ�.
 */
void add_job_node();

/**
 * @brief ��jobs���������������ɾ���ڵ�.
 */
void del_job_node();

/**
 * @brief ���� CTRL + Z ��ݼ�.
 */
void ctrl_z();

/**
 * @brief ����־λ����Ϊ1.
 */
void set_flag();

/**
 * @brief ����bg����.
 */
void bg();

/**
 * @brief ����fg����.
 */
void fg();