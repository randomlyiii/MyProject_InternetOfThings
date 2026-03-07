package com.myback.mapper;

import com.myback.pojo.Mydata;
import org.apache.ibatis.annotations.Insert;
import org.apache.ibatis.annotations.Mapper;
import org.apache.ibatis.annotations.Select;

import java.util.List;

@Mapper
public interface DataMapper {
    /**
     * 获取所有数据
     * @return
     */
    @Select("select id, temperature, humidity, write_time from mydata order by write_time desc")
    List<Mydata> getAllData();

    /**
     * 保存数据
     * @return
     */
    public void updateData(Mydata mydata);
}
