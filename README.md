# player
基于ffmpeg,opencv实现了带服务端的播放器
实现了倍速播放，跳转，暂停等基础功能。
实现了修改分辨率的功能

考虑到QTcpSocket的性能问题，服务端使用asio(non boost)作为tcp server接受客户端连接
因此，服务端使用多线程集成了Qt与asio的事件循环。

#大坑
asio的回调函数内写qt的connect是接受不到消息的，所以我在主线程实例化了QUdpSocket和QTcpServer的指针并进行监听

#未解决的问题\n
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
一开始我是打算用qmake的，但是苦于久久无法实现分层目录故而转向cmake。
Qt官方抛弃了qmake也是一个原因

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
