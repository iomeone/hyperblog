#define __NO__LOG__

#include <tuple>

#include <gtest/gtest.h>
#include <jsoncpp/json/json.h>

#include "server/db.hpp"
#include "testUtils.hpp"

MYSQL *con;
hblog::BlogTable *tab;
hblog::TagTable *tag;

//全局事件
class dbTestEnv : public testing::Environment {
public:
    //所有方法前执行
    virtual void SetUp() {
        using namespace hblog;
        con = MySQLInit();
        tab = new BlogTable(con);
        tag = new TagTable(con);
    }

    //所有方法后
    virtual void TearDown() {
        using namespace hblog;
        MySQLRelease(con);
        con = nullptr;
        delete tab;
        tab = nullptr;
        delete tag;
        tag = nullptr;
    }
};

/***************
 * TEST Insert
 * content:
 * tag_id:
 * create_time:
 * title:
 ***************/
TEST(dbTestEnv, True) {
    std::vector<Json::Value> vec(20);
    for (int i = 0; i < 10; ++i) {
        vec[i]["title"] = genStr(i * 5);
        vec[i]["tag_id"] = genTagId();
        vec[i]["create_time"] = getTime();
        vec[i]["content"] = genStr(i * 327).c_str();
    }

    EXPECT_TRUE(tab->Insert(vec[0])) << 0;
    EXPECT_TRUE(tab->Insert(vec[1])) << 1;
    EXPECT_TRUE(tab->Insert(vec[2])) << 2;
    EXPECT_TRUE(tab->Insert(vec[3])) << 3;
    EXPECT_TRUE(tab->Insert(vec[4])) << 4;
    EXPECT_TRUE(tab->Insert(vec[5])) << 5;
    EXPECT_TRUE(tab->Insert(vec[6])) << 6;
    EXPECT_TRUE(tab->Insert(vec[7])) << 7;
    EXPECT_TRUE(tab->Insert(vec[8])) << 8;
    EXPECT_TRUE(tab->Insert(vec[9])) << 9;
}

/***************************
 * Test blog select
 * blog_id, Json::Value
 ***************************/
TEST(testBlogSelect, True) {
    std::vector<Json::Value> blg(10);
    EXPECT_TRUE(tab->Select(0, &blg[0])) << 0;
    EXPECT_TRUE(tab->Select(1, &blg[1])) << 1;
    EXPECT_TRUE(tab->Select(2, &blg[2])) << 2;
    EXPECT_TRUE(tab->Select(3, &blg[3])) << 3;
    EXPECT_TRUE(tab->Select(4, &blg[4])) << 4;
    EXPECT_TRUE(tab->Select(5, &blg[5])) << 5;
    EXPECT_TRUE(tab->Select(6, &blg[6])) << 6;
    EXPECT_TRUE(tab->Select(7, &blg[7])) << 7;
    EXPECT_TRUE(tab->Select(8, &blg[8])) << 8;
    EXPECT_TRUE(tab->Select(9, &blg[9])) << 9;
}

TEST(testBlogSelectAll, True) {
    Json::Value v;
    EXPECT_TRUE(tab->SelectAll(&v));
}

/*******************************
 * Test Blog Update
 * title
 * tag_id
 * content
 * blog_id
 *******************************/
TEST(testBlogUpdate, True) {
    Json::Value v;
    v["title"] = "update";
    v["tag_id"] = 1;
    v["content"] = "hhhhhhh";
    v["blog_id"] = 1;
    EXPECT_TRUE(tab->Update(v));
}
/****************************
 * Delete Blog
 ****************************/
TEST(testBlogDelete, True) {
    EXPECT_TRUE(tab->Delete(1)) << 1;
    EXPECT_TRUE(tab->Delete(2)) << 2;
    EXPECT_TRUE(tab->Delete(3)) << 3;
    EXPECT_TRUE(tab->Delete(4)) << 4;
    EXPECT_TRUE(tab->Delete(5)) << 5;
    EXPECT_TRUE(tab->Delete(6)) << 6;
    EXPECT_TRUE(tab->Delete(7)) << 7;
    EXPECT_TRUE(tab->Delete(8)) << 8;
    EXPECT_TRUE(tab->Delete(9)) << 9;
    EXPECT_TRUE(tab->Delete(10)) << 10;
}

/**************************
 * Test Insert Tag
 **************************/
TEST(testTagInsert, True) {
    std::vector<Json::Value> tags(10);
    for (int i = 0; i < 10; ++i) {
        std::string tn;
        for (int i = 0; i < rand() % 10; ++i) {
            tn += (rand() % 65 + 26);
        }
        tags[i]["tag_name"] = tn;
    }

    EXPECT_TRUE(tag->Insert(tags[0])) << 0;
    EXPECT_TRUE(tag->Insert(tags[1])) << 1;
    EXPECT_TRUE(tag->Insert(tags[2])) << 2;
    EXPECT_TRUE(tag->Insert(tags[3])) << 3;
    EXPECT_TRUE(tag->Insert(tags[4])) << 4;
    EXPECT_TRUE(tag->Insert(tags[5])) << 5;
    EXPECT_TRUE(tag->Insert(tags[6])) << 6;
    EXPECT_TRUE(tag->Insert(tags[7])) << 7;
    EXPECT_TRUE(tag->Insert(tags[8])) << 8;
    EXPECT_TRUE(tag->Insert(tags[9])) << 9;
}
/**********************
 * Test Select
 **********************/
TEST(testTagSelect, True) {
    Json::Value v;
    EXPECT_TRUE(tag->SelectAll(&v));
}

/**************************
 * Test Delete Tag
 **************************/
TEST(testTagDelete, True) {
    EXPECT_TRUE(tag->Delete(1)) << 1;
    EXPECT_TRUE(tag->Delete(2)) << 2;
    EXPECT_TRUE(tag->Delete(3)) << 3;
    EXPECT_TRUE(tag->Delete(4)) << 4;
    EXPECT_TRUE(tag->Delete(5)) << 5;
    EXPECT_TRUE(tag->Delete(6)) << 6;
    EXPECT_TRUE(tag->Delete(7)) << 7;
    EXPECT_TRUE(tag->Delete(8)) << 8;
    EXPECT_TRUE(tag->Delete(9)) << 9;
    EXPECT_TRUE(tag->Delete(10)) << 10;
}

#if 1

int main(int argc, char *argv[]) {
    //注册全局事件
    testing::AddGlobalTestEnvironment(new dbTestEnv);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

#endif

//TestSuite
/*
 * class FooTest : public testing::Test{
 *  protected:
 *      static void SetUpTestCase() {
 *      }
 *      static void TearDownTestCase() {
 *      }
 * }
 * TEST_F(FooTest, testname) {}
 */

//TestCase
/*
 * class FooTest : public testing::Test {
 * protected:
 *      virtual void SetUp() {
 *      }
 *      virtual void TearDown() {
 *      }
 * }
 * TEST_F(FooTest, testname) {}
 */

#if 0
//参数化测试
class dbTest : public testing::TestWithParam<std::tuple<int, int>> {

};

TEST_P(dbTest, insert) {
    std::tuple<int, int> t = GetParam();
}

std::vector<int> arr{1,2,3,4};
//测试数据的范围
INSTANTIATE_TEST_CASE_P(
        InsertTest,
        dbTest,
        testing::Combine(testing::ValuesIn(arr), testing::ValuesIn(arr))
);
#endif