# HyperBlog

一个简单的博客系统 C++实现 B/S

1. 只支持单个用户

2. 针对文章的crud

3. 针对标签的增删改查



<br><hr>

## c语言操作mysql数据库

1. 创建句柄 MYSQL* mysql_init();
2. 连接数据库 int mysql_real_connect(MYSQL*, ip, user, pwd, db, port, NULL, 0)
3. 设置编码 void mysql_set_character_set(MYSQL* , const char *charset)
4. 执行sql语句 int mysql_query(MYSQL* , sql)
5. 获取结果集 MYSQL_RES* mysql_store_result(MYSQL *)
> 行用MYSQL_ROW表示 行数用int mysql_num_rows(MYSQL_RES*) 获取
> 列的每个字段用字符串表示 列数用int mysql_num_fields(MYSQL_RES*) 获取
5. 释放结果集mysql_free_result(MYSQL_RES*)
6. 断开连接mysql_close(MYSQL*)

<br><hr>

## property属性获取

1. 创建属性对象Properties(file)
2. 用str = obj["key"] 获取属性值