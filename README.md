# HyperBlog

一个简单的博客系统 C++实现 B/S架构 MVC模式

1. 只支持单个用户
2. 针对文章的crud
3. 针对标签的crud

采用前后端分离的设计思路, 前端通过发送http请求给后端, 使用Restful风格的api来进行前后端数据交互, 使用json在客户端和服务器之间传递数据. 

 服务器端用httplib来接收客户端发来的http请求, 用相应的回调函数处理(controller), 收到的数据使用jsoncpp进行解析http正文(Model), 并用c++封装mysql的c语言接口来实现数据访问层,  

前端使用Vue.js + editer.md + bootstrap用restful风格api与后端用json进行交互

# API设计

GET /blog    表示获取所有的blog的列表

返回数据格式

```json
[
    {
    	"blog_id" : 1,
        "tag_id" : 1,
        "title" : "",
        "create_time" : ""
    }
]
```

GET /blog?tag_id=1 获取指定tag_id下的所有blog

返回数据格式

```josn
[
    {
        "blog_id" : 1,
        "tag_id" : 1,
        "title" : "",
        "create_time" : ""
    }
]
```

GET /blog/:blog_id  获取某一个blog

返回格式

```josn
{
    "blog_id" : 1,
    "tag_id" : 1,
    "title" : "",
    "content" : "",
    "create_time" : ""
}
```

<br><hr>

POST  /blog 新增一篇blog

发送格式

```josn
{
    "title" : "",
    "content" : "",
    "tag_id" : 1,
    "create_time" : ""
}
```

<br>

<hr>

PUT  /blog/:blog_id   更新一篇

发送格式

```
{
    "title" : "",
    "content" : "",
    "tag_id" : 1
}
```

<br>

<hr>

DELETE /blog/:blog_id 删除一篇blog

```json

```





<hr><hr>

POST /tag 新增一个tag

发送格式

```json
{
    "tag_name" : ""
}
```

<br>

<hr>

DELETE /tag/:tag_id

```json

```

<br>

<hr>

GET /tag 获取所有tag列表

返回格式

```
[
    {
        "tag_id" : 1,
        "tag_name" : ""
    },
    {
        "tag_id" : 1,
        "tag_name" : ""
    }
]
```

<br><hr>

## c语言操作mysql数据库

1. 创建句柄 `MYSQL* mysql_init()`
2. 连接数据库 `int mysql_real_connect(MYSQL*, ip, user, pwd, db, port, NULL, 0)`
3. 设置编码 `void mysql_set_character_set(MYSQL* , const char *charset)`
4. 转义sql语句 `mysql_real_escape_string()`
5. 执行sql语句`int mysql_query(MYSQL* , sql)`
6. 获取结果集 `MYSQL_RES* mysql_store_result(MYSQL *)`
> 行用MYSQL_ROW表示 行数用`int mysql_num_rows(MYSQL_RES*) `获取
> 列的每个字段用字符串表示 列数用`int mysql_num_fields(MYSQL_RES*)` 获取
5. 释放结果集`mysql_free_result(MYSQL_RES*)`
6. 断开连接`mysql_close(MYSQL*)`

<br><hr>

## property属性获取

1. 创建属性对象Properties(file)
2. 用str = obj["key"] 获取属性值

## jsoncpp的使用

Json::Value 是一个Json对象并且重载了[]运算符可以直接使用val["key"] = val的形式设置json对象, append方法追加对象

Json::Reader是字符串转json的解析器  parse成员函数可以把字符串转成Json::Value对象

Json::FastWriter可以把一个Json::Value对象转换成一个字符串

<br>

<hr>

## cpp-httplib的使用

创建一个`httplib::Server`对象就相当于创建了一个server服务器

`server.set_base_dir()`设置服务器目录

使用`server.listen(ip, port)`设置监听的端口

`server.[Method](route, callback)`设置路由, 使用回调函数来创建服务



<br>

<hr>



## googletest的使用

类似junit对所有函数进行单元测试

`Test(testcasename, testname)`宏用于启动一个测试用例

`EXPECT_XX()`如果与期望不同, 继续向下执行

`ASSERT_XX()`如果与期望不同, 退出当前函数

`::testing::InitGoogleTest(int *argc, char **argv)`

`RUN_ALL_TESTS()`



## libco的使用

todo: