#include "config.h"
#include PROJECT_CLIENTHEAD

zlog_category_t *cli = NULL;
int cfd;
int main(int argc, char **argv)
{
    srand((unsigned)time(NULL));
    /// @brief 开日志
    char cmd[100] = {0};
#ifndef DEBUG
    sprintf(cmd, "rm ");
    strcat(cmd, PROJECT_LOGPATH);
    strcat(cmd, "client*.log");
    system(cmd);
#endif
    cli = my_zlog_init("client");
    zlog_info(cli, "--------start--------");

    /// @brief 解析命令行
    char *options = "h";
    int opt;
    while ((opt = getopt(argc, argv, options)) != -1)
    {
        /**
         * @brief getopt
         * @note optarg —— 指向当前选项参数(如果有) 的指针。
         * @note optind —— 再次调用 getopt()时的下一个 argv指针的索引。
         * @note optopt —— 最后一个未知选项。
         * @note opterr ­——
         * 如果不希望getopt()打印出错信息，则只要将全域变量opterr设为0即可。
         */

        if (opt == 'h') clienthelp();

        ///  @todo help手册
        printf("******************************\n");
    }

    if (argc < 2)
    {
        printf("usage ./client ipaddr port \n");
        exit(1);
    }
    ///  连接服务器
    cfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    struct sockaddr_in addr;
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    inet_aton(argv[1], &addr.sin_addr);
    addr.sin_port     = htons(atoi(argv[2]));
    socklen_t addrlen = sizeof(addr);
reconnect:
    if (-1 == connect(cfd, (struct sockaddr *)&addr, addrlen))
    {
        sleep(rand() % 3);
        zlog_debug(cli, "connect failed :%s", strerror(errno));
        if (connect(cfd, (struct sockaddr *)&addr, addrlen) && errno == EISCONN)
            ;
        else
        {
            zlog_debug(cli, "re connect");
            goto reconnect;
        }
    }
    zlog_info(cli, "connect success");

    printf("登录请输入你的用户名\n");
    printf("注册请输入你想要的用户名\n");
    printf("(不超过15个字符)\n");
    char username[20];
    char passwd[20];
    memset(username, 0, sizeof(username));
    memset(passwd, 0, sizeof(passwd));

    scanf("%16s", username);
    /// login
    if (cli_accessusername(username))
    {
        zlog_info(cli, "login %s access success ", username);
        int errornumber = 0;
    again:
        // printf("请输入密码:\n");
        // scanf("%19s", passwd);
        strcpy(passwd, getpass("请输入密码:\n"));

        if (cli_accesspasswd(username, passwd))
        {
            zlog_info(cli, "login %s passwd right passwd:%s", username, passwd);
            if (useronline(username))
            {
                zlog_debug(cli, "login %s success 挤掉", username);
                printf("您的账号已在别处下线\n");
            }
            else
            {
                zlog_info(cli, "login success");
            }

            printf("登录成功\n");
        }
        else
        {
            errornumber++;
            if (errornumber == 3)
            {
                printf(
                    "密码错误次数太多，暂时锁定帐号%s,"
                    "一分钟以后重新登陆\n",
                    username);
                zlog_info(cli, "login %s passwd error passwd error 3 times ",
                          username);
                exit(0);
            }
            else
            {
                zlog_debug(cli, "login %s passwd error  %d times ", username,
                           errornumber);
                printf("密码错误\n");
                goto again;
            }
        }
    }
    else  ///  注册
    {
        zlog_info(cli, "sign user: %s", username);
        printf("-----------------注册:\n用户名:%s\n密码:", username);
        scanf("%19s", passwd);

        /// mysql

        printf("-----------------注册成功\n");
        printf("您的用户名\n%s\n您的密码\n%s\n请妥善保管\n", username, passwd);
        zlog_info(cli, "注册成功:用户名:%s 密码:%s", username, passwd);
    }

    showmainmenu();
}