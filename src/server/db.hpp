#include <memory>
#include <iostream>

#include <cstring>
#include <cstdio>

#include  <unistd.h>

#include <mysql/mysql.h>
#include <jsoncpp/json/json.h>

#include "Properties.hpp"

#ifdef __DEBUG__
int fd;
#endif

namespace hblog {
    //初始化MySQL
    static MYSQL *MySQLInit(Properties &prop) {
#ifdef __DEBUG__
        fd = open("log", O_RDWR | O_CREAT | O_TRUNC);
#endif

        MYSQL *con = mysql_init(NULL);

        if (!mysql_real_connect(con, prop["ip"].c_str(), prop["user"].c_str(),
                                prop["passwd"].c_str(), prop["db"].c_str(),
                                atoi((prop["dbport"]).c_str()), NULL, 0)) {

//      if (!mysql_real_connect(con, "127.0.0.1", "", "", "blog", 3306, NULL, 0)) {


#ifdef __LOG__
            fprintf(stderr, "数据库连接失败\n");
#endif
            return NULL;
        }
#ifdef __LOG__
        fprintf(stdout, "数据库连接成功\n");
#endif
        mysql_set_character_set(con, "utf8");
        return con;
    }

    static void MySQLRelease(MYSQL *con) {
        mysql_close(con);
    }

    //blog表的操作类
    class BlogTable {
    public:
        BlogTable(MYSQL *con) {
            con_fd = con;
        }

        /****************************************
        * 插入的对象
        *
        * param: blog       要放入数据库的对象
        *
        * blog:
        * {
        *    title:
        *    content: 注意正文中的符号存现在sql语句中
        *    create_time:
        *    tig_id:
        * }
        *****************************************/
        bool Insert(const Json::Value &blog) {
            const std::string &content = blog["content"].asCString();
            const std::string &title = blog["title"].asCString();

            //转义 正文防注入         存放content转义后的字符串
            std::unique_ptr<char> ebuf(new char[content.size() * 2 + 1]);
            std::unique_ptr<char> tbuf(new char[title.size() * 2 + 1]);
            mysql_real_escape_string(con_fd, ebuf.get(),
                                     content.c_str(), content.size());
            mysql_real_escape_string(con_fd, tbuf.get(),
                                     title.c_str(), title.size());

            std::unique_ptr<char> sql(new char[content.size() * 2 + 4096]);

            const char *str = "insert into blog_table values(null, '%s', '%s', %d, '%s')";
            sprintf(sql.get(), str, tbuf.get(),
                    ebuf.get(), blog["tag_id"].asInt(),
                    blog["create_time"].asCString());

            int err = mysql_query(con_fd, sql.get());
            if (err != 0) {
#ifdef __LOG__
                fprintf(stderr, "执行插入失败\n %s\n", sql.get());
#endif
#ifdef __DEBUG__
                write(fd, "\n", 1);
                write(fd, sql.get(), strlen(sql.get()));
                write(fd, "\n", 1);
#endif
                return false;
            }
#ifdef __LOG__
            fprintf(stdout, "插入blog表成功\n");
#endif
            return true;
        }

        /****************************************
         *
         * param: blog_id       查找blog_id的所有
         *        tag_id
         *
         ***************************************/
        bool Select(int32_t blog_id, Json::Value *blog) {
            char sql[1024] = {0};
            sprintf(sql, "select blog_id, title, content, tag_id, create_time "
                         "from blog_table where blog_id=%d",
                    blog_id);
            int err = mysql_query(con_fd, sql);
            if (err != 0) {
                return false;
            }
            MYSQL_RES *res = mysql_store_result(con_fd);
            int rows = mysql_num_rows(res);
            if (rows != 1) {
#ifdef __LOG__
                fprintf(stderr, "查找失败 blog_id= %d\n %s\n", blog_id, sql);
#endif
                return false;
            }
            MYSQL_ROW row = mysql_fetch_row(res);
            (*blog)["blog_id"] = atoi(row[0]);
            (*blog)["title"] = row[1];
            (*blog)["content"] = row[2];
            (*blog)["tag_id"] = row[3];
            (*blog)["create_time"] = row[4];
#ifdef __LOG__
            fprintf(stdout, "查找成功过\n");
#endif
            mysql_free_result(res);
            return true;
        }

        /***************************************
         *
         * param: blogs         存放查找到的所有对象
         *        tag_id        只查找对应tag_id的对象 为空查找所有
         ***************************************/
        bool SelectAll(Json::Value *blogs, const std::string &tag_id = "") {
            char sql[1024] = {0};
            if (tag_id.empty()) {
                sprintf(sql, "select blog_id, title, tag_id, create_time from blog_table");
            } else {
                sprintf(sql, "select blog_id, title, tag_id, create_time "
                             "from blog_table where tag_id=%d", std::stoi(tag_id));
            }

            int err = mysql_query(con_fd, sql);
            if (err != 0) {
#ifdef __LOG__
                fprintf(stderr, "查找所有blog失败\n");
#endif
                return false;
            }

            MYSQL_RES *res = mysql_store_result(con_fd);
            int rows = mysql_num_rows(res);
            for (int i = 0; i < rows; ++i) {
                MYSQL_ROW row = mysql_fetch_row(res);
                //把查询的一行放入一个json对象中
                Json::Value ablog;
                ablog["blog_id"] = atoi(row[0]);
                ablog["title"] = row[1];
                ablog["tag_id"] = atoi(row[2]);
                ablog["create_time"] = row[3];

                blogs->append(ablog);
            }
#ifdef __LOG__
            fprintf(stdout, "查找所有blog成功\n");
#endif

            mysql_free_result(res);
            return true;
        }

        /**************************************
         * 更新
         **************************************/
        bool Update(const Json::Value &blog) {
            const std::string &content = blog["content"].asCString();
            const std::string &title = blog["title"].asCString();
            //转义 正文防注入         存放content转义后的字符串
            std::unique_ptr<char> ebuf(new char[content.size() * 2 + 1]);
            std::unique_ptr<char> tbuf(new char[title.size() * 2 + 1]);
            //所有的从用户处得到的字符串都要经过转义
            mysql_real_escape_string(con_fd, ebuf.get(),
                                     content.c_str(), content.size());
            mysql_real_escape_string(con_fd, tbuf.get(),
                                     title.c_str(), title.size());

            std::unique_ptr<char> sql(new char[content.size() * 2 + 4096]);

            const char *str = "update blog_table set title='%s', content='%s', tag_id=%d where blog_id=%d";
            sprintf(sql.get(), str, tbuf.get(), ebuf.get(),
                    std::stoi(blog["tag_id"].asString()), blog["blog_id"].asInt());
            int err = mysql_query(con_fd, sql.get());
            if (err != 0) {
#ifdef __LOG__
                fprintf(stderr, "更新失败\n");
#endif
                return false;
            }
#ifdef __LOG__
            fprintf(stdout, "更新blog表成功\n");
#endif
            return true;
        }

        bool Delete(int32_t blog_id) {
            char sql[1024 * 4] = {0};
            sprintf(sql, "delete from blog_table where blog_id=%d", blog_id);
            int err = mysql_query(con_fd, sql);
            if (err != 0) {
#ifdef __LOG__
                fprintf(stderr, "删除失败 blog_id= %d\n", blog_id);
#endif
                return false;
            }
#ifdef __LOG__
            fprintf(stdout, "删除成功\n");
#endif
            return true;
        }

    private:
        MYSQL *con_fd;
    };

    /************************************
     * Tag表操作的封装
     ************************************/
    class TagTable {
    public:
        TagTable(MYSQL *con) : con_fd(con) {}

        bool Insert(const Json::Value &tag) {
            char sql[1024 * 4] = {0};

            std::string str = tag["tag_name"].asCString();
            std::unique_ptr<char> buf(new char[str.size() * 2 + 1]);
            mysql_real_escape_string(con_fd, buf.get(), str.c_str(), str.size());
            sprintf(sql, "insert into tag_table values(null, '%s')", buf.get());
            int err = mysql_query(con_fd, sql);
            if (err != 0) {
#ifdef __LOG__
                fprintf(stderr, "tag表插入失败\n");
#endif
                return false;
            }
#ifdef __LOG__
            fprintf(stdout, "tag表插入成功\n");
#endif
            return true;
        }

        bool Delete(int32_t tag_id) {
            char sql[1024] = {0};
            sprintf(sql, "delete from tag_table where tag_id=%d", tag_id);
            int err = mysql_query(con_fd, sql);
            if (err != 0) {
#ifdef __LOG__
                fprintf(stderr, "tag表删除失败 tag_id= %d\n", tag_id);
#endif
                return false;
            }
#ifdef __LOG__
            fprintf(stdout, "tag表删除成功\n");
#endif
            return true;
        }

        bool SelectAll(Json::Value *tags) {
            char sql[1024] = {0};
            sprintf(sql, "select * from tag_table");
            int err = mysql_query(con_fd, sql);
            if (err != 0) {
#ifdef __LOG__
                fprintf(stderr, "tag表查找失败\n");
#endif
                return false;
            }
            MYSQL_RES *res = mysql_store_result(con_fd);
            int rows = mysql_num_rows(res);
            for (int i = 0; i < rows; ++i) {
                MYSQL_ROW row = mysql_fetch_row(res);
                Json::Value atag;
                atag["tag_id"] = atoi(row[0]);
                atag["tag_name"] = row[1];

                tags->append(atag);
            }
#ifdef __LOG__
            fprintf(stdout, "tag表查找成功\n");
#endif
            mysql_free_result(res);
            return true;
        }

    private:
        MYSQL *con_fd;
    };

    //todo: 数据库也得改  test也得做
    class UserTable{
    public:
        UserTable(MYSQL *con) : con_fd(con) {}

        std::string UserPasswd(const std::string &user) {
            char sql[1024 * 4] = {0};
            sprintf(sql, "select password from user_table where user='%s'", user.c_str());
            int err = mysql_query(con_fd, sql);

            if (err != 0) {
#ifdef __LOG__
                fprintf(stderr, "从数据库获取用户密码失败\n");
#endif
                return "";
            }
            MYSQL_RES *res = mysql_store_result(con_fd);
            if (res == NULL) {
#ifdef __LOG__
                fprintf(stderr, "用户不存在\n");
#endif
                return "";
            }
            int rows = mysql_num_rows(res);
            MYSQL_ROW row = mysql_fetch_row(res);
#ifdef __LOG__
            fprintf(stdout, "获取密码成功\n");
#endif
            std::string ret = row[0];
            mysql_free_result(res);
            return ret;
        }

        bool save(const std::string &usr, const std::string &passwd) {
            char sql[1024 * 4] = {0};
            sprintf(sql, "insert into user_table values('%s', '%s')", usr.c_str(), passwd.c_str());
            int err = mysql_query(con_fd, sql);
            if (err != 0) {
#ifdef __LOG__
                fprintf(stderr, "注册失败\n");
#endif
                return false;
            }
#ifdef __LOG__
            fprintf(stdout, "注册成功\n");
#endif
            return true;
        }

    private:
        MYSQL *con_fd;
    };
}; //end namespace hblog