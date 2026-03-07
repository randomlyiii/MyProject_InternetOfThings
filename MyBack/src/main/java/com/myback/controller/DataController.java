package com.myback.controller;

import com.myback.pojo.Mydata;
import com.myback.pojo.Result;
import com.myback.service.DataService;
import lombok.extern.slf4j.Slf4j;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

import java.util.List;

@Slf4j
@RequestMapping("/data")
@RestController
public class DataController {
    @Autowired
    private DataService dataService;

    /**
     * 获取所有数据
     */
    @RequestMapping("/get")
    public Result getData(){
        log.info("获取所有数据");
        List<Mydata> mydataList = dataService.getAllData();
        return Result.success(mydataList);
    }
}
