# 使用 CentOS 7 作为基础镜像
FROM centos:7

# 添加 JDK 到镜像中
COPY jdk17.tar.gz /usr/local/
RUN tar -xzf /usr/local/jdk17.tar.gz -C /usr/local/ &&  rm /usr/local/jdk17.tar.gz

# 设置环境变量
ENV JAVA_HOME=/usr/local/jdk-17.0.10
ENV PATH=$JAVA_HOME/bin:$PATH

ENV OSS_ACCESS_KEY_ID=LTAI5tP6dc4cvccdvvySE39X
ENV OSS_ACCESS_KEY_SECRET=ZSyIT31qhxIkS0dH1H9WzHqPiyM3Ot

#统一编码
ENV LANG=en_US.UTF-8
ENV LANGUAGE=en_US:en
ENV LC_ALL=en_US.UTF-8

# 创建应用目录
RUN mkdir -p /myback
WORKDIR /myback

# 复制应用 JAR 文件到容器
COPY  MyBack-0.0.1-SNAPSHOT.jar  myback.jar

# 暴露端口
EXPOSE 8080

# 运行命令
ENTRYPOINT ["java","-jar","/myback/myback.jar"]