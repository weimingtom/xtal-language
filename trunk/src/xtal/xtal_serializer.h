
#pragma once

namespace xtal{

class Serializer{
public:

	Serializer(const StreamPtr& s);

	void serialize(const AnyPtr& v);

	AnyPtr deserialize();

private:

	void inner_serialize(const AnyPtr& v);
	AnyPtr inner_deserialize();
	
	AnyPtr demangle(const AnyPtr& n);

	int_t register_value(const AnyPtr& v, bool& added);
	int_t append_value(const AnyPtr& v);

	void put_tab(int_t tab);

	void clear(){
		values_ = xnew<Array>();
		map_ = xnew<Map>();
	}

private:

	enum{ SERIAL_NEW, LIB, REF, TNULL, TUNDEFINED, TFALSE, TTRUE, TINT, TFLOAT, TSTRING, TARRAY, TMAP };

private:

	MapPtr map_;
	ArrayPtr values_;
	StreamPtr stream_;
};

}
