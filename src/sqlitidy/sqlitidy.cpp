#include "stdafx.h"
#include "sqlitidy.h"

namespace sqlitidy {

DbValue::DbValue(const std::string& stringValue) : type(SQLITE_TEXT) {
	unsigned len = stringValue.size() + 1;
	this->stringValue = new char[len];
	strcpy(this->stringValue, stringValue.c_str());
}

void DbValue::assign(const DbValue& value) {
	assert(type == value.type);
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

DbContext::DbContext(const std::string& filePath) {
	int ec = ::sqlite3_open(filePath.c_str(), &db);
	assert(ec == SQLITE_OK);
}

DbContext::~DbContext() {
	::sqlite3_close(db);
}

void DbContext::bind(sqlite3_stmt* stmt, int i, const DbValue& value) {
	int ec = SQLITE_OK;
	switch (value.type) {
	case SQLITE_INTEGER:
		ec = ::sqlite3_bind_int(stmt, i, value.intValue);
		break;
	case SQLITE_FLOAT:
		ec = ::sqlite3_bind_double(stmt, i, value.doubleValue);
		break;
	case SQLITE_TEXT:
		ec = ::sqlite3_bind_text(stmt, i, value.stringValue, strlen(value.stringValue), SQLITE_TRANSIENT);
		break;
	default:
		assert(false && "Invalid value type");
	}
	assert(ec == SQLITE_OK);
}

sqlite3_stmt* DbContext::compile(const std::string& sql) {
	sqlite3_stmt* stmt = nullptr;
	const char* tail = nullptr;
	int ec = ::sqlite3_prepare(db, sql.c_str(), sql.size(), &stmt, &tail);
	assert(ec == SQLITE_OK);
	return stmt;
}

int DbContext::step(sqlite3_stmt* stmt) {
	assert(stmt);
	int ec = ::sqlite3_step(stmt);
	assert(ec == SQLITE_DONE || ec == SQLITE_ROW);
	return ec;
}

DbValue DbContext::extract(sqlite3_stmt* stmt, int i) {
	assert(stmt);
	int type = ::sqlite3_column_type(stmt, i);
	switch (type) {
	case SQLITE_INTEGER:
		return ::sqlite3_column_int(stmt, i);
	case SQLITE_FLOAT:
		return ::sqlite3_column_double(stmt, i);
	case SQLITE_TEXT:
		return reinterpret_cast<const char*>(::sqlite3_column_text(stmt, i));
	default:
		assert(false && "Invalid value type");
		return DbValue();
	}
}

} // namespace sqlitidy
