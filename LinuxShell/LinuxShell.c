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
		char c, *arg[20], path[100], *input = NULL;
		int i = 0, j = 0, k = 0, is_pr = 0, is_bg = 0, input_len = 0, status = 0;
		pid_t pid = 0;

		/** 设置 Singal */
		struct sigaction act;
		act.sa_handler = del_job_node;
		sigfillset(&act.sa_mask);
		act.sa_flags = SA_SIGINFO;
		sigaction(SIGCHLD, &act, NULL);
		signal(SIGTSTP, ctrl_z);

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
					pipel();
					add_history_command();
					free(input);
				}
				else {  // 重定向命令
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
		if (strcmp(arg[0], "exit") == 0) {  // exit 命令
			add_history_command(input);

			printf(" Bye!\n");

			for (i = 0; i < k; i++) {
				free(arg[i]);
			}
			free(input);
			break;
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
			for (i = 0; i < k; i++) {
				free(arg[i]);
			}
			free(input);
			continue;
		}
		if(strcmp(arg[0], "bg") == 0){  // bg 命令
			for (i = 0; i < k; i++) {
				free(arg[i]);
			}
			free(input);
			continue;
		}
		if(strcmp(arg[0], "fg") == 0){  // fg 命令
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
				printf(" %s: Executable program not found.\n", arg[0]);
				for (i = 0; i < k; i++) {
					free(arg[i]);
				}
				free(input);
				continue;
			}

			// 执行命令
			if ((pid = fork()) == -1) {
				printf(" %s: Failed to execute.\n", arg[0]);
			}
			else if (pid == 0) {  // 子进程
				if (is_bg == 0) {
					execvp(arg[0], arg);
					// 子进程未被成功执行
					printf(" %s: Error command.\n", arg[0]);
					exit(1);
				}
				if (is_bg == 1) {
					freopen("/dev/null", "w", stdout);
					freopen("/dev/null", "r", stdin);
					// 加入节点
					add_job_node();
					execvp(arg[0], arg);
					// 子进程未被成功执行
					printf(" %s: Error command.\n", arg[0]);
				}
			}
			else if(pid > 0) {  // 父进程
				if (is_bg == 0) {
					waitpid(pid, &status, 0);
					int err = WEXITSTATUS(status);
					if (err) {
						printf(" Error: %s\n", strerror(err));
					}
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
			printf(" Init environment error. The profile param name out of range.");
			exit(1);
		}
	}
	buf[i++] = '\0';

	if (strcmp(buf, "PATH") == 0) {
		while (str[i] != '\0') {
			if (str[i] == ':') {  // 冒号为分隔符，取出该段地址
				if (j >= 128) {
					printf(" Init environment error. The path length out of range.");
					exit(1);
				}
				temp[j++] = '/';
				temp[j] = '\0';

				path = (char*)malloc(j);
				strcpy(path, temp);

				if (k >= 9) {
					printf(" Init environment error. Num of environment path out of range.");
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
		printf(" Init environment error.\n");
		exit(1);
	}

	while (fgets(buf, sizeof(buf), fptr)) {
		init_path(buf);
	}

	
	sig_z = 0;
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
			printf(" System can not find the specified path '%s.\n", route);
		}
	}
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
