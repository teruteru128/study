
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql/mysql.h>

#define SQL_SERV "localhost"
#define USER "user"
#define PASSWD "advanforce"
#define DB_NAME "sandbox"

/*
 * https://qiita.com/Ki4mTaria/items/778ff9186bb4958bb667
 */
int main(void)
{
    MYSQL *conn = NULL;
    MYSQL_RES *resp = NULL;
    MYSQL_ROW row;
    char sql_str[255];

    memset(sql_str, 0, sizeof(sql_str));

    // mysql接続
    conn = mysql_init(NULL);
    if (!mysql_real_connect(conn, SQL_SERV, USER, PASSWD, DB_NAME, 0, NULL, 0))
    {
        fprintf(stderr, "%s\n", mysql_error(conn));
        // error
        exit(-1);
    }

    // クエリ実行
    snprintf(sql_str, sizeof(sql_str) - 1, "select * from tb_test");
    if (mysql_query(conn, sql_str))
    {
        fprintf(stderr, "%s\n", mysql_error(conn));
        // error
        mysql_close(conn);
        exit(-1);
    }

    // レスポンス
    resp = mysql_use_result(conn);
    while ((row = mysql_fetch_row(resp)) != NULL)
    {
        printf("%d : %s\n", atoi(row[0]), row[1]);
    }

    // 後片づけ
    mysql_free_result(resp);
    mysql_close(conn);
    return 0;
}
