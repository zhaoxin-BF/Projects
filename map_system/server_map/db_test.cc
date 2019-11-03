#include "db.hpp"
#include <string>
using namespace std;
//单元测试，根据模块或者函数来进行测试，一般是写完一个封装函数或者模块来进行测试
//使用这个来测试刚才封装的 MySQL 操作是否正确

void TestShopTable(){
  //json序列化
  Json::StyledWriter writer;

  //初始化建立数据库连接
  MYSQL* mysql = map_system::MySQLInit();
  //创建博客系统中，博客表操作对象
  map_system::ShopTable shop_table(mysql);

  //1、测试插入
  Json::Value shop;
  shop["name"] = "一号店";
  shop["address"] = "甘肃省张掖市张掖市";
  shop["lng"] = 100.456409;
  shop["lat"] = 38.932065;
  shop["range_meter"] = 1000;
  shop["color"] = "green";
  
  bool ret = shop_table.Insert(shop);
  printf("insert:%d\n",ret);

  //2、测试查找
  Json::Value shops;
  int rets = shop_table.SelectAll(&shops);
  printf("Select all %d\n", rets);
  //json序列化输出 writer 定义在该文件头部，请参考
  printf("%s\n", writer.write(shops).c_str());

  //3、测试更新
  shop["range_meter"] = 500;
  shop["color"] = "yellow";
  shop["shop_id"] = 1;
  bool retU = shop_table.Upadte(shop);
  printf("update shop %d 成功！%d\n",shop["shop_id"].asInt(), retU);

  //4、测试删除
  bool retD = shop_table.Delete(5);
  printf("delete %d\n", retD);

  map_system::MYSQLRelease(mysql);
}


int main()
{
  TestShopTable();
  return 0;
}
