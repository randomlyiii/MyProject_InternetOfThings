package com.myback.pojo;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

import java.time.LocalDateTime;

@Data
@NoArgsConstructor
@AllArgsConstructor
public class data {
    private Integer id;
    private float temperature;
    private float humidity;
    private LocalDateTime writeTime;
}
