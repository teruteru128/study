
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define WIDTH 78
#define HEIGHT 23
#define MAX_FLY 6
const char *flyMarkList = "o@*+.#";
#define DRAW_SYCLE 50
#define MIN_SPEED 1.0
#define MAX_SPEED 20.0

int stopRequest;

pthread_mutex_t mutex;

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

/* ハエ構造体 */
typedef struct
{
    char mark;    /* 表示キャラクタ */
    double x, y;  /* 座標 */
    double angle; /* 移動方向 (角度) */
    double speed; /* 移動速度 (ピクセル/秒) */
} Fly;

Fly flyList[MAX_FLY];

/* ハエの状態をランダムに初期化 */
void FlyInitRandom(Fly *fly, char mark_)
{
    fly->mark = mark_;
    fly->x = randDouble(0, (double)(WIDTH - 1));
    fly->y = randDouble(0, (double)(HEIGHT - 1));
    fly->angle = randDouble(0, M_2_PI);
    fly->speed = randDouble(MIN_SPEED, MAX_SPEED);
}

/* ハエを移動する */
void FlyMove(Fly *fly)
{
    fly->x += cos(fly->angle);
    fly->y += sin(fly->angle);
    if (fly->x < 0)
    {
        fly->x = 0;
        fly->angle = M_PI - fly->angle;
    }
    else if (fly->x > WIDTH - 1)
    {
        fly->x = WIDTH - 1;
        fly->angle = M_PI - fly->angle;
    }
    if (fly->y < 0)
    {
        fly->y = 0;
        fly->angle = -fly->angle;
    }
    else if (fly->y > HEIGHT - 1)
    {
        fly->y = HEIGHT - 1;
        fly->angle = -fly->angle;
    }
}

/*
 * ハエが指定座標にあるかどうか
 */
int FlyIsAt(const Fly *fly, int x, int y)
{
    return ((int)(fly->x) == x) && ((int)(fly->y) == y);
}

void *doMove(void *arg)
{
    Fly *fly = (Fly *)arg;
    while (!stopRequest)
    {
        pthread_mutex_lock(&mutex);
        FlyMove(fly);
        pthread_mutex_unlock(&mutex);
        mSleep((int)(1000.0 / fly->speed));
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
        pthread_mutex_lock(&mutex);
        drawSecrren();
        pthread_mutex_unlock(&mutex);
        mSleep(DRAW_SYCLE);
    }
    return NULL;
}

/*  */
int main(int argc, char *argv[])
{
    pthread_t drawThread;
    pthread_t moveThread[MAX_FLY];
    int i;
    char buf[40];

    srand((unsigned int)time(NULL));
    pthread_mutex_init(&mutex, NULL);
    clearScreen();
    for (i = 0; i < MAX_FLY; i++)
    {
        FlyInitRandom(&flyList[i], flyMarkList[i]);
    }

    for (i = 0; i < MAX_FLY; i++)
    {
        pthread_create(&moveThread[i], NULL, doMove, (void *)&flyList[i]);
    }
    pthread_create(&drawThread, NULL, doDraw, NULL);

    fgets(buf, sizeof(buf), stdin);
    stopRequest = 1;

    pthread_join(drawThread, NULL);
    for (i = 0; i < MAX_FLY; i++)
    {
        pthread_join(moveThread[i], NULL);
    }
    pthread_mutex_destroy(&mutex);
    return EXIT_SUCCESS;
}
