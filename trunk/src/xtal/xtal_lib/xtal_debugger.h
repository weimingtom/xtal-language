#ifdef WIN32

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#pragma comment(lib,"ws2_32.lib")

struct WinSock{
	WinSock(){ WSAStartup(MAKEWORD(2, 2), &wsaData); }
	~WinSock(){ WSACleanup(); }
	WSADATA wsaData;
};
	
static WinSock win_sock;

inline int close(int sock){
	return closesocket(sock);
}

#else

#include <sys/socket.h>

#endif

#pragma once

namespace xtal{

class DebugStream;
typedef SmartPtr<DebugStream> DebugStreamPtr;

/**
* \biref デバッガとやりとりするためのストリームクラス
* TCPをこの実装ではTCPを用いる
*/
class DebugStream : public Stream{
public:

	DebugStream(){
		socket_ = -1;
	}

	DebugStream(int s){
		socket_ = s;
	}

	DebugStream(const StringPtr& path, const StringPtr& port){
		socket_ = -1;
		open(path, port);
	}

	virtual ~DebugStream(){
		close();
	}

	void open(const StringPtr& host, const StringPtr& port){
		close();
		socket_ = socket(AF_INET, SOCK_STREAM, 0);

		if(is_open()){
			struct addrinfo hints = {0};
			hints.ai_family = AF_UNSPEC;
			hints.ai_socktype = SOCK_STREAM;
			hints.ai_flags = 0;

			struct addrinfo* res = 0;
			int err = getaddrinfo(host->c_str(), port->c_str(), &hints, &res);
			if(err!=0){
				return;
			}

			for(struct addrinfo* ai= res; ai; ai=ai->ai_next){
				socket_ = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
				if(socket_<0){
					break;
				}

				if(connect(socket_, ai->ai_addr, ai->ai_addrlen)<0){
					close();
					continue;
				}
				else{
					break;
				}
			}

			freeaddrinfo(res);
		}
	}

	bool is_open(){
		return socket_>=0;
	}

	virtual void close(){
		if(socket_>=0){
			::close(socket_);
			socket_ = -1;
		}
	}
  
	bool available(){
	   fd_set fdset; 
	   FD_ZERO(&fdset); 
	   FD_SET(socket_ , &fdset);

	   struct timeval timeout;
	   timeout.tv_sec = 0; 
	   timeout.tv_usec = 0;

	   return select(socket_+1, &fdset, 0, 0, &timeout)>0; // && FD_ISSET(socket_, &fdset);
	}

	virtual uint_t read(void* dest, uint_t size){
		int read = recv(socket_, (char*)dest, size, 0);
		if(read<0){
			close();
			return 0;
		}
		return read;
	}

	virtual uint_t write(const void* src, uint_t size){
		int temp = ::send(socket_, (char*)src, size, 0);
		if(temp<0){
			close();
			return 0;
		}
		return temp;
	}

private:
	int socket_;
};

class Debugger{
public:

	/**
	* \brief デバッガをアタッチする
	* デバッガと通信するためのストリーム
	*/
	bool attach(const DebugStreamPtr& stream){
		if(!stream->is_open()){
			return false;
		}

		stream_ = stream;
		debug::set_breakpoint_hook(bind_this(method(&Debugger::linehook), this));
		set_require_source_hook(bind_this(method(&Debugger::require_source_hook), this));
		eval_exprs_ = xnew<Map>();
		code_map_ = xnew<Map>();

		while(ArrayPtr cmd = recv_command()){
			AnyPtr type = cmd->at(0);
			if(raweq(type, Xid(start))){
				break;
			}
			exec_command(cmd);
		}

		return true;
	}

	/**
	* デバッガを更新する
	* 
	*/
	void update(){
		// 次のコマンドが到着していたらコマンドをデシリアライズして実行する
		while(stream_->available()){
			exec_command(ptr_cast<Array>(stream_->deserialize()));
		}
	}

private:

	ArrayPtr recv_command(){
		return ptr_cast<Array>(stream_->deserialize());
	}

	CodePtr require_source_hook(const StringPtr& name){
		ArrayPtr a = xnew<Array>();
		a->push_back(Xid(require));
		a->push_back(name);
		stream_->serialize(a);

		CodePtr ret;
		while(ArrayPtr cmd = recv_command()){
			AnyPtr type = cmd->at(0);
			if(raweq(type, Xid(required_source))){
				ret = ptr_cast<Code>(cmd->at(1));
				if(ret){
					MapPtr map = xnew<Map>();
					map->set_at(Xid(code), ret);
					code_map_->set_at(ret->source_file_name(), map);
				}
				else{
					break;
				}

				continue;
			}

			if(raweq(type, Xid(start))){
				break;
			}

			exec_command(cmd);
		}

		return ret;
	}

	void exec_command(const ArrayPtr& cmd){
		if(!cmd){
			return;
		}

		AnyPtr type = cmd->at(0);

		if(raweq(type, Xid(add_breakpoint))){
			AnyPtr ddd = cmd->at(1);
			if(MapPtr value = ptr_cast<Map>(code_map_->at(cmd->at(1)))){
				if(CodePtr code = ptr_cast<Code>(value->at(Xid(code)))){
					code->add_breakpoint(cmd->at(2)->to_i());
					value->set_at(cmd->at(2)->to_i(), cmd->at(3));
				}
			}

			return;
		}

		if(raweq(type, Xid(remove_breakpoint))){
			if(MapPtr value = ptr_cast<Map>(code_map_->at(cmd->at(1)))){
				if(CodePtr code = ptr_cast<Code>(value->at(Xid(code)))){
					code->remove_breakpoint(cmd->at(2)->to_i());
				}
			}

			return;
		}

		if(raweq(type, Xid(add_eval_expr))){
			eval_exprs_->set_at(cmd->at(1), cmd->at(2));
			return;
		}

		if(raweq(type, Xid(remove_eval_expr))){
			eval_exprs_->erase(cmd->at(1));
			return;
		}
	}

	ArrayPtr make_debug_object(const AnyPtr& v, int depth = 3){
		ArrayPtr ret = xnew<Array>(3);
		ret->set_at(0, v->get_class()->to_s());
		ret->set_at(1, v->to_s());

		// 基本型かチェック
		switch(type(v)){
			case TYPE_NULL:
			case TYPE_UNDEFINED:
			case TYPE_INT:
			case TYPE_FLOAT:
			case TYPE_FALSE:
			case TYPE_TRUE:
			case TYPE_SMALL_STRING:
			case TYPE_LITERAL_STRING:
			case TYPE_INTERNED_STRING:
			case TYPE_STRING:
				return ret;
		}

		if(depth<=0){
			ret->set_at(2, "...");
			return ret;
		}

		switch(type(v)){
			XTAL_DEFAULT{}

			XTAL_CASE(TYPE_ARRAY){
				ArrayPtr children = xnew<Array>();
				Xfor(it, v){
					children->push_back(make_debug_object(it, depth-1));
				}
				ret->set_at(2, children);
				return ret;
			}

			XTAL_CASE(TYPE_VALUES){
				ArrayPtr children = xnew<Array>();
				Xfor(it, v){
					children->push_back(make_debug_object(it, depth-1));
				}
				ret->set_at(2, children);
				return ret;
			}
		}

		if(const MapPtr& a = ptr_cast<Map>(v)){
			MapPtr children = xnew<Map>();
			Xfor2(key, val, v){
				children->set_at(key->to_s(), make_debug_object(val, depth-1));
			}
			ret->set_at(2, children);
			return ret;
		}

		if(const ClassPtr& a = ptr_cast<Class>(v)){
			MapPtr children = xnew<Map>();
			Xfor3(key, skey, val, a->members()){
				children->set_at(key->to_s(), make_debug_object(val, depth-1));
			}
			ret->set_at(2, children);
			return ret;
		}

		AnyPtr data = v->s_save();
		if(const MapPtr& a = ptr_cast<Map>(data)){
			MapPtr children = xnew<Map>();
			Xfor2(key, val, a){
				Xfor2(key2, val2, val){
					children->set_at(key2->to_s(), make_debug_object(val2, depth-1));
				}
			}
			ret->set_at(2, children);
			return ret;
		}

		return ret;
	}

	ArrayPtr make_call_stack_info(const debug::HookInfoPtr& info){
		ArrayPtr ret = xnew<Array>();

		{
			ArrayPtr record = xnew<Array>(3);
			record->set_at(0, info->fun_name());
			record->set_at(1, info->file_name());
			record->set_at(2, info->lineno());
			ret->push_back(record);
		}

		for(int i=2; i<info->call_stack_size(); ++i){
			ArrayPtr record = xnew<Array>(3);
			if(debug::CallerInfoPtr caller = info->caller(i)){
				record->set_at(0, caller->fun_name());
				record->set_at(1, caller->file_name());
				record->set_at(2, caller->lineno());
			}
			ret->push_back(record);
		}

		return ret;
	}

	MapPtr make_eval_expr_info(const debug::HookInfoPtr& info, int level){
		MapPtr ret = xnew<Map>();
		Xfor2(key, value, eval_exprs_){
			if(CodePtr code = ptr_cast<Code>(value)){
				AnyPtr val = info->vm()->eval(code, level);
				
				if(AnyPtr e = info->vm()->catch_except()){
					ret->set_at(key, null);
				}
				else{
					ret->set_at(key, make_debug_object(val));
				}
			}
			else{
				ret->set_at(key, null);
			}
		}

		return ret;
	}

	void send_break(debug::HookInfoPtr info, int level){
		ArrayPtr data = xnew<Array>();
		data->push_back(Xid(break));
		data->push_back(make_eval_expr_info(info, level));
		data->push_back(make_call_stack_info(info));
		data->push_back(level);
		stream_->serialize(data);
	}

	int linehook(debug::HookInfoPtr info){
		if(info->kind()==BREAKPOINT){
			if(MapPtr value = ptr_cast<Map>(code_map_->at(info->file_name()))){
				if(CodePtr code = ptr_cast<Code>(value->at(Xid(code)))){
					if(CodePtr eval = ptr_cast<Code>(value->at(info->lineno()))){
						AnyPtr val = info->vm()->eval(eval);
						info->vm()->catch_except();
						if(!val){
							return debug::REDO;
						}
					}
				}
			}
		}

		int level = 0;
		send_break(info, level);
		while(true){
			if(ArrayPtr cmd = ptr_cast<Array>(stream_->deserialize())){
				AnyPtr type = cmd->at(0);

				if(raweq(type, Xid(move_callstack))){
					level = cmd->at(1)->to_i();
					send_break(info, level);
					continue;
				}

				if(raweq(type, Xid(nostep))){
					send_break(info, level);
					continue;
				}

				if(raweq(type, Xid(run))){
					return debug::RUN;
				}

				if(raweq(type, Xid(step_into))){
					return debug::STEP_INTO;
				}

				if(raweq(type, Xid(step_over))){
					return debug::STEP_OVER;
				}

				if(raweq(type, Xid(step_out))){
					return debug::STEP_OUT;
				}

				exec_command(cmd);
				continue;
			}
		}
	}

private:
	DebugStreamPtr stream_;
	MapPtr eval_exprs_;
	MapPtr code_map_;
};

}
