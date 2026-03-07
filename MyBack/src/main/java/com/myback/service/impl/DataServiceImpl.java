package com.myback.service.impl;

import com.myback.mapper.DataMapper;
import com.myback.pojo.Mydata;
import com.myback.service.DataService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.util.List;

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
}
