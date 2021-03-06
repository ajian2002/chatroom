#include "config.h"
#ifdef PROJECT_SERVER
#include PROJECT_SERVERHEAD
extern zlog_category_t *ser;
#else
#include PROJECT_CLIENTHEAD
extern zlog_category_t *cli;
#endif

bool recv_info(int cfd, info *ms)
{
    zlog_category_t *tmp = NULL;
#ifdef PROJECT_SERVER
    tmp = ser;
#else
    tmp = cli;
#endif

    memset(ms, 0, sizeof(info));
    // 记录返回值
    int returnnumber = 0;
    // 记录错误次数
    int errornumber = 0;
    int ret = 0;
    size_t lll = sizeof(info);
    // ret = recv(cfd, &lll, sizeof(long), 0);
rerecv:;
    (ret = recv(cfd, ms, lll, MSG_WAITALL));
    if (ret == lll)
    {
        // zlog_error(tmp, "recv %ld return yes", ret);
        return true;
    }
    zlog_error(tmp, "recv %d return no", ret);
    //     if (ret == 0) return false;
    //     if (ret > 0) returnnumber += ret;
    //     if (returnnumber != lll)
    //     {
    //         // #ifdef PROJECT_CLIENT
    //         //         zlog_warn(tmp, "cli:::this recv %d, all recv %d", ret, returnnumber);
    //         // #else
    //         //         zlog_warn(tmp, "ser:::this recv %d, all recv %d", ret, returnnumber);
    //         // #endif
    //         if (errno == EWOULDBLOCK || errno == EAGAIN)
    //         {
    //             // 服务端不能卡死,客户端close之后服务端收到EAGAIN,三次失败直接close
    //             // 客户端适时等待能提高准确率,永远重试
    // #ifdef PROJECT_CLIENT
    //             sleep(1);  // magic number
    //             goto rerecv;
    // #endif
    //         }

    //         zlog_warn(tmp, "recv failed %s  should recv:%ld really recv:%d", show_errno(), lll,
    //                   returnnumber);
    //         errornumber++;
    //         if (errornumber > 30)
    //         {
    //             zlog_warn(tmp, "can't recv info over 30");
    //             goto over;
    //         }
    //         goto rerecv;

    //     over:;
    //         zlog_error(tmp, "recv info failed %s:%s over   ", show_errno(), strerror(errno));
    //         return false;
    //     }
    return false;
}

bool send_info(int cfd, info *ms)
{
    zlog_category_t *tmp = NULL;
#ifdef PROJECT_SERVER
    tmp = ser;
#else
    tmp = cli;
#endif
    // 记录返回值
    int returnnumber = 0;
    // 记录错误次数
    int errornumber = 0;
    int ret = 0;
    size_t lll = sizeof(info);
    // ret = send(cfd, &lll, sizeof(size_t), 0);
    (ret = send(cfd, ms, lll, 0));
    if (ret == lll)
    {
        // zlog_error(tmp, "send %ld return yes", ret);
        return true;
    }
    zlog_error(tmp, "send %d return no", ret);
    // resend:;
    //     ret = send(cfd, ms + returnnumber, lll - returnnumber, 0);
    //     if (ret > 0) returnnumber += ret;
    //     if (ret == 0) return false;
    //     if (lll != returnnumber)
    //     {
    //         // #ifdef PROJECT_CLIENT
    //         //         zlog_warn(tmp, "cli:::this recv %d, all recv %d", ret, returnnumber);
    //         // #else
    //         //         zlog_warn(tmp, "ser:::this recv %d, all recv %d", ret, returnnumber);
    //         // #endif
    //         if (errno == EWOULDBLOCK || errno == EAGAIN)
    //         {
    // #ifdef PROJECT_CLIENT
    //             sleep(1);  // magic number
    //             goto resend;
    // #endif
    //         }
    //         zlog_warn(tmp, "send failed %s   ", show_errno());
    //         errornumber++;
    //         if (errornumber > 30)
    //         {
    //             zlog_warn(tmp, "can't send info over 30");
    //             goto over;
    //         }
    //         goto resend;

    //     over:;
    //         zlog_error(tmp, "send info failed %s:%s over ", show_errno(), strerror(errno));
    //         return false;
    //     }
    return false;
}

bool recv_file(int cfd, char *path, long int f_size)
{
    zlog_category_t *tmp = NULL;
#ifdef PROJECT_SERVER
    tmp = ser;
#else
    tmp = cli;
#endif
    // 记录返回值
    int returnnumber = 0;
    // 记录错误次数
    int errornumber = 0;
    // char cmd[100]   = {0};
    char dir[PATH_MAX] = {0};
    int ret = 0;
    strcpy(dir, path);
    // sprintf(cmd, "mkdir -p %s", dirname(dir));
    // signal(SIGCHLD, SIG_DFL);
    // if (system(cmd) && errno != EISDIR)  //避免多次系统调用开销大
    // {
    //     signal(SIGCHLD, SIG_IGN);
    //     zlog_error(tmp, "%s error:%s ", cmd, show_errno());
    //     return false;
    // }
    // signal(SIGCHLD, SIG_IGN);
    // char dir[100] = {0};
    // strcpy(dir, path);
    dirname(dir);
    // zlog_error(tmp, "dir path is ::::::%s", dir);
    if (-1 == mkdir(dir, 0777))
    {
        if (errno != EEXIST)
        {
            zlog_error(tmp, "creat %s error:%s ", dir, show_errno());
            return false;
        }
    }
    // path:/home/ajian/3dd6dd93-8490-427d-bc44-63b668568981/Makefile
    int fd = open(path, O_RDWR | O_APPEND | O_CREAT, 0777);
    if (fd < 0)
    {
        zlog_error(tmp, "%s can't creat file %s  ", path, show_errno());
        return false;
    }
    ftruncate(fd, f_size);

    char *buf = (char *)mmap(NULL, f_size, PROT_WRITE, MAP_SHARED, fd, 0);
    if (buf == NULL)
    {
        zlog_error(tmp, "mmap error %s", show_errno());
        close(fd);
        return false;
    }
    // off_t offset = 0;
resend:;
    // if (f_size != offset + (returnnumber = recv(cfd, buf, f_size -
    // offset, 0)))
    returnnumber = recv(cfd, buf, f_size, 0);
    if (returnnumber == 0)
    {
        close(fd);
        msync(buf, f_size, MS_ASYNC);
        munmap(buf, f_size);
        return false;
    }
    if (returnnumber < 0)
    {
        zlog_warn(tmp, "recv file failed %s", show_errno());

        if (errno == EWOULDBLOCK || errno == EAGAIN)
        {
#ifdef PROJECT_CLIENT
            sleep(1);  // magic number
            goto resend;
#endif
        }

        errornumber++;
        if (errornumber > 30)
        {
            zlog_warn(tmp, "can't recv file over 3");
            goto over;
        }
        goto resend;

    over:;
        zlog_error(tmp, "recv file failed %s:%s over", show_errno(), strerror(errno));
        close(fd);
        msync(buf, f_size, MS_ASYNC);
        munmap(buf, f_size);
        return false;
    }
    if (returnnumber > 0)
    {
        ret += returnnumber;
        if (f_size != ret)
        {
            // offset += returnnumber;
            // zlog_info(tmp, "已接收%lf", offset * 1.0 / f_size);
            goto resend;
        }
    }

    close(fd);
    msync(buf, f_size, MS_ASYNC);
    munmap(buf, f_size);
    return true;
}

bool send_file(int cfd, char *path, long int f_size)
{
    zlog_category_t *tmp = NULL;
#ifdef PROJECT_SERVER
    tmp = ser;
#else
    tmp = cli;
#endif
    // 记录返回值
    int returnnumber = 0;
    // 记录错误次数
    int errornumber = 0;
    int ret = 0;
    int fd = open(path, O_RDONLY);
    if (fd < 0)
    {
        zlog_error(tmp, "open %s error", path);
        return false;
    }

// off_t offset = 0;
resend:;
    /*
    ：sendfile——在文件描述符之间传输数据描述ssize_t sendfile(int out_fd, int
    in_fd, off_t *offset, size_t
    count);sendfile()在一个文件描述符和另一个文件描述符之间复制数据。因为这种复制是在内核中完成的，所以sendfile()比read(2)和write(2)的组合更高效，后者需要在用户空间之间来回传输数据。in_fd应该是打开用于读取的文件描述符，而out_fd应该是打开用于写入的文件描述符。如果offset不为NULL，则它指向一个保存文件偏移量的变量，sendfile()将从这个变量开始从in_fd读取数据。当sendfile()返回时，这个变量将被设置为最后一个被读取字节后面的字节的偏移量。如果offset不为NULL，则sendfile()不会修改当前值租用文件偏移in_fd;否则，将调整当前文件偏移量以反映从in_fd读取的字节数。如果offset为NULL，则从当前文件偏移量开始从in_fd读取数据，并通过调用更新文件偏移量。count是要在文件描述符之间复制的字节数。in_fd参数必须对应于支持类似mmap(2)的操作的文件(也就是说，它不能是套接字)。在2.6.33之前的Linux内核中，out_fd必须引用一个套接字。从Linux
    2.6.33开始，它可以是任何文件。如果是一个常规文件，则sendfile()适当地更改文件偏移量。
    */
    // TODO(ajian): 断点续传
    // fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) &~ O_NONBLOCK);
    // if (offset + (returnnumber = sendfile(cfd, fd, &offset, f_size) !=
    // f_size))

    returnnumber = sendfile(cfd, fd, NULL, f_size);
    if (returnnumber == 0)
    {
        zlog_error(tmp, "return 0");
        close(fd);
        return false;
    }
    if (returnnumber < 0)
    {
        zlog_warn(tmp, "send file failed %s", show_errno());
        if (errno == EWOULDBLOCK || errno == EAGAIN)
        {
#ifdef PROJECT_CLIENT
            sleep(1);  // magic number
            goto resend;
#endif
        }
        errornumber++;
        if (errornumber > 30)
        {
            zlog_warn(tmp, "can't send file over 3");
            goto over;
        }
        goto resend;

    over:;
        close(fd);
        zlog_error(tmp, "send file failed %s:%s over", show_errno(), strerror(errno));
        return false;
    }
    if (returnnumber > 0)
    {
        // offset += returnnumber;
        // zlog_info(tmp, "已发送%lf", offset * 1.0 / f_size);
        ret += returnnumber;
        if (ret != f_size)
        {
            // zlog_error(tmp, "returnnumber %d offset %ld", returnnumber,
            // offset);
            goto resend;
        }
    }

    close(fd);
    zlog_info(tmp, " send file ok");
    // zlog_info(tmp, "offset=%ld", offset);
    return true;
}
