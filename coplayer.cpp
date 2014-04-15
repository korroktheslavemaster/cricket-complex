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

struct Player {
  string name;
  int id;
  string teamName;
  Player(string name, int id, string teamName):name(name), id(id), teamName(teamName){}
  Player():name(""), id(0), teamName(""){}
  bool operator<(const Player& p2) const {
    return id < p2.id;
  }
};
struct Graph {
  int n;
  vector<vector<double> > adjMat; // 
  map<Player, int> playerToIdx; // maps a player to its index in adjMat
  map<int, Player> idxToPlayer; // for printing...
  Graph():n(0) {}
  void addWeightToEdge(Player p1, Player p2, double wt) {
    addPlayer(p1);
    addPlayer(p2);
    int idx1 = playerToIdx[p1];
    int idx2 = playerToIdx[p2];
    adjMat[idx1][idx2] += wt;
    adjMat[idx2][idx1] += wt;
  }
private:
  void addPlayer(Player p) {
    if(p.name == "") {
      printf("weird player name\n");
      assert(0);
    }
    if(playerToIdx.find(p) == playerToIdx.end()) {
      idxToPlayer[n] = p;
      playerToIdx[p] = n++;
      adjMat.push_back(vector<double>(n-1, 0));
      for (int i = 0; i < n; ++i)
        adjMat[i].push_back(0);
    }
  }
};
struct Date {
  int dd, mm, yyyy;
  Date(int dd, int mm, int yyyy):dd(dd), mm(mm), yyyy(yyyy){}
  Date():dd(1), mm(1), yyyy(1950){}
  bool operator<(const Date &d) const {
    if(yyyy < d.yyyy)
      return true;
    else if(yyyy > d.yyyy)
      return false;
    else if(mm < d.mm)
      return true;
    else if(mm > d.mm)
      return false;
    else return dd < d.dd;
  }
};
Player getPlayerInfo(sql::Statement *stmt, int id) {
  char buf[1000];
  sprintf(buf, "select * from PLAYERS where ID = %d LIMIT 1;", id);
  sql::ResultSet *pdata = stmt->executeQuery(buf);
  while(pdata->next()) {    
    Player p;
    p.id = id;
    p.name = pdata->getString("NAME");
    p.teamName = pdata->getString("TEAM_NAME");
    return p;
  }
  assert(0);
  return Player();
}

vector<double> getCb(vector<vector<double> > &adjMat) {
  int n = adjMat.size();
  vector<double> cb(n, 0);
  for (int i = 0; i < n; ++i)
  {
    assert(adjMat[i].size() == n);
    double s = 0; // sum of weights
    int k = 0; // degree
    for (int j = 0; j < n; ++j)
    {
      s += adjMat[i][j];
      if(adjMat[i][j])
        k++;
    }
    for (int j = 0; j < n; ++j)
    {
      for (int k = 0; k < n; ++k)
      {
        if(adjMat[i][j] && adjMat[j][k] && adjMat[i][k])
          cb[i] += (adjMat[i][j] + adjMat[i][k])/2.0;
      }
    }
    // assert(s != 0 && k-1 != 0);
    if(s == 0 || k-1 == 0)
      cb[i] = 0;
    else
      cb[i] /= (s*(k-1));
  }
  return cb;
}
double getAvg(vector<double> &arr) { // not true avg, takes avg of non-zero terms only.
  int n = arr.size();
  assert(n>0);
  double avg = 0;
  int count = 0;
  for (int i = 0; i < n; ++i)
  {
    if(arr[i]) {
      avg += arr[i];
      count++;
    }
  }
  assert(count > 0);
  return avg / count;
}
double getMin(vector<double> &arr) { // not true min, takes min of non-zero terms only.
  int n = arr.size();
  assert(n>0);
  double min = 9999999; //safe bet
  for (int i = 0; i < n; ++i)
  {
    if(arr[i] < min && arr[i] != 0)
      min = arr[i];
  }
  return min;
}
double getMax(vector<double> &arr) { // not true max, takes max of non-zero terms only.
  int n = arr.size();
  assert(n>0);
  double max = 0; //safe bet
  for (int i = 0; i < n; ++i)
  {
    if(arr[i] > max && arr[i] != 0)
      max = arr[i];
  }
  return max;
}

int getDeg(vector<double> &arr) { // gets num of nonzero elements
  int n = arr.size();
  int count = 0;
  for (int i = 0; i < n; ++i)
  {
    count += !!arr[i];
  }
  return count;
}
vector<double> getKPrimes(vector<vector<double> > adjMat) {
  int n = adjMat.size();
  vector<double> kprime(n, 0);
  int count = 0;
  double avg = 0;
  for (int i = 0; i < n; ++i)
  {
    for (int j = 0; j < n; ++j)
    {
      avg += adjMat[i][j];
      if(adjMat[i][j])
        count++;
    }
  }
  if(count)
    avg = avg / count;
  else
    avg = 1;
  double min = 999999999;
  for (int i = 0; i < n; ++i)
  {
    for (int j = 0; j < n; ++j)
    {
      adjMat[i][j] /= avg;
      if(adjMat[i][j] && adjMat[i][j] < min)
        min = adjMat[i][j];
    }
  }
  if(!min) min = 1;
  for (int i = 0; i < n; ++i)
  {
    for (int j = 0; j < n; ++j)
    {
      adjMat[i][j] /= min;
      adjMat[i][j] = (int)adjMat[i][j];
    }
  }
  for (int i = 0; i < n; ++i)
  {
    double ki = 0;
    double sum = 0;
    for (int j = 0; j < n; ++j)
    {
      if(adjMat[i][j])
        ki++;
      sum += adjMat[i][j];
    }
    kprime[i] = sqrt((float)ki*sum);
  }
  return kprime;
}
void deleteNode(vector<vector<double> > &adjMat, map<int,Player> &idxToPlayer, int idx) {
  assert(idx < adjMat.size());
  int n = adjMat.size();
  for (int i = idx; i < n-1; ++i)
  {
    adjMat[i] = adjMat[i+1];
    for (int j = idx; j < n-1; ++j)
    {
      adjMat[i][j] = adjMat[i][j+1];
    }
    adjMat[i].pop_back();
  }
  adjMat.pop_back();
  for (int i = idx; i < n-1; ++i)
  {
    idxToPlayer[i] = idxToPlayer[i+1];
  }
}
vector<double> kShellDecomp(vector<vector<double> > adjMat, map<int,Player> idxToPlayer) {
  int ks = 1;
  int numRemoved = 0;
  vector<double> kprime;
  map<int, Player> idxToPlayerFinal;
  int numLeft = adjMat.size();
  while(adjMat.size()) {
    // printf("here, ajmat size = %d\n", adjMat.size());
    int n = adjMat.size();
    kprime = getKPrimes(adjMat);
    int flag = 0;
    for (int i = 0; i < n; ++i)
    {
      if(kprime[i] <= ks ) {
        // printf("%s, %g\n", idxToPlayer[i].name.c_str(), kprime[i]);
        deleteNode(adjMat, idxToPlayer, i);
        flag = 1;
        numRemoved++;
        break;
      }
    }
    if(flag) {
      kprime = getKPrimes(adjMat);
    } else {
      ks++;
      printf("ks is now %d\n", ks); 
      idxToPlayerFinal = idxToPlayer; 
      numLeft = adjMat.size();    
    }    
  }
  for (int i = 0; i < numLeft; ++i)
  {
    printf("%s\n", idxToPlayerFinal[i].name.c_str());
  }
  // for (int i = 0; i < kprime.size(); ++i)
  // {
  //   if(kprime[i] >= ks) {
  //     if(kprime[i] > ks)
  //       kprime[i] = ks+1;
  //     printf("%s\n", idxToPlayer[i].name.c_str());
  //   }
  // }
  return kprime;
}
int main(int argc, char const *argv[])
{  
  char dateStart[] = "2000-01-01";
  char dateEnd[] = "2010-12-31";
  char teamName[] = "Australia";
  char buf[1000];
  Graph graph;
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
    sprintf(buf, "select * from MATCHES where DATE_OF_PLAY between '%s' and '%s' and (TEAM_PLAYED1 = '%s' or TEAM_PLAYED2 = '%s');", dateStart, dateEnd, teamName, teamName);
    res = stmt->executeQuery(buf);
    while(res->next()) {
      sprintf(buf, "select * from PARTNERSHIPS where MATCH_ID = %d;", res->getInt("MATCH_ID"));
      sql::ResultSet *pshipData = stmt->executeQuery(buf);
      while(pshipData->next()) {
        Player p1 = getPlayerInfo(stmt, pshipData->getInt("PLAYER1"));
        Player p2 = getPlayerInfo(stmt, pshipData->getInt("PLAYER2"));
        if(p1.teamName == teamName && p2.teamName == teamName) {
          graph.addWeightToEdge(p1, p2, pshipData->getInt("RUNS"));
        }        
      }   
    }
    // for (int i = 0; i < graph.adjMat.size(); ++i)
    // {
    //   for (int j = i; j < graph.adjMat[i].size(); ++j)
    //   {
    //     if(graph.adjMat[i][j] != 0) {
    //       printf("%s, %lf, %s\n", graph.idxToPlayer[i].name.c_str(), graph.adjMat[i][j], graph.idxToPlayer[j].name.c_str());
    //     } 
    //   }
    // }


    // vector<double> cb = getCb(graph.adjMat);
    // for (int i = 0; i < graph.n; ++i)
    // {
    //   printf("%s, %lf\n", graph.idxToPlayer[i].name.c_str(), cb[i]);
    // }

    vector<double> kprimes = kShellDecomp(graph.adjMat, graph.idxToPlayer);
    // printf("numPlayers = %d, or %d. last name = %s.\n", graph.adjMat.size(), graph.n, graph.idxToPlayer[graph.n-1].name.c_str());
    // int pid;
    // while(scanf("%d\n", &pid) != EOF) {
    //   sprintf(buf, "select * from PLAYERS where ID = %d LIMIT 1", pid);
    //   res = stmt->executeQuery(buf);
    //   while (res->next()) { // performing stuff for a particular player
    //     printf("%d, %s, %s\n", res->getInt("ID"), res->getString("NAME").c_str(), res->getString("TEAM_NAME").c_str());
    //   }
    // }

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