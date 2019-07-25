#include <iostream>
#include <mysql/mysql.h>

#if 0
int main(int argc, char *argv[]) {
    //1.创建一个句柄
    MYSQL *con = mysql_init(NULL);
    //连接数据库
    void *err = mysql_real_connect(con, "127.0.0.1", "qz", "915531401",
                                   "blog", 3306, NULL, 0);
    if (err == NULL) {
        std::cerr << "db connect filed" << std::endl;
        std::cerr << mysql_error(con) << std::endl;
    }

    //设置编码
    mysql_set_character_set(con, "utf8");

    /*
    //插入
    char buf[1024*4] = {0};
    char title[] = "立一个flag";
    char content[] = "我要拿30万";
    int tag = 1;
    char date[] = "2019-07-25";

    sprintf(buf, "insert into blog_table values(null, '%s', '%s', %d, '%s')", title, content, tag, date);
    std::cout << buf << std::endl;

    //执行sql
    int ret = mysql_query(con, buf);
    if( ret < 0) {
        std::cout << "执行sql失败" << std::endl;
    }
     */
    char sql[1024 * 4] = "select * from blog_table;";
    int ret = mysql_query(con, sql);
    if (ret < 0) {
        std::cout << "query error" << std::endl;
    }

    //遍历结果集
    MYSQL_RES* mres = mysql_store_result(con);
    //a) 获取行数, 列数
    int row = mysql_num_rows(mres);
    int col = mysql_num_fields(mres);
    //b)遍历结果集
    for (int i=0; i<row; ++i) {
        MYSQL_ROW r = mysql_fetch_row(mres);
        for (int j=0; j<col; ++j) {
            printf("%s ", r[j]);
        }
    }
    //c)释放结果集
    mysql_free_result(mres);

    mysql_close(con);
    return 0;
}
#endif

#if 1
#define __TEST__
#include <iostream>
#include "Properties.hpp"

int main() {
    Properties p("./.property");
    p.test();
    return 0;
}
#endif