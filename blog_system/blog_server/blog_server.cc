//本文件为博客服务器的核心文件

#include "httplib.h"
#include "db.hpp"
#include <signal.h>
MYSQL* mysql = NULL;

int main() {
  using namespace httplib;
  using namespace blog_system;
  //命名空间在函数中使用，其作用域为函数花括号内
  //不建议在头文件中使用
  //需全局使用则放在函数体外
  
  //1、与数据库建立连接
  mysql = blog_system::MySQLInit();
  signal(SIGINT,[](int){blog_system::MYSQLRelease(mysql); exit(1);});
  //ctrl + c 出发sigint信号，当按下ctrl+c的时候，数据库就会退出
  
  //2、创建相关的操作对像
  BlogTable blog_table(mysql);
  TagTable blod_table(mysql);

  //3、创建服务器，并设置"路由"，这里的"路由"功能是把方法和path映射到对应的处理函数中去
  Server server;
  
  // 下面不是由常见的函数进行业务操作，
  // 而是用C++11 中的新特性lambda表答式来进行操作
  
  //业务一：新增一篇博客
  server.Post("/blog", [&blog_table](const Request& req, Response&  resp)
  {
    //这里blog_table直接传入用到了lambda的捕捉方式，避免了对象拷贝的操作
    printf("新增一篇博客！\n");

    //1.获取请求中的body(数据)并解析成json
    Json::Reader reader;
    //先用json reader把字符串转成json对象，然后用json writer把json对象转换成字符串。最后用reader.parse进行解析
    //传过来的body中是字符串，而我们需要的是json的对象。所以我们需要做的是先把字符串转换成json格式的
    Json::FastWriter writer;
    Json::Value req_json;
    Json::Value resp_json;
    bool ret = reader.parse(req.body, req_json);
    if(!ret)
    {
      //解析出错，给出用户提示
      printf("解析请求失败！%s\n",req.body.c_str());
      //打印出解析失败的数据是什么样的
      //构造一个响应对象，告诉客户端出错了
      resp_json["ok"] = false;
      resp_json["reason"] = "input data parse error!";
      resp.status = 400;
      //设置好返回值以及对象类型，json返回的格式是固定的applicant/json 
      
      resp.set_content(writer.write(resp_json),"application/json");
      return;
    }

    //2、对用户传过来的参数进行校验
    if(req_json["title"].empty() || req_json["content"].empty() || req_json["create_time"].empty() || req_json["tag_id"].empty()){
      printf("请求数据格式有错！%s\n",req.body.c_str());
      resp_json["ok"] = false;
      resp_json["reason"] = "Request data format error!";
      resp.status = 400;
      resp.set_content(writer.write(resp_json),"application/json");
      return;
    }
      //3、当用户输入正确的操作时。
      ret = blog_table.Insert(req_json);
      if(!ret){
        printf("插入博客失败！\n");
        resp_json["ok"] = false;
        resp_json["reason"] = "blog insert faild!";
        resp.status = 500;
        //500状态码表示服务器遇到了一个未曾预料到的状况
        resp.set_content(writer.write(resp_json),"application/json");
        return;
      }

      //4、数据没有问题，构建一个正确的响应发送给客户端
      printf("插入成功！\n");
      resp_json["ok"] = true;
      resp_json["reason"] = "Success!";
      resp.status = 200;
      resp.set_content(writer.write(resp_json),"application/json");
      return;
    });
  server.listen("0.0.0.0",8888);
  return 0;
}

