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
		 * 重置变量
		 * arg 用户输入的命令所含参数
		 * path 当前所在路径
		 * input 用户输入的命令
		 * is_pr 是否为管道或重定向命令
		 * is_bg 是否为后台命令
		 * input_len 用户输入命令的长度
		 * status 
		 */
		char c, *arg[20], *input = NULL;
		int i = 0, j = 0, k = 0, is_pr = 0, is_bg = 0, input_len = 0, status = 0;
		pid_t pid = 0;

		/** 设置 Singal */
		struct sigaction act;
		act.sa_handler = del_job_node;
		sigfillset(&act.sa_mask);
		act.sa_flags = SA_SIGINFO;
		sigaction(SIGCHLD, &act, NULL);
		signal(SIGTSTP, ctrl_z);
		signal(SIGINT, ctrl_c);

		/** 打印提示符 */
		getcwd(path, sizeof(char) * 100);
		printf(PROMPT, path);

		/** 获取输入 */
		while ((c = getchar()) == ' ' || c == '\t' || c == '\n' || c == EOF) {  // 跳过空格等无用信息
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


		/** 解析命令 */
		/** 1. 管道和重定向命令 */
		for (i = 0; i <= input_len; i++) {
			if (input[i] == '<' || input[i] == '>' | input[i] == '|') {
				if (input[i] == '|') {  // 管道命令
					add_history_command(input);
					pipel(input, input_len);
					free(input);
				}
				else {  // 重定向命令
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

		/** 2. 解析命令参数 */
		for (i = 0, j = 0, k = 0; i <= input_len; i++) {
			if (input[i] == ' ' || input[i] == '\0') {
				if (j == 0) {  // 省略多个相连的空格
					continue;
				}
				else {  // 取出一个参数
					buf[j++] = '\0';
					arg[k] = (char*)malloc(sizeof(char) * j);
					strcpy(arg[k++], buf);
					j = 0;
				}
			}
			else {
				if (i > 0 && input[i - 1] == ' ' && input[i] == '&' && input[i + 1] == '\0') {  // 后台执行
					is_bg = 1;
					continue;
				}
				buf[j++] = input[i];
			}
		}

		/** 3. 内部命令 */
		if (strcmp(arg[0], "exit") == 0 || strcmp(arg[0], "quit") == 0) {  // exit/quit 命令
			add_history_command(input);

			printf("%sBye!%s\n", COLOR_BLUE, COLOR_DEFINE);

			for (i = 0; i < k; i++) {
				free(arg[i]);
			}
			free(input);
			exit(0);
		}
		if(strcmp(arg[0], "history") == 0){  // history 命令
			add_history_command(input);

			show_history_command();

			for (i = 0; i < k; i++) {
				free(arg[i]);
			}
			free(input);
			continue;
		}
		if(strcmp(arg[0], "cd") == 0){  // cd 命令
			add_history_command(input);

			cd(arg[1]);

			for (i = 0; i < k; i++) {
				free(arg[i]);
			}
			free(input);
			continue;
		}
		if(strcmp(arg[0], "jobs") == 0){ // jobs 命令
			add_history_command(input);

			jobs();

			for (i = 0; i < k; i++) {
				free(arg[i]);
			}
			free(input);
			continue;
		}
		if(strcmp(arg[0], "bg") == 0){  // bg 命令
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
		if(strcmp(arg[0], "fg") == 0){  // fg 命令
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

		/** 4. 外部命令 */
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

			// 执行命令
			if ((pid = fork()) == -1) {
				printf("%s: Failed to execute.\n", arg[0]);
			}
			else if (pid == 0) {  // 子进程
				if (is_bg == 0) {
					execvp(arg[0], arg);
					// 子进程未被成功执行
					printf("%s: Error command.\n", arg[0]);
					exit(1);
				}
				if (is_bg == 1) {
					freopen("/dev/null", "w", stdout);
					freopen("/dev/null", "r", stdin);

					execvp(arg[0], arg);
					// 子进程未被成功执行
					printf("%s: Error command.\n", arg[0]);
					exit(1);
				}
			}
			else if(pid > 0) {  // 父进程
				// 将子进程加入任务节点
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

			// 释放变量内存
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
			if (str[i] == ':') {  // 冒号为分隔符，取出该段地址
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
	if (env_history.end == env_history.start) {  // 数组开始循环
		env_history.start = (env_history.start + 1) % MAX_HISTORY;
	}
	strcpy(env_history.history_command[env_history.end], input);
}

void show_history_command() {
	int index = 0;
	if (env_history.start == env_history.end) {  // 空
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
	int redir_in = 0, redir_out = 0, flag_io = 0, is_bg = 0, i, j, k, status = 0;  // flag_io: 1代表IN, 0代表OUT
	char *file_in = NULL, *file_out = NULL, *arg[20];
	pid_t pid;

	memset(arg, sizeof(arg), NULL);

	// 解析命令参数
	for (i = 0, j = 0, k = 0; i <= input_len; i++) {
		if (input[i] == ' ' || input[i] == '\0') {
			if (j == 0) {  // 省略多个相连的空格
				continue;
			}
			else {  // 取出一个参数
				buf[j++] = '\0';
				arg[k] = (char*)malloc(sizeof(char) * j);
				strcpy(arg[k++], buf);
				j = 0;
			}
		}
		else if ((input[i] == '<' || input[i] == '>') && i < input_len && input[i + 1] == ' ' && i > 0 && input[i - 1] == ' ') {  // 重定向参数
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

			// 重定向的文件
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
		else { // 普通参数
			if (i > 0 && input[i - 1] == ' ' && input[i] == '&' && input[i + 1] == '\0') {  // 后台执行
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

	// 查找命令是否存在
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

	// 执行命令
	if ((pid = fork()) == -1) {
		printf("%s: Failed to execute.\n", arg[0]);
	}
	else if (pid == 0) {  // 子进程
		// 重定向, 这里使用文件流会导致文件末尾出现乱码，因此使用dup2操作文件标识符fd
		if (redir_in == 1) { // 输入重定向
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
		if (redir_out == 1) { // 输出重定向
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
		// 子进程未被成功执行
		printf("%s: Error command.\n", arg[0]);
		exit(1);
	}
	else if (pid > 0) {  // 父进程
		// 将子进程加入任务节点
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

	// 释放变量内存
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

	// 解析命令参数
	for (i = 0, j = 0, k = 0; i <= input_len; i++) {
		if (input[i] == ' ' || input[i] == '\0') {
			if (j == 0) {  // 省略多个相连的空格
				continue;
			}
			else {  // 取出一个参数
				buf[j++] = '\0';
				arg[cmd_count][k] = (char*)malloc(sizeof(char) * j);
				strcpy(arg[cmd_count][k++], buf);
				j = 0;
			}
		}
		else if (input[i] == '|' && i < input_len && input[i + 1] == ' ' && i > 0 && input[i - 1] == ' ') {  // 管道符
			cmd_count++;
			k = 0;
			j = 0;
		}
		else if ((input[i] == '>' || input[i] == '<') && i < input_len && input[i + 1] == ' ' && i > 0 && input[i - 1] == ' ') { // 包含重定向
			if (input[i] == '>' && cmd_count > 0 && redir_out == 0) { // 输出重定向
				redir_out = 1;
				// 解析文件名
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
			if (input[i] == '<' && cmd_count == 0 && redir_in == 0) { // 输入重定向
				redir_in = 0;
				// 解析文件名
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
			// 出现错误
			printf("Error: Unexpected redirect format.\n");
			return 0;
		}
		else { // 普通参数
			if (i > 0 && input[i - 1] == ' ' && input[i] == '&' && input[i + 1] == '\0') {  // 后台执行
				is_bg = 1;
				continue;
			}
			else {
				buf[j++] = input[i];
			}
		}
	}

	int exe_count = 0, pre_fd = 0, fds[2];
	// 创建管道
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
	// 执行命令
	while(exe_count <= cmd_count) {
		// 查找命令是否存在
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

		// 创建进程
		if ((pid = fork()) == -1) {
			printf("%s: Failed to execute.\n", arg[0]);
		}
		else if (pid == 0) { // 子进程
			// 重定向第一条命令的输入
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
			// 重定向最后一条命令的输出
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

			// 上一条管道的输出(读)端，命令的输入
			if (exe_count > 0) {
				dup2(pre_fd, STDIN_FILENO);
			}
			close(pre_fd);
			close(fds[0]);
			// 管道的输入(写)端，命令的输出
			if (exe_count != cmd_count) {
				dup2(fds[1], STDOUT_FILENO);
			}
			close(fds[1]);

			execvp(arg[exe_count][0], arg[exe_count]);
			// 子进程未被成功执行
			printf("%s: Error command.\n", arg[exe_count][0]);
			exit(1);
		}
		else if (pid > 0) { // 父进程
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
	// 更新链表
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

	// 从链表中删除对应节点
	NODE* node = head;
	int pid = sig_info->si_pid;
	if (node->pid == pid) {  // 要删除的节点为头结点
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
	if (_pid == 0) {  // 前台没有运行程序
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
	if (_pid == 0) {  // 前台没有运行程序
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
