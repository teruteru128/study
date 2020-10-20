#include <config.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define WIDTH 78
#define HEIGHT 23
#define MAX_FLY 1
#define DRAW_SYCLE 50

int stopRequest;

/*
 * ミリ秒単位でスリープする
 */
void mSleep(int msec)
{
    struct timespec ts;
    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;
    nanosleep(&ts, NULL);
}

/*
 * minValue以上maxValue未満のランダム値を得る
 */
double randDouble(double minValue, double maxValue)
{
    return minValue + (double)rand() / ((double)RAND_MAX + 1) * (maxValue - minValue);
}

/*
 * 画面クリア
 */
void clearScreen()
{
    fputs("\033[2J", stdout); /* このエスケープコードをターミナルに送ると画面がクリアされる */
}

/*
 * カーソル移動
 */
void moveCursor(int x, int y)
{
    printf("\033[%d;%dH", y, x); /* このエスケープコードをターミナルに送るとカーソル1がx,yになる。 */
}

/*
 * カーソル位置保存
 */
void saveCursor()
{
    printf("\0337"); /* このエスケープコードをターミナルに送るとカーソル位置を記憶する */
}

/*
 * カーソル位置保存
 */
void restoreCursor()
{
    printf("\0338"); /* このエスケープコードをターミナルに送ると記憶したカーソル位置に戻る */
}

/* ハエ構造体 */
typedef struct
{
    char mark;    /* 表示キャラクタ */
    double x, y;  /* 座標 */
    double angle; /* 移動方向 (角度) */
    double speed; /* 移動速度 (ピクセル/秒) */
    double destX, destY; /* 目標地点 */
    int busy; /* 移動中フラグ */
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} Fly;

Fly flyList[MAX_FLY];

/* ハエの状態をランダムに初期化 */
void FlyInitCenter(Fly *fly, char mark_)
{
    fly->mark = mark_;
    fly->x = (double)WIDTH / 2.0;
    fly->y = (double)HEIGHT / 2.0;
    fly->angle = 0;
    fly->speed = 2;
    fly->destX = fly->x;
    fly->destY = fly->y;
    fly->busy = 0;
    pthread_mutex_init(&fly->mutex, NULL);
    pthread_cond_init(&fly->cond, NULL);
}

void FlyDestroy(Fly *fly)
{
    pthread_mutex_destroy(&fly->mutex);
    pthread_cond_destroy(&fly->cond);
}

/* ハエを移動する */
void FlyMove(Fly *fly)
{
    int i;
    pthread_mutex_lock(&fly->mutex);
    fly->x += cos(fly->angle);
    fly->y += sin(fly->angle);
    pthread_mutex_unlock(&fly->mutex);
}

/*
 * ハエが指定座標にあるかどうか
 */
int FlyIsAt(Fly *fly, int x, int y)
{
    int res;
    pthread_mutex_lock(&fly->mutex);
    res = ((int)(fly->x) == x) && ((int)(fly->y) == y);
    pthread_mutex_unlock(&fly->mutex);
    return res;
}

/*
 * 目標地点に合わせて移動方向と速度を調整する
 */
void FlySetDirection(Fly *fly)
{
    pthread_mutex_lock(&fly->mutex);
    double dx = fly->destX-fly->x;
    double dy = fly->destY-fly->y;
    fly->angle = atan2(dy, dx);
    fly->speed = hypot(dx, dy) / 5.0;
    if(fly->speed < 2)
        fly->speed = 2;
    pthread_mutex_unlock(&fly->mutex);
}

/*
 * 目標地点までの距離を得る
 */
double FlyDistanceToDestination(Fly *fly)
{
    double dx, dy;
    pthread_mutex_lock(&fly->mutex);
    dx = fly->destX-fly->x;
    dy = fly->destY-fly->y;
    pthread_mutex_unlock(&fly->mutex);
    return hypot(dx, dy);
}

/*
 * 目標地点がセットされるまで待つ
 */
void FlyWaitForSetDestination(Fly *fly)
{
    pthread_mutex_lock(&fly->mutex);
    if(pthread_cond_wait(&fly->cond, &fly->mutex) != 0)
    {
        printf("Fatai error on pthread_cond_wait.\n");
        exit(1);
    }
    pthread_mutex_unlock(&fly->mutex);
}

/*
 * ハエの目標地点をセットする
 */
int FlySetDestination(Fly *fly, double x, double y)
{
    if(fly->busy)
        return 0;
    pthread_mutex_lock(&fly->mutex);
    fly->destX = x;
    fly->destY = y;
    pthread_cond_signal(&fly->cond);
    pthread_mutex_unlock(&fly->mutex);
    return 1;
}

void *doMove(void *arg)
{
    Fly *fly = (Fly *)arg;
    while (!stopRequest)
    {
        fly->busy = 0;
        FlyWaitForSetDestination(fly);
        while((FlyDistanceToDestination(fly) < 1))
            continue;
        fly->busy = 1;
        FlySetDirection(fly);
        while((FlyDistanceToDestination(fly) >= 1) && !stopRequest)
        {
            FlyMove(fly);
            mSleep((int)(1000.0 / fly->speed));
        }
    }
    return NULL;
}

/*
 * スクリーンを描画する
 */
void drawSecrren()
{
    int x, y;
    char ch;
    int i;

    moveCursor(0, 0);
    for (y = 0; y < HEIGHT; y++)
    {
        for (x = 0; x < WIDTH; x++)
        {
            ch = 0;
            for (i = 0; i < MAX_FLY; i++)
            {
                if (FlyIsAt(&flyList[i], x, y))
                {
                    ch = flyList[i].mark;
                    break;
                }
            }
            if (ch != 0)
            {
                putchar(ch);
            }
            else if ((y == 0) || (y == HEIGHT - 1))
            {
                putchar('-');
            }
            else if ((x == 0) || (x == WIDTH - 1))
            {
                putchar('|');
            }
            else
            {
                putchar(' ');
            }
        }
        putchar('\n');
    }
}

/*
 * スクリーンを描画する
 */
void *doDraw(void *arg)
{
    while (!stopRequest)
    {
        drawSecrren();
        mSleep(DRAW_SYCLE);
    }
    return NULL;
}

/*  */
int main(int argc, char *argv[])
{
    pthread_t drawThread;
    pthread_t moveThread;
    int i;
    char buf[40] , *cp;
    double destX, destY;

    clearScreen();
    FlyInitCenter(&flyList[0], '@');

    pthread_create(&moveThread, NULL, doMove, (void *)&flyList[0]);
    pthread_create(&drawThread, NULL, doDraw, NULL);

    /* メインスレッドはなにか入力されるのを待ち、ハエの目標点をセットする */
    while(1)
    {
        printf("Destination? ");
        fflush(stdout);
        fgets(buf, sizeof(buf), stdin);
        if(strncmp(buf, "stop", 4) == 0)
            break;
        destX = strtod(buf, &cp);
        destY = strtod(cp, &cp);
        if(!FlySetDestination(&flyList[0], destX, destY))
        {
            printf("The fly is busy now. Try later.\n");
        }
    }
    stopRequest = 1;

    pthread_join(drawThread, NULL);
    pthread_join(moveThread, NULL);
    FlyDestroy(&flyList[0]);
    return EXIT_SUCCESS;
}
