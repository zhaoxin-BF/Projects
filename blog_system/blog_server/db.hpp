////////////////////////
//创建封装数据库的类
////////////////////////

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <mysql/mysql.h>
#include <jsoncpp/json/json.h>


namespace blog_system{
//函数一“与MySQL建立连接
static MYSQL* MySQLInit(){
  //初始化一个MySQL句柄并建立连接
  //1.创建连接句柄
  MYSQL* connect_fd = mysql_init(NULL);
  //2.和数据库建立连接
  //vim 中打开其他文件操作 esc-tabe path  然后保存path文件就回到源文件了      eg:tabe ../mysql/insert.cc   :w(退出保存返回)
	if(!mysql_real_connect(connect_fd,"127.0.0.1",
		"root","Zhaoxin..521","blog_system",3306,NULL,0))
    {
		  printf("连接失败，%s\n",mysql_error(connect_fd));
		  // mysql_error can tell u why connections failed
		  // this fun can translate the error code into a paragraph. 
		  return NULL;
    }

	// 3.设置编码方式
	// mysql server 部分在最初安装的时候已经设置成了utf8
	// 也得在客户端设置成utf8
  mysql_set_character_set(connect_fd,"utf8");
  return connect_fd;
}

//函数二：释放MySQL连接
static void MYSQLRelease(MYSQL* connect_fd){
    mysql_close(connect_fd);
}

//////////////////////////////////
//创建一个类，用于操作博客表的类
class BlogTable{
public:
  BlogTable(MYSQL* connect_fd):mysql_(connect_fd){

  //通过这个构造函数获取到一个 数据库 的操作句柄
  }
  //以下操作相关数据都统一使用json的方式
  //Json::Value jsoncpp 中最核心的类
  //Json::Value 就表示一个具体的json对象
  //形如
  //{
  //  title:"博客标题"，
  //  content:"博客正文"，
  //  create_time:"博客创建时间"，
  //  tag_id:"标签id",
  //}
  //用json当参数，最大的好处就是方便扩展
  bool Insert(const Json::Value& blog){
    const std::string& content = blog["content"].asCString();
    //为什么to的长度是2*size + 1 这是文档要求
    //char* to = new char[content.size() * 2 + 1];
    
    // 智能指针 方便释放内存。
    std::unique_ptr<char> to(new char[content.size() * 2 + 1]);
    mysql_real_escape_string(mysql_,to.get(),    
        content.c_str(), content.size());
    
    //核心就是拼装SQL语句1024*100(100字节)
    //智能指针，动态开辟空间，方便扩容(因为不知道文章会是多大)
    std::unique_ptr<char> sql(new char[content.size()*2 + 4096]);

    sprintf(sql.get(),"insert into blog_table values(null,'%s','%s',%d,'%s')",
        blog["title"].asCString(),
        to.get(),
        blog["tag_id"].asInt(),
        blog["create_time"].asCString());
    
    int ret = mysql_query(mysql_, sql.get());
    if(ret != 0){
      printf("执行插入博客失败！%s\n",mysql_error(mysql_));
      return false;
    }
    
    printf("执行插入博客成功！\n");
    return true;
  }

  //blogs 作为一个输出型参数。
  bool SelectAll(Json::Value* blogs, const std::string& tag_id = ""){
    //查找不需要 拼接很长的字符串
    char sql[1024*4] = {0};
    if(tag_id == "")
    {
      //此时不需要按照tag来筛选结果
      sprintf(sql,"select blog_id,title,tag_id,create_time from blog_table");
    } else{
      //此时需要按tag来筛选 
      sprintf(sql,"select blog_id,title,tag_id,create_time from blog_table where tag_id = %d",std::stoi(tag_id));//string 转 int 
    }

    int ret = mysql_query(mysql_, sql);
    if(ret != 0)
    {
      printf("执行查找所有博客失败！%s\n",mysql_error(mysql_));
      return false;
    }


    printf("执行查找所有博客成功！\n");

    //获取结果集合，然后把结果写到blogs参数中，返回给调用者
    MYSQL_RES* result = mysql_store_result(mysql_);
    //获取多少行数据
    int rows = mysql_num_rows(result);
    //遍历每一行数据
    for(int i = 0; i < rows; ++i){
      //取出一行
      MYSQL_ROW row = mysql_fetch_row(result);
      Json::Value blog;
      //row[] 中的下标和上面的 select 语句中写的列的顺序是相关的
      blog["blog_id"] = atoi(row[0]);
      blog["title"] = row[1];
      blog["tag_id"] = atoi(row[2]);
      blog["create_time"] = row[3];
      blogs->append(blog);
    }
    //result结果集需要及时释放
    mysql_free_result(result);
    printf("执行查找所有博客成功！共查到 %d 条博客\n",rows);
    return true;
  }
  
  //blog 同样是输出型参数，根据当前的blog_id 在数据库中找到具体的blog
  //博客内容通过blog 参数返回给调用者
  bool SelectOne(int32_t blog_id, Json::Value* blog){
    char sql[1024] = {0};
    sprintf(sql,"select blog_id,title,content,tag_id,create_time from blog_table where blog_id = %d",blog_id);
    int ret = mysql_query(mysql_, sql);
    if(ret != 0) {
      printf("执行查找博客失败！ %s\n", mysql_error(mysql_));
      return false;
    }
    MYSQL_RES* result=mysql_store_result(mysql_);
    int rows = mysql_num_rows(result);
    if(rows != 1){
      printf("查找到的博客不是只有一条，实际有%d条！\n",rows);
      return false;
    }

    MYSQL_ROW row = mysql_fetch_row(result);
    (*blog)["blog_id"] = atoi(row[0]);
    (*blog)["title"] = row[1];
    (*blog)["content"] = row[2];
    (*blog)["tag_id"] = atoi(row[3]);
    (*blog)["create_time"] = row[4];
    printf("查到一条博客\n");
    
    return true;
  }


  bool Upadte(const Json::Value& blog){  
    const std::string& content = blog["content"].asCString();
    //为什么to的长度是2*size + 1 这是文档要求
    //char* to = new char[content.size() * 2 + 1];
    // 智能指针 方便释放内存。
    std::unique_ptr<char> to(new char[content.size() * 2 + 1]);
    mysql_real_escape_string(mysql_,to.get(),content.c_str(), content.size());
    //核心就是拼装SQL语句1024*100(100字节)
    //智能指针，动态开辟空间，方便扩容(因为不知道文章会是多大)
    std::unique_ptr<char> sql(new char[content.size() * 2 + 4096]);

//sprintf(sql.get(),"upadte blog_table set title = '%s',content='%s',tag_id=%d where blog_id = %d",blog["title"].asCString(),to.get(),blog["tag_id"].asInt(),blog["blog_id"].asInt());

sprintf(sql.get(),"update blog_table set title = '%s',content='%s',tag_id=%d where blog_id = %d",blog["title"].asCString(),to.get(),blog["tag_id"].asInt(),blog["blog_id"].asInt());
    int ret = mysql_query(mysql_, sql.get());
    if(ret != 0){
      printf("更新博客失败！ %s\n", mysql_error(mysql_));
      return false;
    }

    printf("更新博客成功！\n");
    return true;
  }

  bool Delete(int32_t blog_id){
    char sql[1024 * 4] = {0};
    sprintf(sql, "Delete from blog_table where blog_id = %d",
        blog_id);
    int ret = mysql_query(mysql_, sql);
    if(ret != 0){
      printf("删除失败！%s\n",mysql_error(mysql_));
      return false;
    }

    printf("删除成功！\n");
    return true;
  }
private:
  MYSQL* mysql_;
};

class TagTable{ 
public:
  TagTable(MYSQL* mysql):mysql_(mysql){}

  bool Insert(const Json::Value& tag){
    char sql[1024 * 4];
    sprintf(sql,"Insert into tag_table values(null,'%s')",
        tag["tag_name"].asCString());
    int ret = mysql_query(mysql_,sql);
    if(ret != 0){
      printf("插入标签失败！ %s\n", mysql_error(mysql_));
      return false;
    }

    printf("插入标签成功！\n");
    return true;
  }

  bool Delete(int32_t tag_id){
    char sql[1024 * 4];
    sprintf(sql,"Delete from tag_table where tag_id=%d",tag_id);

    int ret = mysql_query(mysql_, sql);
    if(ret != 0)
    {
      printf("删除标签失败！%s\n",mysql_error(mysql_));
      return false;
    }

    printf("删除标签成功！\n");
    return true;
  }

  bool SelectAll(Json::Value* tags){
    char sql[1024 * 4];
    sprintf(sql,"select * from tag_table;");

    int ret = mysql_query(mysql_, sql);
    if(ret != 0){
      printf("查找标签失败！%s\n",mysql_error(mysql_));
      return false;
    }
    MYSQL_RES* result = mysql_store_result(mysql_);
    int rows = mysql_num_rows(result);
    for(int i = 0;i < rows; ++i){
      MYSQL_ROW row = mysql_fetch_row(result);
      Json::Value tag;
      tag["tag_id"] = atoi(row[0]);
      tag["rag_name"] = row[1];
      tags->append(tag);
    }

    printf("查找标签成功！\n");
    return true;
  }
private:
  MYSQL* mysql_;
};

}//end blog_system 





