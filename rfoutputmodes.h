#ifndef RFOUTPUTMODES_H
#define RFOUTPUTMODES_H

#include "outputmode.h"
#include "sysparm.h"


class PulseMode : public OutputMode
{
private:
    uint32 pulse_sub;
    bool last_impedance_state;


public:
    explicit PulseMode(SysParm *sysParm);    //생성자 소멸자는 cpp 에서 구현할것
    ~PulseMode() override;
    const int8* getModeTitle() override;
    void RFStart() override;
    void RFStop() override;
    void RFPowerControl() override;
    void SafetyDutyCycle() override;
    RF_Output_Mode_en getMode() override {return RF_DEVICE_OP_MODE_PULSE; }
    RF_Imp_Mode_en getImpedanceControlMethod() override {return RF_DEVICE_IMP_MODE_MANUAL; }
    void MRFA_DoTestImpedanceState() override;

    uint16 getVMSforWatt(uint16 power, uint16 imp);
    uint32 getTableIndexforVMS(uint32 data);
    uint16 getVMSDAC_Index(uint16 impedance);
    uint32 counting;
#if 1
#endif
    uint16 HIVMSDAC_Value[22] = {
        //315, 236, 212, 202, 195, //300 R= 212
        //194, 192, 190, 190, 189,
        188, 187, 187, 186, 186,
        186, 185, 186, 184, 183,
        183, 182
    };

    uint16 LIVMSDAC_Value[22] = {
        303, 227, 204, 196, 192,
        189, 189, 185, 185, 182,
        //182, 182, 182, 182, 182,
        //179, 175, 172, 169, 164,
        //161, 159
    };
    uint16 VMSDAC_Value_array[27] = {
        318, 236, 214, 204, 197,   //303, 227, 204, 196, 192,
        199, 207, 207, 195, 194,   //189, 189, 185, 185, 182,
        175, 174, 177, 174, 178,   //188, 174, 177, 174, 178,
        183, 184, 197, 201, 200,   //1600, 1700, 1800, 1900, 2000
        200, 195, 181, 189, 188,
        188, 187
    };



};

class ConstantMode : public OutputMode
{
private:
    bool last_impedance_state;

public:
    explicit ConstantMode(SysParm *sysParm);    //생성자 소멸자는 cpp 에서 구현할것
    ~ConstantMode() override;
    const int8* getModeTitle() override;
    void RFStart() override;
    void RFStop() override;
    void RFPowerControl() override;
    void SafetyDutyCycle() override;
    RF_Output_Mode_en getMode() override {return RF_DEVICE_OP_MODE_CONSTANT; }
    RF_Imp_Mode_en getImpedanceControlMethod() override {return RF_DEVICE_IMP_MODE_MANUAL; }
    void MRFA_DoTestImpedanceState() override;

    uint16 getVMSforWatt(uint16 power, uint16 imp);
    uint32 getTableIndexforVMS(uint32 data);
    uint16 getVMSDAC_Index(uint16 impedance);
    uint32 counting;

    uint16 HIVMSDAC_Value[22] = {
        //315, 236, 212, 202, 195, //300 R= 212
        //194, 192, 190, 190, 189,
        188, 187, 187, 186, 186,
        186, 185, 186, 184, 183,
        183, 182
    };

    uint16 LIVMSDAC_Value[22] = {
        303, 227, 204, 196, 192,
        189, 189, 185, 185, 182,
        //182, 182, 182, 182, 182,
        //179, 175, 172, 169, 164,
        //161, 159
    };
    uint16 VMSDAC_Value_array[27] = {
        318, 236, 214, 204, 197,   //303, 227, 204, 196, 192,
        199, 207, 207, 195, 194,   //189, 189, 185, 185, 182,
        175, 174, 177, 174, 178,   //188, 174, 177, 174, 178,
        183, 184, 197, 201, 200,   //1600, 1700, 1800, 1900, 2000
        200, 195, 181, 189, 188,
        188, 187
    };

};

#endif // RFOUTPUTMODES_H
