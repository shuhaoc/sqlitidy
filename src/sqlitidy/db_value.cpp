#include "stdafx.h"
#include <cassert>
#include <sqlite3.h>
#include "sqlitidy/db_value.h"

namespace sqlitidy {

DbValue::DbValue(const std::string& stringValue) : type(SQLITE_TEXT) {
	unsigned len = stringValue.size() + 1;
	this->stringValue = new char[len];
	strcpy(this->stringValue, stringValue.c_str());
}

void DbValue::assign(const DbValue& value) {
	assert(type == value.type || type == SQLITE_NULL);
	if (this->type == SQLITE_NULL) {
		this->type = value.type;
	}
	switch (type) {
	case SQLITE_INTEGER:
		intValue = value.intValue;
		break;
	case SQLITE_FLOAT:
		doubleValue = value.doubleValue;
		break;
	case SQLITE_TEXT: {
		delete[] stringValue;
		unsigned len = strlen(value.stringValue) + 1;
		stringValue = new char[len];
		strcpy(stringValue, value.stringValue);
	}
	break;
	default:
		assert(false && "Invalid value type");
	}
}

std::ostream& operator << (std::ostream& os, const DbValue& value) {
	switch (value.type) {
	case SQLITE_INTEGER:
		os << value.intValue;
		break;
	case SQLITE_FLOAT:
		os << value.doubleValue;
		break;
	case SQLITE_TEXT:
		os << "\"" << value.stringValue << "\"";
		break;
	default:
		assert(false && "Invalid value type");
	}
	return os;
}

} // namespace sqlitidy
