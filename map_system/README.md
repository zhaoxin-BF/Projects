# 百度地图二次开发——数据库设计
## drop database if exists map_system;
## create database map_system;
## use map_system;
## drop table if exists shop_table;
## create table shop_table(
  ## shop_id int not null primary key auto_increment,//店铺id
  ## name varchar(50),//店铺名称
  ## address varchar(100),//店铺地址
  ## lng double,//店铺坐标经度
  ## lat double,//店铺坐标纬度
  ## range_meter int,//店铺范围
  ## color varchar(10)//店铺范围颜色
  ## );
![image](https://github.com/zhaoxin-BF/Projects/map_system/client_map/assets/image/baidu_map.jpg)
