#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <mysql/mysql.h>

int main()
{
    MYSQL * mysql = NULL;
    //初始化mysql句柄
    mysql = mysql_init(NULL);
    if(mysql == NULL){
        printf("mysql init error\n");
        return -1;
    }
    //连接服务器mysql_real_connect(句柄，服务器IP，用户名，密码，库名称，端口号套接字文件，客户端标志)
    if(mysql_real_connect(mysql, "127.0.0.1","root","zhuyang0718.","vod_system",0,NULL,0)==NULL)
    {
        printf("connect mysql failed:%s\n",mysql_error(mysql));
        return -1;
    }
    //设置字符集
    int ret = mysql_set_character_set(mysql,"utf8");
    if(ret != 0)
    {
        printf("character set failed:%s\n",mysql_error(mysql));
        return -1;
    }
    //选择数据库mysql_select_db(mysql,"vod_system");
    char * select = "select * from tb_video;";
    ret = mysql_query(mysql,select);
    if(ret != 0){
        printf("query sql failed:%s\n",mysql_error(mysql));
        return -1;
    }
    //保存结果集到本地MYSQL_RES * mysql_store_result(句柄)
    MYSQL_RES *res = mysql_store_result(mysql);
    if(ret!=0)
    {
        printf("store result failed:%s\n",mysql_error(mysql));
        return -1;
    }
    //获取结果集条数mysql_num_row（结果集）
    //获取结果集列数mysql_num_fields（结果集）
    int row_num = mysql_num_rows(res);
    int row_col = mysql_num_fields(res);
    for(int i = 0; i < row_num;++i)
    {
        //逐条遍历获取结果集mysql_fetch_row（结果集）
        MYSQL_ROW row = mysql_fetch_row(res);
        for(int j = 0; j < row_col; ++j)
            printf("%s\t",row[j]);
        printf("\n");
    }
    //释放结果集
    mysql_free_result(res);
    mysql_close(mysql);
    return 0;
}

