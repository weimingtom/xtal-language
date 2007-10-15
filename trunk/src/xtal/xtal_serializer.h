
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

	bool check_id(const InternedStringPtr& id);
	void check_id_and_throw(const InternedStringPtr& id);
	
	AnyPtr demangle(const AnyPtr& n);

	int_t register_value(const AnyPtr& v, bool& added);
	int_t append_value(const AnyPtr& v);

	void put_tab(int_t tab);

	void clear(){
		values_ = xnew<Array>();
		map_ = xnew<Map>();
	}

private:

	enum{ SERIAL_NEW, LIB, REF, TNULL, TNOP, TFALSE, TTRUE, TINT, TFLOAT, TSTRING, TARRAY, TMAP };

private:

	MapPtr map_;
	ArrayPtr values_;
	StreamPtr stream_;
};

}
