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
		char c, *arg[20], *input = NULL;
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
					pipel(input, input_len);
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
		if (strcmp(arg[0], "exit") == 0 || strcmp(arg[0], "quit") == 0) {  // exit/quit ����
			add_history_command(input);

			printf("%sBye!%s\n", COLOR_BLUE, COLOR_DEFINE);

			for (i = 0; i < k; i++) {
				free(arg[i]);
			}
			free(input);
			exit(0);
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
					exit(1);
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

int redirect(char* input, int input_len) {
	int redir_in = 0, redir_out = 0, flag_io = 0, is_bg = 0, i, j, k, status = 0;  // flag_io: 1����IN, 0����OUT
	char *file_in = NULL, *file_out = NULL, *arg[20];
	pid_t pid;

	memset(arg, sizeof(arg), NULL);

	// �����������
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
		else if ((input[i] == '<' || input[i] == '>') && i < input_len && input[i + 1] == ' ' && i > 0 && input[i - 1] == ' ') {  // �ض������
			if (input[i] == '<') {
				flag_io = 1;
				redir_in++;
				if (redir_in > 1) {
					printf("Error redirect command.\n");
					for (i = 0; i < k; i++) {
						free(arg[i]);
					}
					if (redir_in != 0) {
						free(file_in);
					}
					if (redir_out != 0) {
						free(file_out);
					}
					return 0;
				}
			}
			if (input[i] == '>') {
				flag_io = 0;
				redir_out++;
				if (redir_out > 1) {
					printf("Error redirect command.\n");
					for (i = 0; i < k; i++) {
						free(arg[i]);
					}
					free(input);
					free(file_in);
					free(file_out);
					return 0;
				}
			}

			// �ض�����ļ�
			i += 2;
			while (input[i] == ' ') {
				i++;
			}
			while (input[i] != '\0' && input[i] != ' ') {
				buf[j++] = input[i++];
			}
			buf[j] = '\0';
			if (flag_io == 1) {
				file_in = (char*)malloc(sizeof(char) * j);
				strcpy(file_in, buf);
			}
			else {
				file_out = (char*)malloc(sizeof(char) * j);
				strcpy(file_out, buf);
			}

			j = 0;
			continue;
		}
		else { // ��ͨ����
			if (i > 0 && input[i - 1] == ' ' && input[i] == '&' && input[i + 1] == '\0') {  // ��ִ̨��
				is_bg = 1;
				continue;
			}
			else {
				buf[j++] = input[i];
			}
		}
	}
	/*for (i = 0; i < k; i++) {
		printf("arg[%d]: '%s'\n", i, arg[i]);
	}
	printf("redirect: in-%d  out-%d\nredirect_file: in-%s  out-%s\n", redir_in, redir_out, file_in, file_out);*/

	// ���������Ƿ����
	if (is_founded(arg[0]) == 0) {
		printf("%s: Executable program not found.\n", arg[0]);
		for (i = 0; i < k; i++) {
			free(arg[i]);
		}
		if (redir_in != 0) {
			free(file_in);
		}
		if (redir_out != 0) {
			free(file_out);
		}
		return 0;
	}

	// ִ������
	if ((pid = fork()) == -1) {
		printf("%s: Failed to execute.\n", arg[0]);
	}
	else if (pid == 0) {  // �ӽ���
		// �ض���, ����ʹ���ļ����ᵼ���ļ�ĩβ�������룬���ʹ��dup2�����ļ���ʶ��fd
		if (redir_in == 1) { // �����ض���
			int fd_in = open(file_in, O_RDONLY, S_IRUSR | S_IWUSR);
			if (fd_in == -1) {
				printf("Cannot open '%s'.\n", file_in);
				for (i = 0; i < k; i++) {
					free(arg[i]);
				}
				if (redir_in != 0) {
					free(file_in);
				}
				if (redir_out != 0) {
					free(file_out);
				}
				exit(1);
			}
			if (dup2(fd_in, STDIN_FILENO) == -1) {
				printf("Failed to redirect to '%s'.\n", file_in);
				exit(1);
			}
			close(fd_in);
		}
		else {
			if (is_bg == 1) {
				freopen("/dev/null", "r", stdin);
			}
		}
		if (redir_out == 1) { // ����ض���
			int fd_out = open(file_out, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
			if (fd_out == -1) {
				printf("Cannot open '%s'.\n", file_out);
				for (i = 0; i < k; i++) {
					free(arg[i]);
				}
				if (redir_in != 0) {
					free(file_in);
				}
				if (redir_out != 0) {
					free(file_out);
				}
				exit(1);
			}
			if (dup2(fd_out, STDOUT_FILENO) == -1) {
				printf("Failed to redirect to '%s'.\n", file_out);
				exit(1);
			}
			close(fd_out);
		}
		else {
			if (is_bg == 1) {
				freopen("/dev/null", "w", stdout);
			}
		}
		execvp(arg[0], arg);
		// �ӽ���δ���ɹ�ִ��
		printf("%s: Error command.\n", arg[0]);
		exit(1);
	}
	else if (pid > 0) {  // ������
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
	if (redir_in != 0) {
		free(file_in);
	}
	if (redir_out != 0) {
		free(file_out);
	}

	return 1;
}

int pipel(char* input, int input_len) {
	int i = 0, j = 0, k = 0, is_bg = 0, cmd_count = 0, redir_out = 0, redir_in = 0, status = 0;
	char* arg[12][20], * file_in = NULL, * file_out = NULL;
	pid_t pid;

	// �����������
	for (i = 0, j = 0, k = 0; i <= input_len; i++) {
		if (input[i] == ' ' || input[i] == '\0') {
			if (j == 0) {  // ʡ�Զ�������Ŀո�
				continue;
			}
			else {  // ȡ��һ������
				buf[j++] = '\0';
				arg[cmd_count][k] = (char*)malloc(sizeof(char) * j);
				strcpy(arg[cmd_count][k++], buf);
				j = 0;
			}
		}
		else if (input[i] == '|' && i < input_len && input[i + 1] == ' ' && i > 0 && input[i - 1] == ' ') {  // �ܵ���
			cmd_count++;
			k = 0;
			j = 0;
		}
		else if ((input[i] == '>' || input[i] == '<') && i < input_len && input[i + 1] == ' ' && i > 0 && input[i - 1] == ' ') { // �����ض���
			if (input[i] == '>' && cmd_count > 0 && redir_out == 0) { // ����ض���
				redir_out = 1;
				// �����ļ���
				i += 2;
				while (input[i] == ' ') {
					i++;
				}
				while (input[i] != '\0' && input[i] != ' ') {
					buf[j++] = input[i++];
				}
				buf[j] = '\0';
				file_out = (char*)malloc(sizeof(char) * j);
				strcpy(file_out, buf);
				j = 0;
				continue;
			}
			if (input[i] == '<' && cmd_count == 0 && redir_in == 0) { // �����ض���
				redir_in = 0;
				// �����ļ���
				i += 2;
				while (input[i] == ' ') {
					i++;
				}
				while (input[i] != '\0' && input[i] != ' ') {
					buf[j++] = input[i++];
				}
				buf[j] = '\0';
				file_in = (char*)malloc(sizeof(char) * j);
				strcpy(file_in, buf);
				j = 0;
				continue;
			}
			// ���ִ���
			printf("Error: Unexpected redirect format.\n");
			return 0;
		}
		else { // ��ͨ����
			if (i > 0 && input[i - 1] == ' ' && input[i] == '&' && input[i + 1] == '\0') {  // ��ִ̨��
				is_bg = 1;
				continue;
			}
			else {
				buf[j++] = input[i];
			}
		}
	}

	int exe_count = 0, pre_fd = 0, fds[2];
	// �����ܵ�
	if (pipe(fds) == -1) {
		for (i = 0; i < cmd_count; i++) {
			free(arg[i]);
		}
		if (redir_in != 0) {
			free(file_in);
		}
		if (redir_out != 0) {
			free(file_out);
		}
		printf("Failed to create pipe.\n");
		return 0;
	}
	// ִ������
	while(exe_count <= cmd_count) {
		// ���������Ƿ����
		if (is_founded(arg[exe_count][0]) == 0) {
			printf("%s: Executable program not found.\n", arg[exe_count][0]);
			for (i = 0; i < cmd_count; i++) {
				free(arg[i]);
			}
			if (redir_in != 0) {
				free(file_in);
			}
			if (redir_out != 0) {
				free(file_out);
			}
			return 0;
		}

		// ��������
		if ((pid = fork()) == -1) {
			printf("%s: Failed to execute.\n", arg[0]);
		}
		else if (pid == 0) { // �ӽ���
			// �ض����һ�����������
			if (exe_count == 0 && redir_in == 1) {
				int fd_in = open(file_in, O_RDONLY, S_IRUSR | S_IWUSR);
				if (fd_in == -1) {
					printf("Cannot open '%s'.\n", file_in);
					for (i = 0; i < cmd_count; i++) {
						free(arg[i]);
					}
					if (redir_in != 0) {
						free(file_in);
					}
					if (redir_out != 0) {
						free(file_out);
					}
					exit(1);
				}
				if (dup2(fd_in, STDIN_FILENO) == -1) {
					printf("Failed to redirect to '%s'.\n", file_in);
					exit(1);
				}
				close(fd_in);
			}
			else {
				if (exe_count == 0 && is_bg == 1) {
					freopen("/dev/null", "r", stdin);
				}
			}
			// �ض������һ����������
			if (exe_count == cmd_count && redir_out == 1) {
				int fd_out = open(file_out, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
				if (fd_out == -1) {
					printf("Cannot open '%s'.\n", file_out);
					for (i = 0; i < cmd_count; i++) {
						free(arg[i]);
					}
					if (redir_in != 0) {
						free(file_in);
					}
					if (redir_out != 0) {
						free(file_out);
					}
					exit(1);
				}
				if (dup2(fd_out, STDOUT_FILENO) == -1) {
					printf("Failed to redirect to '%s'.\n", file_out);
					exit(1);
				}
				close(fd_out);
			}
			else {
				if (exe_count == cmd_count && is_bg == 1) {
					freopen("/dev/null", "w", stdout);
				}
			}

			// ��һ���ܵ������(��)�ˣ����������
			if (exe_count > 0) {
				dup2(pre_fd, STDIN_FILENO);
			}
			close(pre_fd);
			close(fds[0]);
			// �ܵ�������(д)�ˣ���������
			if (exe_count != cmd_count) {
				dup2(fds[1], STDOUT_FILENO);
			}
			close(fds[1]);

			execvp(arg[exe_count][0], arg[exe_count]);
			// �ӽ���δ���ɹ�ִ��
			printf("%s: Error command.\n", arg[exe_count][0]);
			exit(1);
		}
		else if (pid > 0) { // ������
			pre_fd = fds[0];

			if (is_bg == 0) {
				_pid = pid;
				waitpid(pid, &status, WUNTRACED | WCONTINUED);

				int err = WEXITSTATUS(status);
				if (err) {
					printf("Error: %s\n", strerror(err));
					return 0;
				}
				_pid = 0;
			}
		}

		exe_count++;
	}

	return 1;
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
			printf("%s%d%s\t%d\t%s\t%s\n", COLOR_BLUE, index, COLOR_DEFINE, node->pid, node->state, node->command);
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
