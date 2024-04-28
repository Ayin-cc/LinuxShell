/*****************************************************************//**
 * @file   LinuxShell.c
 * @brief  A simple Linux Shell supports external command, pipe, I/O redirect and background job.
 *         It's for OS assignment, written in C and built by CMake.
 * @author Ayin
 * @date   April 2024
 *********************************************************************/

#include "LinuxShell.h"

int main(int argc, char argv[]) {
	init_environment();

	while (1) {
		/**
		 * ���ñ���
		 * arg �û������������������
		 * path ��ǰ����·��
		 * input �û����������
		 * is_pr �Ƿ�Ϊ�ܵ����ض�������
		 * is_bg �Ƿ�Ϊ��̨����
		 * input_len �û���������ĳ���
		 * status 
		 */
		char c, *arg[20], path[100], * input = NULL;
		int i = 0, j = 0, k = 0, is_pr = 0, is_bg = 0, input_len = 0, status = 0;
		pid_t pid = 0;

		/** ���� Singal */
		struct sigaction act;
		act.sa_handler = del_job_node;
		sigfillset(&act.sa_mask);
		act.sa_flags = SA_SIGINFO;
		sigaction(SIGCHLD, &act, NULL);
		signal(SIGTSTP, ctrl_z);

		printf(PROMPT, path);


		/** ��ȡ���� */
		while ((c = getchar()) == ' ' || c == '\t' || c == '\n' || c == EOF) {  // �����ո��������Ϣ
			if (c == '\n') {
				printf(PROMPT, path);
				continue;
			}
		}
		while (c != '\n') {
			buf[input_len++] = c;
			c = getchar();
		}
		buf[input_len] = '\0';
		//puts(buf);
		input = (char*)malloc(sizeof(char) * (input_len + 1));
		strcpy(input, buf);


		/** �������� */
		/** 1. �ܵ����ض������� */
		for (i = 0; i <= input_len; i++) {
			if (input[i] == '<' || input[i] == '>' | input[i] == '|') {
				if (input[i] == '|') {  // �ܵ�����
					pipel();
					add_history_command();
					free(input);
				}
				else {  // �ض�������
					redirect(input, input_len);
					add_history_command();
					free(input);
				}
				is_pr = 1;
				break;
			}
		}
		if (is_pr == 1)
			continue;

		/** 2. ����������� */
		for (i = 0, j = 0, k = 0; i <= input_len; i++) {
			if (input[i] == ' ' || input[i] == '\0') {
				if (j == 0) {  // ʡ�Զ�������Ŀո�
					continue;
				}
				else {  // ȡ��һ������
					buf[j++] = '\0';
					arg[k] = (char*)malloc(sizeof(char) * j);
					strcpy(arg[k++], buf);
					j = 0;
				}
			}
			else {
				if (i > 0 && input[i - 1] == ' ' && input[i] == '&' && input[i + 1] == '\0') {  // ��ִ̨��
					is_bg = 1;
					continue;
				}
				buf[j++] = input[i];
			}
		}

		/** 3. �ڲ����� */
		if (strcmp(arg[0], "exit") == 0) {  // exit ����
			add_history_command();
			printf("Bye!");
			for (i = 0; i < k; i++) {
				free(arg[i]);
			}
			free(input);
			break;
		}
		if(strcmp(arg[0], "history") == 0){  // history ����
			add_history_command();
			show_history_command();
			for (i = 0; i < k; i++) {
				free(arg[i]);
			}
			free(input);
			continue;
		}
		if(strcmp(arg[0], "cd") == 0){  // cd ����
			for (i = 0; i < k; i++) {
				free(arg[i]);
			}
			free(input);
			continue;
		}
		if(strcmp(arg[0], "jobs") == 0){ // jobs ����
			for (i = 0; i < k; i++) {
				free(arg[i]);
			}
			free(input);
			continue;
		}
		if(strcmp(arg[0], "bg") == 0){  // bg ����
			for (i = 0; i < k; i++) {
				free(arg[i]);
			}
			free(input);
			continue;
		}
		if(strcmp(arg[0], "fg") == 0){  // fg ����
			for (i = 0; i < k; i++) {
				free(arg[i]);
			}
			free(input);
			continue;
		}

		/** 4. �ⲿ���� */
		if (is_pr == 0) {
			if (is_founded(arg[0]) == 0) {
				printf(" %s: Executable program not found.\n", arg[0]);
				for (i = 0; i < k; i++) {
					free(arg[i]);
				}
				free(input);
				continue;
			}

			// ִ������
			if ((pid = fork()) == -1) {
				printf(" %s: Failed to execute.", arg[0]);
			}
			else if (pid == 0) {  // �ӽ���
				if (is_bg == 0) {
					execvp(arg[0], arg);
					// �ӽ���δ���ɹ�ִ��
					printf(" %s: Error command.", arg[0]);
					exit(1);
				}
				if (is_bg == 1) {
					freopen("/dev/null", "w", stdout);
					freopen("/dev/null", "r", stdin);
					// ����ڵ�, 
					add_job_node();
					signal(SIGCHLD, del_job_node);
					execvp(arg[0], arg);
					// �ӽ���δ���ɹ�ִ��
					printf(" %s: Error command.", arg[0]);
				}
			}
			else if(pid > 0) {  // ������
				if (is_bg == 0) {
					waitpid(pid, &status, 0);
					int err = WEXITSTATUS(status);
					if (err) {
						printf("Error: %s\n", strerror(err));
					}
				}
			}

			// �ͷű����ڴ�
			for (i = 0; i < k; i++) {
				free(arg[i]);
			}
			free(input);
		}
	}
}

void init_environment() {
}

void init_path() {
}

void add_history_command() {
}

void show_history_command() {
}

int is_founded(char* exec) {
	return 0;
}

int redirect(char* in, int len) {
	return 0;
}

int pipel() {
	return 0;
}

void cd() {
}

void jobs() {
}

void add_job_node() {
}

void del_job_node() {
}

void ctrl_z() {
}

void set_flag() {
}

void bg() {
}

void fg() {
}
