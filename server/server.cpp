#define __LOG__

#include <signal.h>

//#include <hiredis/hiredis.h>

#include "httplib.h"
#include "db.hpp"

MYSQL *mysql = NULL;

int main() {
    using namespace httplib;
    using namespace hblog;
    mysql = MySQLInit("./resource/.property");
    if(mysql == NULL) {
        exit(1);
    }

    signal(SIGINT, [](int signo) {
        MySQLRelease(mysql);
        ::exit(0);
    });

    BlogTable bt(mysql);
    TagTable tt(mysql);

    Server server;

    /*******************************
     * POST /blog
     * 新增一篇blog
     * 正确时返回 OK
     * 错误是返回错误信息
     *******************************/
    server.Post("/blog", [&bt](const Request &req, Response &resp) {
#ifdef __LOG__
        fprintf(stdout, "增加blog\n");
#endif
        //获取request的body 解析成json
        Json::Reader reader;
        Json::Value jreq;
        Json::Value jresp;
        Json::FastWriter fw;
        int err = reader.parse(req.body, jreq);
        if (!err) {
#ifdef __LOG__
            fprintf(stderr, "解析请求失败 line: %d\n", __LINE__);
#endif
            jresp["OK"] = false;
            jresp["Reason"] = "输入的数据无法解析";
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
     * DELETE /blog/:blog_id
     * 删除一篇blog
     ********************************/
    server.Delete(R"(/blog/(\d+))", [&bt](const Request &req, Response &resp) {
        int32_t bid = std::stoi(req.matches[1].str());
        Json::FastWriter fw;
        Json::Value jresp;

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
     * POST /tag
     * 增加tag
     ****************************/
    server.Post("/tag", [&tt](const Request &req, Response &resp) {
        Json::FastWriter fw;
        Json::Reader reader;
        Json::Value jreq;
        Json::Value jresp;
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
     * DELETE /tag/:tag_id
     * 删除tag
     **********************************/
    server.Delete(R"(/tag/(\d+))", [&tt](const Request &req, Response &resp) {
        int32_t tid = std::stoi(req.matches[1].str());
        Json::Value jresp;
        Json::FastWriter fw;

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
            return ;
        }
#ifdef __LOG__
        fprintf(stdout, "获取所有tag成功\n");
#endif
        jresp["OK"] = true;
        resp.set_content(fw.write(jresp), "applicaton/json");
        return;
    });

    server.set_base_dir("./resource");
    server.listen("0.0.0.0", 9155);
    return 0;
}