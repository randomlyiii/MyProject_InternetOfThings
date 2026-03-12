--
-- Current Database: `back`
--

CREATE DATABASE /*!32312 IF NOT EXISTS*/ `back` /*!40100 DEFAULT CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci */ /*!80016 DEFAULT ENCRYPTION='N' */;

USE `back`;

--
-- Table structure for table `clazz`
--

DROP TABLE IF EXISTS `mydate`;
-- 操作日志表
create table mydata(
                    id int unsigned primary key auto_increment comment 'ID',
                    temperature float comment '温度',
                    humidity float comment '湿度',
                    write_time datetime comment '记录时间'
) comment '数据表';

insert into mydata(temperature, humidity, write_time) values (null, null, null);

UNLOCK TABLES;

--can be omitted

-- -- 1. 先删除所有旧的 app_user 用户，避免权限冲突（必须执行）
-- DROP USER IF EXISTS 'app_user'@'%';

-- -- 2. 创建用户：允许 任意IP(%) 登录，密码 Test@123
-- CREATE USER IF NOT EXISTS 'app_user'@'%' IDENTIFIED BY 'Test@123';

-- -- 3. 核心权限：仅授予 back 库的所有权限，无法操作其他库
-- GRANT ALL PRIVILEGES ON back.* TO 'app_user'@'%';

-- -- 4. 刷新权限，立即生效
-- FLUSH PRIVILEGES;
