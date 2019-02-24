#include <stdio.h>
#include <stdlib.h>
#include <graphic.h>
#include <string.h>

#define CHAR_WIDTH	8
#define CHAR_HEIGHT	16
/*
#define padInfo.char_lines	40
#define padInfo.char_culomns	12
*/
#define NAME_FILE_LEN	12
#define NAME_STATUS_LEN	12

#define MSG_ERR	1
#define MSG_TIP	2

#define MAX_ARGS_NR	16

struct Cursor
{
	int x,y,color;
};

struct Cursor cursor;

struct FileInfo
{
	char name[NAME_FILE_LEN];
	uint8 *buffer;
	uint8 *text;
	int size;
};

static struct FileInfo fileInfo;

struct PadInfo
{
	uint8 *charBuffer;
	int charColor, bgColor;
	int existColor, enterColor;
	int statusColor,errorColor, tipColor;
	int charLength;
	char status[NAME_STATUS_LEN];
	char *argv[MAX_ARGS_NR];
	int char_lines, char_culomns;
	
};

static struct PadInfo padInfo;

static void initAll();
static void myKeyboard(int key, int x, int y);

static void charWrite(int x, int y, uint8 ch);
static void charClear(int x, int y);
static uint8 charRead(int x, int y);
static void charShow();
static void makeFileBuf();
static int writeFileBuf();
static int readFileBuf();
static void makeCharBuf();
static void loadFram();
static void cleanCharBuf();
static void cleanFileBuf();
static void resetAll();
static void showStatus();
static int getCharLength();
static void putMessage(int type, char *msg);

int screen_width, screen_height;
/*
F6:read
F7:write
F12:reset
*/

int main(int argc, char *argv[])
{
	init_graphic();
	
	initAll();

	draw_rect(cursor.x*CHAR_WIDTH, cursor.y*CHAR_HEIGHT+14, CHAR_WIDTH, 2, cursor.color);
	refresh(cursor.x*CHAR_WIDTH, cursor.y*CHAR_HEIGHT+14, cursor.x*CHAR_WIDTH + CHAR_WIDTH, cursor.y*CHAR_HEIGHT+14+2);
	showStatus();
	
	int key;
	while(1){
		key = getchar();
		if(key != -1){
			myKeyboard(key, 0, 0);
		}
	}
	
	return 0;
}

static void initAll()
{
	cursor.x = 0;
	cursor.y = 0;
	cursor.color = RGB(0xff,0xff,0xff);
	padInfo.charColor = RGB(0xff,0xff,0xff);
	padInfo.bgColor = RGB(0,0,0);
	padInfo.existColor  = RGB(0,128,0);
	padInfo.enterColor  = RGB(128,0,0);
	padInfo.statusColor  = RGB(0,0,225);
	padInfo.errorColor  = RGB(225,0,0);
	padInfo.tipColor  = RGB(0,225,0);
	
	get_screen(&screen_width,&screen_height);
	padInfo.char_lines = screen_width/CHAR_WIDTH-1;
	padInfo.char_culomns = screen_height/CHAR_HEIGHT - 1;
	
	padInfo.charBuffer = malloc(padInfo.char_lines*padInfo.char_culomns);
	fileInfo.buffer = malloc(padInfo.char_lines*padInfo.char_culomns);
	int i;
	for(i = 0; i < MAX_ARGS_NR; i++){
		padInfo.argv[i] = NULL;
	}
	
	
}

static void myKeyboard(int key, int x, int y)
{
	if(key == GUI_KEY_F4){
		free(padInfo.charBuffer);
		free(fileInfo.buffer);
		graphic_exit();
		exit(1);
	}else if(key == GUI_KEY_UP){
		//擦去光标和字符
		draw_rect(cursor.x*CHAR_WIDTH, cursor.y*CHAR_HEIGHT, CHAR_WIDTH, CHAR_HEIGHT, padInfo.bgColor);
		refresh(cursor.x*CHAR_WIDTH, cursor.y*CHAR_HEIGHT, cursor.x*CHAR_WIDTH + CHAR_WIDTH, cursor.y*CHAR_HEIGHT+CHAR_HEIGHT);
	
		//显示字符
		
		charShow();
		cursor.y--;
		
		if(cursor.y < 0){
			cursor.y = 0;
		}
		//擦去光标和字符
		draw_rect(cursor.x*CHAR_WIDTH, cursor.y*CHAR_HEIGHT, CHAR_WIDTH, CHAR_HEIGHT, padInfo.bgColor);
		refresh(cursor.x*CHAR_WIDTH, cursor.y*CHAR_HEIGHT, cursor.x*CHAR_WIDTH + CHAR_WIDTH, cursor.y*CHAR_HEIGHT+CHAR_HEIGHT);
	
		//显示字符
		charShow();
		
		//显示光标
		draw_rect(cursor.x*CHAR_WIDTH, cursor.y*CHAR_HEIGHT+14, CHAR_WIDTH, 2, cursor.color);
		refresh(cursor.x*CHAR_WIDTH, cursor.y*CHAR_HEIGHT+14, cursor.x*CHAR_WIDTH + CHAR_WIDTH, cursor.y*CHAR_HEIGHT+14+2);
	
	}else if(key == GUI_KEY_DOWN){
		//擦去光标和字符
		draw_rect(cursor.x*CHAR_WIDTH, cursor.y*CHAR_HEIGHT, CHAR_WIDTH, CHAR_HEIGHT, padInfo.bgColor);
		refresh(cursor.x*CHAR_WIDTH, cursor.y*CHAR_HEIGHT, cursor.x*CHAR_WIDTH + CHAR_WIDTH, cursor.y*CHAR_HEIGHT+CHAR_HEIGHT);
	
		//显示字符
		charShow();
		cursor.y++;
		if(cursor.y > padInfo.char_culomns-1){
			cursor.y = padInfo.char_culomns-1;
		}
		
		//擦去光标和字符
		draw_rect(cursor.x*CHAR_WIDTH, cursor.y*CHAR_HEIGHT, CHAR_WIDTH, CHAR_HEIGHT, padInfo.bgColor);
		refresh(cursor.x*CHAR_WIDTH, cursor.y*CHAR_HEIGHT, cursor.x*CHAR_WIDTH + CHAR_WIDTH, cursor.y*CHAR_HEIGHT+CHAR_HEIGHT);
	
		//显示字符
		charShow();
		
		//显示光标
		draw_rect(cursor.x*CHAR_WIDTH, cursor.y*CHAR_HEIGHT+14, CHAR_WIDTH, 2, cursor.color);
		refresh(cursor.x*CHAR_WIDTH, cursor.y*CHAR_HEIGHT+14, cursor.x*CHAR_WIDTH + CHAR_WIDTH, cursor.y*CHAR_HEIGHT+14+2);
	
	}else if(key == GUI_KEY_LEFT){
		//擦去光标和字符
		draw_rect(cursor.x*CHAR_WIDTH, cursor.y*CHAR_HEIGHT, CHAR_WIDTH, CHAR_HEIGHT, padInfo.bgColor);
		refresh(cursor.x*CHAR_WIDTH, cursor.y*CHAR_HEIGHT, cursor.x*CHAR_WIDTH + CHAR_WIDTH, cursor.y*CHAR_HEIGHT+CHAR_HEIGHT);
	
		//显示字符
		charShow();
		
		cursor.x--;
		if(cursor.x < 0){
			if(cursor.y > 0){
				cursor.y--;
				cursor.x = padInfo.char_lines-1;
			}else{
				cursor.x = 0;
			}
		}else{
			//擦去光标和字符
			draw_rect(cursor.x*CHAR_WIDTH, cursor.y*CHAR_HEIGHT, CHAR_WIDTH, CHAR_HEIGHT, padInfo.bgColor);
			refresh(cursor.x*CHAR_WIDTH, cursor.y*CHAR_HEIGHT, cursor.x*CHAR_WIDTH + CHAR_WIDTH, cursor.y*CHAR_HEIGHT+CHAR_HEIGHT);
	
			//显示字符
			charShow();
		
		}
		
		//显示光标
		draw_rect(cursor.x*CHAR_WIDTH, cursor.y*CHAR_HEIGHT+14, CHAR_WIDTH, 2, cursor.color);
		refresh(cursor.x*CHAR_WIDTH, cursor.y*CHAR_HEIGHT+14, cursor.x*CHAR_WIDTH + CHAR_WIDTH, cursor.y*CHAR_HEIGHT+14+2);
	
	}else if(key == GUI_KEY_RIGHT){
		//擦去光标和字符
		draw_rect(cursor.x*CHAR_WIDTH, cursor.y*CHAR_HEIGHT, CHAR_WIDTH, CHAR_HEIGHT, padInfo.bgColor);
		refresh(cursor.x*CHAR_WIDTH, cursor.y*CHAR_HEIGHT, cursor.x*CHAR_WIDTH + CHAR_WIDTH, cursor.y*CHAR_HEIGHT+CHAR_HEIGHT);
	
		//显示字符
		charShow();
		
		cursor.x++;
		if(cursor.x > padInfo.char_lines-1){
			cursor.x = 0;
			if(cursor.y < padInfo.char_culomns-1){
				cursor.y++;
			}else{
				cursor.y = padInfo.char_culomns-1;
				cursor.x = padInfo.char_lines;
			}
		}
		
		//擦去光标和字符
		draw_rect(cursor.x*CHAR_WIDTH, cursor.y*CHAR_HEIGHT, CHAR_WIDTH, CHAR_HEIGHT, padInfo.bgColor);
		refresh(cursor.x*CHAR_WIDTH, cursor.y*CHAR_HEIGHT, cursor.x*CHAR_WIDTH + CHAR_WIDTH, cursor.y*CHAR_HEIGHT+CHAR_HEIGHT);
	
		//显示字符
		charShow();
		
		//显示光标
		draw_rect(cursor.x*CHAR_WIDTH, cursor.y*CHAR_HEIGHT+14, CHAR_WIDTH, 2, cursor.color);
		refresh(cursor.x*CHAR_WIDTH, cursor.y*CHAR_HEIGHT+14, cursor.x*CHAR_WIDTH + CHAR_WIDTH, cursor.y*CHAR_HEIGHT+14+2);
	
	}else if(key == GUI_KEY_HOME){
		//擦去光标和字符
		draw_rect(cursor.x*CHAR_WIDTH, cursor.y*CHAR_HEIGHT, CHAR_WIDTH, CHAR_HEIGHT, padInfo.bgColor);
		refresh(cursor.x*CHAR_WIDTH, cursor.y*CHAR_HEIGHT, cursor.x*CHAR_WIDTH + CHAR_WIDTH, cursor.y*CHAR_HEIGHT+CHAR_HEIGHT);
	
		//显示字符
		charShow();
		
		cursor.x = 0;
		
		//擦去光标和字符
		draw_rect(cursor.x*CHAR_WIDTH, cursor.y*CHAR_HEIGHT, CHAR_WIDTH, CHAR_HEIGHT, padInfo.bgColor);
		refresh(cursor.x*CHAR_WIDTH, cursor.y*CHAR_HEIGHT, cursor.x*CHAR_WIDTH + CHAR_WIDTH, cursor.y*CHAR_HEIGHT+CHAR_HEIGHT);
	
		//显示字符
		charShow();
		//显示光标
		draw_rect(cursor.x*CHAR_WIDTH, cursor.y*CHAR_HEIGHT+14, CHAR_WIDTH, 2, cursor.color);
		refresh(cursor.x*CHAR_WIDTH, cursor.y*CHAR_HEIGHT+14, cursor.x*CHAR_WIDTH + CHAR_WIDTH, cursor.y*CHAR_HEIGHT+14+2);
	
	}else if(key == GUI_KEY_END){
		//擦去光标和字符
		draw_rect(cursor.x*CHAR_WIDTH, cursor.y*CHAR_HEIGHT, CHAR_WIDTH, CHAR_HEIGHT, padInfo.bgColor);
		refresh(cursor.x*CHAR_WIDTH, cursor.y*CHAR_HEIGHT, cursor.x*CHAR_WIDTH + CHAR_WIDTH, cursor.y*CHAR_HEIGHT+CHAR_HEIGHT);
	
		//显示字符
		charShow();
		
		cursor.x = padInfo.char_lines-1;
		
		//擦去光标和字符
		draw_rect(cursor.x*CHAR_WIDTH, cursor.y*CHAR_HEIGHT, CHAR_WIDTH, CHAR_HEIGHT, padInfo.bgColor);
		refresh(cursor.x*CHAR_WIDTH, cursor.y*CHAR_HEIGHT, cursor.x*CHAR_WIDTH + CHAR_WIDTH, cursor.y*CHAR_HEIGHT+CHAR_HEIGHT);
	
		//显示字符
		charShow();
		//显示光标
		draw_rect(cursor.x*CHAR_WIDTH, cursor.y*CHAR_HEIGHT+14, CHAR_WIDTH, 2, cursor.color);
		refresh(cursor.x*CHAR_WIDTH, cursor.y*CHAR_HEIGHT+14, cursor.x*CHAR_WIDTH + CHAR_WIDTH, cursor.y*CHAR_HEIGHT+14+2);
	}else if(key == GUI_KEY_F6){
		makeFileBuf();
		readFileBuf();
	}else if(key == GUI_KEY_F7){
		makeFileBuf();
		writeFileBuf();
	}else if(key == GUI_KEY_F12){	
		resetAll();
	}else if(key == '\n'){
		//擦去光标
		draw_rect(cursor.x*CHAR_WIDTH, cursor.y*CHAR_HEIGHT+14, CHAR_WIDTH, 2, padInfo.bgColor);
		refresh(cursor.x*CHAR_WIDTH, cursor.y*CHAR_HEIGHT+14, cursor.x*CHAR_WIDTH + CHAR_WIDTH, cursor.y*CHAR_HEIGHT+14+2);
		charWrite(cursor.x, cursor.y, key);
		charShow();
		if(cursor.y < padInfo.char_culomns-1){
			cursor.x = 0;
			cursor.y++;
		}
		//显示光标
		draw_rect(cursor.x*CHAR_WIDTH, cursor.y*CHAR_HEIGHT+14, CHAR_WIDTH, 2, cursor.color);
		refresh(cursor.x*CHAR_WIDTH, cursor.y*CHAR_HEIGHT+14, cursor.x*CHAR_WIDTH + CHAR_WIDTH, cursor.y*CHAR_HEIGHT+14+2);
	
	}else if(key == '\b'){
		//擦去光标
		draw_rect(cursor.x*CHAR_WIDTH, cursor.y*CHAR_HEIGHT+14, CHAR_WIDTH, 2, padInfo.bgColor);
		refresh(cursor.x*CHAR_WIDTH, cursor.y*CHAR_HEIGHT+14, cursor.x*CHAR_WIDTH + CHAR_WIDTH, cursor.y*CHAR_HEIGHT+14+2);
	
		cursor.x--;
		if(cursor.x < 0 ){
			if(cursor.y > 0){
				cursor.y--;
				cursor.x = padInfo.char_lines-1;
			}else{
				cursor.x = 0;
			}
		}
		charClear(cursor.x, cursor.y);
		
		//擦除字符
		draw_rect(cursor.x*CHAR_WIDTH, cursor.y*CHAR_HEIGHT, CHAR_WIDTH, CHAR_HEIGHT, padInfo.bgColor);
		refresh(cursor.x*CHAR_WIDTH, cursor.y*CHAR_HEIGHT, cursor.x*CHAR_WIDTH + CHAR_WIDTH, cursor.y*CHAR_HEIGHT+CHAR_HEIGHT);
	
		//显示光标
		draw_rect(cursor.x*CHAR_WIDTH, cursor.y*CHAR_HEIGHT+14, CHAR_WIDTH, 2, cursor.color);
		refresh(cursor.x*CHAR_WIDTH, cursor.y*CHAR_HEIGHT+14, cursor.x*CHAR_WIDTH + CHAR_WIDTH, cursor.y*CHAR_HEIGHT+14+2);
	
	}else{
		//擦去光标
		draw_rect(cursor.x*CHAR_WIDTH, cursor.y*CHAR_HEIGHT, CHAR_WIDTH, CHAR_HEIGHT, padInfo.bgColor);
		refresh(cursor.x*CHAR_WIDTH, cursor.y*CHAR_HEIGHT, cursor.x*CHAR_WIDTH + CHAR_WIDTH, cursor.y*CHAR_HEIGHT+CHAR_HEIGHT);
	
		charWrite(cursor.x, cursor.y, key);
		//显示字符
		charShow();
		cursor.x++;
		if(cursor.x > padInfo.char_lines-1){
			cursor.x = 0;
			if(cursor.y < padInfo.char_culomns-1){
				cursor.y++;
			}else{
				
				cursor.x = padInfo.char_lines-1;
			}
			
		}
		//显示光标
		draw_rect(cursor.x*CHAR_WIDTH, cursor.y*CHAR_HEIGHT+14, CHAR_WIDTH, 2, cursor.color);
		refresh(cursor.x*CHAR_WIDTH, cursor.y*CHAR_HEIGHT+14, cursor.x*CHAR_WIDTH + CHAR_WIDTH, cursor.y*CHAR_HEIGHT+14+2);
	
	}
	showStatus();
}

static void charWrite(int x, int y, uint8 ch)
{
	
	padInfo.charBuffer[y*padInfo.char_lines+x] = ch;
}
static void charClear(int x, int y)
{
	
	padInfo.charBuffer[y*padInfo.char_lines+x] = 0;
}

static uint8 charRead(int x, int y)
{
	return padInfo.charBuffer[y*padInfo.char_lines+x];
}

static void charShow()
{
	uint8 ch = charRead(cursor.x, cursor.y);
	if(ch != 0){
		if(ch == '\n'){
			draw_rect(cursor.x*CHAR_WIDTH, cursor.y*CHAR_HEIGHT, CHAR_WIDTH, CHAR_HEIGHT, padInfo.enterColor);
			refresh(cursor.x*CHAR_WIDTH, cursor.y*CHAR_HEIGHT, cursor.x*CHAR_WIDTH + CHAR_WIDTH, cursor.y*CHAR_HEIGHT+CHAR_HEIGHT);
	
		}else{
			draw_rect(cursor.x*CHAR_WIDTH, cursor.y*CHAR_HEIGHT, CHAR_WIDTH, CHAR_HEIGHT, padInfo.existColor);
			refresh(cursor.x*CHAR_WIDTH, cursor.y*CHAR_HEIGHT, cursor.x*CHAR_WIDTH + CHAR_WIDTH, cursor.y*CHAR_HEIGHT+CHAR_HEIGHT);
	
		}
		if(ch != '\n' ){
			draw_char(cursor.x*CHAR_WIDTH, cursor.y*CHAR_HEIGHT, ch, padInfo.charColor);
			refresh(cursor.x*CHAR_WIDTH, cursor.y*CHAR_HEIGHT, cursor.x*CHAR_WIDTH + CHAR_WIDTH, cursor.y*CHAR_HEIGHT+CHAR_HEIGHT);
	
		}
	}
}

static void makeFileBuf()
{
	int i,j;
	uint8 ch;
	i = 0;
	j = 0;
	
	cleanFileBuf();
	
	while(i < padInfo.char_culomns*padInfo.char_lines){
		ch = padInfo.charBuffer[i];
		if(ch != 0){
			fileInfo.buffer[j] = ch;
			j++;
		}
		i++;
	}
	
	//在最后加上0
	fileInfo.buffer[j] = 0;

	int pos = strpos((char *)fileInfo.buffer, '\n');
	memset(fileInfo.name, 0, NAME_FILE_LEN);
	strncpy((char *)fileInfo.name, (char *)fileInfo.buffer, pos);
	fileInfo.name[pos] = 0;
	
	//1 is '\n', we skip it
	fileInfo.text = fileInfo.buffer+pos+1;

	fileInfo.size = j - (pos+1);

	putMessage(MSG_TIP, "FILE BUF DONE");
	
}

static void makeCharBuf()
{
	//导入内容
	int i,j;
	uint8 ch;
	i = padInfo.char_lines;
	j = 0;
	int odd;
	while(i < padInfo.char_culomns*padInfo.char_lines && fileInfo.buffer[j]){
		//获取一个字符
		ch = fileInfo.buffer[j];
		padInfo.charBuffer[i] = ch;
		if(ch == '\n'){
			odd = i%padInfo.char_lines;
			i -= odd;
			i += padInfo.char_lines;
		}else{
			i++;
		}
		j++;
		
	}
	//文件内容指向读入的数据
	fileInfo.text = fileInfo.buffer;
	
	putMessage(MSG_TIP, "CHAR BUF DONE");
}

static int writeFileBuf()
{
	if(fileInfo.name[0] != '/' || fileInfo.name[1] == 0){
		putMessage(MSG_ERR, "NO NAME");
		return -1;
	}
	int fd = fopen(fileInfo.name, O_CREAT|O_RDWR);
	if(fd == -1){
		putMessage(MSG_ERR, "OPEN FAILED");
		return -1;
	}
	
	int write = fwrite(fd, fileInfo.text, fileInfo.size);
	if(write == -1){
		putMessage(MSG_ERR, "WRITE FAILED");
		fclose(fd);
		return -1;
	}
	fclose(fd);
	putMessage(MSG_TIP, "WRITE DONE");
	return 0;
}

static int readFileBuf()
{
	if(fileInfo.name[0] != '/' || fileInfo.name[1] == 0){
		putMessage(MSG_ERR, "NO NAME");
		return -1;
	}
	int fd = fopen(fileInfo.name, O_RDWR);
	
	if(fd == -1){
		
		putMessage(MSG_ERR, "OPEN FAILED");
		return -1;
	}
	
	struct stat stat;
	fstat(fileInfo.name, &stat);
	/*
	char s[40];
	sprintf(s,"file size:%d",stat.st_size);
	gluRectangle(0, padInfo.char_culomns*CHAR_HEIGHT+16, padInfo.char_lines*CHAR_WIDTH, CHAR_HEIGHT, padInfo.bgColor);
	gluString(0, padInfo.char_culomns*CHAR_HEIGHT+16, s, COLOR_RED);
	*/
	//printf("read file size:%d\n", stat.st_size);
	
	cleanFileBuf();
	
	int read = fread(fd, fileInfo.buffer, stat.st_size);
	if(read == -1){
		putMessage(MSG_ERR, "RED FAILED");
		fclose(fd);
		return -1;
	}
	
	fileInfo.size = read;
	fileInfo.text = fileInfo.buffer;
	
	putMessage(MSG_TIP, "READ DONE");
	
	fclose(fd);
	//printf("file load sucess!\n");
	makeCharBuf();
	loadFram();
	return 0;
}

static void loadFram()
{
	//先清空
	draw_rect(0, 0, padInfo.char_lines*CHAR_WIDTH, padInfo.char_culomns*CHAR_HEIGHT, padInfo.bgColor);
	refresh(0, 0, padInfo.char_lines*CHAR_WIDTH, padInfo.char_culomns*CHAR_HEIGHT);
	
	//导入内容
	uint8 ch;
	int x,y;
	for(y = 0; y < padInfo.char_culomns; y++){
		for(x = 0; x < padInfo.char_lines; x++){
			ch = charRead(x,y);
			if(ch == '\n'){
				draw_rect(x*CHAR_WIDTH, y*CHAR_HEIGHT, CHAR_WIDTH, CHAR_HEIGHT, padInfo.enterColor);
				refresh(x*CHAR_WIDTH, y*CHAR_HEIGHT, x*CHAR_WIDTH + CHAR_WIDTH, y*CHAR_HEIGHT+CHAR_HEIGHT);
	
			}else if(ch != 0){
				draw_rect(x*CHAR_WIDTH, y*CHAR_HEIGHT, CHAR_WIDTH, CHAR_HEIGHT, padInfo.existColor);
				refresh(x*CHAR_WIDTH, y*CHAR_HEIGHT, x*CHAR_WIDTH + CHAR_WIDTH, y*CHAR_HEIGHT+CHAR_HEIGHT);
	
			}
			if(ch != '\n' && ch != 0){
				//draw_string(x*CHAR_WIDTH, y*CHAR_HEIGHT, ch, padInfo.charColor);
				draw_char(x*CHAR_WIDTH, y*CHAR_HEIGHT, ch, padInfo.charColor);
				refresh(x*CHAR_WIDTH, y*CHAR_HEIGHT, x*CHAR_WIDTH + CHAR_WIDTH, y*CHAR_HEIGHT+CHAR_HEIGHT);
	
			}
		}
	}
	
	//显示光标
	draw_rect(cursor.x*CHAR_WIDTH, cursor.y*CHAR_HEIGHT+14, CHAR_WIDTH, 2, cursor.color);
	refresh(cursor.x*CHAR_WIDTH, cursor.y*CHAR_HEIGHT+14, cursor.x*CHAR_WIDTH + CHAR_WIDTH, cursor.y*CHAR_HEIGHT+14+2);
	//刷新
	
}

static void cleanCharBuf()
{
	memset(padInfo.charBuffer, 0, padInfo.char_lines*padInfo.char_culomns);
}


static void cleanFileBuf()
{
	memset(fileInfo.buffer, 0, padInfo.char_lines*padInfo.char_culomns);
}

static void resetAll()
{
	cleanCharBuf();
	cleanFileBuf();
	cursor.x = 0;
	cursor.y = 0;
	fileInfo.text = fileInfo.buffer;
	fileInfo.size = 0;
	makeCharBuf();
	loadFram();
	
}

static void showStatus()
{
	draw_rect(0, padInfo.char_culomns*CHAR_HEIGHT, padInfo.char_lines*CHAR_WIDTH-120, CHAR_HEIGHT, padInfo.bgColor);
	refresh(0, padInfo.char_culomns*CHAR_HEIGHT, padInfo.char_lines*CHAR_WIDTH-120, padInfo.char_culomns*CHAR_HEIGHT+CHAR_HEIGHT);
	
	padInfo.charLength = getCharLength();
	
	char s[64];
	sprintf(s,"length:%d Ln:%d Col:%d",padInfo.charLength, cursor.x,cursor.y);
	draw_string(0, padInfo.char_culomns*CHAR_HEIGHT, s, padInfo.statusColor);
	
	refresh(0, padInfo.char_culomns*CHAR_HEIGHT, CHAR_WIDTH*padInfo.char_lines,padInfo.char_culomns*CHAR_HEIGHT+ CHAR_HEIGHT);
}

static int getCharLength()
{
	int i = 0, j = 0;
	while(i < padInfo.char_culomns*padInfo.char_lines){
		if(padInfo.charBuffer[i] != 0){
			j++;
		}
		i++;
	}
	return j;
} 

static void putMessage(int type, char *msg)
{
	int len = strlen(msg);
	draw_rect(padInfo.char_lines*CHAR_WIDTH-120, padInfo.char_culomns*CHAR_HEIGHT, 120, CHAR_HEIGHT, padInfo.bgColor);
	if(type == MSG_ERR){
		draw_string(padInfo.char_lines*CHAR_WIDTH-120, padInfo.char_culomns*CHAR_HEIGHT, msg, padInfo.errorColor);
		refresh(padInfo.char_lines*CHAR_WIDTH-120, padInfo.char_culomns*CHAR_HEIGHT, padInfo.char_lines*CHAR_WIDTH-120+len*CHAR_WIDTH,padInfo.char_culomns*CHAR_HEIGHT+ CHAR_HEIGHT);
	
	}else if(type == MSG_TIP){
		draw_string(padInfo.char_lines*CHAR_WIDTH-120, padInfo.char_culomns*CHAR_HEIGHT, msg, padInfo.tipColor);
		refresh(padInfo.char_lines*CHAR_WIDTH-120, padInfo.char_culomns*CHAR_HEIGHT, padInfo.char_lines*CHAR_WIDTH-120+len*CHAR_WIDTH,padInfo.char_culomns*CHAR_HEIGHT+ CHAR_HEIGHT);
	}
}
