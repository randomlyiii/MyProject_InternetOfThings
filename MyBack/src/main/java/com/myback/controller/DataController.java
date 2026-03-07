package com.myback.controller;

import com.myback.pojo.Result;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

@RequestMapping("/data")
@RestController
public class DataController {
    /**
     * 获取所有数据
     * @return
     */
    @RequestMapping("/get")
    public Result getData(){
        return Result.success();
    }
}
