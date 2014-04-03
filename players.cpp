#include <stdlib.h>
#include <iostream>
#include <vector>
#include <set>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <map>
using namespace std;
/*
  Include directly the different
  headers from cppconn/ and mysql_driver.h + mysql_util.h
  (and mysql_connection.h). This will reduce your build time!
*/
#include "mysql_connection.h"

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>

int main(int argc, char const *argv[])
{  
  char buf[100];
  try {
    sql::Driver *driver;
    sql::Connection *con;
    sql::Statement *stmt;
    sql::ResultSet *res;
    sql::ResultSetMetaData *resMD;
    /* Create a connection */
    driver = get_driver_instance();
    con = driver->connect("tcp://127.0.0.1:3306", "root", "pavilion");
    /* Connect to the MySQL test database */
    con->setSchema("cnt");

    stmt = con->createStatement();
    int pid;
    while(scanf("%d\n", &pid) != EOF) {
      sprintf(buf, "select * from PLAYERS where ID = %d LIMIT 1", pid);
      res = stmt->executeQuery(buf);
      while (res->next()) { // performing stuff for a particular player
        printf("%d, %s, %s\n", res->getInt("ID"), res->getString("NAME").c_str(), res->getString("TEAM_NAME").c_str());
      }
    }
    delete res;
    delete stmt;
    delete con;

  } catch (sql::SQLException &e) {
    cout << "# ERR: SQLException in " << __FILE__;
    // cout << "(" << __FUNCTION__ << ") on line " »
    //    << __LINE__ << endl;
    cout << "# ERR: " << e.what();
    cout << " (MySQL error code: " << e.getErrorCode();
    cout << ", SQLState: " << e.getSQLState() << " )" << endl;
  }

  cout << endl;

  return EXIT_SUCCESS;
}