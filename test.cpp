/* Copyright 2008, 2010, Oracle and/or its affiliates. All rights reserved.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; version 2 of the License.

There are special exceptions to the terms and conditions of the GPL
as it is applied to this software. View the full text of the
exception in file EXCEPTIONS-CONNECTOR-C++ in the directory of this
software distribution.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*/

/* Standard C++ includes */
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
static char colors[][100] = {"navy", "blue", "skyblue", "purple", "green", "yellow", "orange", "red", "maroon" };
// static char colors[][100] = {"violet", "blue", "pink", "skyblue", "green", "yellow", "orange", "red", "white" };
int nColors = sizeof(colors)/sizeof(char[100]);
using namespace std;
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

bool isShitTeam(char teamName[]) {
  if(strcmp(teamName, "Ireland") == 0 || strcmp(teamName, "Scotland") == 0)
    strcpy(teamName, "England");
  char shitList[][100] = {"United States of America", "Asia XI", "Africa XI", "ICC World XI"};
  int nShitList = sizeof(shitList)/sizeof(char[100]);
  for (int i = 0; i < nShitList; ++i)
  {
    if(!strcmp(teamName, shitList[i]))
      return true;
  }
  return false;
}
struct Player {
  int ID;
  char name[50], fullName[50];
  int runs;
  char teamName[50];
  int teamID;
};
struct PlayerScore{
  int ID;
  int score;
  char team[50];
};
struct Match {
  int matchID;
  Date date;
  char team0[50], team1[50];
  int score0, score1;
  bool whoWon;
  bool isDraw;
  vector<Player> players;
  bool operator<(const Match &m) const {
    return matchID < m.matchID;
  }
};

struct PPLink {
  int pid1, pid2;
  double w;
  PPLink():pid1(-1), pid2(-1), w(0){}
  PPLink(int pid1, int pid2, double w):pid1(pid1), pid2(pid2), w(w){if(pid2<pid1)swap(pid2, pid1);}
};
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
vector<double> getKPrimes(vector<vector<double> > &adjMat, vector<double> &oldKPrime) {
  int n = adjMat.size();
  vector<double> kprime(n, 0);
  for (int i = 0; i < n; ++i)
  {
    vector<double> copyArr(n, 0);
    double sum = 0;
    for (int j = 0; j < n; ++j)
    {
      copyArr[j] = adjMat[i][j];
      sum += copyArr[j];
    }
    if(sum == 0) { //deleted node
      kprime[i]= oldKPrime[i];
      continue;
    }
    // double avg = getAvg(copyArr);
    // assert(avg != 0);
    // for (int j = 0; j < n; ++j)
    // {
    //   copyArr[j] /= avg;
    // }
    double min = getMin(copyArr);
    double max = getMax(copyArr);
    int deg = getDeg(copyArr);
    assert(min != 0);
    for (int j = 0; j < n; ++j)
    {
      copyArr[j] = (int)(copyArr[j]);
    }
    for (int j = 0; j < n; ++j)
    {
      kprime[i] += copyArr[j];
    }
    // printf("deg/21 = %d, summation = %lf. min, max = %lf, %lf\n", deg/21, kprime[i], min, max);
    kprime[i] = sqrt(kprime[i]*(double)deg/21.0);
  }
  return kprime;
}
vector<double> kShellDecomp(vector<vector<double> > &adjMat, vector<int> &pidArr) {
  int ks = 1;
  int n = adjMat.size();
  vector<double> kprime(n, 1);
  vector<bool> isDeleted(n, 0);
  int numRemoved = 0;
  while(numRemoved < n - 30) {
    kprime = getKPrimes(adjMat, kprime);
    int flag = 0;
    for (int i = 0; i < n; ++i)
    {
      if(kprime[i] <= ks && !isDeleted[i]) {
        numRemoved++;
        isDeleted[i] = 1;
        flag = 1;
        for (int j = 0; j < n; ++j)
        {
          adjMat[j][i] = adjMat[i][j] = 0;
        }
        kprime[i] = ks;
        break;
      }
    }
    if(flag) {
      kprime = getKPrimes(adjMat, kprime);
    } else {
      ks++;
      printf("ks is now %d\n", ks);      
    }    
  }
  for (int i = 0; i < n; ++i)
  {
    if(kprime[i] >= ks) {
      if(kprime[i] > ks)
        kprime[i] = ks+1;
      printf("%d\n", pidArr[i]);
    }
  }
  return kprime;
}
int main(void)
{
  vector<PPLink> edgeList;
  set<int> pidList; // set of unique pid
  char buf[1000];
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
    res = stmt->executeQuery("select * from MATCHES where DATE_OF_PLAY between '2005-01-01' and '2010-12-31'");
    while (res->next()) { // performing stuff for a particular match
      int matchID = res->getInt("MATCH_ID");
      char teamName1[50], teamName2[50];
      int teamScore1 = res->getInt("TEAM1_SCORE"), teamScore2 = res->getInt("TEAM2_SCORE");
      strcpy(teamName1, res->getString("TEAM_PLAYED1").c_str());
      strcpy(teamName2, res->getString("TEAM_PLAYED2").c_str());
      if(teamScore1 == 0 || teamScore2 == 0 || isShitTeam(teamName1) || isShitTeam(teamName2))
        continue;
      sprintf(buf, "select * from PERFORMANCE_BATTING where MATCH_ID = %d", matchID);
      sql::ResultSet *playerData = stmt->executeQuery(buf); //should have 22 results?
      vector<PlayerScore> pScores;
      while(playerData->next()) {
        PlayerScore p;
        p.ID = playerData->getInt("PLAYER_ID");
        p.score = playerData->getInt("RUNS");
        sprintf(buf, "select TEAM_NAME from PLAYERS where ID = %d LIMIT 1", p.ID);
        sql::ResultSet *pTeam = stmt->executeQuery(buf);
        while(pTeam->next())
          strcpy(p.team, pTeam->getString(1).c_str()); //should happen only once
        if(strcmp(p.team, "East and Central Africa") == 0) //wtf country
          strcpy(p.team, "East Africa");
        if(strcmp(p.team, "Ireland") == 0 || strcmp("Scotland", p.team) == 0)
          strcpy(p.team, "England");
        pScores.push_back(p);
        pidList.insert(p.ID);
      }
      // assert(pScores.size() == 22); 
      for (int i = 0; i < pScores.size(); ++i)
      {
        for (int j = i+1; j < pScores.size(); ++j)
        {
          double w = pScores[i].score+pScores[j].score;
          int sum = 0;
          if(strcmp(pScores[i].team, teamName1) == 0)
            sum += teamScore1;
          else if(strcmp(pScores[i].team, teamName2) == 0)
            sum += teamScore2;
          else {
            printf("pid %d, team '%s', candidates: '%s', '%s'\n", pScores[i].ID, pScores[i].team, teamName1, teamName2);
            sum += teamScore1;
          }
          if(strcmp(pScores[j].team, teamName1) == 0)
            sum += teamScore1;
          else if(strcmp(pScores[j].team, teamName2) == 0)
            sum += teamScore2;
          else {
            printf("pid %d, team '%s', candidates: '%s', '%s'\n", pScores[j].ID, pScores[j].team, teamName1, teamName2);
            sum += teamScore1;
          }
          if(!sum) {
            printf("sum = 0 ... matchID = \n");
            sum = 1;
          }
          w /= sum;
          edgeList.push_back(PPLink(pScores[i].ID, pScores[j].ID, w));
        }
      }
    }
    vector<int> pidArr; // now making it an arr for convenience;
    std::map<int, int> pidToIdx;
    for(std::set<int>::iterator it = pidList.begin(); it != pidList.end(); it++) {
      pidArr.push_back(*it);
      pidToIdx[*it] = pidArr.size()-1;
    }
    vector<vector<double> > adjMat(pidArr.size(), vector<double>(pidArr.size(), 0));
    for (int i = 0; i < edgeList.size(); ++i)
    {
      adjMat[pidToIdx[edgeList[i].pid1]][pidToIdx[edgeList[i].pid2]] += edgeList[i].w;
      adjMat[pidToIdx[edgeList[i].pid2]][pidToIdx[edgeList[i].pid1]] += edgeList[i].w;
    }
    printf("num nodes = %d\n", pidArr.size());
    vector<double> kprime = kShellDecomp(adjMat, pidArr);
    // for (int i = 0; i < kprime.size(); ++i)
    // {
    //   printf("%d %lf\n", pidArr[i], kprime[i]);
    // }
    // printf("graph G {\n");
    // for(int i = 0; i < pidArr.size(); i++) {
    //   printf("{node [color=%s, style=filled] %d}\n", colors[(int)(kprime[i])-1], pidArr[i]);
    // }
    // for (int i = 0; i < pidArr.size(); ++i)
    // {
    //   for (int j = i+1; j < pidArr.size(); ++j)
    //   {
    //     if(adjMat[i][j] == 0)
    //       continue;
    //     printf("%d -- %d /*%lf*/\n", pidArr[i], pidArr[j], adjMat[i][j]);
    //   }
    // }
    // printf("}\n");
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