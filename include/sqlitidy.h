#pragma once

#include <string>
#include <array>
#include <vector>
#include <sstream>
#include <cassert>
#include <sqlite3.h>

#include "db_value.h"
#include "db_object_traits.h"

namespace sqlitidy {

#pragma warning(push)
#pragma warning(disable: 4127)

class DbContext {
public:
	DbContext(const std::string& filePath);

	~DbContext();

	template <typename ObjectT> void createTable();

	template <typename ObjectT> bool isTableExist();

	template <typename ObjectT> bool isExist(ObjectT& object);

	template <typename ObjectT> bool save(ObjectT& object);

	template <typename ObjectT> bool load(const DbValue& id, ObjectT& object);

	template <typename ObjectT> void remove(ObjectT& object);

	template <typename ObjectT> void all(std::vector<ObjectT*>& list);

	template <typename ObjectT, unsigned ParamCount> void where(
		const std::string& clause, const std::array<DbValue, ParamCount>& parameters, std::vector<ObjectT*>& list);

	template <typename ObjectT> void where(const std::string& clause, std::vector<ObjectT*>& list);

private:
	sqlite3_stmt* compile(const std::string& sql);

	int step(sqlite3_stmt* stmt);

	void bind(sqlite3_stmt* stmt, int i, const DbValue& value);

	DbValue extract(sqlite3_stmt* stmt, int i);

	template <typename ObjectT> void extractList(sqlite3_stmt* stmt, std::vector<ObjectT*>& list);

	sqlite3* db;
};


template <typename ObjectT> bool DbContext::isExist(ObjectT& object) {
	assert(db);

	std::ostringstream sql;
	sql << "select count(" << DbObjectTraits<ObjectT>::keyName << ") from " << DbObjectTraits<ObjectT>::tableName
	    << " where " << DbObjectTraits<ObjectT>::keyName << " = ?;";

	sqlite3_stmt* stmt = compile(sql.str());
	DbValue value;
	DbObjectTraits<ObjectT>::call(GetValue, &object, &DbObjectTraits<ObjectT>::keyName, &value);
	bind(stmt, 1, value);
	assert(step(stmt) == SQLITE_ROW);

	int count = ::sqlite3_column_int(stmt, 0);
	::sqlite3_finalize(stmt);
	return count > 0;
}

template <typename ObjectT> bool DbContext::save(ObjectT& object) {
	assert(db);

	bool isUpdating = isExist(object);

	std::vector<std::string> fieldNames;
	DbObjectTraits<ObjectT>::call(GetFieldNames, &fieldNames);

	std::ostringstream sql;
	sql << "replace into " << DbObjectTraits<ObjectT>::tableName << " (";
	for (unsigned i = 0; i < fieldNames.size(); i++) {
		if (isUpdating || fieldNames[i] != DbObjectTraits<ObjectT>::keyName || !DbObjectTraits<ObjectT>::pkAutoInc) {
			sql << fieldNames[i];
			if (i != fieldNames.size() - 1) {
				sql << ", ";
			}
		}
	}
	sql << ") values (";
	for (unsigned i = 0; i < fieldNames.size(); i++) {
		if (isUpdating || fieldNames[i] != DbObjectTraits<ObjectT>::keyName || !DbObjectTraits<ObjectT>::pkAutoInc) {
			sql << "?";
			if (i != fieldNames.size() - 1) {
				sql << ", ";
			}
		}
	}
	sql << ");";

	sqlite3_stmt* stmt = compile(sql.str());
	assert(stmt);

	int j = 1;
	for (unsigned i = 0; i < fieldNames.size(); i++) {
		if (isUpdating || fieldNames[i] != DbObjectTraits<ObjectT>::keyName || !DbObjectTraits<ObjectT>::pkAutoInc) {
			DbValue value;
			DbObjectTraits<ObjectT>::call(GetValue, &object, &fieldNames[i], &value);
			bind(stmt, j, value);
			j++;
		}
	}

	assert(step(stmt) == SQLITE_DONE);
	::sqlite3_finalize(stmt);

	DbValue key;
	DbObjectTraits<ObjectT>::call(GetValue, &object, &DbObjectTraits<ObjectT>::keyName, &key);
	if (!isUpdating && key.type == SQLITE_INTEGER && DbObjectTraits<ObjectT>::pkAutoInc) {
		DbValue lastId = static_cast<int>(::sqlite3_last_insert_rowid(db));
		DbObjectTraits<ObjectT>::call(SetValue, &object, &DbObjectTraits<ObjectT>::keyName, &lastId);
	}

	return isUpdating;
}

template <typename ObjectT> bool DbContext::load(const DbValue& id, ObjectT& object) {
	assert(db);

	std::ostringstream sql;
	sql << "select * from " << DbObjectTraits<ObjectT>::tableName
	    << " where " << DbObjectTraits<ObjectT>::keyName << " = ?;";

	sqlite3_stmt* stmt = compile(sql.str());
	bind(stmt, 1, id);

	std::vector<std::string> fieldNames;
	DbObjectTraits<ObjectT>::call(GetFieldNames, &fieldNames);

	bool isExist = step(stmt) == SQLITE_ROW;
	if (isExist) {
		for (unsigned i = 0; i < fieldNames.size(); i++) {
			DbValue value = extract(stmt, i);
			DbObjectTraits<ObjectT>::call(SetValue, &object, &fieldNames[i], &value);
		}
	}

	::sqlite3_finalize(stmt);
	return isExist;
}

template <typename ObjectT> void DbContext::remove(ObjectT& object) {
	assert(db);

	std::ostringstream sql;
	sql << "delete from " << DbObjectTraits<ObjectT>::tableName
	    << " where " << DbObjectTraits<ObjectT>::keyName << " = ?;";

	sqlite3_stmt* stmt = compile(sql.str());
	DbValue value;
	DbObjectTraits<ObjectT>::call(GetValue, &object, &DbObjectTraits<ObjectT>::keyName, &value);
	bind(stmt, 1, value);

	assert(step(stmt) == SQLITE_DONE);
	::sqlite3_finalize(stmt);
}

template <typename ObjectT> void DbContext::extractList(sqlite3_stmt* stmt, std::vector<ObjectT*>& list) {
	while (step(stmt) == SQLITE_ROW) {
		std::vector<std::string> fieldNames;
	DbObjectTraits<ObjectT>::call(GetFieldNames, &fieldNames);

		ObjectT* object = new ObjectT();
		for (unsigned i = 0; i < fieldNames.size(); i++) {
			DbValue value = extract(stmt, i);
			DbObjectTraits<ObjectT>::call(SetValue, object, &fieldNames[i], &value);
		}
		list.push_back(object);
	}
}

template <typename ObjectT, unsigned ParamCount> void DbContext::where(
		const std::string& clause, const std::array<DbValue, ParamCount>& parameters, std::vector<ObjectT*>& list) {
	assert(db);

	std::ostringstream sql;
	sql << "select * from " << DbObjectTraits<ObjectT>::tableName
	    << " where " << clause << ";";

	sqlite3_stmt* stmt = compile(sql.str());
	for (unsigned i = 0; i < parameters.size(); i++) {
		bind(stmt, i + 1, parameters[i]);
	}
	extractList(stmt, list);

	::sqlite3_finalize(stmt);
}

template <typename ObjectT> void DbContext::where(const std::string& clause, std::vector<ObjectT*>& list) {
	where(clause, std::array<DbValue, 0>(), list);
}

template <typename ObjectT> void DbContext::all(std::vector<ObjectT*>& list) {
	assert(db);

	std::ostringstream sql;
	sql << "select * from " << DbObjectTraits<ObjectT>::tableName << ";";

	sqlite3_stmt* stmt = compile(sql.str());
	extractList(stmt, list);

	::sqlite3_finalize(stmt);
}

template <typename ObjectT> void DbContext::createTable() {
	assert(db);

	std::ostringstream sql;
	sql << "create table " << DbObjectTraits<ObjectT>::tableName << " (";

	std::vector<std::string> fieldNames;
	DbObjectTraits<ObjectT>::call(GetFieldNames, &fieldNames);

	ObjectT placeholder;
	for (unsigned i = 0; i < fieldNames.size(); i++) {
		sql << fieldNames[i];
		DbValue value;
		DbObjectTraits<ObjectT>::call(GetValue, &placeholder, &fieldNames[i], &value);
		switch (value.type) {
		case SQLITE_INTEGER: sql << " integer"; break;
		case SQLITE_FLOAT: sql << " float"; break;
		case SQLITE_TEXT: sql << " text"; break;
		default:
			assert(false && "column type not supported");
		}
		if (fieldNames[i] == DbObjectTraits<ObjectT>::keyName) {
			sql << " primary key";
			if (DbObjectTraits<ObjectT>::pkAutoInc && value.type == SQLITE_INTEGER) {
				sql << " autoincrement";
			}
		}
		sql << " not null";
		if (i != fieldNames.size() - 1) {
			sql << ", ";
		}
	}

	sql << ");";

	sqlite3_stmt* stmt = compile(sql.str());
	assert(step(stmt) == SQLITE_DONE);

	::sqlite3_finalize(stmt);
}

template <typename ObjectT> bool DbContext::isTableExist() {
	assert(db);

	std::string sql = "select count(*) from sqlite_master where type = 'table' and name = ?;";

	sqlite3_stmt* stmt = compile(sql);
	bind(stmt, 1, DbObjectTraits<ObjectT>::tableName);
	assert(step(stmt) == SQLITE_ROW);

	int count = ::sqlite3_column_int(stmt, 0);

	::sqlite3_finalize(stmt);
	return count == 1;
}

#pragma warning(pop)

} // namespace sqlitidy
