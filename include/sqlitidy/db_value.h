#pragma once

#include <string>

namespace sqlitidy {

struct DbValue {
	int type;
	union {
		void* nullValue;
		int intValue;
		double doubleValue;
		char* stringValue;
	};

	DbValue(const DbValue& value) { assign(value); }

	DbValue& operator = (const DbValue& value) { assign(value); return *this; }

	~DbValue() { if (type == SQLITE_TEXT) delete[] stringValue; }

	DbValue() : type(SQLITE_NULL), nullValue(nullptr) { }

	DbValue(int intValue) : type(SQLITE_INTEGER), intValue(intValue) { }

	DbValue(double doubleValue) : type(SQLITE_FLOAT), doubleValue(doubleValue) { }

	DbValue(const std::string& stringValue);

	operator int() const { assert(type == SQLITE_INTEGER); return this->intValue; }

	operator double() const { assert(type == SQLITE_FLOAT); return this->doubleValue; }

	operator std::string() const { assert(type == SQLITE_TEXT); return this->stringValue; }

private:
	void assign(const DbValue& value);
};

std::ostream& operator << (std::ostream& os, const DbValue& value);

} // namespace sqlitidy
