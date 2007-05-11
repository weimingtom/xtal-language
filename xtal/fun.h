
#pragma once

namespace xtal{

// fwd decl
class FunImpl;
class MethodImpl;
class FiberImpl;

void InitFun();

class Fun : public Any{
public:

	Fun(const Frame& outer, const Any& athis, const Code& code, FunCore* core);

	Fun(const Null& = null)
		:Any(null){}

	explicit Fun(FunImpl* p)
		:Any((AnyImpl*)p){}

	const Frame& outer() const;
	const Code& code() const;
	int_t pc() const;
	const u8* source() const;
	const ID& param_name_at(size_t i) const;
	int_t param_size() const;
	bool used_args_object() const;
	int_t defined_file_line_number() const;
	FunCore* core() const;
	void set_core(FunCore* fc) const;
	
	FunImpl* impl() const{ return (FunImpl*)Any::impl(); }
};

class Method : public Fun{
public:

	Method(const Frame& outer, const Code& code, FunCore* core);

	Method(const Null&)
		:Fun(null){}

	explicit Method(MethodImpl* p)
		:Fun((FunImpl*)p){}

	MethodImpl* impl() const{ return (MethodImpl*)Any::impl(); }
};

void InitFiber();

class Fiber : public Fun{
public:

	Fiber(const Frame& outer, const Any& th, const Code& code, FunCore* core);

	Fiber(const Null&)
		:Fun(null){}

	explicit Fiber(FiberImpl* p)
		:Fun((FunImpl*)p){}

	void iter_first(const VMachine& vm);

	void iter_next(const VMachine& vm);
	
	void iter_break();

	FiberImpl* impl() const{ return (FiberImpl*)Any::impl(); }
};

}

