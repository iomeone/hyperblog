#include <map>

#include <signal.h>
#include <sys/mman.h>

//#include <hiredis/hiredis.h>
//设计先行 需求分析
#include "httplib.h"
#include "db.hpp"
#include "authentication.hpp"


/*
class aThreadPool : public httplib::TaskQueue {
public:
    aThreadPool(int n) {
        while (n --> 0) {
            auto i = std::make_shared<std::thread>(Worker, this);
            pool.push_back(i);
        }
    }

    ~aThreadPool() {}

    virtual void enqueue(std::function<void()> fn) override {
        std::unique_lock<std::mutex> l(lock);
        task.push_back(fn);
        cv.notify_one();
    }

    virtual void shutdown() override {
        stop = true;
        cv.notify_all();

        for (auto i : pool) {
            i->join();
        }
    }

protected:
    static void Worker(aThreadPool *t) {
        for (;;) {
            std::function<void()> f;
            {
                std::unique_lock<std::mutex> l(t->lock);
                t->cv.wait(l, [&](){return !t->task.empty() || t->stop; });
                if (t->stop && t->task.empty()) {
                    break;
                }
                f = t->task.front();
                t->task.pop_front();
            }
            if (f == nullptr) {
                continue;
            }
            f();
        }
    }

private:
    std::list<std::shared_ptr<std::thread>> pool;
    std::list<std::function<void()>> task;
    std::condition_variable cv;
    std::mutex lock;
    std::atomic_bool stop;
};
 */

struct Lst {
    httplib::Server *s;
    const char *ip;
    int port;
};

void* CoListen(void *arg) {
    Lst* l = static_cast<Lst*>(arg);
    l->s->listen(l->ip, l->port);
}

class CoPool : public httplib::TaskQueue {
public:
    CoPool() {
    }

    virtual void enqueue(std::function<void()> fn) override {
        stCoRoutine_t *c;
        co_create(&c, NULL, work, &fn);
        co_resume(c);
    }

    virtual void shutdown() override {

    }
protected:
    static void* work(void *arg) {
        (*static_cast<std::function<void()>*>(arg))();
    }
};

MYSQL *mysql = NULL;
//             user + passwd md5  user
//                cookie      user
std::map<std::string, std::string> userlist;

int main(int argc, char *argv[]) {
    using namespace httplib;
    using namespace hblog;

    Properties prop("config.json");

    mysql = MySQLInit(prop);

    if (mysql == NULL) {
        exit(1);
    }

    signal(SIGINT, [](int signo) {
        MySQLRelease(mysql);
        ::exit(0);
    });

    BlogTable bt(mysql);
    TagTable tt(mysql);
    UserTable ut(mysql);

    Server server;
    //server.new_task_queue = []{ return new aThreadPool(CPPHTTPLIB_THREAD_POOL_COUNT); };
    server.new_task_queue = []{ return new CoPool; };

    /*******************************
     * 登录成功后返回setcookie
     * value是密码的MD5值
     * Set-Cookie : name=value; max-age=100
     * 用户名密码从数据库中取 加入list 计算密码md5 和发来的MD5对比
     * 管理登录
     *******************************/
    server.Post("/login", [&ut](const Request &req, Response &resp) {
        Json::Reader reader;
        Json::Value val;
        std::string user;
        std::string passwd;

        reader.parse(req.body, val);
        user = val["user"].asString();
        passwd = val["password"].asString();

        std::string realpas = ut.UserPasswd(user);

        //密码错误  或用户不存在
        if (realpas.empty() || !auth(realpas, passwd)) {
#ifdef __LOG__
            fprintf(stderr, "登录失败\n");
#endif
            resp.status = 401;
            resp.set_content("{\"OK\" : false, \"Reason\" : \"用户名, 或密码错误\"}",
                             "application/json");
            return;
        }
#ifdef __LOG__
            fprintf(stdout, "登录成功\n");
#endif
        resp.status = 200;
        std::string cookie;
        std::string cok = CalCookie(user, passwd, cookie);
        userlist.insert(std::make_pair(cookie, user));

        resp.set_header("Set-Cookie", cok.c_str());
        resp.set_content("{\"OK\" : true}", "application/json");
        return;
    });

    /*******************************
     * 管理注册  注册后也就登陆了  -to do-:  修改密码  注册
     *******************************/
    server.Post("/reg", [&ut](const Request &req, Response &resp) {
        Json::Reader reader;
        Json::Value val;
        std::string user;
        std::string passwd;

        reader.parse(req.body, val);
        user = val["user"].asString();
        passwd = val["password"].asString();

        //不能注册*_*
        if (!ut.save(user, passwd)) {
            resp.status = 1;
            resp.set_content(R"({"OK" : false, "Reason" : "I don't know"})", "application/json");
            return;
        }

        std::string cookie;
        std::string cok = CalCookie(user, passwd, cookie);
        userlist.insert(std::make_pair(cookie, user));

        resp.set_header("Set-Cookie", cok.c_str());
        resp.set_content("{\"OK\" : true}", "application/json");
        return;
    });

    /*****************************
     * todo:
     * 登出
     *****************************/
    //server.Post("/logout", [](const Request &req, Response &resp) {});

    /*******************************
     * //只能登录的修改***
     * POST /blog
     * 新增一篇blog
     * 正确时返回 OK
     * 错误是返回错误信息
     *******************************/
    server.Post("/blog", [&bt](const Request &req, Response &resp) {
#ifdef __LOG__
        //fprintf(stdout, "权限认证失败\n");
#endif
        //获取request的body 解析成json
        Json::Reader reader;
        Json::Value jreq;
        Json::Value jresp;
        Json::FastWriter fw;
        int err = reader.parse(req.body, jreq);
        if (!err || !isLogin(req.get_header_value("Cookie", 0))) {
#ifdef __LOG__
            fprintf(stderr, "解析请求失败或没有权限 line: %d\n", __LINE__);
#endif
            jresp["OK"] = false;
            jresp["Reason"] = "输入的数据无法解析或没有权限";
            resp.set_content(fw.write(jresp), "application/json");
            resp.status = 400;
            return;
        }
        //对json校验
        if (jreq["title"].empty() ||
            jreq["content"].empty() ||
            jreq["tag_id"].empty() ||
            jreq["create_time"].empty()) {
#ifdef __LOG__
            fprintf(stderr, "格式出错 line: %d\n", __LINE__);
#endif
            jresp["OK"] = false;
            jresp["Reason"] = "数据输入不合法";

            resp.set_content(fw.write(jresp), "application/json");
            resp.status = 400;
            return;
        }

        err = bt.Insert(jreq);
        if (!err) {
#ifdef __LOG__
            fprintf(stderr, "插入失败 line: %d", __LINE__);
#endif
            jresp["OK"] = false;
            jresp["Reason"] = "数据库插入失败";

            resp.set_content(fw.write(jresp), "application/json");
            resp.status = 500;

            return;
        }
#ifdef __LOG__
        printf("插入成功");
#endif
        jresp["OK"] = true;
        resp.set_content(fw.write(jresp), "application/json");
        return;
    });

    /*******************************************
     * GET  /blog?tag_id=1
     * 获取所有的blogs
     * 如果有查询参数 则只输出属于指定tag_id的blog
     *******************************************/
    server.Get("/blog", [&bt](const Request &req, Response &resp) {
        //获取tag_id
        const std::string &tid = req.get_param_value("tag_id");
        Json::Value jresp;
        Json::FastWriter fw;

        if (!tid.empty()) {
            bool ret = bt.SelectAll(&jresp, tid);
            if (!ret) {
#ifdef __LOG__
                fprintf(stderr, "获取所有blog失败 tat_id= %s\n", tid.c_str());
#endif
                jresp["OK"] = false;
                jresp["Reason"] = "从数据库获取所有blog失败";
                resp.status = 500;
                resp.set_content(fw.write(jresp), "application/json");
                return;
            }
        }
        int err = bt.SelectAll(&jresp);
        if (!err) {
#ifdef __LOG__
            fprintf(stderr, "获取所有blog失败\n");
#endif
            jresp["OK"] = "false";
            jresp["Reason"] = "数据库操作失败";
            resp.status = 500;
            resp.set_content(fw.write(jresp), "application/json");
            return;
        }
#ifdef __LOG__
        fprintf(stdout, "获取所有blog成功\n");
#endif
        resp.set_content(fw.write(jresp), "application/json");
        return;
    });

    /******************************
     * GET /blog/:blog_id
     * 查看谋篇blog
     ******************************/
    server.Get(R"(/blog/(\d+))", [&bt](const Request &req, Response &resp) {
        int32_t bid = std::stoi(req.matches[1].str());
        Json::Value jresp;
        Json::FastWriter fw;
        bool err = bt.Select(bid, &jresp);
        if (!err) {
#ifdef __LOG__
            fprintf(stderr, "获取blog失败 blog_id= %d\n", bid);
#endif
            resp.status = 404;
            jresp["OK"] = false;
            jresp["Reason"] = "blog不存在或数据库错误";
            resp.set_content(fw.write(jresp), "application/json");
            return;
        }
#ifdef __LOG__
        fprintf(stdout, "获取blog成功\n");
#endif
        resp.set_content(fw.write(jresp), "application/json");
        return;
    });

    /*******************************
     * //只能登录的修改***
     * PUT /blog/:blog_id
     * 修改一篇blog
     * blog的title content tag_id 不能没有
     *******************************/
    server.Put(R"(/blog/(\d+))", [&bt](const Request &req, Response &resp) {
        int32_t bid = std::stoi(req.matches[1].str());

        Json::Value jresp;
        Json::Value jreq;
        Json::FastWriter fw;
        Json::Reader reader;
        if (!isLogin(req.get_header_value("Cookie", 0))){
#ifdef __LOG__
            fprintf(stdout, "权限认证失败\n");
#endif
            resp.status = 500;
            jresp["OK"] = false;
            jresp["Reason"] = "没有权限";
            resp.set_content(fw.write(jresp), "application/json");
            return;
        }
        int err = reader.parse(req.body, jreq);
        if (!err) {
#ifdef __LOG__
            fprintf(stderr, "修改失败 line: %d\n", __LINE__);
#endif
            resp.status = 400;
            jresp["OK"] = false;
            jresp["Reason"] = "服务器无法解析正文";
            resp.set_content(fw.write(jresp), "application/json");
            return;
        }
        if (jreq["title"].empty() || jreq["content"].empty() || jreq["tag_id"].empty()) {
            //提交的修改内容有错
#ifdef __LOG__
            fprintf(stderr, "修改失败 line: %d\n", __LINE__);
#endif
            resp.status = 400;
            jresp["OK"] = false;
            jresp["Reason"] = "内容不正确";
            resp.set_content(fw.write(jresp), "application/json");
            return;
        }

        jreq["blog_id"] = bid;
        err = bt.Update(jreq);
        if (!err) {
#ifdef __LOG__
            fprintf(stderr, "更新数据失败 line: %d\n", __LINE__);
#endif
            resp.status = 500;
            jresp["OK"] = false;
            jresp["Reason"] = "数据库错误";
            resp.set_content(fw.write(jresp), "applicatin/json");
            return;
        }
#ifdef __LOG__
        fprintf(stdout, "更新成功\n");
#endif
        jresp["OK"] = true;
        resp.set_content(fw.write(jresp), "application/json");
        return;
    });

    /********************************
     * //只能登录的修改***
     * DELETE /blog/:blog_id
     * 删除一篇blog
     ********************************/
    server.Delete(R"(/blog/(\d+))", [&bt](const Request &req, Response &resp) {
        int32_t bid = std::stoi(req.matches[1].str());
        Json::FastWriter fw;
        Json::Value jresp;

        if (!isLogin(req.get_header_value("Cookie", 0))){
#ifdef __LOG__
            fprintf(stdout, "权限认证失败\n");
#endif
            resp.status = 500;
            jresp["OK"] = false;
            jresp["Reason"] = "没有权限";
            resp.set_content(fw.write(jresp), "application/json");
            return;
        }

        int err = bt.Delete(bid);
        if (!err) {
#ifdef __LOG__
            fprintf(stderr, "删除blog失败\n");
#endif
            resp.status = 500;
            jresp["OK"] = false;
            jresp["Reason"] = "数据库错误";
            resp.set_content(fw.write(jresp), "application/json");
            return;
        }
#ifdef __LOG__
        fprintf(stdout, "删除成功\n");
#endif
        jresp["OK"] = true;
        resp.set_content(fw.write(jresp), "applicatin/json");
        return;
    });

    /****************************
     * //只能登录的修改***
     * POST /tag
     * 增加tag
     ****************************/
    server.Post("/tag", [&tt](const Request &req, Response &resp) {
        Json::FastWriter fw;
        Json::Reader reader;
        Json::Value jreq;
        Json::Value jresp;
        if (!isLogin(req.get_header_value("Cookie", 0))){
#ifdef __LOG__
            fprintf(stdout, "权限认证失败\n");
#endif
            resp.status = 500;
            jresp["OK"] = false;
            jresp["Reason"] = "没有权限";
            resp.set_content(fw.write(jresp), "application/json");
            return;
        }
        int err = reader.parse(req.body, jreq);
        if (!err) {
#ifdef __LOG__
            fprintf(stderr, "增加tag错误 line: %d\n", __LINE__);
#endif
            jresp["OK"] = false;
            jresp["Reason"] = "url参数解析失败";
            resp.status = 400;
            resp.set_content(fw.write(jresp), "application/json");
            return;
        }

        if (jreq["tag_name"].empty()) {
#ifdef __LOG__
            fprintf(stderr, "增加tag错误 line: %d\n", __LINE__);
#endif
            jresp["OK"] = false;
            jresp["Reason"] = "内容错误";
            resp.status = 400;
            resp.set_content(fw.write(jresp), "applicatin/json");
            return;
        }

        err = tt.Insert(jreq);
        if (!err) {
#ifdef __LOG__
            fprintf(stderr, "获取插入数据库错误 line: %d\n", __LINE__);
#endif
            jresp["OK"] = false;
            jresp["Reason"] = "数据库插入错误";
            resp.status = 500;
            resp.set_content(fw.write(jresp), "application/json");
            return;
        }
#ifdef __LOG__
        fprintf(stdout, "插入tag成功\n");
#endif
        jresp["OK"] = true;
        resp.set_content(fw.write(jresp), "applicaton/json");
        return;
    });

    /**********************************
     * //只能登录的修改***
     * DELETE /tag/:tag_id
     * 删除tag
     **********************************/
    server.Delete(R"(/tag/(\d+))", [&tt](const Request &req, Response &resp) {
        int32_t tid = std::stoi(req.matches[1].str());
        Json::Value jresp;
        Json::FastWriter fw;
        if (!isLogin(req.get_header_value("Cookie", 0))){
#ifdef __LOG__
            fprintf(stdout, "权限认证失败\n");
#endif
            resp.status = 500;
            jresp["OK"] = false;
            jresp["Reason"] = "没有权限";
            resp.set_content(fw.write(jresp), "application/json");
            return;
        }
        int err = tt.Delete(tid);
        if (!err) {
#ifdef __LOG__
            fprintf(stderr, "删除失败\n");
#endif
            resp.status = 500;
            jresp["OK"] = false;
            jresp["Reason"] = "从数据库删除失败";
            resp.set_content(fw.write(jresp), "application/json");
            return;
        }
#ifdef __LOG__
        fprintf(stdout, "删除tag成功\n");
#endif
        jresp["OK"] = true;
        resp.set_content(fw.write(jresp), "application/json");
        return;
    });

    /*************************
     * GET /tag
     * 查看所有标签
     *************************/
    server.Get("/tag", [&tt](const Request &req, Response &resp) {
        Json::FastWriter fw;
        Json::Value jresp;
        int err = tt.SelectAll(&jresp);
        if (!err) {
#ifdef __LOG__
            fprintf(stderr, "获取所有tag失败\n");
#endif
            jresp["OK"] = false;
            jresp["Reason"] = "数据库查找失败";
            resp.status = 500;
            resp.set_content(fw.write(jresp), "application/json");
            return;
        }
#ifdef __LOG__
        fprintf(stdout, "获取所有tag成功\n");
#endif
        resp.status = 200;
        resp.set_content(fw.write(jresp), "applicaton/json");
        return;
    });

    server.Get("/", [](const Request &req, Response &resp) {
        //请求html
        int file = open("src/resource/front/index.html", O_RDONLY);
        if (file < 0) {  //文件打开失败
            fprintf(stderr, "index.html 打开失败\n");
            resp.status = 404;
            return ;
        }
        struct stat st;
        stat("src/resource/index.html", &st);
        void *m = mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, file, 0);
        resp.set_content(static_cast<char*>(m), st.st_size, "text/html");
        munmap(m, st.st_size);
        return;
    });
	
	server.Get("/shutdown", [](const Request &req, Response &resp) {
		exit(1);
	});

    server.set_base_dir("src/resource/");
    Lst lst;
    lst.ip = prop["servip"].c_str();
    lst.port = std::stoi(prop["servport"]);
    lst.s = &server;

    stCoRoutine_t *t;
    co_create(&t, NULL, CoListen, &lst);
    co_resume(t);

    co_eventloop(co_get_epoll_ct(), NULL, NULL);

    return 0;
}

