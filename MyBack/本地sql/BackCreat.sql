-- 数据库创建语句
create database if not exists back;

use back;

drop table if exists log;
drop table if exists MyDate;
drop table if exists Data;
drop table if exists Mydata;
-- 操作日志表
create table Mydata(
                    id int unsigned primary key auto_increment comment 'ID',
                    temperature float comment '温度',
                    humidity float comment '湿度',
                    write_time datetime comment '记录时间'
) comment '数据表';

insert into Mydata(temperature, humidity, write_time) values (23.5, 45.5, now());
