/** \file src/xtal/xtal_serializer.h
* \brief src/xtal/xtal_serializer.h
*/

#ifndef XTAL_SERIALIZER_H_INCLUDE_GUARD
#define XTAL_SERIALIZER_H_INCLUDE_GUARD

#pragma once

namespace xtal{

class Serializer{
public:

	Serializer(const StreamPtr& s);

	~Serializer();

	void serialize(const AnyPtr& v);

	AnyPtr deserialize();

private:

	void inner_serialize(const AnyPtr& v);
	AnyPtr inner_deserialize();
	
	AnyPtr demangle(const AnyPtr& n);

	int_t register_value(const AnyPtr& v);
	int_t append_value(const AnyPtr& v);

	void put_tab(int_t tab);

	void clear();

	AnyPtr inner_deserialize_serial_new();
	AnyPtr inner_deserialize_name();
	AnyPtr inner_deserialize_string(bool intern);
	AnyPtr inner_deserialize_array();
	AnyPtr inner_deserialize_values();
	AnyPtr inner_deserialize_map();
	AnyPtr inner_deserialize_code();

private:

	enum{ 
		SERIAL_NEW,
		NAME,
		FILELOCAL,
		REF,
		
		TNULL = 16, 
		TUNDEFINED, 
		TFALSE, 
		TTRUE,
		TINT32, 
		TFLOAT32,
		TINT64, 
		TFLOAT64,
		TSTRING,
		TID,
		TARRAY, 
		TVALUES,
		TMAP 
	};

private:

	MapPtr map_;
	xarray values_;

	StreamPtr stream_;

	XTAL_DISALLOW_COPY_AND_ASSIGN(Serializer);
};

}

#endif // XTAL_SERIALIZER_H_INCLUDE_GUARD
