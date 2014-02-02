// test.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "sqlitidy.h"

using namespace std;
using namespace sqlitidy;

struct User {
	int id;
	string name;
	int age;

	static const string tableName;

	static const string keyName;

	static const bool pkAutoInc = true;

	static const array<string, 3> fieldNames;

	SQLITIDY_VALUE_GETTER_BEGIN
	SQLITIDY_VALUE_GETTER(id)
	SQLITIDY_VALUE_GETTER(name)
	SQLITIDY_VALUE_GETTER(age)
	SQLITIDY_VALUE_GETTER_END

	SQLITIDY_VALUE_SETTER_BEGIN
	SQLITIDY_INT_VALUE_SETTER(id)
	SQLITIDY_STRING_VALUE_SETTER(name)
	SQLITIDY_INT_VALUE_SETTER(age)
	SQLITIDY_VALUE_SETTER_END

	User() : id(-1), age(-1) { }

private:
	User(const User&);
	User& operator = (const User&);
};

const string User::tableName = "tb_user";

const string User::keyName = "id";

const array<string, 3> User::fieldNames = { "id", "name", "age" };

template <typename Container> void deleteContainer(const Container& container) {
	for (auto i = container.begin(); i != container.end(); i++) {
		delete *i;
	}
}

int _tmain(int /*argc*/, _TCHAR* /*argv*/[]) {
	DbContext ctx("test.db");
	if (!ctx.isTableExist<User>()) {
		ctx.createTable<User>();
	}
	//User u;
	//cin >> u.id >> u.name >> u.age;
	//ctx.save(u);
	//cout << u.id << u.name << u.age << endl;
	//User u2;
	//ctx.load(u.id, u2);
	//cout << u2.id << u2.name << u2.age << endl;
	//array<DbValue, 1> p1 = { 14 };
	vector<User*> ul;
	//ctx.where("id > ?", p1, ul);
	//ctx.where("id > 10", ul);
	ctx.all(ul);
	for_each(ul.begin(), ul.end(), [] (User* u) {
		cout << u->id << u->name << u->age << endl;
	});
	deleteContainer(ul);
	return 0;
}
