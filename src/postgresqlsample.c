#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <postgresql/libpq-fe.h>

/**
 * @brief https://chapati.wiki.fc2.com/wiki/C%E8%A8%80%E8%AA%9E%2FPostgreSql%2FUbuntu
 * 
 * @param argc 
 * @param argv 
 * @return int 
 */
int main(int argc, char const *argv[])
{
    /* 変数定義 */
    char dbName[255] = "chapatidb"; /* データベース名はハードコーディング */
    char sql[255];
    int i;
    PGconn *con;
    PGresult *res;
    char *kou1,*kou2,*kou3;

    /* DBとの接続 */
    con = PQsetdb("","",NULL,NULL,dbName);
    if ( PQstatus(con) == CONNECTION_BAD ) { /* 接続が失敗したときのエラー処理 */
        fprintf(stderr,"Connection to database '%s' failed.\n",dbName);
        fprintf(stderr,"%s",PQerrorMessage(con));
        exit(1);
    }

    /* select文の発行 */
    sprintf(sql,"select * from users");
    res = PQexec(con,sql);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) { /* SQLの実行に失敗したときのエラー処理 */
        fprintf(stderr,"%s",PQerrorMessage(con));
        exit(1);
    }

    printf("id name email\n");
    printf("--------------------------------------\n");
    for(i = 0; i < 3 ;i++) {
        kou1 = PQgetvalue(res,i,0);
        kou2 = PQgetvalue(res,i,1);
        kou3 = PQgetvalue(res,i,2);
        printf("%s %s %s\n",kou1,kou2,kou3);
    }
    PQclear(res);
    return 0;
}
