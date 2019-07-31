#define __NO__LOG__

#include <tuple>

#include <unistd.h>
#include <fcntl.h>

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <jsoncpp/json/json.h>

#include "../src/server/db.hpp"
#include "testUtils.hpp"

MYSQL *con;
hblog::BlogTable *tab;
hblog::TagTable *tag;

//全局事件
class dbTestEnv : public testing::Environment {
public:
    //所有方法前执行
    virtual void SetUp() {
        srand(time(0));
        using namespace hblog;
        con = MySQLInit("testconfig.json");
        if (con == NULL) {
            std::cout << "connect db err" << std::endl;
            exit(1);
        }
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
TEST(TestBlogInsert, True) {
    std::vector<Json::Value> vec(10);
    for (int i = 0; i < 10; ++i) {
        vec[i]["title"] = genStr(i * 3 + 1);
        vec[i]["tag_id"] = genTagId();
        vec[i]["create_time"] = getTime();
        vec[i]["content"] = genStr(1 + i * 55);

        EXPECT_TRUE(tab->Insert(vec[i])) << i;
    }
}

/***************************
 * Test blog select
 * blog_id, Json::Value
 ***************************/
TEST(testBlogSelect, True) {
    std::vector<Json::Value> blg(10);
    for (int i = 0; i < 3; ++i) {
        EXPECT_TRUE(tab->Select(i + 1, &blg[i])) << i;
    }
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
    for (int i = 0; i < 10; ++i) {
        EXPECT_TRUE(tab->Delete(i + 1)) << i + 1;
    }
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

        EXPECT_TRUE(tag->Insert(tags[i])) << i;
    }
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
    for (int i = 0; i < 10; ++i) {
        EXPECT_TRUE(tag->Delete(i + 1)) << i + 1;
    }
}

#if 0
class BlogTableMock : public hblog::BlogTable {
public:
    MOCK_METHOD1(Insert, bool(const Json::Value&));
};

TEST(Mtest, insert) {
    BlogTableMock btm;
    EXPECT_CALL(btm, Insert(_)).WillRepeatedly(Return(true));
}

#endif


/*
 * ********************** mian ******
 */

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