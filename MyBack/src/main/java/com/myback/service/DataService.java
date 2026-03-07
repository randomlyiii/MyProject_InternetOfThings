package com.myback.service;

import com.myback.pojo.Mydata;

import java.util.List;

public interface DataService {
    /**
     * 获取所有数据
     * @return
     */
    List<Mydata> getAllData();
}
