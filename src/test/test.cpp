// test.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <sqlitidy/sqlitidy.h>

using namespace std;
using namespace sqlitidy;

struct User {
	int id;
	string name;
	int age;
	double height;

	User() : id(-1), age(-1), height(0) { }

private:
	User(const User&);
	User& operator = (const User&);
};

SQLITIDY_MAP_BEGIN(User, "tb_user", "id", true)
	SQLITIDY_MAP_FIELD(id)
	SQLITIDY_MAP_FIELD(name)
	SQLITIDY_MAP_FIELD(age)
	SQLITIDY_MAP_FIELD(height)
SQLITIDY_MAP_END

template <typename Container> void deleteContainer(const Container& container) {
	for (auto i = container.begin(); i != container.end(); i++) {
		delete *i;
	}
}

int _tmain(int /*argc*/, _TCHAR* /*argv*/[]) {
	DbObjectTraits<User>::tableName = "tb_user_1";

	DbContext ctx("test.db");
	if (!ctx.isTableExist<User>()) {
		ctx.createTable<User>();
	}
	User u;
	cin >> u.id >> u.name >> u.age >> u.height;
	ctx.save(u);
	cout << u.id << u.name << u.age << u.height << endl;
	User u2;
	ctx.load(u.id, u2);
	cout << u2.id << u2.name << u2.age << u.height << endl;
	array<DbValue, 1> p1 = { 1.6 };
	vector<User*> ul;
	//ctx.where("id > ?", p1, ul);
	//ctx.where("id > 10", ul);
	//ctx.all(ul);
	ctx.where("height > ?", p1, ul);
	for_each(ul.begin(), ul.end(), [] (User* u) {
		cout << u->id << u->name << u->age << u->height << endl;
	});
	ctx.remove(u);
	deleteContainer(ul);
	return 0;
}

