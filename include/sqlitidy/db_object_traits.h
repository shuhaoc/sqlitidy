#pragma once

#include <string>
#include <vector>
#include <sstream>

#include "db_value.h"

namespace sqlitidy {

template <typename ObjectT> class DbObjectTraits {
public:	
	static std::string tableName;

	static std::string keyName;

	static bool pkAutoInc;

	static void call(int function, ...);

	static std::string toString(const ObjectT& object);
};

enum { GetFieldNames, GetValue, SetValue };


template <typename ObjectT> std::string DbObjectTraits<ObjectT>::toString(const ObjectT& object) {
	std::ostringstream oss;
	std::vector<std::string> fieldNames;
	DbObjectTraits<ObjectT>::call(GetFieldNames, &fieldNames);
	for (unsigned i = 0; i < fieldNames.size(); i++) {
		DbValue value;
		DbObjectTraits<ObjectT>::call(GetValue, &object, &fieldNames[i], &value);
		oss << fieldNames[i] << " = " << value;
		if (i != fieldNames.size() - 1) {
			oss << ", ";
		}
	}
	return oss.str();
}

} // namespace sqlitidy


#define SQLITIDY_MAP_BEGIN(class_name, table_name, key_name, pk_auto_inc) \
	std::string DbObjectTraits<class_name>::tableName = table_name; \
	std::string DbObjectTraits<class_name>::keyName = key_name; \
	bool DbObjectTraits<class_name>::pkAutoInc = pk_auto_inc; \
	void DbObjectTraits<class_name>::call(int function, ...) { \
		va_list list; \
		va_start(list, function); \
		std::vector<std::string>* fieldNames = nullptr; \
		class_name* object = nullptr; \
		std::string* name = nullptr; \
		sqlitidy::DbValue* value = nullptr; \
		if (function == sqlitidy::GetFieldNames) { \
			fieldNames = va_arg(list, std::vector<std::string>*); \
		} else if (function == sqlitidy::GetValue || function == sqlitidy::SetValue) { \
			object = va_arg(list, class_name*); \
			name = va_arg(list, std::string*); \
			value = va_arg(list, sqlitidy::DbValue*); \
		}

#define SQLITIDY_MAP_FIELD(field_name) \
	if (function == sqlitidy::GetFieldNames) { \
		fieldNames->push_back(#field_name); \
	} else if (function == sqlitidy::GetValue) { \
		if (*name == #field_name) { *value = object->field_name; } \
	} else if (function == sqlitidy::SetValue) { \
		if (*name == #field_name) { object->field_name = *value; } \
	}

#define SQLITIDY_MAP_END va_end(list); \
}