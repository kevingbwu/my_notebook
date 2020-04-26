# How_Network_Works

![Image of the book](image/book.png)

## Chapter1 浏览器生成消息

### 生成HTTP请求消息

浏览器的第一步工作是解析URL

HTTP协议：方法（进行怎样的操作）+ URI（对什么）

Web服务器响应消息：状态码 + 头字段 + 网页数据

最常用方法：GET，POST

请求消息：请求行 + 消息头 + 空行 + 消息体

响应消息：状态行 + 消息头 + 空行 + 消息体

### 向DNS服务器查询Web服务器的IP地址

IP地址：网络号 + 主机号，共32比特，主机号全0表示整个子网，主机号全1表示广播

### 全世界DNS服务器的大接力

记录类型：A（Address）

### 委托协议栈发送消息

1. 创建套接字（创建套接字阶段）
2. 将管道连接到服务器端的套接字上（连接阶段）
3. 收发数据（通信阶段）
4. 断开管道并删除套接字（断开阶段）