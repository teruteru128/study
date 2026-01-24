
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

/**
 * リダイレクトでファイルディスクリプタを渡すサンプル
 * fdsample 3 3< input.txt
 * みたいに使ってね
*/
int main(int argc, char *argv[])
{
	if(argc < 2)
	{
        fprintf(stderr, "Usage: %s fd fd< file\n", argv[0]);
		return 1;
	}
	int fd = strtol(argv[1], NULL, 10);
    errno = 0;
	if(fcntl(fd, F_GETFD) == -1 && errno == EBADF)
	{
		perror("bad fd");
		return EXIT_FAILURE;
	}
	char buffer[BUFSIZ];
	ssize_t readed = read(fd, buffer, BUFSIZ);
	if(readed < 0)
	{
        perror("read");
		return 1;
	}
	close(fd);
	printf("[%s]\n", buffer);
	return 0;
}

