/*****************************************************************//**
 * @file   LinuxShell.c
 * @brief  A simple Linux Shell supports external command, pipe, I/O redirect and background job.
 *         It's written in C and built by CMake.
 * @author Ayin
 * @date   April 2024
 *********************************************************************/

#include "LinuxShell.h"

int main() {
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
		char c, *arg[20], path[100], *input = NULL;
		int i = 0, j = 0, k = 0, is_pr = 0, is_bg = 0, input_len = 0, status = 0;
		pid_t pid = 0;

		/** ���� Singal */
		struct sigaction act;
		act.sa_handler = del_job_node;
		sigfillset(&act.sa_mask);
		act.sa_flags = SA_SIGINFO;
		sigaction(SIGCHLD, &act, NULL);
		signal(SIGTSTP, ctrl_z);
		signal(SIGINT, ctrl_c);

		/** ��ӡ��ʾ�� */
		getcwd(path, sizeof(char) * 100);
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
					add_history_command(input);
					pipel();					
					free(input);
				}
				else {  // �ض�������
					add_history_command(input);
					redirect(input, input_len);					
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
			add_history_command(input);

			printf(" Bye!\n");

			for (i = 0; i < k; i++) {
				free(arg[i]);
			}
			free(input);
			break;
		}
		if(strcmp(arg[0], "history") == 0){  // history ����
			add_history_command(input);

			show_history_command();

			for (i = 0; i < k; i++) {
				free(arg[i]);
			}
			free(input);
			continue;
		}
		if(strcmp(arg[0], "cd") == 0){  // cd ����
			add_history_command(input);

			cd(arg[1]);

			for (i = 0; i < k; i++) {
				free(arg[i]);
			}
			free(input);
			continue;
		}
		if(strcmp(arg[0], "jobs") == 0){ // jobs ����
			add_history_command(input);

			jobs();

			for (i = 0; i < k; i++) {
				free(arg[i]);
			}
			free(input);
			continue;
		}
		if(strcmp(arg[0], "bg") == 0){  // bg ����
			add_history_command(input);
			if (k != 2) {
				printf("Usage: bg <pid>.\n");
				continue;
			}

			bg(atoi(arg[1]));

			for (i = 0; i < k; i++) {
				free(arg[i]);
			}
			free(input);
			continue;
		}
		if(strcmp(arg[0], "fg") == 0){  // fg ����
			add_history_command(input);
			if (k != 2) {
				printf("Usage: fg <pid>.\n");
				continue;
			}

			fg(atoi(arg[1]));

			for (i = 0; i < k; i++) {
				free(arg[i]);
			}
			free(input);
			continue;
		}

		/** 4. �ⲿ���� */
		if (is_pr == 0) {
			add_history_command(input);

			if (is_founded(arg[0]) == 0) {
				printf("%s: Executable program not found.\n", arg[0]);
				for (i = 0; i < k; i++) {
					free(arg[i]);
				}
				free(input);
				continue;
			}

			// ִ������
			if ((pid = fork()) == -1) {
				printf("%s: Failed to execute.\n", arg[0]);
			}
			else if (pid == 0) {  // �ӽ���
				if (is_bg == 0) {
					execvp(arg[0], arg);
					// �ӽ���δ���ɹ�ִ��
					printf("%s: Error command.\n", arg[0]);
					exit(1);
				}
				if (is_bg == 1) {
					freopen("/dev/null", "w", stdout);
					freopen("/dev/null", "r", stdin);

					execvp(arg[0], arg);
					// �ӽ���δ���ɹ�ִ��
					printf("%s: Error command.\n", arg[0]);
				}
			}
			else if(pid > 0) {  // ������
				// ���ӽ��̼�������ڵ�
				add_job_node(arg[0], pid);

				if (is_bg == 0) {
					_pid = pid;
					waitpid(pid, &status, WUNTRACED | WCONTINUED);
					int err = WEXITSTATUS(status);
					if (err) {
						printf("Error: %s\n", strerror(err));
					}
					_pid = 0;
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

void init_path(char* str) {
	int i = 0, j = 0, k = 0;
	char c, buf[64], temp[128], *path;
	while ((c = str[i]) != '=') {
		buf[i++] = c;
		if (i > 63) {
			printf("Init environment error. The profile param name out of range.");
			exit(1);
		}
	}
	buf[i++] = '\0';

	if (strcmp(buf, "PATH") == 0) {
		while (str[i] != '\0') {
			if (str[i] == ':') {  // ð��Ϊ�ָ�����ȡ���öε�ַ
				if (j >= 128) {
					printf("Init environment error. The path length out of range.");
					exit(1);
				}
				temp[j++] = '/';
				temp[j] = '\0';

				path = (char*)malloc(j);
				strcpy(path, temp);

				if (k >= 9) {
					printf("Init environment error. Num of environment path out of range.");
					exit(1);
				}
				env_path[k++] = path;
				env_path[k] = NULL;
				//printf("%s\n", env_path[k - 1]);

				j = 0;
				i++;
			}
			else {
				temp[j++] = str[i++];
			}
		}
	}
	
}

void init_environment() {
	char buf[1024];
	FILE* fptr;
	if ((fptr = fopen("ysh_profile", "r")) == NULL) {
		printf(" Init environment error. Unable to open 'ysh_profile'.\n");
		exit(1);
	}

	while (fgets(buf, sizeof(buf), fptr)) {
		init_path(buf);
	}

	_pid = 0;
	strcpy(env_history.history_command[0], "Init the environment");
	env_history.start = 0;
	env_history.end = 0;
	head = NULL;
	end = NULL;
}

void add_history_command(char* input) {
	env_history.end = (env_history.end + 1) % MAX_HISTORY;
	if (env_history.end == env_history.start) {  // ���鿪ʼѭ��
		env_history.start = (env_history.start + 1) % MAX_HISTORY;
	}
	strcpy(env_history.history_command[env_history.end], input);
}

void show_history_command() {
	int index = 0;
	if (env_history.start == env_history.end) {  // ��
		return;
	}
	else if (env_history.start < env_history.end) {  // 0 ~ end
		for (int i = 0; i <= env_history.end; i++) {
			printf(" %d\t%s\n", index++, env_history.history_command[i]);
		}
	}
	else {  // start ~ MAX + 0 ~ end;
		for (int i = env_history.start; i < MAX_HISTORY; i++) {
			printf(" %d\t%s\n", index++, env_history.history_command[i]);
		}
		for (int i = 0; i <= env_history.end; i++) {
			printf(" %d\t%s\n", index++, env_history.history_command[i]);
		}
	}
}

int is_founded(char* exec) {
	int i = 0;
	while (env_path[i] != NULL) {
		strcpy(buf, env_path[i]);
		strcat(buf, exec);
		if (access(buf, F_OK) == 0) {
			return 1;
		}
		i++;
	}

	return 0;
}

int redirect(char* in, int len) {
	return 0;
}

int pipel() {
	return 0;
}

void cd(char* route) {
	if (route != NULL) {
		if (chdir(route) < 0) {
			printf("System can not find the specified path '%s.\n", route);
		}
	}
}

void jobs() {
	if (head != NULL) {
		NODE* node = head;
		int index = 0;

		while (node != NULL) {
			printf("%d\t%d\t%s\t%s\n", index, node->pid, node->state, node->command);
			node = node->link;
		}
	}
	else {
		printf("No jobs.\n");
	}
}

void add_job_node(char* command, pid_t pid) {
	NODE* new_job;
	new_job = (NODE*)malloc(sizeof(NODE));
	strcpy(new_job->command, command);
	new_job->pid = pid;
	strcpy(new_job->state, "running");
	new_job->link = NULL;
	// ��������
	if (head == NULL) {
		head = new_job;
		end = new_job;
	}
	else {
		end->link = new_job;
		end = new_job;
	}
}

void del_job_node(int sig, siginfo_t* sig_info) {
	if (head == NULL) {
		return;
	}

	// ��������ɾ����Ӧ�ڵ�
	NODE* node = head;
	int pid = sig_info->si_pid;
	if (node->pid == pid) {  // Ҫɾ���Ľڵ�Ϊͷ���
		head = head->link;
		free(node);
		return;
	}

	while (node->pid != pid && node->link != NULL) {
		node = node->link;
	}
	NODE* pre = head;
	while (pre->link != link) {
		pre = pre->link;
	}
	pre->link = node->link;
	free(node);

	return;
}

void ctrl_z() {
	if (_pid == 0) {  // ǰ̨û�����г���
		return;
	}

	NODE* node = head;
	while (node->pid != _pid) {
		node = node->link;
		if (node == NULL) {
			printf("Failed to stop.\n");
			return;
		}
	}
	strcpy(node->state, "stopped");
	kill(_pid, SIGSTOP);
	printf('\n');
	jobs();

	return;
}

void ctrl_c() {
	if (_pid == 0) {  // ǰ̨û�����г���
		return;
	}
	NODE* node = head;
	while (node->pid != _pid) {
		node = node->link;
		if (node == NULL) {
			printf("Failed to terminate.\n");
			return;
		}
	}
	kill(_pid, SIGTERM);
	printf('\n');
	
	return;
}

void bg(pid_t pid) {
	NODE* node = head;
	while (node->pid != pid) {
		node = node->link;
		if (node == NULL) {
			printf("No such job. Please check the pid.\n");
			return;
		}
	}
	strcpy(node->state, "running");
	kill(node->pid, SIGCONT);

	return;
}

void fg(pid_t pid) {
	NODE* node = head;
	while (node->pid != pid) {
		node = node->link;
		if (node == NULL) {
			printf("No such job. Please check the pid.\n");
			return;
		}
	}

	_pid = pid;
	strcpy(node->state, "running");
	kill(node->pid, SIGCONT); 
	waitpid(node->pid, NULL, 0);

	return;
}
