#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/const.h>
#include <sys/keyboard.h>

#define CMD_LINE_LEN 128
#define MAX_ARG_NR 16
#define MAX_PATH_LEN 128

//var
char final_path[MAX_PATH_LEN] = {0}; // 用于清洗路径时的缓冲
char final_path2[MAX_PATH_LEN] = {0}; // 用于清洗路径时的缓冲

char cmd_line[CMD_LINE_LEN] = {0};
char cwd_cache[MAX_PATH_LEN] = {0};
char *cmd_argv[MAX_ARG_NR];
int argc = -1;

//func
void print_prompt();
int cmd_parse(char * cmd_str, char **argv, char token);
int read_key(char *buf);
void readline( char *buf, uint32_t count);

void wash_path(char *old_abs_path, char * new_abs_path);
void make_clear_abs_path(char *path, char *final_path);
char* path_parse(char* pathname, char* name_store);
void ls(char *pathname, int detail);

int wait(int *status);

//cmd
void cmd_cls(uint32_t argc, char** argv);
void cmd_pwd(uint32_t argc, char** argv);
char *cmd_cd(uint32_t argc, char** argv);
void cmd_ls(uint32_t argc, char** argv);
void cmd_help(uint32_t argc, char** argv);
void cmd_ps(uint32_t argc, char** argv);
int cmd_mkdir(uint32_t argc, char** argv);
int cmd_rmdir(uint32_t argc, char** argv);
int cmd_rm(uint32_t argc, char** argv);
int cmd_cat(int argc, char *argv[]);
int cmd_echo(int argc, char *argv[]);
int cmd_type(int argc, char *argv0[]);
void cmd_dir(uint32_t argc, char** argv);
void cmd_ver(uint32_t argc, char** argv);
void cmd_time(uint32_t argc, char** argv);
void cmd_date(uint32_t argc, char** argv);
int cmd_rename(uint32_t argc, char** argv);
int cmd_move(uint32_t argc, char** argv);
int cmd_copy(uint32_t argc, char** argv);
void cmd_reboot(uint32_t argc, char** argv);
void cmd_exit(uint32_t argc, char** argv);
void cmd_mm(uint32_t argc, char** argv);

int main(int argc, char *argv0[])
{
	memset(cwd_cache, 0, MAX_PATH_LEN);
	strcpy(cwd_cache, "/");
	
	char buf[MAX_PATH_LEN] = {0};
	int arg_idx = 0;

	int next_line = 1;
	int status = 0;
	int daemon = 0;	/*后台程序*/
	
	//clear();
	while(1){
		print_prompt();
		memset(cmd_line, 0, CMD_LINE_LEN);
		readline(cmd_line, CMD_LINE_LEN);
		
		if(cmd_line[0] == 0){
			continue;
		}
		
		argc = -1;
		argc = cmd_parse(cmd_line, cmd_argv, ' ');
		
		if(argc == -1){
			printf("shell: num of arguments exceed %d\n",MAX_ARG_NR);
			continue;
		}

		arg_idx = 0;
		while(arg_idx < argc){
			make_clear_abs_path(cmd_argv[arg_idx], buf);
			arg_idx++;
		}

		if(!strcmp("cls", cmd_argv[0])){
			cmd_cls(argc, cmd_argv);
			next_line = 0;
		}else if(!strcmp("pwd", cmd_argv[0])){
			cmd_pwd(argc, cmd_argv);
		}else if(!strcmp("cd", cmd_argv[0])){
			if(cmd_cd(argc, cmd_argv) != NULL){
				memset(cwd_cache,0, MAX_PATH_LEN);
				strcpy(cwd_cache, final_path);
			}
		}else if(!strcmp("ls", cmd_argv[0])){
			cmd_ls(argc, cmd_argv);
			
		}else if(!strcmp("ps", cmd_argv[0])){
			cmd_ps(argc, cmd_argv);
			
		}else if(!strcmp("help", cmd_argv[0])){
			cmd_help(argc, cmd_argv);
			
		}else if(!strcmp("mkdir", cmd_argv[0])){
			cmd_mkdir(argc, cmd_argv);
			
		}else if(!strcmp("rmdir", cmd_argv[0])){
			cmd_rmdir(argc, cmd_argv);
			
		}else if(!strcmp("rm", cmd_argv[0])){
			cmd_rm(argc, cmd_argv);
			
		}else if(!strcmp("echo", cmd_argv[0])){
			cmd_echo(argc, cmd_argv);
			
		}else if(!strcmp("type", cmd_argv[0])){
			cmd_type(argc, cmd_argv);
		}else if(!strcmp("cat", cmd_argv[0])){
			cmd_cat(argc, cmd_argv);	
		}else if(!strcmp("dir", cmd_argv[0])){
			cmd_dir(argc, cmd_argv);
		}else if(!strcmp("ver", cmd_argv[0])){
			cmd_ver(argc, cmd_argv);
		}else if(!strcmp("time", cmd_argv[0])){
			cmd_time(argc, cmd_argv);
		}else if(!strcmp("date", cmd_argv[0])){
			cmd_date(argc, cmd_argv);
		}else if(!strcmp("rename", cmd_argv[0])){
			cmd_rename(argc, cmd_argv);
		}else if(!strcmp("mv", cmd_argv[0])){
			cmd_move(argc, cmd_argv);
		}else if(!strcmp("copy", cmd_argv[0])){
			cmd_copy(argc, cmd_argv);
		}else if(!strcmp("reboot", cmd_argv[0])){
			cmd_reboot(argc, cmd_argv);
		}else if(!strcmp("exit", cmd_argv[0])){
			cmd_exit(argc, cmd_argv);
		}else if(!strcmp("mm", cmd_argv[0])){
			cmd_mm(argc, cmd_argv);
			
		}else{
			make_clear_abs_path(cmd_argv[0], final_path);
			//printf("operate %s\n",final_path);
			cmd_argv[argc] = NULL;
			
			int pid = execv(( char *)final_path, ( char **)cmd_argv);
			/*是否把进程当作后台程序运行*/
			arg_idx = 0;
			while(arg_idx < argc){
				if(!strcmp(cmd_argv[arg_idx], "&")){
					daemon = 1;
				}
				arg_idx++;
			}
			
			if(pid != -1){
				//run success
			
				//printf("child pid %d.\n",pid);
				if(!daemon){
					_wait(&status);
				}
				
				//printf("status %d.\n",status);
			}else{
				printf("boshell: unknown cmd!");
			}
			
			
		}
		//如果是清屏就不换行
		
		if(next_line){
			printf("\n");
		}
		next_line = 1;
		daemon = 0;	
	}
}
void cmd_mm(uint32_t argc, char** argv)
{
	int mem_size, mem_free;
	get_memory(&mem_size, &mem_free);
	printf("physical memory size: %d bytes ( %d mb)\nphysical memory free: %d bytes ( %d mb)\n",\
	mem_size, mem_size/(1024*1024), mem_free, mem_free/(1024*1024));
	
}
void cmd_exit(uint32_t argc, char** argv)
{
	exit(1);
}

void cmd_reboot(uint32_t argc, char** argv)
{
	reboot(BOOT_KBD);
}

int cmd_copy(uint32_t argc, char** argv)
{
	if(argc < 3){
		printf("copy: command syntax is incorrect.\n");	
		return -1;
	}

	if(!strcmp(argv[1], ".") || !strcmp(argv[1], "..")){
		printf("copy: src pathnamne can't be . or .. \n");	
		return -1;
	}
	if(!strcmp(argv[2], ".") || !strcmp(argv[2], "..")){
		printf("copy: dst pathname can't be . or .. \n");	
		return -1;
	}

	make_clear_abs_path(argv[1], final_path);
	make_clear_abs_path(argv[2], final_path2);

	if(!copy(final_path, final_path2)){
		printf("copy: %s to", final_path);	
		printf(" %s sucess!\n", final_path2);
		return 0;
	}else{
		printf("copy: %s to", final_path);	
		printf(" %s faild!\n", final_path2);
		return -1;
	}
}

int cmd_move(uint32_t argc, char** argv)
{
	//printf("echo: arguments %d\n", argc);
	if(argc < 3){
		printf("move: command syntax is incorrect.\n");	
		return -1;
	}

	if(!strcmp(argv[1], ".") || !strcmp(argv[1], "..")){
		printf("move: src pathnamne can't be . or .. \n");	
		return -1;
	}
	if(!strcmp(argv[2], ".") || !strcmp(argv[2], "..")){
		printf("move: dst pathname can't be . or .. \n");	
		return -1;
	}

	make_clear_abs_path(argv[1], final_path);
	make_clear_abs_path(argv[2], final_path2);

	if(!move(final_path, final_path2)){
		printf("move: %s to", final_path);	
		printf(" %s sucess!\n", final_path2);
		return 0;
	}else{
		printf("move: %s to", final_path);	
		printf(" %s faild!\n", final_path2);
		return -1;
	}
}

int cmd_rename(uint32_t argc, char** argv)
{
	//printf("echo: arguments %d\n", argc);
	if(argc < 3){
		printf("rename: command syntax is incorrect.\n");	
		return -1;
	}

	if(!strcmp(argv[1], ".") || !strcmp(argv[1], "..")){
		printf("rename: pathnamne can't be . or .. \n");	
		return -1;
	}
	if(!strcmp(argv[2], ".") || !strcmp(argv[2], "..")){
		printf("rename: new name can't be . or .. \n");	
		return -1;
	}

	make_clear_abs_path(argv[1], final_path);
	if(!rename(final_path, argv[2])){
		printf("rename: %s to", final_path);	
		printf(" %s sucess!\n", argv[2]);
		return 0;
	}else{
		printf("rename: %s to", final_path);	
		printf(" %s faild!\n", argv[2]);
		return -1;
	}
}


void cmd_time(uint32_t argc, char** argv)
{
	struct time tm;
	gettime(&tm);
	
	printf("current time: ");
	printf("%d:",tm.hour);
	printf("%d:",tm.minute);
	printf("%d\n",tm.second);
	
}

void cmd_date(uint32_t argc, char** argv)
{
	struct time tm;
	gettime(&tm);
	
	printf("current date: ");
	printf("%d/",tm.year);
	printf("%d/",tm.month);
	printf("%d\n",tm.day);
	
}

void cmd_ver(uint32_t argc, char** argv)
{
	
	printf("\n%s ",OS_NAME);
	printf("[Version %s]\n",OS_VERSION);
	
}

void cmd_dir(uint32_t argc, char** argv)
{
	char *pathname = NULL;
	
	pathname = argv[1];
	if(pathname == NULL){
		if(!getcwd(final_path, MAX_PATH_LEN)){
			pathname = final_path;
		}else{
			printf("ls: getcwd for default path faild!\n");
			return;
		}
	}else{
		make_clear_abs_path(pathname, final_path);
		pathname = final_path;
	}
	//printf("ls: path:%s\n", pathname);
	ls(pathname, 1);
	if(!1){
		printf("\n");
	}
}
/*
cat: print a file
*/
int cmd_cat(int argc, char *argv[])
{
	//printf("argc: %d\n", argc);
	if(argc == 1){	//只有一个参数，自己的名字，退出
		printf("cat: please input filename!\n");
		return 0;
	}
	if(argc > 2){
		printf("cat: only support 2 argument!\n");
		return -1;
	}
	
	make_clear_abs_path(argv[1], final_path);

	
	int fd = fopen(final_path, O_RDONLY);
	if(fd == -1){
		printf("cat: fd %d error\n", fd);
		return 0;
	}
	
	struct stat stat;
	fstat(final_path, &stat);
	
	char *buf = (char *)malloc(stat.st_size);
	
	int bytes = fread(fd, buf, stat.st_size);
	//printf("read:%d\n", bytes);
	fclose(fd);
	
	int i = 0;
	while(i < bytes){
		putchar(buf[i]);
		i++;
	}
	free(buf);
	printf("\n");
	return 0;
}

/*
type filename
*/
int cmd_type(int argc, char *argv[])
{
	//printf("argc: %d\n", argc);
	if(argc == 1){	//只有一个参数，自己的名字，退出
		printf("type: please input filename!\n");
		return 0;
	}
	if(argc > 2){
		printf("cat: only support 2 argument!\n");
		return -1;
	}
	
	int fd = fopen(final_path, O_RDONLY);
	if(fd == -1){
		printf("cat: fd %d error\n", fd);
		return 0;
	}
	
	struct stat stat;
	fstat(final_path, &stat);
	
	char *buf = (char *)malloc(stat.st_size);
	
	int bytes = fread(fd, buf, stat.st_size);
	//printf("read:%d\n", bytes);
	fclose(fd);
	
	int i = 0;
	while(i < bytes){
		putchar(buf[i]);
		i++;
	}
	free(buf);
	
	printf("\n");
	return 0;
}

/*
echo string			//show a string
echo string>file 	//output a string to file
echo string>>file	//apend a string to file
*/
int cmd_echo(int argc, char *argv[])
{
	//printf("echo: arguments %d\n", argc);
	if(argc == 1){	//只有一个参数，自己的名字，退出
		printf("echo: no arguments.\n");
		return -1;
	}
	if(argc > 2){
		printf("echo: only support 2 argument!");
		return -1;
	}
	char operate = 0;
	//获取操作类型
	char *p = argv[1];
	while(*p){
		if(*p == '>'){
			operate++;
		}
		p++;
	}
	
	//让p指向文件名
	p = argv[1];
	//知道遇到>才停止，如果没有>就遇到结束符停止
	while(*p != '>' && *p != '\0'){
		p++;
	}
	
	char pathname[256] = {0};

	if(operate == 0){//直接显示
		printf("%s\n", argv[1]);
	}else if(operate == 1){	//输出到文件
		//先获取文件路径
		strcpy(pathname, p+1);	//跳过一个>
		//printf("output: %s\n", pathname);
		
		make_clear_abs_path(pathname, final_path);
		
		//printf("abs: %s\n", abs_path);
		//printf("abs: %s\n", abs_path);
		
		//在文本最后添加'\0'
		*p = '\0';
		p = argv[1];
	
		int fd = fopen(final_path, O_CREAT|O_RDWR);
		//printf("fd:%d\n", fd);
		fwrite(fd, p, strlen(p));
		//printf("write:%d\n", bytes);
		fclose(fd);
		//printf("%s", p);
	}else if(operate == 2){	//输出到文件
		//先获取文件路径
		strcpy(pathname, p+2);	//跳过两个>
		//printf("output: %s\n", pathname);
		
		make_clear_abs_path(pathname, final_path);
		//printf("abs: %s\n", abs_path);
		
		//在文本最后添加'\0'
		*p = '\0';
		p = argv[1];
		
		int fd = fopen(final_path, O_CREAT|O_RDWR);
		//printf("fd:%d\n", fd);
		//指向末尾
		lseek(fd, 0, SEEK_END);
		fwrite(fd, p, strlen(p));
		//printf("write:%d at pos:%d\n", bytes, pos);
		fclose(fd);
		//printf("%s", p);
	}
	//printf("\n");
	return 0;
}

void cmd_help(uint32_t argc, char** argv)
{
	if(argc != 1){
		printf("help: no argument support!\n");
		return;
	}
	printf("  cat         print a file.\n");
	printf("  cls         clean screen.\n");
	printf("  cd          change current work dirctory.\n");
	printf("  copy        copy a file.\n");
	printf("  date        get current date.\n");
	printf("  dir         list files in current dirctory.\n");
	printf("  exit        exit shell.\n");
	printf("  ls          list files in current dirctory.\n");
	printf("  mkdir       create a dir.\n");
	printf("  mm          print memory info.\n");
	printf("  mv          move a file.\n");
	printf("  ps          print tasks.\n");
	printf("  pwd         print work directory.\n");
	printf("  rmdir       remove a dir.\n");
	printf("  rename      reset file/dirctory name.\n");
	printf("  reboot      reboot system.\n");
	printf("  rm          delete a file.\n");
	printf("  time        get current time.\n");
	printf("  ver         show os version.\n");
	
	
}

void cmd_ps(uint32_t argc, char** argv)
{
	if(argc != 1){
		printf("ps: no argument support!\n");
		return;
	}
	ps();
}

void cmd_ls(uint32_t argc, char** argv)
{
	char *pathname = NULL;
	int detail = 0;
	uint32_t arg_path_nr = 0;
	uint32_t arg_idx = 1;	//跳过argv[0]
	while(arg_idx < argc){
		if(argv[arg_idx][0] == '-'){	//参数形式
			if(!strcmp(argv[arg_idx], "-l")){
				detail = 1;
			}else if(!strcmp(argv[arg_idx], "-h")){
				printf("  -l list all infomation about the file.\n\
  -h get command  help\n\
  list all files in the current dirctory if no option.\n");
				return;
			}
		}else {	//是个路径
			if(arg_path_nr == 0){
				pathname = argv[arg_idx];
				
				arg_path_nr = 1;
			}else{
				printf("ls: only support one path!\n");
				return;
			}
		}
		arg_idx++;
	}
	if(pathname == NULL){
		if(!getcwd(final_path, MAX_PATH_LEN)){
			pathname = final_path;
		}else{
			printf("ls: getcwd for default path faild!\n");
			return;
		}
	}else{
		make_clear_abs_path(pathname, final_path);
		pathname = final_path;
	}
	//printf("ls: path:%s\n", pathname);
	ls(pathname, detail);
	if(!detail){
		printf("\n");
	}
	
}

char *cmd_cd(uint32_t argc, char** argv)
{
	//printf("pwd: argc %d\n", argc);
	if(argc > 2){
		printf("cd: only support 1 argument!\n");
		return NULL;
	}
	
	if(argc == 1){	//只有cd
		final_path[0] = '/';
		final_path[1] = 0;
	}else{
		//不只有cd ，例如 cd a
		make_clear_abs_path(argv[1], final_path);
	}
	//现在已经有了绝对路径
	//尝试改变目录，如果没有就失败
	printf("cd: operate %s\n",final_path);
	
	if(chdir(final_path) == -1){
		printf("cd: no such directory %s\n",final_path);
		return NULL;
	}
	return final_path;
}

void cmd_pwd(uint32_t argc, char** argv)
{
	//printf("pwd: argc %d\n", argc);
	if(argc != 1){
		printf("pwd: no argument support!\n");
		return;
	}else{
		if(!getcwd(final_path, MAX_PATH_LEN)){
			printf("%s\n", final_path);
		}else{
			printf("pwd: get current work directory failed!\n");
		}
	}
}

void cmd_cls(uint32_t argc, char** argv)
{
	//printf("cls: argc %d\n", argc);
	if(argc != 1){
		printf("cls: no argument support!\n");
		return;
	}
	clear();
}

int cmd_mkdir(uint32_t argc, char** argv)
{
	int ret = -1;
	if(argc != 2){
		printf("mkdir: no argument support!\n");
	}else{
		make_clear_abs_path(argv[1], final_path);
		
		
		/*如果不是根目录*/
		if(strcmp(final_path, "/")){
			if(mkdir(final_path) == 0){
				printf("mkdir: create a dir %s success.\n", final_path);
				ret = 0;
			}else{
				printf("mkdir: create directory %s faild!\n", argv[1]);
			}
			
		}
		
	}
	return ret;
}

int cmd_rmdir(uint32_t argc, char** argv)
{
	int ret = -1;
	if(argc != 2){
		printf("mkdir: no argument support!\n");
	}else{
		make_clear_abs_path(argv[1], final_path);
		/*如果不是根目录*/
		if(strcmp(final_path, "/")){
			if(rmdir(final_path) == 0){
				printf("rmdir: remove %s success.\n", final_path);
				ret = 0;
			}else{
				printf("rmdir: remove %s faild!\n", final_path);
			}
		}
	}
	return ret;
}

int cmd_rm(uint32_t argc, char** argv)
{
	int ret = -1;
	if(argc != 2){
		printf("rm: no argument support!\n");
	}else{
		make_clear_abs_path(argv[1], final_path);
		//printf("rm: path %s\n", final_path);
		/*如果不是根目录*/
		if(strcmp(final_path, "/")){
			if(unlink(final_path) == 0){
				printf("rm: delete %s success.\n", final_path);
				ret = 0;
			}else{
				printf("rm: delete %s faild!\n", final_path);
			}
		}
	}
	return ret;
}

void make_clear_abs_path(char *path, char *final_path)
{
	char abs_path[MAX_PATH_LEN] = {0};
	/* 先判断是否输入的是绝对路径 */
	if(path[0] != '/'){
		//不是绝对路径，把它拼接成绝对路径
		memset(abs_path,0, MAX_PATH_LEN);
		
		if (!getcwd(abs_path, MAX_PATH_LEN)) {
		//we get path from cache is ok! but from getcwd will task some errors!
		//strcpy(abs_path, cwd_cache);
			//printf("cwd %s\n", abs_path);
			if (!((abs_path[0] == '/') && (abs_path[1] == 0))) {
				// 若 abs_path 表示的当前目录不是根目录/
				strcat(abs_path, "/");
			}
		}
	}
	strcat(abs_path, path);
	wash_path(abs_path, final_path);
}

void wash_path(char *old_abs_path, char * new_abs_path)
{
	char name[MAX_FILE_NAME_LEN]= {0};
	char *sub_path = old_abs_path;
	sub_path = path_parse(sub_path, name);
	if(name[0] == 0){	//只有根目录"/"
		new_abs_path[0] = '/';
		new_abs_path[1] = 0;
		return;
	}
	//避免传给new_abs_path的缓冲不干净
	new_abs_path[0] = 0;
	strcat(new_abs_path, "/");
	while(name[0]){
		//如果是上一级目录
		if(!strcmp(name,"..")){
			
			char *slash_ptr = (char *)strrchr(new_abs_path, '/');
			/*如果未到 new_abs_path 中的顶层目录，就将最右边的'/'替换为 0，
			这样便去除了 new_abs_path 中最后一层路径，相当于到了上一级目录 
			*/
			if(slash_ptr != new_abs_path){
				// 如 new_abs_path 为“/a/b”， ".."之后则变为“/a
				*slash_ptr = 0;
			}else{ // 如 new_abs_path 为"/a"， ".."之后则变为"/"
				/* 若 new_abs_path 中只有 1 个'/'，即表示已经到了顶层目录,
				就将下一个字符置为结束符 0 
				*/
				*(slash_ptr + 1) = 0;
			}
			
		}else if(strcmp(name,".")){
			// 如果路径不是‘.’，就将 name 拼接到 new_abs_path
			if (strcmp(new_abs_path, "/")) {
				// 如果 new_abs_path 不是"/"
				// 就拼接一个"/",此处的判断是为了避免路径开头变成这样"//"
				strcat(new_abs_path, "/");
			}
			strcat(new_abs_path, name);
		}
		memset(name, 0, MAX_FILE_NAME_LEN);
		if(sub_path){
			sub_path = path_parse(sub_path, name);
		}
	}
}

int cmd_parse(char * cmd_str, char **argv, char token)
{
	if(cmd_str == NULL){
		return -1;
	}
	int arg_idx = 0;
	while(arg_idx < MAX_ARG_NR){
		argv[arg_idx] = NULL;
		arg_idx++;
	}
	char *next = cmd_str;
	int argc = 0;
	while(*next){
		//跳过token字符
		while(*next == token){
			next++;
		}
		//如果最后一个参数后有空格 例如"cd / "
		if(*next ==0){
			break;
		}
		//存入一个字符串地址，保存一个参数项
		argv[argc] = next;
		
		//每一个参数确定后，next跳到这个参数的最后面
		while(*next && *next != token){
			next++;
		}
		//如果此时还没有解析完，就把这个空格变成'\0'，当做字符串结尾
		if(*next){
			*next++ = 0;
		}
		//参数越界，解析失败
		if(argc > MAX_ARG_NR){
			return -1;
		}
		//指向下一个参数
		argc++;
		//让下一个字符串指向0
		argv[argc] = 0;
	}
	return argc;
}

void readline( char *buf, uint32_t count)
{
	char *pos = buf;
	while(read_key(pos) && (pos - buf) < count){
		switch(*pos){
			case '\n':
			case '\r':
				//当到达底部了就不在继续了，目前还没有设定
				*pos = 0;
				printf("\n");
				return;
			case '\b':
				if(buf[0] != '\b'){
					--pos;
					printf("\b");
				}
				break;
			default:
				printf("%c", *pos);
				pos++;
		}
	}
	printf("readline: error!");
}

int read_key(char *buf)
{
	char key;
	do{
		key = getchar();
	}while(key == -1);
	*buf = key;
	return 1;
}


/* 将最上层路径名称解析出来*/
char* path_parse(char* pathname, char* name_store) 
{
   if (pathname[0] == '/') {   // 根目录不需要单独解析
    /* 路径中出现1个或多个连续的字符'/',将这些'/'跳过,如"///a/b" */
       while(*(++pathname) == '/');
   }

   /* 开始一般的路径解析 */
   while (*pathname != '/' && *pathname != 0) {
      *name_store++ = *pathname++;
   }
   if (pathname[0] == 0) {   // 若路径字符串为空则返回NULL
      return NULL;
   }
   return pathname; 
}

void print_prompt()
{
	printf("%s>", cwd_cache);
}

void ls(char *pathname, int detail)
{
	struct dir *dir = opendir(pathname);
	if(dir == NULL){
		printf("opendir failed!\n");
	}
	rewinddir(dir);
	
	struct dir_entry *de;
	char type;
	de = readdir(dir);
	while(de != NULL){
		if(de->name[0]){	//显示有名字的
			if(detail){
				if(de->attributes&ATTR_DIRECTORY){
					type = 'd';
				}else{
					type = '-';
				}
				printf("%d/%d/%d ",
					DATA16_TO_DATE_YEA(de->create_date),
					DATA16_TO_DATE_MON(de->create_date),
					DATA16_TO_DATE_DAY(de->create_date));
				printf("%d:%d:%d ",
					DATA16_TO_TIME_HOU(de->create_time),
					DATA16_TO_TIME_MIN(de->create_time),
					DATA16_TO_TIME_SEC(de->create_time));
				printf("%c %d %s \n", type, de->size, de->name);
			}else{
				printf("%s ", de->name);
			}
		}
		de = readdir(dir);
	}
	closedir(dir);
}

int wait(int *status)
{
	int pid;
	do{
		pid = _wait(status);
	}while(pid == -1);
	return pid;
}

