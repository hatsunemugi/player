# player
基于ffmpeg,opencv实现了带服务端的播放器
实现了倍速播放，跳转，暂停等基础功能。
实现了修改分辨率的功能

界面使用qml进行编写
得益于qml的特性，大大加快了开发进度
考虑到QTcpSocket的性能问题，服务端使用asio(non boost)作为tcp server接受客户端连接
因此，服务端使用多线程集成了Qt与asio的事件循环。

#Demo


https://github.com/hatsunemugi/player/assets/52739182/c9f4b21f-e156-4bbb-9c06-b104940abc5d




https://github.com/hatsunemugi/player/assets/52739182/540f52dd-346d-4e2e-b783-2cc00039076f



模仿QHttpServer api实现了函数
route(QString,std::function<QVariant(QVariant)>);

可以简化代理，例如

    route("/version",[&](){

        return "1.0.0";
    
    });

#关于编译原理

实现了一个轻量的解释器以进行词法分析，仅包含一个hpp头文件。

    if(Command::isEmpty(s)||s.isEmpty())//不接受空字符串
        return;
    const QVector<Word>& words = Command::analyse(s);
    print(words);


#关于数据库

本项目使用sqlite，保存在程序运行目录下的media.db

#关于QThread/std::thread

使用继承自QObject的类并moveToThread(QThread)可以轻易实现较为复杂的功能，而使用继承自QThread的类并在QThread::run()中实现功能却会处处不顺心。

项目中我使用std::thread来处理asio的事件循环，使用detach()以实现分离线程，若是使用join()则会阻塞；

#大坑

asio的回调函数内写qt的connect是接受不到消息的，所以我在主线程实例化了QUdpSocket和QTcpServer的指针并进行监听

#未解决的问题

ffmpeg读取视频帧出现错误，好在Qt有信号量可以用来同步音视频，我就开了两个线程分别处理音视频：

视频使用opencv进行处理，opencv简洁的api写起来十分顺畅，爱了；

音频的delay精确值由于我无法正确读取avframe，转而使用平均帧率进行计算。

#如何编译

使用Qt6.5，修改cmakelist中的

set(OPENCV_PATH C:/Users/c/opencv/build)

set(FFMPEG_PATH C:/Users/c/lib/ffmpeg)

这两行，将其改为你安装的地址(客户端)

对于服务端来说，你还要修改

set(ASIO_PATH C:/Users/c/lib/asio)

#关于make

一开始我是打算用qmake的，但是苦于久久无法实现分层目录故而转向cmake。Qt官方抛弃了qmake也是一个原因

cmake模板

    file(GLOB FILES "相对路径/*.*")#FILES中存储了绝对路径

    foreach(filepath ${FILES})

        message(${filepath} "\t" ${CMAKE_CURRENT_SOURCE_DIR})
    
        string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/" "" filename ${filepath}) #.+/(.+)\\\\..*" "\\\\1 转换为相对路径
    
        list(APPEND FILES_R ${filename})
    
    endforeach(filepath)

    list(REMOVE_DUPLICATES FILES_R) #表示删除非第一个重名值

使用上述模板可以自动添加文件到构建系统。

使用include_directories("...")可以省去恶心的前缀
