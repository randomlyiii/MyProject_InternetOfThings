package com.myback.pojo;

import lombok.Data;
import lombok.AllArgsConstructor;
import lombok.NoArgsConstructor;

import java.time.LocalDateTime;

@Data
@NoArgsConstructor
@AllArgsConstructor
public class Mydata {
    private Integer id;
    private float temperature;
    private float humidity;
    private LocalDateTime writeTime;
}
