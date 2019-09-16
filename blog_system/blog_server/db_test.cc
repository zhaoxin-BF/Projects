#include "db.hpp"
#include <string>
using namespace std;
//单元测试，根据模块或者函数来进行测试，一般是写完一个封装函数或者模块来进行测试
//使用这个来测试刚才封装的 MySQL 操作是否正确

void TestBlogTable(){
  //json序列化
  Json::StyledWriter writer;

  //初始化建立数据库连接
  MYSQL* mysql = blog_system::MySQLInit();
  //创建博客系统中，博客表操作对象
  blog_system::BlogTable blog_table(mysql);

  //1、测试插入
  Json::Value blog;
  //blog["title"] = "我的第一篇博客";
  //blog["content"] = "相信你自己，you can do it!";
  //blog["tag_id"] = 1;
  //blog["create_time"] = "2019/08/31";
  //bool ret = blog_table.Insert(blog);
  //printf("insert:%d\n",ret);
  
  //2、测试查找
  //Json::Value blogs;
  //int ret = blog_table.SelectAll(&blogs);
  //printf("Select all %d\n", ret);
  //json序列化输出 writer 定义在该文件头部，请参考
  //printf("%s\n", writer.write(blogs).c_str());

  //2_1、测试根据标签查找
  Json::Value blogs;
  string tag_id="1";
  int ret = blog_table.SelectAll(&blogs, tag_id);
  printf("Select all %d\n", ret);
  //json序列化输出 writer 定义在该文件头部，请参考
  printf("%s\n", writer.write(blogs).c_str());
  

  //3、测试单个查找
  //int ret = blog_table.SelectOne(1,&blog);
  //printf("select one %d\n",ret);
  //printf("blog : %s",writer.write(blog).c_str());

  //4、测试修改博客
  //blog["title"] = "第一篇修改的博客";
  //blog["content"] = "相信你自己,You can do it!";
  //blog["tag_id"] = 1;
  //blog["blog_id"] = 1;
  //bool ret = blog_table.Upadte(blog);
  //printf("update blog %d 成功！\n",blog["blog_id"].asInt());

  //5、测试删除博客
  //bool ret = blog_table.Delete(5);
  //printf("delete %d\n", ret);

  blog_system::MYSQLRelease(mysql);
}

void TestTagTable()
{
  MYSQL* mysql = blog_system::MySQLInit();
  blog_system::TagTable tag_table(mysql);
  
  //1、测试标签插入
  //Json::Value tag;
  //tag["tag_name"] = "C++语言";
  //bool ret = tag_table.Insert(tag);
  //printf("insert %d\n", ret);

  //2、测试标签删除
  //bool ret = tag_table.Delete(4);
  //printf("delete tag %d\n", ret);


  //3、测试标签查找
  //4、Json序列化 
  //Json::StyledWriter writer;
  //Json::Value tags;
  //bool ret = tag_table.SelectAll(&tags);
  //printf("SelectAll %d\n", ret);
  //printf("%s\n", writer.write(tags).c_str());

  blog_system::MYSQLRelease(mysql);
}

int main()
{
  TestBlogTable();
  //TestTagTable();
  return 0;
}
