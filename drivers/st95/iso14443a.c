/*
 * Copyright (C) 2018 Unwired Devices [info@unwds.com]
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup
 * @ingroup
 * @brief
 * @{
 * @file		iso14443a.c
 * @brief       protocol ISO14443A for ST95
 * @author      Mikhail Perkov
 */
 
#include "st95.h"
#include "iso14443a.h"

/**
 * @brief This function calculate and check BCC
 * 
 * @param[in]   length: Length of data
 * @param[in]   data:   Pointer to the data
 * @param[in]   bcc:    BCC
 * 
 * @return  true:   Valid BCC
 * @return  false:  in case of an error
 */
static bool _iso14443a_check_bcc(uint8_t length, uint8_t * data, uint8_t bcc)
{
    uint8_t bcc_tmp = 0;

	for (uint8_t i = 0; i < length; i++) {
        bcc_tmp	^= data[i];
    }
	
	if((bcc_tmp ^ bcc) == 0) {
		return true;
    }
	else {
		return false;
    }
}

/**
 * @brief   This function returns UID size 
 * 
 * @param[in]   uid_byte: REQA uid byte
 * 
 * @return  UID size 
 */
static uint8_t _iso14443a_get_uidsize(uint8_t uid_byte)
{
	uint8_t size = 0;
	
	size = (uid_byte >> ISO14443A_OFFSET_UID_SIZE) & ISO14443A_MASK_UID_SIZE;
	return size;
}

/**
 * @brief   This function check if ID is complete
 * @brief   This check is carry out on SAK byte.
 * 
 * @param[in]   sak_byte: SAK byte
 * 
 * @return  0:  UID is complete
 * @return  1:  in case of an error
 */
static uint8_t _iso14443a_is_uid_complete(uint8_t sak_byte)
{
	if ((sak_byte & ISO14443A_MASK_SAK_UID_NOT_COMPLETE) == ISO14443A_SAK_UID_NOT_COMPLETE) {
		return ST95_ERROR;
    }
	else {
		return ST95_OK;
    }        
}

/**
 * @brief   This function send REQA command to a tag
 * 
 * @param[in]   dev:            Pointer to ST95 device descriptor
 * @param[out]  rxbuff:         Pointer to the receive buffer
 * @param[in]   size_rx_buff:   Size of the receive buffer
 * 
 * @return  0:  the command has been successfully executed
 * @return  1:  in case of an error
 */ 
static uint8_t _iso14443a_reqa(const st95_t * dev, uint8_t * rxbuff, uint16_t size_rx_buff)
{
	uint8_t data = ISO14443A_CMD_REQA;
    
	/* Control byte: Not used topaz format, Not SplitFrame, Not append CRC, 7 significant bits in last byte */	
    uint8_t ctrl_byte = ISO14443A_NUM_SIGN_BIT_7;
    
    if(_st95_cmd_send_receive(dev, &data, sizeof(data), ctrl_byte, rxbuff, size_rx_buff) == ST95_OK) {
        return ST95_OK;
    }

    return ST95_ERROR;
}

/**
 * @brief   This function send Anticollision command  (Level 1)
 * 
 * @param[in]   dev:            Pointer to ST95 device descriptor
 * @param[out]  rxbuff:         Pointer to the receive buffer
 * @param[in]   size_rx_buff:   Size of the receive buffer
 * 
 * @return  0:  the command has been successfully executed
 * @return  1:  in case of an error
 */
static uint8_t _iso14443a_anticollision_1(const st95_t * dev, uint8_t * rxbuff, uint16_t size_rx_buff)
{
	uint8_t data[2] = { ISO14443A_SELECT_LVL1, ISO14443A_NVB_20 };
       
	/* Control byte: Not used topaz format, Not SplitFrame, Not append CRC, 8 significant bits in last byte */	
    uint8_t ctrl_byte = ISO14443A_NUM_SIGN_BIT_8;
    
    if(_st95_cmd_send_receive(dev, data, sizeof(data), ctrl_byte, rxbuff, size_rx_buff) == ST95_OK) {
        return ST95_OK;
    }

    return ST95_ERROR;    
}

/**
 * @brief   This function send Anticollision command  (Level 2)
 * 
 * @param[in]   dev:            Pointer to ST95 device descriptor
 * @param[out]  rxbuff:         Pointer to the receive buffer
 * @param[in]   size_rx_buff:   Size of the receive buffer
 * 
 * @return  0:  the command has been successfully executed
 * @return  1:  in case of an error
 */
static uint8_t _iso14443a_anticollision_2(const st95_t * dev, uint8_t * rxbuff, uint16_t size_rx_buff)
{
	uint8_t data[2] = { ISO14443A_SELECT_LVL2, ISO14443A_NVB_20 };
    
	/* Control byte: Not used topaz format, Not SplitFrame, Not append CRC, 8 significant bits in last byte */	
    uint8_t ctrl_byte = ISO14443A_NUM_SIGN_BIT_8;
    
    if(_st95_cmd_send_receive(dev, data, sizeof(data), ctrl_byte, rxbuff, size_rx_buff) == ST95_OK) {
        return ST95_OK;
    }
    
    return ST95_ERROR;
}

/**
 * @brief   This function send Anticollision command  (Level 3)
 * 
 * @param[in]   dev:            Pointer to ST95 device descriptor
 * @param[out]  rxbuff:         Pointer to the receive buffer
 * @param[in]   size_rx_buff:   Size of the receive buffer
 * 
 * @return  0:  the command has been successfully executed
 * @return  1:  in case of an error
 */
static uint8_t _iso14443a_anticollision_3(const st95_t * dev, uint8_t * rxbuff, uint16_t size_rx_buff)
{
	uint8_t data[2] = { ISO14443A_SELECT_LVL3, ISO14443A_NVB_20 };
    
	/* Control byte: Not used topaz format, Not SplitFrame, Not append CRC, 8 significant bits in last byte */	
    uint8_t ctrl_byte = ISO14443A_NUM_SIGN_BIT_8;
    
   if(_st95_cmd_send_receive(dev, data, sizeof(data), ctrl_byte, rxbuff, size_rx_buff) == ST95_OK) {
        return ST95_OK;
    }
    
    return ST95_ERROR;
}

/**
 * @brief   This function send Select command  (Level 1) to a tag
 * 
 * @param[in]   dev:            Pointer to ST95 device descriptor
 * @param[in]   num:            Number of byte of UID in parameters
 * @param[in]   uid_sel:        Pointer on UID append to Select command
 * @param[out]  rxbuff:         Pointer to the receive buffer
 * @param[in]   size_rx_buff:   Size of the receive buffer
 * 
 * @return  0:  the command has been successfully executed
 * @return  1:  in case of an error
 */
static uint8_t _iso14443a_select_1(const st95_t * dev, uint8_t num, uint8_t * uid_sel, uint8_t * rxbuff, uint16_t size_rx_buff)
{
	uint8_t data[ISO14443A_CMD_MAX_BYTE] = { 0 };
    	
	data[0] = ISO14443A_SELECT_LVL1;
	data[1] = ISO14443A_NVB_70;
	memcpy(data + 2, uid_sel, num);
    
    /* Control byte: Not used topaz format, Not SplitFrame, Append CRC, 8 significant bits in last byte */	
    uint8_t ctrl_byte = ISO14443A_NUM_SIGN_BIT_8 | ISO14443A_APPEND_CRC;
    
    if(_st95_cmd_send_receive(dev, data, num + 2, ctrl_byte, rxbuff, size_rx_buff) == ST95_OK) {
        return ST95_OK;
    }
    
    return ST95_ERROR;   
}

/**
 * @brief   This function send Select command  (Level 2) to a tag
 * 
 * @param[in]   dev:            Pointer to ST95 device descriptor
 * @param[in]   num:            Number of byte of UID in parameters
 * @param[in]   uid_sel:        Pointer on UID append to Select command
 * @param[out]  rxbuff:         Pointer to the receive buffer
 * @param[in]   size_rx_buff:   Size of the receive buffer
 * 
 * @return  0:  the command has been successfully executed
 * @return  1:  in case of an error
 */
static uint8_t _iso14443a_select_2(const st95_t * dev, uint8_t num, uint8_t * uid_sel, uint8_t * rxbuff, uint16_t size_rx_buff)
{
	uint8_t data[ISO14443A_CMD_MAX_BYTE] = { 0 };
	
	data[0] = ISO14443A_SELECT_LVL2;
	data[1] = ISO14443A_NVB_70;
	memcpy(data + 2, uid_sel, num);
	
	/* Control byte: Not used topaz format, Not SplitFrame, Append CRC, 8 significant bits in last byte */	
    uint8_t ctrl_byte = ISO14443A_NUM_SIGN_BIT_8 | ISO14443A_APPEND_CRC;
    
   if(_st95_cmd_send_receive(dev, data, num + 2, ctrl_byte, rxbuff, size_rx_buff) == ST95_OK) {
        return ST95_OK;
    }
    
    return ST95_ERROR;   
}

/**
 * @brief   This function send Select command  (Level 3) to a tag
 * 
 * @param[in]   dev:            Pointer to ST95 device descriptor
 * @param[in]   num:            Number of byte of UID in parameters
 * @param[in]   uid_sel:        Pointer on UID append to Select command
 * @param[out]  rxbuff:         Pointer to the receive buffer
 * @param[in]   size_rx_buff:   Size of the receive buffer
 * 
 * @return  0:  the command has been successfully executed
 * @return  1:  in case of an error
 */
static uint8_t _iso14443a_select_3(const st95_t * dev, uint8_t num, uint8_t * uid_sel, uint8_t * rxbuff, uint16_t size_rx_buff)
{
    uint8_t data[ISO14443A_CMD_MAX_BYTE] = { 0 };

    data[0] = ISO14443A_SELECT_LVL3;
    data[1] = ISO14443A_NVB_70;
    memcpy(data + 2, uid_sel, num);

    /* Control byte: Not used topaz format, Not SplitFrame, Append CRC, 8 significant bits in last byte */	
    uint8_t ctrl_byte = ISO14443A_NUM_SIGN_BIT_8 | ISO14443A_APPEND_CRC;

    if(_st95_cmd_send_receive(dev, data, num + 2, ctrl_byte, rxbuff, size_rx_buff) == ST95_OK) {
        return ST95_OK;
    }

    return ST95_ERROR;   
}

/**
 * @brief   This function get iso14443A UID card
 * 
 * @param[in]   dev:            Pointer to ST95 device descriptor
 * @param[out]  length_uid:     Pointer to the length UID
 * @param[out]  uid:            Pointer to the card UID
 * @param[out]  sak:            Pointer to the card SAK(Select ACK) byte
 * 
 * @return  0:  the command has been successfully executed
 * @return  1:  in case of an error
 */
int iso14443a_get_uid(const st95_t * dev, uint8_t * length_uid, uint8_t * uid, uint8_t * sak)
{  
    uint8_t iso_rxbuf[ISO14443A_ANSWER_MAX_BYTE] = { 0 };

    if(_iso14443a_reqa(dev, iso_rxbuf, ISO14443A_ANSWER_MAX_BYTE) != ST95_OK) {
        return ST95_ERROR;
    }

    // UIDsize : (2 bits) value: 0 for single, 1 for double,  2 for triple and 3 for RFU
    uint8_t uid_size = _iso14443a_get_uidsize(iso_rxbuf[ISO14443A_OFFSET_ATQA_FIRST_BYTE]);

    // Select cascade level 1
    if(_iso14443a_anticollision_1(dev, iso_rxbuf, ISO14443A_ANSWER_MAX_BYTE) != ST95_OK) {
        return ST95_ERROR;
    }

    //  Check BCC
    if(!_iso14443a_check_bcc(4, iso_rxbuf + ST95_DATA_OFFSET, iso_rxbuf[ST95_DATA_OFFSET + 4])) {
        return ST95_ERROR;
    }
    // copy UID from ST95 response
    if (uid_size == ISO14443A_ATQA_SINGLE){
        memcpy(uid, &iso_rxbuf[ST95_DATA_OFFSET], ISO14443A_UID_SINGLE );
    }
    else {
        memcpy(uid, &iso_rxbuf[ST95_DATA_OFFSET + 1], ISO14443A_UID_SINGLE - 1 );
    }
    // Send Select command 1
    if(_iso14443a_select_1(dev, ISO14443A_NUM_BYTE_SELECT, &iso_rxbuf[ISO14443A_OFFSET_UID_SELECT], iso_rxbuf, ISO14443A_ANSWER_MAX_BYTE) != ST95_OK) {
        return ST95_ERROR;
    }

    if(_iso14443a_is_uid_complete(iso_rxbuf[ISO14443A_OFFSET_SAK_BYTE]) == ST95_OK) {
        *sak = iso_rxbuf[ISO14443A_OFFSET_SAK_BYTE];
        *length_uid = ISO14443A_UID_SINGLE;
        return ST95_OK;
    }

    // Select cascade level 2
    if(_iso14443a_anticollision_2(dev, iso_rxbuf, ISO14443A_ANSWER_MAX_BYTE) != ST95_OK) {
        return ST95_ERROR;
    }

    //  Check BCC
    if(!_iso14443a_check_bcc(4, iso_rxbuf + ST95_DATA_OFFSET, iso_rxbuf[ST95_DATA_OFFSET + 4])) {
        return ST95_ERROR;
    }

    // copy UID from ST95 response
    if (uid_size == ISO14443A_ATQA_DOUBLE) {
        memcpy(&uid[ISO14443A_UID_SINGLE - 1], &iso_rxbuf[ST95_DATA_OFFSET], ISO14443A_UID_SINGLE );
    }
    else {
        memcpy(&uid[ISO14443A_UID_SINGLE - 1], &iso_rxbuf[ST95_DATA_OFFSET], ISO14443A_UID_SINGLE - 1);
    }
    // Send Select command 2
    if(_iso14443a_select_2(dev, ISO14443A_NUM_BYTE_SELECT, &iso_rxbuf[ISO14443A_OFFSET_UID_SELECT], iso_rxbuf, ISO14443A_ANSWER_MAX_BYTE) != ST95_OK) {
        return ST95_ERROR;
    }

    if(_iso14443a_is_uid_complete(iso_rxbuf[ISO14443A_OFFSET_SAK_BYTE]) == ST95_OK) {
        *sak = iso_rxbuf[ISO14443A_OFFSET_SAK_BYTE];
        *length_uid = ISO14443A_UID_DOUBLE;
        return ST95_OK;
    }

    // Select cascade level 2
    if(_iso14443a_anticollision_3(dev, iso_rxbuf, ISO14443A_ANSWER_MAX_BYTE) != ST95_OK) {
        return ST95_ERROR;
    }

    //  Check BCC
    if(!_iso14443a_check_bcc(4, iso_rxbuf + ST95_DATA_OFFSET, iso_rxbuf[ST95_DATA_OFFSET + 4])) {
        return ST95_ERROR;
    }

    // copy UID from ST95 response
    if (uid_size == ISO14443A_ATQA_TRIPLE) {
        memcpy(&uid[ISO14443A_UID_DOUBLE - 1], &iso_rxbuf[ST95_DATA_OFFSET], ISO14443A_UID_DOUBLE );
    }

    // Send Select command 3
    if(_iso14443a_select_3(dev, ISO14443A_NUM_BYTE_SELECT, &iso_rxbuf[ISO14443A_OFFSET_UID_SELECT], iso_rxbuf, ISO14443A_ANSWER_MAX_BYTE) != ST95_OK) {
        return ST95_ERROR;
    }

    if(_iso14443a_is_uid_complete(iso_rxbuf[ISO14443A_OFFSET_SAK_BYTE]) == ST95_OK) {
        *sak = iso_rxbuf[ISO14443A_OFFSET_SAK_BYTE];
        *length_uid = ISO14443A_UID_TRIPLE;
        return ST95_OK;
    }

    return ST95_ERROR;
}