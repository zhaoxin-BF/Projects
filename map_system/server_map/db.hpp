////////////////////////
//创建封装数据库的类
////////////////////////

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <mysql/mysql.h>
#include <jsoncpp/json/json.h>


namespace map_system{
  //函数一“与MySQL建立连接
static MYSQL* MySQLInit(){
  //初始化一个MySQL句柄并建立连接
  //1.创建连接句柄
  MYSQL* connect_fd = mysql_init(NULL);
  //2.和数据库建立连接
  //vim 中打开其他文件操作 esc-tabe path  然后保存path文件就回到源文件了      eg:tabe ../mysql/insert.cc   :w(退出保存返回)
	if(!mysql_real_connect(connect_fd,"127.0.0.1",
		"root","Zhaoxin..521","map_system",3306,NULL,0))
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
//创建一个类，用于操作shop表的类
class ShopTable{
public:
  ShopTable(MYSQL* connect_fd):mysql_(connect_fd){

  //通过这个构造函数获取到一个 数据库 的操作句柄
  }
  //以下操作相关数据都统一使用json的方式
  //Json::Value jsoncpp 中最核心的类
  //Json::Value 就表示一个具体的json对象
  //形如
  //{
  //  name:"店铺名称_临潼校区店"，
  //  address:"店铺地址_西安市临潼区西安工程大学(临潼校区)"，
  //  lng:"经度坐标"，
  //  lat:"纬度坐标",
  //  range_meter:"范围_500米的范围内",
  //  color:"画圈的颜色_green",  
  //}
  //用json当参数，最大的好处就是方便扩展
  
  //一、插入店铺信息
  bool Insert(const Json::Value& shop){
    char sql[1024 * 4];
    sprintf(sql,"insert into shop_table values(null,'%s','%s',%9.6lf,%9.6lf,%d,'%s')",
        shop["name"].asCString(),
        shop["address"].asCString(),
        shop["lng"].asDouble(),
        shop["lat"].asDouble(),
        shop["range_meter"].asInt(),
        shop["color"].asCString());
    
    int ret = mysql_query(mysql_, sql);
    if(ret != 0){
      printf("执行插入位置失败！%s\n",mysql_error(mysql_));
      return false;
    }
    
    printf("执行插入位置成功！\n");
    return true;
  }

  //二、查询所有店铺信息
  //blogs 作为一个输出型参数。
  bool SelectAll(Json::Value* shops){
    //查找不需要 拼接很长的字符串
    char sql[1024*4] = {0};
    
    sprintf(sql,"select shop_id,name,address,lng,lat,range_meter,color from shop_table");

    int ret = mysql_query(mysql_, sql);
    if(ret != 0)
    {
      printf("执行查找所有shop失败！%s\n",mysql_error(mysql_));
      return false;
    }


    printf("执行查找所有shop成功！\n");

    //获取结果集合，然后把结果写到blogs参数中，返回给调用者
    MYSQL_RES* result = mysql_store_result(mysql_);
    //获取多少行数据
    int rows = mysql_num_rows(result);
    //遍历每一行数据
    for(int i = 0; i < rows; ++i){
      //取出一行
      MYSQL_ROW row = mysql_fetch_row(result);
      Json::Value shop;
      //row[] 中的下标和上面的 select 语句中写的列的顺序是相关的
      shop["shop_id"] = atoi(row[0]);
      shop["name"] = row[1];
      shop["address"] = row[2];
      shop["lng"] = atof(row[3]);
      shop["lat"] = atof(row[4]);
      shop["range_meter"] = atoi(row[5]);
      shop["color"] = row[6];
      shops->append(shop);// 添加到传入传出参数blogs中
    }
    //result结果集需要及时释放
    mysql_free_result(result);
    printf("执行查找所有店铺成功！共查到 %d 个店铺\n",rows);
    return true;
  }
  
  //三、更新某一店铺信息
  bool Upadte(const Json::Value& shop){  
    char sql[1024 * 4];
    sprintf(sql,"update shop_table set range_meter = %d,color='%s' where shop_id = %d",shop["range_meter"].asInt(),shop["color"].asCString(),shop["shop_id"].asInt());
    int ret = mysql_query(mysql_, sql);
    if(ret != 0){
      printf("更新距离信息失败！ %s\n", mysql_error(mysql_));
      return false;
    }

    printf("更新距离信息成功！\n");
    return true;
  }

  //五、删除某一店铺
  bool Delete(int32_t shop_id){
    char sql[1024 * 4] = {0};
    sprintf(sql, "Delete from shop_table where shop_id = %d",shop_id);
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

}//end blog_system 





