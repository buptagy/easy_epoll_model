# 编译器
CXX = g++

# 编译选项
CXXFLAGS = -Wall -std=c++11

# 目标文件
TARGETS = epoll_server epoll_client

# 目标规则
all: $(TARGETS)

epoll_server: server.cpp
	$(CXX) $(CXXFLAGS) -o epoll_server server.cpp

epoll_client: epoll_client.cpp
	$(CXX) $(CXXFLAGS) -o epoll_client epoll_client.cpp

# 清理规则
clean:
	rm -f $(TARGETS)
