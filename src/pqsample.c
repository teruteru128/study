
#define _FORTIFY_SOURCE 3
#define _GNU_SOURCE 1
#include <postgresql/libpq-fe.h>
#include <stdio.h>

int main(int argc, char *argv[], char *envp[])
{
    char *coninfo;
    PGconn *con;
    PGresult *result;
    int i;
    coninfo = "host=localhost port=5432 dbname=postgres user=postgres password=password";
    con = PQconnectdb(coninfo);
    if(PQstatus(con) != CONNECTION_OK)
    {
        fprintf(stderr, "%s", PQerrorMessage(con));
        return 1;
    }

    result = PQexec(con, "SELECT datname from pg_database;");
    if(PQresultStatus(result) != PGRES_TUPLES_OK)
    {
        fprintf(stderr, "%s", PQerrorMessage(con));
        return 1;
    }
    for(i = 0; i < PQntuples(result); i++)
    {
        printf("%s\n", PQgetvalue(result, i, 0));
    }
    PQclear(result);
    PQfinish(con);
    return 0;
}
