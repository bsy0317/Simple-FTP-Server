#include <openssl/md5.h>
#include <sys/stat.h>
#include <fcntl.h>

char* getMD5(char* file_name)
{
    MD5_CTX lctx;
    unsigned char digest[16];
    char* md5msg;
    int i;
    int fd;
    struct stat status;
    char *data;

    md5msg = (char *)malloc(sizeof(char)*40);
    MD5_Init(&lctx);

    fd = open(file_name, O_RDONLY);
    if (fd < 0)
    {
        return NULL;
    }
    fstat(fd, &status);

    data = (char *)malloc(status.st_size);
    read (fd, data, status.st_size);

    MD5_Update(&lctx, data, status.st_size);
    MD5_Final(digest, &lctx);

    for (i = 0; i < sizeof(digest); ++i)
    {
        sprintf(md5msg+(i*2), "%02x", digest[i]);
    }
    free(data);
    return md5msg;
}