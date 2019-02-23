#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BF_SEGEMENT_SIZE 5000

static char *data_segment;
static char *code_segment;
static  int *stack_segment;

static int code_length;
static int stack_length;
static int bf_eip;

static int bf_load_file(char *argv[]);
static int update();
static int start(char *argv[]);

int main(int argc,char *argv[])
{
	if(argc < 2){
		printf("brainfuck: argument failed!\n");
		return -1;
	}
	int ret = start(argv);
	if(ret < 0){
		return -1;
	}
	update();
	
	return 0;	
}

static int start(char *argv[])
{
	if(bf_load_file(argv) != -1){
		stack_length = 0;
	
		data_segment = (char *)malloc(BF_SEGEMENT_SIZE);
		if(data_segment == NULL){
			printf("brainfuck: malloc for data segment failed!\n");
			return -1;
		}
		stack_segment = (int *)malloc(BF_SEGEMENT_SIZE);
		if(stack_segment == NULL){
			free(data_segment);
			printf("brainfuck: malloc for stack segment failed!\n");
			return -1;
		}
		
		bf_eip = 0;
		return 0;
	}
	return -1;
}

static int update()
{
	
	int j, k;
	char* ptr = data_segment+BF_SEGEMENT_SIZE/2;
	while(bf_eip < code_length) {
        switch(code_segment[bf_eip]) {
            case '+':
                (*ptr)++;
                break;
			case '-':
                (*ptr)--;
                break;
            case '>':
                ptr++;
                break;
            case '<':
				ptr--;
                break;
            case '.':
				putchar((int)(*ptr));
				//charShow(*ptr);
				break;
            case ',':						
				*ptr=getchar();
				//*ptr = getchar();
                break;
			case '[':
                if(*ptr) {
                    stack_segment[stack_length++]=bf_eip;
                } else {	
					for(k = bf_eip,j = 0; k < code_length; k++) {
                        code_segment[k] == '['&&j++;
                        code_segment[k] == ']'&&j--;
						if(j == 0)break;
                    }
                    if(j==0)
                        bf_eip=k;
                    else {
						printf("\nbrainfuck: -.-!\n");
                        return 3;
                    }
                }
                break;
			case ']':
				bf_eip = stack_segment[stack_length-- - 1]-1;
                break;
            default:
				break;
		}
        bf_eip++;
    }
    printf("\nbrainfuck: ^_^\n");
	
	free(data_segment);
	free(stack_segment);
	
	return 0;
}

static int bf_load_file(char *argv[])
{
    int fd = fopen(argv[1], O_RDONLY);
	if(fd == -1){
		printf("brainfuck: open file %s failed!\n", argv[1]);
		return -1;
	}
	
	/*检测是否是brainfuck文件，后缀名字是.bf*/
	
	struct stat stat;
	fstat(argv[1], &stat);
	
	code_segment = (char *)malloc(stat.st_size);
	
	int read = fread(fd, code_segment, stat.st_size);
	if(read == -1){
		printf("brainfuck: read file failed!\n");
		
		fclose(fd);
		return -1;
	}
	code_length = stat.st_size;
	return 0;
}


