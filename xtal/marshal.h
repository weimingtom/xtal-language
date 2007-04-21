
#pragma once

#include "any.h"
#include "array.h"
#include "mapimpl.h"
#include "stream.h"

namespace xtal{

class Marshal{
public:

	Marshal(const Stream& s);

	void dump(const Any& v);

	Any load();

private:

	void inner_dump(const Any& v);

	Any inner_load();

	enum{ VALUE, VALUE2, LIB, LIB2, REF, TNULL, INT, FLOAT, STRING, TID, ARRAY, MAP };

	Any demangle(int_t n);

	int_t register_dvalue(const Any& v, bool& added);

	int_t register_lvalue(const Any& v);

	StrictMap dmap_;
	Array dvalues_;

	StrictMap lmap_;
	Array lvalues_;

	Stream stream_;
};

}
