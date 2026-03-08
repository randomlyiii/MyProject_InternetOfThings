package com.myback.controller;

import com.myback.pojo.Mydata;
import com.myback.pojo.Result;
import com.myback.service.DataService;
import lombok.extern.slf4j.Slf4j;
import org.apache.ibatis.annotations.Update;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.*;

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
    @GetMapping
    public Result getData(){
        log.info("获取所有数据");
        List<Mydata> mydataList = dataService.getAllData();
        return Result.success(mydataList);
    }

    /**
     * 接收JSON格式数据并保存到数据库
     */
    @PostMapping
    public Result updateData(@RequestBody Mydata mydata){
        log.info("接收到的数据：{}", mydata);
        dataService.updateData(mydata);
        return Result.success();
    }
}
