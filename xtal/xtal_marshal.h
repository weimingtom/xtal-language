
#pragma once

#include "xtal_any.h"
#include "xtal_array.h"
#include "xtal_mapimpl.h"
#include "xtal_stream.h"

namespace xtal{

class Marshal{
public:

	Marshal(const Stream& s);

	void serialize(const Any& v);

	Any deserialize();

	void xtalize(const Any& v);

private:

	void inner_serialize(const Any& v);
	Any inner_deserialize();
	void inner_xtalize(const Any& v, int_t tab);

	bool check_id(const ID& id);
	Any demangle(int_t n);

	int_t register_value(const Any& v, bool& added);
	int_t append_value(const Any& v);

	void put_tab(int_t tab);

	void clear(){
		values_.clear();
		map_.clear();
	}

private:

	enum{ SERIAL_NEW, LIB, REF, TNULL, TINT, TFLOAT, TSTRING, TID, TARRAY, TMAP, TFALSE, TTRUE, TNOP };

private:

	StrictMap map_;
	Array values_;

	Stream stream_;
};

}
