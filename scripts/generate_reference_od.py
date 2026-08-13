#!/usr/bin/env python3
"""Derive the reference CANopenNode V4 Object Dictionary from its upstream base.

This script is deliberately deterministic.  A released product shall retain the
corresponding CANopenEditor project and regenerate the OD after each profile or
PDO-map change; this reference demonstrates the generated C layout only.
"""
from pathlib import Path
import shutil

ROOT = Path(__file__).resolve().parents[1]
UPSTREAM = ROOT / "third_party" / "CanOpenSTM32" / "CANopenNode" / "example"
OUTPUT = ROOT / "Generated"
OUTPUT.mkdir(exist_ok=True)

shutil.copy2(UPSTREAM / "OD.h", OUTPUT / "OD.h")
shutil.copy2(UPSTREAM / "OD.c", OUTPUT / "OD.c")

header = (OUTPUT / "OD.h").read_text()
source = (OUTPUT / "OD.c").read_text()

app_type = r'''typedef struct {
    /* CiA 401 reference process data. */
    uint8_t x6000_readDigitalInputs;
    uint8_t x6200_writeDigitalOutputs;
    int16_t x6401_readAnalogInput1;
    int16_t x6411_readAnalogInput2;
    int16_t x6422_writeAnalogOutput1;

    /* CiA 402 reference process data. */
    uint16_t x603F_errorCode;
    uint16_t x6040_controlword;
    uint16_t x6041_statusword;
    int8_t x6060_modesOfOperation;
    int8_t x6061_modesOfOperationDisplay;
    int32_t x6064_positionActualValue;
    int32_t x607A_targetPosition;
    int16_t x6071_targetTorque;
    int16_t x6077_torqueActualValue;
    int32_t x606C_velocityActualValue;
    int32_t x60FF_targetVelocity;
} OD_APP_t;

#ifndef OD_ATTR_APP
#define OD_ATTR_APP
#endif
extern OD_ATTR_APP OD_APP_t OD_APP;

'''
needle = "} OD_RAM_t;\n\n#ifndef OD_ATTR_PERSIST_COMM"
if needle not in header:
    raise RuntimeError("Unexpected upstream OD.h layout while inserting OD_APP_t")
header = header.replace(needle, "} OD_RAM_t;\n\n" + app_type + "#ifndef OD_ATTR_PERSIST_COMM", 1)

shortcut_macros = r'''
#define OD_ENTRY_H6000_readDigitalInputs &OD->list[33]
#define OD_ENTRY_H603F_errorCode &OD->list[34]
#define OD_ENTRY_H6040_controlword &OD->list[35]
#define OD_ENTRY_H6041_statusword &OD->list[36]
#define OD_ENTRY_H6060_modesOfOperation &OD->list[37]
#define OD_ENTRY_H6061_modesOfOperationDisplay &OD->list[38]
#define OD_ENTRY_H6064_positionActualValue &OD->list[39]
#define OD_ENTRY_H606C_velocityActualValue &OD->list[40]
#define OD_ENTRY_H6071_targetTorque &OD->list[41]
#define OD_ENTRY_H6077_torqueActualValue &OD->list[42]
#define OD_ENTRY_H607A_targetPosition &OD->list[43]
#define OD_ENTRY_H60FF_targetVelocity &OD->list[44]
#define OD_ENTRY_H6200_writeDigitalOutputs &OD->list[45]
#define OD_ENTRY_H6401_readAnalogInput1 &OD->list[46]
#define OD_ENTRY_H6411_readAnalogInput2 &OD->list[47]
#define OD_ENTRY_H6422_writeAnalogOutput1 &OD->list[48]
'''
needle = "#define OD_ENTRY_H1A03_TPDOMappingParameter &OD->list[32]\n"
if needle not in header:
    raise RuntimeError("Unexpected upstream OD.h list shortcut layout")
header = header.replace(needle, needle + "\n" + shortcut_macros, 1)

source_app_init = r'''
OD_ATTR_APP OD_APP_t OD_APP = {
    .x6000_readDigitalInputs = 0x00,
    .x6200_writeDigitalOutputs = 0x00,
    .x6401_readAnalogInput1 = 0,
    .x6411_readAnalogInput2 = 0,
    .x6422_writeAnalogOutput1 = 0,
    .x603F_errorCode = 0x0000,
    .x6040_controlword = 0x0000,
    .x6041_statusword = 0x0040, /* Switch-on disabled. */
    .x6060_modesOfOperation = 0,
    .x6061_modesOfOperationDisplay = 0,
    .x6064_positionActualValue = 0,
    .x607A_targetPosition = 0,
    .x6071_targetTorque = 0,
    .x6077_torqueActualValue = 0,
    .x606C_velocityActualValue = 0,
    .x60FF_targetVelocity = 0
};

'''
needle = "};\n\n\n\n/*******************************************************************************\n    All OD objects (constant definitions)"
if needle not in source:
    raise RuntimeError("Unexpected upstream OD.c initialization layout")
source = source.replace(needle, "};\n\n" + source_app_init + "/*******************************************************************************\n    All OD objects (constant definitions)", 1)

object_members = r'''    OD_obj_var_t o_6000_readDigitalInputs;
    OD_obj_var_t o_6200_writeDigitalOutputs;
    OD_obj_var_t o_6401_readAnalogInput1;
    OD_obj_var_t o_6411_readAnalogInput2;
    OD_obj_var_t o_6422_writeAnalogOutput1;
    OD_obj_var_t o_603F_errorCode;
    OD_obj_var_t o_6040_controlword;
    OD_obj_var_t o_6041_statusword;
    OD_obj_var_t o_6060_modesOfOperation;
    OD_obj_var_t o_6061_modesOfOperationDisplay;
    OD_obj_var_t o_6064_positionActualValue;
    OD_obj_var_t o_6071_targetTorque;
    OD_obj_var_t o_6077_torqueActualValue;
    OD_obj_var_t o_607A_targetPosition;
    OD_obj_var_t o_606C_velocityActualValue;
    OD_obj_var_t o_60FF_targetVelocity;
'''
needle = "    OD_obj_record_t o_1A03_TPDOMappingParameter[9];\n"
if needle not in source:
    raise RuntimeError("Unexpected upstream OD.c object member layout")
source = source.replace(needle, needle + object_members, 1)

object_definitions = r'''    .o_6000_readDigitalInputs = {
        .dataOrig = &OD_APP.x6000_readDigitalInputs,
        .attribute = ODA_SDO_R | ODA_TPDO,
        .dataLength = 1
    },
    .o_6200_writeDigitalOutputs = {
        .dataOrig = &OD_APP.x6200_writeDigitalOutputs,
        .attribute = ODA_SDO_RW | ODA_RPDO,
        .dataLength = 1
    },
    .o_6401_readAnalogInput1 = {
        .dataOrig = &OD_APP.x6401_readAnalogInput1,
        .attribute = ODA_SDO_R | ODA_TPDO,
        .dataLength = 2
    },
    .o_6411_readAnalogInput2 = {
        .dataOrig = &OD_APP.x6411_readAnalogInput2,
        .attribute = ODA_SDO_R | ODA_TPDO,
        .dataLength = 2
    },
    .o_6422_writeAnalogOutput1 = {
        .dataOrig = &OD_APP.x6422_writeAnalogOutput1,
        .attribute = ODA_SDO_RW | ODA_RPDO,
        .dataLength = 2
    },
    .o_603F_errorCode = {
        .dataOrig = &OD_APP.x603F_errorCode,
        .attribute = ODA_SDO_R | ODA_TPDO,
        .dataLength = 2
    },
    .o_6040_controlword = {
        .dataOrig = &OD_APP.x6040_controlword,
        .attribute = ODA_SDO_RW | ODA_RPDO,
        .dataLength = 2
    },
    .o_6041_statusword = {
        .dataOrig = &OD_APP.x6041_statusword,
        .attribute = ODA_SDO_R | ODA_TPDO,
        .dataLength = 2
    },
    .o_6060_modesOfOperation = {
        .dataOrig = &OD_APP.x6060_modesOfOperation,
        .attribute = ODA_SDO_RW | ODA_RPDO,
        .dataLength = 1
    },
    .o_6061_modesOfOperationDisplay = {
        .dataOrig = &OD_APP.x6061_modesOfOperationDisplay,
        .attribute = ODA_SDO_R | ODA_TPDO,
        .dataLength = 1
    },
    .o_6064_positionActualValue = {
        .dataOrig = &OD_APP.x6064_positionActualValue,
        .attribute = ODA_SDO_R | ODA_TPDO,
        .dataLength = 4
    },
    .o_6071_targetTorque = {
        .dataOrig = &OD_APP.x6071_targetTorque,
        .attribute = ODA_SDO_RW | ODA_RPDO,
        .dataLength = 2
    },
    .o_6077_torqueActualValue = {
        .dataOrig = &OD_APP.x6077_torqueActualValue,
        .attribute = ODA_SDO_R | ODA_TPDO,
        .dataLength = 2
    },
    .o_607A_targetPosition = {
        .dataOrig = &OD_APP.x607A_targetPosition,
        .attribute = ODA_SDO_RW | ODA_RPDO,
        .dataLength = 4
    },
    .o_606C_velocityActualValue = {
        .dataOrig = &OD_APP.x606C_velocityActualValue,
        .attribute = ODA_SDO_R | ODA_TPDO,
        .dataLength = 4
    },
    .o_60FF_targetVelocity = {
        .dataOrig = &OD_APP.x60FF_targetVelocity,
        .attribute = ODA_SDO_RW | ODA_RPDO,
        .dataLength = 4
    }
'''
needle = "    }\n};\n\n\n/*******************************************************************************\n    Object dictionary\n"
if needle not in source:
    raise RuntimeError("Unexpected upstream OD.c object initializer layout")
source = source.replace(needle, "    },\n" + object_definitions + "};\n\n\n/*******************************************************************************\n    Object dictionary\n", 1)

entries = r'''    {0x6000, 0x01, ODT_VAR, &ODObjs.o_6000_readDigitalInputs, NULL},
    {0x603F, 0x01, ODT_VAR, &ODObjs.o_603F_errorCode, NULL},
    {0x6040, 0x01, ODT_VAR, &ODObjs.o_6040_controlword, NULL},
    {0x6041, 0x01, ODT_VAR, &ODObjs.o_6041_statusword, NULL},
    {0x6060, 0x01, ODT_VAR, &ODObjs.o_6060_modesOfOperation, NULL},
    {0x6061, 0x01, ODT_VAR, &ODObjs.o_6061_modesOfOperationDisplay, NULL},
    {0x6064, 0x01, ODT_VAR, &ODObjs.o_6064_positionActualValue, NULL},
    {0x606C, 0x01, ODT_VAR, &ODObjs.o_606C_velocityActualValue, NULL},
    {0x6071, 0x01, ODT_VAR, &ODObjs.o_6071_targetTorque, NULL},
    {0x6077, 0x01, ODT_VAR, &ODObjs.o_6077_torqueActualValue, NULL},
    {0x607A, 0x01, ODT_VAR, &ODObjs.o_607A_targetPosition, NULL},
    {0x60FF, 0x01, ODT_VAR, &ODObjs.o_60FF_targetVelocity, NULL},
    {0x6200, 0x01, ODT_VAR, &ODObjs.o_6200_writeDigitalOutputs, NULL},
    {0x6401, 0x01, ODT_VAR, &ODObjs.o_6401_readAnalogInput1, NULL},
    {0x6411, 0x01, ODT_VAR, &ODObjs.o_6411_readAnalogInput2, NULL},
    {0x6422, 0x01, ODT_VAR, &ODObjs.o_6422_writeAnalogOutput1, NULL},
'''
needle = "    {0x0000, 0x00, 0, NULL, NULL}\n"
if needle not in source:
    raise RuntimeError("Unexpected upstream OD.c ODList terminator")
source = source.replace(needle, entries + needle, 1)

(OUTPUT / "OD.h").write_text(header)
(OUTPUT / "OD.c").write_text(source)

eds = (UPSTREAM / "DS301_profile.eds").read_text()
eds = eds.replace("FileName=DS301_profile.eds", "FileName=stm32f767_canopen_reference.eds")
eds = eds.replace("ProductName=New Product", "ProductName=STM32F767 CANopen Reference")
eds = eds.replace("DynamicChannelsSupported=0", "DynamicChannelsSupported=1")
eds = eds.replace("SupportedObjects=30\n1=0x1003", """SupportedObjects=46
1=0x1003""", 1)
optional_append = """\n31=0x6000
32=0x603F
33=0x6040
34=0x6041
35=0x6060
36=0x6061
37=0x6064
38=0x606C
39=0x6071
40=0x6077
41=0x607A
42=0x60FF
43=0x6200
44=0x6401
45=0x6411
46=0x6422
"""
eds = eds.replace("30=0x1A03\n", "30=0x1A03\n" + optional_append, 1)
profile_entries = r'''
[6000]
ParameterName=Read digital inputs
ObjectType=0x7
;StorageLocation=RAM
DataType=0x0005
AccessType=ro
DefaultValue=0x00
PDOMapping=1

[603F]
ParameterName=Error code
ObjectType=0x7
;StorageLocation=RAM
DataType=0x0006
AccessType=ro
DefaultValue=0x0000
PDOMapping=1

[6040]
ParameterName=Controlword
ObjectType=0x7
;StorageLocation=RAM
DataType=0x0006
AccessType=rw
DefaultValue=0x0000
PDOMapping=1

[6041]
ParameterName=Statusword
ObjectType=0x7
;StorageLocation=RAM
DataType=0x0006
AccessType=ro
DefaultValue=0x0040
PDOMapping=1

[6060]
ParameterName=Modes of operation
ObjectType=0x7
;StorageLocation=RAM
DataType=0x0002
AccessType=rw
DefaultValue=0
PDOMapping=1

[6061]
ParameterName=Modes of operation display
ObjectType=0x7
;StorageLocation=RAM
DataType=0x0002
AccessType=ro
DefaultValue=0
PDOMapping=1

[6064]
ParameterName=Position actual value
ObjectType=0x7
;StorageLocation=RAM
DataType=0x0004
AccessType=ro
DefaultValue=0
PDOMapping=1

[606C]
ParameterName=Velocity actual value
ObjectType=0x7
;StorageLocation=RAM
DataType=0x0004
AccessType=ro
DefaultValue=0
PDOMapping=1

[6071]
ParameterName=Target torque
ObjectType=0x7
;StorageLocation=RAM
DataType=0x0003
AccessType=rw
DefaultValue=0
PDOMapping=1

[6077]
ParameterName=Torque actual value
ObjectType=0x7
;StorageLocation=RAM
DataType=0x0003
AccessType=ro
DefaultValue=0
PDOMapping=1

[607A]
ParameterName=Target position
ObjectType=0x7
;StorageLocation=RAM
DataType=0x0004
AccessType=rw
DefaultValue=0
PDOMapping=1

[60FF]
ParameterName=Target velocity
ObjectType=0x7
;StorageLocation=RAM
DataType=0x0004
AccessType=rw
DefaultValue=0
PDOMapping=1

[6200]
ParameterName=Write digital outputs
ObjectType=0x7
;StorageLocation=RAM
DataType=0x0005
AccessType=rw
DefaultValue=0x00
PDOMapping=1

[6401]
ParameterName=Read analog input 1
ObjectType=0x7
;StorageLocation=RAM
DataType=0x0003
AccessType=ro
DefaultValue=0
PDOMapping=1

[6411]
ParameterName=Read analog input 2
ObjectType=0x7
;StorageLocation=RAM
DataType=0x0003
AccessType=ro
DefaultValue=0
PDOMapping=1

[6422]
ParameterName=Write analog output 1
ObjectType=0x7
;StorageLocation=RAM
DataType=0x0003
AccessType=rw
DefaultValue=0
PDOMapping=1
'''
( ROOT / "ObjectDictionary" / "stm32f767_canopen_reference.eds").write_text(eds + "\n" + profile_entries)
print("Generated/OD.h, Generated/OD.c, and ObjectDictionary/stm32f767_canopen_reference.eds updated.")
