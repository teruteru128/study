
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql/mysql.h>

#define USER "user"
#define PASSWD "advanforce"
#define DB_NAME "sandbox"
#define UNIX_SOCKET_PATH "/var/run/mysqld/mysqld.sock"

/*
 * https://qiita.com/Ki4mTaria/items/778ff9186bb4958bb667
 */
int main(void)
{
    MYSQL *conn = NULL;
    MYSQL_RES *resp = NULL;
    MYSQL_ROW row;
    char sql_str[255] = "";

    // mysql接続
    conn = mysql_init(NULL);
    if (!mysql_real_connect(conn, NULL, USER, PASSWD, DB_NAME, 0, UNIX_SOCKET_PATH, 0))
    {
        fprintf(stderr, "%s\n", mysql_error(conn));
        // error
        exit(-1);
    }

    // クエリ実行
    snprintf(sql_str, 255, "select * from tb_test");
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
