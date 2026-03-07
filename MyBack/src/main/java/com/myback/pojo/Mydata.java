package com.myback.pojo;

import lombok.AllArgsConstructor;
import lombok.NoArgsConstructor;

import java.time.LocalDateTime;

@lombok.Data
@NoArgsConstructor
@AllArgsConstructor
public class Data {
    private Integer id;
    private float temperature;
    private float humidity;
    private LocalDateTime writeTime;
}
