/*
 * wmy_spidnn.c
 *
 *  Created on: 2018年5月14日
 *      Author: wmy
 */

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "driverlib/pin_map.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/pwm.h"
#include "driverlib/interrupt.h"
#include "driverlib/fpu.h"
#include "driverlib/qei.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include "time.h"
#include "inc/hw_i2c.h"
#include "driverlib/rom.h"
#include "driverlib/adc.h"
#include "driverlib/uart.h"
#include "inc/hw_gpio.h"
#include "inc/hw_ints.h"
#include "string.h"
#include "driverlib/timer.h"

typedef struct//神经元细胞单体
{
    double INPUT;
    double STATUS;
    double OUTPUT;
}NEURON;

typedef struct
{
    double W_SET_P;//权重值
    double W_SET_I;//权重值
    double W_SET_D;//权重值
    double W_NOW_P;//权重值
    double W_NOW_I;//权重值
    double W_NOW_D;//权重值
    double W_P_OUT;//权重值
    double W_I_OUT;//权重值
    double W_D_OUT;//权重值
    double W_coefficient;//很重要！！！！
    double T_P_OUT;//阈值
    double T_I_OUT;//阈值
    double T_D_OUT;//阈值
    double D_neuron_LAST_INPUT;//微分神经元上一次输入值
    NEURON INPUT_SET;//输入层
    NEURON INPUT_NOW;//输入层
    NEURON P_neuron;//隐含层
    NEURON I_neuron;//隐含层
    NEURON D_neuron;//隐含层
    NEURON OUTPUT;//输出层
}SPIDNN;

double Absolute_SPIDNN_FPU(double x)//取绝对值
{
    if(x>=0)
    {
        return x;
    }
    else if(x<0)
    {
        return (-x);
    }
    return 0;
}

double Threshold_Function(double x,double threshold_value)
{
    threshold_value=Absolute_SPIDNN_FPU(threshold_value);
    if(x>threshold_value)
    {
        return threshold_value;
    }
    else if(x<-threshold_value)
    {
        return (-threshold_value);
    }
    return x;
}

double SPIDNN_Forward_Control(SPIDNN * spidnn, double setpoint, double nowpoint)//神经元网络PID前向控制算法
{
    spidnn->INPUT_NOW.INPUT=nowpoint;
    spidnn->INPUT_SET.INPUT=setpoint;
    spidnn->INPUT_NOW.STATUS=spidnn->INPUT_NOW.INPUT;
    spidnn->INPUT_SET.STATUS=spidnn->INPUT_SET.INPUT;
    spidnn->INPUT_NOW.OUTPUT=spidnn->INPUT_NOW.STATUS;
    spidnn->INPUT_SET.OUTPUT=spidnn->INPUT_SET.STATUS;
    spidnn->P_neuron.INPUT=spidnn->W_NOW_P*spidnn->INPUT_NOW.OUTPUT+spidnn->W_SET_P*spidnn->INPUT_SET.OUTPUT;
    spidnn->P_neuron.STATUS=spidnn->P_neuron.INPUT;
    spidnn->P_neuron.OUTPUT=Threshold_Function(spidnn->P_neuron.STATUS,spidnn->T_P_OUT);
    spidnn->I_neuron.INPUT=spidnn->W_NOW_I*spidnn->INPUT_NOW.OUTPUT+spidnn->W_SET_I*spidnn->INPUT_SET.OUTPUT;
    spidnn->I_neuron.STATUS=spidnn->I_neuron.STATUS+spidnn->I_neuron.INPUT;
    spidnn->I_neuron.OUTPUT=Threshold_Function(spidnn->I_neuron.STATUS,spidnn->T_I_OUT);
    spidnn->D_neuron.INPUT=spidnn->W_NOW_D*spidnn->INPUT_NOW.OUTPUT+spidnn->W_SET_D*spidnn->INPUT_SET.OUTPUT;
    spidnn->D_neuron.STATUS=spidnn->D_neuron.INPUT-spidnn->D_neuron_LAST_INPUT;
    spidnn->D_neuron.OUTPUT=Threshold_Function(spidnn->D_neuron.STATUS,spidnn->T_D_OUT);
    spidnn->D_neuron_LAST_INPUT = spidnn->D_neuron.INPUT ;
    spidnn->OUTPUT.INPUT =spidnn->W_P_OUT * spidnn->P_neuron.OUTPUT + spidnn->W_I_OUT *  spidnn->I_neuron.OUTPUT + spidnn->W_D_OUT * spidnn->D_neuron.OUTPUT;
    spidnn->OUTPUT.STATUS = spidnn->OUTPUT.INPUT;
    spidnn->OUTPUT.OUTPUT=spidnn->OUTPUT.STATUS;
    return spidnn->OUTPUT.OUTPUT;
}

void SPIDNN_Back_Control(SPIDNN * spidnn, double setpoint, double nowpoint)//神经元网络PID反向传播算法
{
    spidnn->W_P_OUT=spidnn->W_P_OUT+spidnn->W_coefficient*(setpoint-nowpoint)*spidnn->P_neuron.OUTPUT;
    spidnn->T_P_OUT=spidnn->T_P_OUT+(setpoint-nowpoint);
    spidnn->W_I_OUT=spidnn->W_I_OUT+spidnn->W_coefficient*(setpoint-nowpoint)*spidnn->I_neuron.OUTPUT;
    spidnn->T_I_OUT=spidnn->T_I_OUT+(setpoint-nowpoint);
    spidnn->W_D_OUT=spidnn->W_D_OUT+spidnn->W_coefficient*(setpoint-nowpoint)*spidnn->D_neuron.OUTPUT;
    spidnn->T_D_OUT=spidnn->T_D_OUT+(setpoint-nowpoint);
}
