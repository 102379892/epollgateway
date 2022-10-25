# **********************************************************************
# 
#     说		明： 
#     作		者：
#     最后修改日期： 
#     版		本： 
# **********************************************************************

#定义编译连接指令变量（可以是cc、gcc、g++等）
CC = g++

#定义主目标文件变量, 动态库的名字应符合“lib*.so*”这种格式
TARGET = gateway

#定义OBJ文件变量
OBJS = clog/CLog.o clog/CLogHandle.o clog/SyncClearLog.o \
	config/IniReader.o config/Config.o common/Common.o common/point.o \
	lock/ThreadMutex.o lock/RWThreadMutex.o \
	thread/Thread.o sock/BaseSocket.o sock/EpoolCtl.o recv/SockClient.o \
	prase/CmdPrase.o prase/ReportPrase.o \
	json/jsoncpp.o http/httpheader.o http/httpclient.o \
	main.o

#定义库文件路径变量	    
LIBS = -L. -L/usr/lib64 -L/usr/lib -L/usr/local/lib64 -L/usr/local/lib  \
	-lpthread -lcurl

#定义头文件路径变量
INCLUDES = -I. -I/usr/include -I/usr/local/include
				   
#定义编译选项变量
CFLAGS = -g -std=c++11

#定义连接选项变量
LFLAGS = -g -std=c++11 -Wl,-rpath,/usr/lib64:/usr/lib:/usr/local/lib64:/usr/local/lib
	
.PHONY: all 
all:$(TARGET)

#“.PHONY”来显示地指明一个目标是“伪目标”
.PHONY : clean
clean :
	-rm $(TARGET) $(OBJS)

#格式：目标集合: 目标模式 : 目标依赖模式
#“$<”表示所有的依赖目标集
#“$@”表示目标模式集
#@echo "正在编译各个源文件"
$(OBJS): %.o: %.cpp 
	$(CC) -c $(CFLAGS) $(INCLUDES) $< -o $@
	
#主目标依赖命令(使用了字符“@”使得make执行命令不显示在屏幕上)
$(TARGET) : $(OBJS) 
	$(CC)  $(LFLAGS) -o  $(TARGET) $(OBJS) $(LIBS)
