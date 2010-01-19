
#include "../src/xtal/xtal.h"
#include "../src/xtal/xtal_macro.h"

#include "../src/xtal/xtal_lib/xtal_winthread.h"
#include "../src/xtal/xtal_lib/xtal_cstdiostream.h"
#include "../src/xtal/xtal_lib/xtal_winfilesystem.h"
#include "../src/xtal/xtal_lib/xtal_chcode.h"
#include "../src/xtal/xtal_lib/xtal_errormessage.h"

#include "../src/xtal/xtal_codebuilder.h"
	
using namespace xtal;

class InteractiveScanner : public xpeg::Scanner{
public:

	InteractiveScanner(){
		line_ = 1;
		continue_stmt_ = false;
		continue_line_ = false;
	}
	
	virtual int_t do_read(AnyPtr* buffer, int_t max){
		if(continue_line_){
			return read_line(buffer, max);
		}
		else{
			if(continue_stmt_){
				stdout_stream()->put_s(format(XTAL_STRING("ix:%03d>    "))->call(line_)->to_s());
			}
			else{
				stdout_stream()->put_s(format(XTAL_STRING("ix:%03d>"))->call(line_)->to_s());
			}

			continue_stmt_ = true;
		
			return read_line(buffer, max);
		}

		return 0;
	}

	int_t read_line(AnyPtr* buffer, int_t max){
		continue_line_ = true;

		for(int_t i=0; i<max; ++i){
			int_t ret = stdin_stream()->read_charactors(buffer + i, 1);
			if(ret==0){
				return i;
			}

			if(raweq(buffer[i], n_ch_)){
				line_++;
				continue_line_ = false;
				return i+1;
			}
		}

		return max;
	}

	void end_stmt(){
		continue_stmt_ = false;
	}

private:
	int_t line_;
	bool continue_stmt_;
	bool continue_line_;
};

void interactive_compile_loop(const VMachinePtr& vm){
	SmartPtr<InteractiveScanner> p = unchecked_ptr_cast<InteractiveScanner>(vm->arg(0));
	vm->eval(p, 0);
	
	XTAL_CATCH_EXCEPT(e){
		stdout_stream()->println(e);
	}

	p->end_stmt();
	vm->return_result();
}

void interactive_compile(){
	if(CodePtr code = Xsrc((
		quitvalue: false;
		fun quit(){ quitvalue = true; }

		while(!quitvalue){ 
			interactive_compile_loop(arg[0], arg[1]); 
		}
	))){
		code->def_fun(XTAL_STRING("interactive_compile_loop"), &interactive_compile_loop);
		code->call(xnew<InteractiveScanner>());
	}
}

int main(int argc, char** argv){

	CStdioStdStreamLib cstd_std_stream_lib;
	WinThreadLib win_thread_lib;
	WinFilesystemLib win_filesystem_lib;
	SJISChCodeLib sjis_ch_code_lib;

	Setting setting;
	setting.thread_lib = &win_thread_lib;
	setting.std_stream_lib = &cstd_std_stream_lib;
	setting.filesystem_lib = &win_filesystem_lib;
	setting.ch_code_lib = &sjis_ch_code_lib;

	initialize(setting);

	XTAL_PROTECT{
		bind_error_message();

		interactive_compile();

		XTAL_CATCH_EXCEPT(e){
			stderr_stream()->println(e);
		}
	}
	XTAL_OUT_OF_MEMORY{
		puts("out of memory");
	}
		
	uninitialize();

	return 0;
}
