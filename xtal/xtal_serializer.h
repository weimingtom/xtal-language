
#pragma once

#include "xtal_any.h"
#include "xtal_array.h"
#include "xtal_map.h"
#include "xtal_stream.h"

namespace xtal{

class Serializer{
public:

	Serializer(const StreamPtr& s);

	void serialize(const AnyPtr& v);

	AnyPtr deserialize();

	void xtalize(const AnyPtr& v);

private:

	void inner_serialize(const AnyPtr& v);
	AnyPtr inner_deserialize();
	void inner_xtalize(const AnyPtr& v, int_t tab);

	bool check_id(const InternedStringPtr& id);
	AnyPtr demangle(int_t n);

	int_t register_value(const AnyPtr& v, bool& added);
	int_t append_value(const AnyPtr& v);

	void put_tab(int_t tab);

	void clear(){
		values_ = xnew<Array>();
		map_ = xnew<Map>();
	}

private:

	enum{ SERIAL_NEW, LIB, REF, TNULL, TINT, TFLOAT, TSTRING, TID, TARRAY, TMAP, TFALSE, TTRUE, TNOP };

private:

	MapPtr map_;
	ArrayPtr values_;
	StreamPtr stream_;
};

}
