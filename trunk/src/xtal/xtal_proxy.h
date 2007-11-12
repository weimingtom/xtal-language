
#pragma once

namespace xtal{

class AtProxy{
	AnyPtr obj;
	AnyPtr key;
public:

	AtProxy(const AnyPtr& obj, const AnyPtr& key)
		:obj(obj), key(key){}

	const AtProxy& operator =(const AnyPtr& value);

	operator const AnyPtr&();

	const AnyPtr& operator ->();
};

class SendProxy{
	AnyPtr obj;
	IDPtr primary_key;
	AnyPtr secondary_key;
public:

	SendProxy(const AnyPtr& obj, const IDPtr& primary_key, const AnyPtr& secondary_key);

	~SendProxy();

	//const AtProxy& operator =(const AnyPtr& value);

	operator const AnyPtr&(){
		execute();
		return obj;
	}

	const AnyPtr& operator ->(){
		execute();
		return obj;
	}

public:

	const AnyPtr& operator ()(){
		execute();
		return obj;
	}

	const AnyPtr& operator ()(const Param& a0);

	const AnyPtr& operator ()(const Param& a0, const Param& a1);

	const AnyPtr& operator ()(const Param& a0, const Param& a1, const Param& a2);

	const AnyPtr& operator ()(const Param& a0, const Param& a1, const Param& a2, const Param& a3);

	const AnyPtr& operator ()(const Param& a0, const Param& a1, const Param& a2, const Param& a3, const Param& a4);

private:

	void execute();
};

}
