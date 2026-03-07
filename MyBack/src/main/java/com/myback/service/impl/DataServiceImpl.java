package com.myback.service.impl;

import com.myback.mapper.DataMapper;
import com.myback.pojo.Mydata;
import com.myback.service.DataService;
import lombok.Data;
import lombok.extern.slf4j.Slf4j;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.time.LocalDateTime;
import java.util.List;

@Slf4j
@Service
public class DataServiceImpl implements DataService {
    @Autowired
    private DataMapper dataMapper;

    /**
     * 获取所有数据
     *
     * @return
     */
    @Override
    public List<Mydata> getAllData() {
        return dataMapper.getAllData();
    }

    /**
     * 保存数据
     * @return
     */
    @Override
    public void updateData(Mydata mydata) {
        // 补全数据
        mydata.setId(1);
        mydata.setWriteTime(LocalDateTime.now());

        log.info("保存的数据：{}", mydata);
        dataMapper.updateData(mydata);
    }
}
