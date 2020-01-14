# 这是一个简单的个人博客管理系统项目，以下是项目总结说明书(未完待续...)

## 访问地址：http://39.96.179.159:8888/

## 注意事项：
### 1、因为路由使用了正则表达式所以编译的时候需要使用更高版本低的gcc编译器
```
#!/bin/bash    
#yum install devtoolset-7-gcc  devtoolset-7-cc-c++  
source /opt/rh/devtoolset-7/enable                                                                                               
echo "gcc 编译器以设置为7.3.1版本"
```
### 2、为保持服务持续在后台运行可是用nohup &命令...简单的shell脚本
```
nohup ./Projects/blog_system/blog_server/server_blog &
echo "博客服务器开启成功"
```

### 3、项目使用jsoncpp 可直接使用yum 进行安装
``` yum install jsoncpp ```

### 4、httplib库通过github下载即可
地址：https://github.com/yhirose/cpp-httplib

### 5、mysql数据库需要设置一个持续在线的参数

### 6、linux 进入mysql的参数
```mysql -uroot -p   password:Zhaoxin..521```
