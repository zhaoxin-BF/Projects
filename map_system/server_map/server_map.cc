//本文件为地图服务器的核心文件

#include "httplib.h"
#include "db.hpp"
#include <signal.h>
MYSQL* mysql = NULL;

int main() {
  using namespace httplib;
  using namespace map_system;
  
  //1、与数据库建立连接
  mysql = map_system::MySQLInit();
  signal(SIGINT,[](int){map_system::MYSQLRelease(mysql); exit(1);});

  //2、创建相关的操作对象
  ShopTable shop_table(mysql);

  //3、创建服务器，并设置"路由"，这里的"路由"功能是把方法和path映射到对应的处理函数中去
  Server server;
  
  //业务一：新增店铺信息
  
  server.Post("/shop",[&shop_table](const Request& req, Response& resp )
      {
        Json::Reader reader;

        Json::FastWriter writer;
        Json::Value req_json;
        Json::Value resp_json;

        //1、解析数据
        bool ret = reader.parse(req.body, req_json);
        if(!ret)
      {
          //解析出错，打印给出提示
          printf("解析请求失败！%s\n",req.body.c_str());
          resp_json["ok"] = false;
          resp_json["reason"] = "input data parse error!";
          resp.status = 400;
          //设置好返回值及对象，json返回的格式是固定的applicant/Json 
          resp.set_content(writer.write(resp_json),"application/json");
          return ;
        }
        
        //2、判断数据类型,进行校验
        if(req_json["name"].empty() || req_json["address"].empty() || 
            req_json["lng"].empty() || req_json["lat"].empty() ||
            req_json["range_meter"].empty() || req_json["color"].empty()){
          printf("请求数据格式有误%s\n",req.body.c_str());
          resp_json["ok"] = false;
          resp_json["reason"] = "Request data format error!";
          resp.status = 400;

          resp.set_content(writer.write(resp_json), "application/json");
          return;
        }

        //3、判断插入正确与否
        ret = shop_table.Insert(req_json);
        if(!ret)
        {
          printf("插入店铺信息失败");
          resp_json["ok"] = false;
          resp_json["reason"] = "shop insert error!";
          resp.status = 500;

          resp.set_content(writer.write(resp_json), "application/json");
          return;
        }
        
        //4、构造响应
        printf("插入成功！\n");
        resp_json["ok"] = true;
        resp_json["reason"] = "Success!";
        resp.status = 200;
        resp.set_content(writer.write(resp_json),"application/json");
        return;
      });
  //业务二：查找所有店铺信息
  server.Get("/shop",[&shop_table](const Request& req, Response& resp)
      {
        printf("查看所有店铺信息！");

        Json::FastWriter writer;
        Json::Value req_json;
        Json::Value resp_json;

        //调用数据库操作查询函数获取结果
        bool ret = shop_table.SelectAll(&req_json);
        if(!ret)
        {
          printf("查找店铺信息失败！\n");
          resp_json["ok"] = false;
          resp_json["reason"] = "SelectAll faild unexcept problem!";
          resp.status = 500;

          resp.set_content(writer.write(resp_json),"application/json");
          return;
        }

        //正确查询结果，返回客户端
        resp_json["ok"] = true;
        resp.status = 200;
        resp.set_content(writer.write(req_json), "application/json");
        return;
      });

  //业务三：修改店铺信息
  server.Put(R"(/shop/(\d+))", [&shop_table](const Request& req, Response& resp){
      //获取店铺的id  
      int32_t shop_id = std::stoi(req.matches[1].str());

      printf("修改id为%d 的店铺信息\n", shop_id);
      //2.获取到请求并解析结果
      Json::Reader reader;
      Json::FastWriter writer;
      Json::Value req_json;
      Json::Value resp_json;

      bool ret = reader.parse(req.body, req_json);
      if(!ret)
      {
          printf("解析请求失败\n");
          resp_json["ok"] = false;
          resp_json["reason"] = " update parse error!";
          resp.status = 400;
          //设置好返回值及对象，json返回的格式是固定的applicant/Json 
          resp.set_content(writer.write(resp_json),"application/json");
          return ;
      }

      //3、检验数据是否合格
      if(req_json["range_meter"].empty() || req_json["color"].empty())
      {

          resp_json["ok"] = false;
          resp_json["reason"] = "Upadte data format error!";
          resp.status = 400;
          //设置好返回值及对象，json返回的格式是固定的applicant/Json 
          resp.set_content(writer.write(resp_json),"application/json");
          return ;
      }
      //4、调用数据库来完成更新操作
      req_json["shop_id"] = shop_id;
      ret = shop_table.Upadte(req_json);
      if(!ret)
      {

          resp_json["ok"] = false;
          resp_json["reason"] = "Upadte data database error!";
          resp.status = 400;
          //设置好返回值及对象，json返回的格式是固定的applicant/Json 
          resp.set_content(writer.write(resp_json),"application/json");
          return ;
      }
      //5、构造正确的响应
      
      resp_json["ok"] = true;
      resp_json["reason"] = "Success!";
      resp.status = 200;
      //设置好返回值及对象，json返回的格式是固定的applicant/Json 
      resp.set_content(writer.write(resp_json),"application/json");
      return ;
      });

  //业务四：删除店铺信息
  server.Delete(R"(/shop/(\d+))",[&shop_table](const Request& req, Response& resp){
        Json::Value resp_json;
        Json::FastWriter writer;

        //获取到shop_id
        int32_t shop_id = std::stoi(req.matches[1].str());
        printf("删除id为%d的店铺信息\n", shop_id);

        //2、调用数据库操作
        bool ret = shop_table.Delete(shop_id);
        if(!ret)
        {
          resp_json["ok"] = false;
          resp_json["reason"] = "Delete database error!";
          resp.status = 500;
          //设置好返回值及对象，json返回的格式是固定的applicant/Json 
          resp.set_content(writer.write(resp_json),"application/json");

          return;
        }

        
        resp_json["ok"] = true;
        resp_json["reason"] = "Success!";
        resp.status = 200;
        //设置好返回值及对象，json返回的格式是固定的applicant/Json 
        resp.set_content(writer.write(resp_json),"application/json");
        return;      
      });

  server.set_base_dir("/root/Projects/map_system/client_map");//设置基本目录
  server.listen("0.0.0.0",8080);//监听8080店口
  return 0;
}

