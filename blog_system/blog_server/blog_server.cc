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
  TagTable tag_table(mysql);

  //3、创建服务器，并设置"路由"，这里的"路由"功能是把方法和path映射到对应的处理函数中去
  Server server;
  
  // 下面不是由常见的函数进行业务操作，
  // 而是用C++11 中的新特性lambda表答式来进行操作
  
//业务一：新增一篇博客(Postman 检验合格)
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
// 业务二：查看所有博客(Postman 测试合格)
server.Get("/blog",[&blog_table](const Request& req, Response& resp)
    {
      printf("查看所有博客。\n");
      //1.尝试获取所有博客
      const std::string& tag_id = req.get_param_value("tag_id",0);

      //获取某个键位相对应的值位，这里的key就是tag_id.值位就是id值了
      //这里还有一种就是tag_id参数为空的情况，这种场景下就直接返回所有博客就可以了
      //也不需要在进行什么合法性的判定
      Json::FastWriter writer;
      Json::Value req_json;
      Json::Value resp_json;

      //调用数据库操作来获取所有博客查询结果
      bool ret = blog_table.SelectAll(&req_json, tag_id);
      if(!ret)
      {
        printf("查找博客失败！\n");
        resp_json["ok"] = false;
        resp_json["reason"] = "SelecteAll failed unexcept problem!";
        resp.status = 500;
        resp.set_content(writer.write(resp_json),"application/json");
        return ;
      }

      //查询正确，构造正确的响应结果发送给客户端
      resp_json["ok"] = true;
      resp.status = 200;
      resp.set_content(writer.write(req_json),"application/json");
      return ;
    });

//业务三：查看某个博客情况(Postman 检测合格)
server.Get(R"(/blog/(\d+))", [&blog_table](const Request& req, Response& resp){
      Json::FastWriter writer;
      //1.解析到过去的blog_id
      int32_t blog_id = std::stoi(req.matches[1].str());
      printf("查id 为%d 的一篇博客\n", blog_id);

      //直接进行数据库操作
      Json::Value resp_json;
      bool ret = blog_table.SelectOne(blog_id, &resp_json);
      if(!ret)
      {
        resp_json["ok"] = false;
        resp_json["reason"] = "查看特定博客失败："+std::to_string(blog_id);
        resp.status = 404;
        resp.set_content(writer.write(resp_json), "application/json");
        return ;
      }

      //一个正确的响应发送给客户端
      resp_json["ok"] = true;
      resp_json["reason"] = "Success!";
      resp.status = 200;
      resp.set_content(writer.write(resp_json), "application/json");
      
      return ;
    });

//业务四：修改某个博客  （Postman 检验合格）
server.Put(R"(/blog/(\d+))", [&blog_table](const Request& req, Response& resp){
    //1.获取修改博客的id
    int32_t  blog_id = std::stoi(req.matches[1].str());
    printf("修改id为%d的博客！\n", blog_id);
    //2.获取到的请求并解析结果
    Json::Reader reader;
    Json::FastWriter writer;
    Json::Value req_json;
    Json::Value resp_json;
    
    bool ret = reader.parse(req.body, req_json);
    if(!ret)
    {
      printf("解析请求失败！%s\n",req.body.c_str());
      resp_json["ok"] = false;
      resp_json["reason"] = "update blog parse Request failed!";
      resp.status = 400;
      resp.set_content(writer.write(resp_json), "application/json");
      return ;
    }
      //3.检验三是否符合预期
      if(req_json["title"].empty() || req_json["content"].empty() || req_json["tag_id"].empty())
      {
        resp_json["ok"] = false;
        resp_json["reason"] = "Upate blog Request format error!";
        resp.status = 400;
        resp.set_content(writer.write(resp_json), "application/json");
        return ;
      }

      //4.调用数据库来完成正确操作
      req_json["blog_id"] = blog_id;
      ret = blog_table.Upadte(req_json);
      if(!ret)
      {
        printf("修改id为 %d 的博客失败！\n", blog_id);
        resp_json["ok"] = false;
        resp_json["reason"] = "update blog database failed!";
        resp.status = 500;
        resp.set_content(writer.write(resp_json), "application/json");

        return ; 
      }

      //构造一个正确的响应结果
      resp_json["ok"] = true;
      resp_json["reason"] = "Success!";
      resp.status = 200;
      resp.set_content(writer.write(resp_json), "application/json");
      
      return ;
    });
//业务五：删除博客(Postman 检验合格)
server.Delete(R"(/blog/(\d+))", [&blog_table](const Request& req, Response& resp){
      Json::Value resp_json;
      Json::FastWriter writer;

      //获取到blog_id
      int32_t blog_id = std::stoi(req.matches[1].str());
      printf("删除id为%d的博客\n", blog_id);

      //2.调用数据库操作
      bool ret = blog_table.Delete(blog_id);
      if(!ret)
      {
        printf("删除博客%d失败\n", blog_id);

        resp_json["ok"] = false;
        resp_json["reason"] = "Delete blog failed!";
        resp.status = 500;
        resp.set_content(writer.write(resp_json), "application/json");

        return ;
      }

      resp_json["ok"] = true;
      resp_json["reason"] = "Success!";
      resp.status = 200;
      resp.set_content(writer.write(resp_json), "application/json");
      return ;
  });

//标签业务一：增加标签(Postman 检验合格)
  server.Post("/tag", [&tag_table](const Request& req, Response& resp)
      {
       Json::FastWriter writer;
       Json::Reader reader;
       Json::Value req_json;
       Json::Value resp_json;
        
       //解析数据
       bool ret = reader.parse(req.body, req_json);
       if(!ret)
       {
          printf("数据解析失败！\n");
          resp_json["ok"] = false;
          resp_json["reason"] = "数据解析失败！";
          resp.status = 400;
          resp.set_content(writer.write(resp_json), "application/json");

          return ;
       }
        //2.对数据进行校验
        if(req_json["tag_name"].empty())
        {
          printf("标签名字为空，新增失败！\n");
          resp_json["ok"] = false;
          resp_json["reason"] = "数据校验不通过！";
          resp.status = 400;
          resp.set_content(writer.write(resp_json), "application/json");

          return ;
        }
        //3.进行数据插入操作
        ret = tag_table.Insert(req_json);
        if(!ret)
        {
          printf("新增标签插入数据库失败!\n");
          resp_json["ok"] = false;
          resp_json["reason"] = "insert into database failed!";
          resp.status = 500;
          resp.set_content(writer.write(resp_json), "application/json");
          return ;
        }
        //4.构造正确响应结果给客户端
        resp_json["ok"] = true;
        resp.status = 200;
        resp.set_content(writer.write(resp_json), "application/json");

        return;
      });
//标签业务二：删除标签(Postman 检验合格)
  server.Delete(R"(/tag/(\d+))",[&tag_table](const Request& req, Response& resp){
        Json::FastWriter writer;
        Json::Value resp_json;
        //1.解析出tag_id
        int32_t tag_id = std::stoi(req.matches[1].str());
        printf("删除id为%d的标签！\n" , tag_id);
        
        //2.执行删除操作
        bool ret = tag_table.Delete(tag_id);
        if(!ret)
        {
          printf("执行删除标签%d\n失败",tag_id);
          resp_json["ok"] = false;
          resp_json["reason"] = "Insert tag database faild!";
          resp.status = 500;
          resp.set_content(writer.write(resp_json), "application/json");
          return ;
        }

        //3.发送正确响应给客户端
        resp_json["ok"] = true;
        resp_json["reason"] = "Success!";
        resp.set_content(writer.write(resp_json),"application/json");
        return ;
      });
//标签业务五：查看所有标签
  server.Get("/tag", [&tag_table](const Request& req, Response& resp){
        //1.不需要解析参数，直接返回查询结果就OK了
        Json::FastWriter writer;
        Json::Value resp_json;
        bool ret = tag_table.SelectAll(&resp_json);
        if(!ret)
        {
          printf("查询标签结果失败！\n");
          resp_json["ok"] = false;
          resp_json["reason"] = "insert into database failed!";
          resp.status = 500;
          resp.set_content(writer.write(resp_json), "application/json");
          return ;
        }

        //2.构造正确响应发送给客户端
        //resp_json["ok"] = true;
        //resp_json["reason"] = "Success!";
        //resp.status = 200;
        resp.set_content(writer.write(resp_json), "application/json");
        return ;
      });
  server.set_base_dir("/root/wwwroot");
  server.listen("0.0.0.0",8888);
  return 0;
}

