/*
 * wmy_spidnn.h
 *
 *  Created on: 2018年5月14日
 *      Author: wmy
 */

#ifndef SPIDNN_WMY_SPIDNN_H_
#define SPIDNN_WMY_SPIDNN_H_

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
    double W_coefficient;
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

extern double Absolute_SPIDNN_FPU(double x);//取绝对值
extern double Threshold_Function(double x,double threshold_value);
extern double SPIDNN_Forward_Control(SPIDNN * spidnn, double setpoint, double nowpoint);//神经元网络PID前向控制算法
extern void SPIDNN_Back_Control(SPIDNN * spidnn, double setpoint, double nowpoint);//神经元网络PID反向传播算法

#endif /* SPIDNN_WMY_SPIDNN_H_ */
