
#include <fstream>
#include <stdio.h>
#include <stdarg.h>

#include "platform.h" // 添加平台兼容性头文件

/*
这个类实现了类似Unix tee命令的功能，将输出同时发送到控制台和文件
*/

class tee
{
public:
	tee();
	~tee(void);
	void Open(char* filename);
	void OpenAppend(char* filename);
	void Close();
	void p(const char* text);
	void pf(const char* format, ...);
	void enable(bool onoff);

private:
	std::ofstream outfile;
	int flag;
	bool enable_output;
};


tee::tee()
{
	flag = 0;
	enable_output = true;
}

tee::~tee()
{
	this->Close();
}

void tee::OpenAppend(char*filename)
{
	if (flag != 0) {
		outfile.close();
	}
	outfile.open(filename, std::ofstream::out | std::ofstream::app);
	flag = 1;
}


void tee::Open(char*filename)
{
	if (flag != 0) {
		outfile.close();
	}
	outfile.open(filename);
	flag = 1;
}

void tee::Close()
{
	if (flag != 0) {
		outfile.close();
	}
	flag = 0;
}

void tee::p(const char* text)
{
	if (enable_output == false) {
		return;
	}

	printf("%s", text);
	if (flag == 1) {
		outfile << text;
		outfile.flush();
	}
	fflush(stdout);
}

void tee::pf(const char* format, ...)
{
	if (enable_output == false) {
		return;
	}

	char buffer[256];
	va_list args;
	va_start(args, format);
	
#ifdef OS_WINDOWS
	vsprintf_s(buffer, 256, format, args);
#else
	vsnprintf(buffer, 256, format, args);
#endif

	va_end(args);

	this->p(buffer);
}

void tee::enable(bool onoff){
	enable_output = onoff;
}







