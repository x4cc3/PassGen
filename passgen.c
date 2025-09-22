#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <errno.h>

void gen_password(char * memory, int lenght)
{
    if (!memory || lenght <= 0) return;

    const char *alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                           "abcdefghijklmnopqrstuvwxyz"
                           "0123456789"
                           "!@#$%&*()-_=+?";
    size_t alphalen = strlen(alphabet);

    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0)
    {
        
        srand((unsigned) time(NULL) ^ (unsigned) getpid());
        for (int i = 0; i < lenght; ++i)
            memory[i] = alphabet[rand() % alphalen];
        memory[lenght] = '\0';
        return;
    }

    unsigned char buf[4096];
    int remaining = lenght;
    int pos = 0;
    while (remaining > 0)
    {
        int toread = remaining < (int)sizeof(buf) ? remaining : (int)sizeof(buf);
        ssize_t r = read(fd, buf, toread);
        if (r <= 0)
        {
            if (r == 0) break; 
            if (errno == EINTR) continue;
            break;
        }

        for (ssize_t i = 0; i < r && remaining > 0; ++i)
        {
            memory[pos++] = alphabet[buf[i] % alphalen];
            --remaining;
        }
    }

    close(fd);
    memory[lenght] = '\0';
}

int main (int argc, const char* argv[]) 
{
    if (argc != 2)
    {
        printf("Usage <passgen> <num-of-bytes>\n");
        return 0;
    }

    int length = atoi(argv[1]);
    if (length <= 0)
    {
        fprintf(stderr, "Invalid length\n");
        return 1;
    }

    printf("Generating password with %d bytes.\n", length);

    char *password = malloc((size_t)length + 1);
    if (!password)
    {
        perror("malloc");
        return 1;
    }
    gen_password(password, length);
    printf("Generated password:\n%s\n", password);
    free(password);
}