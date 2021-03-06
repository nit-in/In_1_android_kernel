/*
 * aw88194.c   aw88194 codec module
 *
 * Copyright (c) 2018 AWINIC Technology CO., LTD
 *
 *  Author: Nick Li <liweilei@awinic.com.cn>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */

#include <linux/module.h>
#include <linux/i2c.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <linux/of_gpio.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/firmware.h>
#include <linux/i2c.h>
#include <linux/debugfs.h>
#include <linux/version.h>
#include <linux/input.h>
#include <linux/timer.h>
#include <linux/workqueue.h>
#include <linux/hrtimer.h>
#include <linux/syscalls.h>
#include <sound/tlv.h>
#include "aw881xx.h"
#include "aw88194.h"
#include "aw88194_reg.h"


static uint16_t aw88194_base_addr[] = {
	AW88194_SPK_REG_ADDR,
	AW88194_SPK_DSP_FW_ADDR,
	AW88194_SPK_DSP_CFG_ADDR,
	AW88194_VOICE_REG_ADDR,
	AW88194_VOICE_DSP_FW_ADDR,
	AW88194_VOICE_DSP_CFG_ADDR,
	AW88194_FM_REG_ADDR,
	AW88194_FM_DSP_FW_ADDR,
	AW88194_FM_DSP_CFG_ADDR,
	AW88194_RCV_REG_ADDR,
	AW88194_RCV_DSP_FW_ADDR,
	AW88194_RCV_DSP_CFG_ADDR,
};

const unsigned char aw88194_reg_access[AW88194_REG_MAX] = {
	[AW88194_REG_ID] = AW88194_REG_RD_ACCESS,
	[AW88194_REG_SYSST] = AW88194_REG_RD_ACCESS,
	[AW88194_REG_SYSINT] = AW88194_REG_RD_ACCESS,
	[AW88194_REG_SYSINTM] = AW88194_REG_RD_ACCESS | AW88194_REG_WR_ACCESS,
	[AW88194_REG_SYSCTRL] = AW88194_REG_RD_ACCESS | AW88194_REG_WR_ACCESS,
	[AW88194_REG_I2SCTRL] = AW88194_REG_RD_ACCESS | AW88194_REG_WR_ACCESS,
	[AW88194_REG_I2SCFG1] = AW88194_REG_RD_ACCESS | AW88194_REG_WR_ACCESS,
	[AW88194_REG_PWMCTRL] = AW88194_REG_RD_ACCESS | AW88194_REG_WR_ACCESS,
	[AW88194_REG_HAGCCFG1] = AW88194_REG_RD_ACCESS | AW88194_REG_WR_ACCESS,
	[AW88194_REG_HAGCCFG2] = AW88194_REG_RD_ACCESS | AW88194_REG_WR_ACCESS,
	[AW88194_REG_HAGCCFG3] = AW88194_REG_RD_ACCESS | AW88194_REG_WR_ACCESS,
	[AW88194_REG_HAGCCFG4] = AW88194_REG_RD_ACCESS | AW88194_REG_WR_ACCESS,
	[AW88194_REG_HAGCCFG5] = AW88194_REG_RD_ACCESS | AW88194_REG_WR_ACCESS,
	[AW88194_REG_HAGCCFG6] = AW88194_REG_RD_ACCESS | AW88194_REG_WR_ACCESS,
	[AW88194_REG_HAGCCFG7] = AW88194_REG_RD_ACCESS | AW88194_REG_WR_ACCESS,
	[AW88194_REG_HAGCCFG8] = AW88194_REG_RD_ACCESS | AW88194_REG_WR_ACCESS,
	[AW88194_REG_SYSCTRL2] = AW88194_REG_RD_ACCESS | AW88194_REG_WR_ACCESS,
	[AW88194_REG_PRODID] = AW88194_REG_RD_ACCESS,
	[AW88194_REG_DBGCTRL] = AW88194_REG_RD_ACCESS | AW88194_REG_WR_ACCESS,
	[AW88194_REG_I2SCFG2] = AW88194_REG_RD_ACCESS | AW88194_REG_WR_ACCESS,
	[AW88194_REG_I2SSTAT] = AW88194_REG_RD_ACCESS,
	[AW88194_REG_I2SCAPCNT] = AW88194_REG_RD_ACCESS,
	[AW88194_REG_TM] = AW88194_REG_RD_ACCESS | AW88194_REG_WR_ACCESS,
	[AW88194_REG_CRCIN] = AW88194_REG_RD_ACCESS | AW88194_REG_WR_ACCESS,
	[AW88194_REG_CRCOUT] = AW88194_REG_RD_ACCESS,
	[AW88194_REG_DSPMADD] = AW88194_REG_RD_ACCESS | AW88194_REG_WR_ACCESS,
	[AW88194_REG_DSPMDAT] = AW88194_REG_RD_ACCESS | AW88194_REG_WR_ACCESS,
	[AW88194_REG_WDT] = AW88194_REG_RD_ACCESS,
	[AW88194_REG_ACR1] = AW88194_REG_RD_ACCESS | AW88194_REG_WR_ACCESS,
	[AW88194_REG_ACR2] = AW88194_REG_RD_ACCESS | AW88194_REG_WR_ACCESS,
	[AW88194_REG_ASR1] = AW88194_REG_RD_ACCESS,
	[AW88194_REG_ASR2] = AW88194_REG_RD_ACCESS,
	[AW88194_REG_DSPCFG] = AW88194_REG_RD_ACCESS | AW88194_REG_WR_ACCESS,
	[AW88194_REG_ASR3] = AW88194_REG_RD_ACCESS,
	[AW88194_REG_ASR4] = AW88194_REG_RD_ACCESS,
	[AW88194_REG_BSTCTRL1] = AW88194_REG_RD_ACCESS | AW88194_REG_WR_ACCESS,
	[AW88194_REG_BSTCTRL2] = AW88194_REG_RD_ACCESS | AW88194_REG_WR_ACCESS,
	[AW88194_REG_BSTCTRL3] = AW88194_REG_RD_ACCESS | AW88194_REG_WR_ACCESS,
	[AW88194_REG_BSTDBG1] = AW88194_REG_RD_ACCESS | AW88194_REG_WR_ACCESS,
	[AW88194_REG_BSTDBG2] = AW88194_REG_RD_ACCESS | AW88194_REG_WR_ACCESS,
	[AW88194_REG_PLLCTRL1] = AW88194_REG_RD_ACCESS | AW88194_REG_WR_ACCESS,
	[AW88194_REG_PLLCTRL2] = AW88194_REG_RD_ACCESS | AW88194_REG_WR_ACCESS,
	[AW88194_REG_AMPDBG1] = AW88194_REG_RD_ACCESS | AW88194_REG_WR_ACCESS,
	[AW88194_REG_AMPDBG2] = AW88194_REG_RD_ACCESS | AW88194_REG_WR_ACCESS,
	[AW88194_REG_CDACTRL1] = AW88194_REG_RD_ACCESS | AW88194_REG_WR_ACCESS,
	[AW88194_REG_CDACTRL2] = AW88194_REG_RD_ACCESS | AW88194_REG_WR_ACCESS,
	[AW88194_REG_ISECTRL1] = AW88194_REG_RD_ACCESS | AW88194_REG_WR_ACCESS,
	[AW88194_REG_SADCCTRL] = AW88194_REG_RD_ACCESS | AW88194_REG_WR_ACCESS,
	[AW88194_REG_VBAT] = AW88194_REG_RD_ACCESS,
	[AW88194_REG_TEMP] = REG_RD_ACCESS,
	[AW88194_REG_TEST] = REG_RD_ACCESS | REG_WR_ACCESS,
	[AW88194_REG_TEST2] = REG_RD_ACCESS | REG_WR_ACCESS,
	[AW88194_REG_EFCTR1] = REG_RD_ACCESS | REG_WR_ACCESS,
	[AW88194_REG_EFCTR2] = REG_RD_ACCESS | REG_WR_ACCESS,
	[AW88194_REG_EFWH] = REG_RD_ACCESS | REG_WR_ACCESS,
	[AW88194_REG_EFWM] = REG_RD_ACCESS | REG_WR_ACCESS,
	[AW88194_REG_EFWL] = REG_RD_ACCESS | REG_WR_ACCESS,
	[AW88194_REG_EFRH] = REG_RD_ACCESS,
	[AW88194_REG_EFRM] = REG_RD_ACCESS,
	[AW88194_REG_EFRL] = REG_RD_ACCESS,
	[AW88194_REG_TESTDET] = REG_RD_ACCESS,
};

/******************************************************
 *
 * aw88194 i2c write/read
 *
 ******************************************************/
static int aw88194_reg_writes(struct aw881xx *aw88194,
	uint8_t reg_addr, uint8_t *buf, uint16_t len)
{
	int ret = -1;

	ret = aw881xx_reg_writes(aw88194, reg_addr, buf, len);
	if (ret < 0)
		pr_err("%s: aw881x_i2c_writes fail, ret=%d",
			__func__, ret);

	return ret;
}

/*
static int aw88194_reg_reads(struct aw881xx *aw88194,
	uint8_t reg_addr, uint8_t *buf, uint16_t len)
{
	int ret = -1;

	ret = aw881xx_reg_reads(aw88194, reg_addr, buf, len);
	if (ret < 0)
		pr_err("%s: aw881xx_reg_reads fail, ret=%d",
			__func__, ret);

	return ret;
}
*/

static int aw88194_reg_write(struct aw881xx *aw88194,
	uint8_t reg_addr, uint16_t reg_data)
{
	int ret = -1;

	ret = aw881xx_reg_write(aw88194, reg_addr, reg_data);
	if (ret < 0)
		pr_err("%s: aw881xx_reg_write fail, ret=%d",
			__func__, ret);

	return ret;

}

static int aw88194_reg_read(struct aw881xx *aw88194,
	uint8_t reg_addr, uint16_t *reg_data)
{
	int ret = -1;

	ret = aw881xx_reg_read(aw88194, reg_addr, reg_data);
	if (ret < 0)
		pr_err("%s: aw881xx_reg_read fail, ret=%d",
			__func__, ret);

	return ret;
}

static int aw88194_reg_write_bits(struct aw881xx *aw88194,
	uint8_t reg_addr, uint16_t mask, uint16_t reg_data)
{
	int ret = -1;

	ret = aw881xx_reg_write_bits(aw88194, reg_addr, mask, reg_data);
	if (ret < 0)
		pr_err("%s: aw881xx_reg_write_bits fail, ret=%d",
			__func__, ret);

	return ret;
}

static int aw88194_dsp_write(struct aw881xx *aw88194,
	uint16_t dsp_addr, uint16_t dsp_data)
{
	int ret = -1;

	ret = aw881xx_dsp_write(aw88194, dsp_addr, dsp_data);
	if (ret < 0)
		pr_err("%s: aw881xx_dsp_write fail, ret=%d",
			__func__, ret);

	return ret;
}

static int aw88194_dsp_read(struct aw881xx *aw88194,
	uint16_t dsp_addr, uint16_t *dsp_data)
{
	int ret = -1;

	ret = aw881xx_dsp_read(aw88194, dsp_addr, dsp_data);
	if (ret < 0)
		pr_err("%s: aw881xx_dsp_read fail, ret=%d",
			__func__, ret);

	return ret;
}

/******************************************************
 *
 * aw88194 cali store
 *
 ******************************************************/
void aw88194_set_cali_re(void *aw881xx)
{
	struct aw881xx *aw88194 = (struct aw881xx *)aw881xx;
	uint16_t cali_re = 0;
	uint16_t dsp_ra = 0;

	aw88194_dsp_read(aw88194, AW88194_DSP_REG_CFG_ADPZ_RA,
		&dsp_ra);

	cali_re = aw88194->cali_re + dsp_ra;

	aw88194_dsp_write(aw88194, AW88194_DSP_REG_CFG_ADPZ_RE,
		cali_re);
}


/******************************************************
 *
 * aw88194 control
 *
 ******************************************************/
static void aw88194_run_mute(struct aw881xx *aw88194, bool mute)
{
	pr_debug("%s: enter\n", __func__);

	if (mute) {
		aw88194_reg_write_bits(aw88194, AW88194_REG_PWMCTRL,
			AW88194_BIT_PWMCTRL_HMUTE_MASK,
			AW88194_BIT_PWMCTRL_HMUTE_ENABLE);
	} else {
		aw88194_reg_write_bits(aw88194, AW88194_REG_PWMCTRL,
			AW88194_BIT_PWMCTRL_HMUTE_MASK,
			AW88194_BIT_PWMCTRL_HMUTE_DISABLE);
	}
}

void aw88194_run_pwd(void *aw881xx, bool pwd)
{
	struct aw881xx *aw88194 = (struct aw881xx *)aw881xx;

	pr_debug("%s: enter\n", __func__);

	if (pwd) {
		aw88194_reg_write_bits(aw88194, AW88194_REG_SYSCTRL,
			AW88194_BIT_SYSCTRL_PW_MASK,
			AW88194_BIT_SYSCTRL_PW_PDN);
	} else {
		aw88194_reg_write_bits(aw88194, AW88194_REG_SYSCTRL,
			AW88194_BIT_SYSCTRL_PW_MASK,
			AW88194_BIT_SYSCTRL_PW_ACTIVE);
	}
}

static void aw88194_dsp_enable(struct aw881xx *aw88194, bool dsp)
{
	pr_debug("%s: enter\n", __func__);

	if (dsp) {
		aw88194_reg_write_bits(aw88194, AW88194_REG_SYSCTRL,
			AW88194_BIT_SYSCTRL_DSP_MASK,
			AW88194_BIT_SYSCTRL_DSP_WORK);
	} else {
		aw88194_reg_write_bits(aw88194, AW88194_REG_SYSCTRL,
			AW88194_BIT_SYSCTRL_DSP_MASK,
			AW88194_BIT_SYSCTRL_DSP_BYPASS);
	}
}

static void aw88194_memclk_select(struct aw881xx *aw88194, unsigned char flag)
{
	pr_debug("%s: enter\n", __func__);

	if (flag == AW88194_MEMCLK_PLL) {
		aw88194_reg_write_bits(aw88194, AW88194_REG_SYSCTRL2,
			AW88194_BIT_SYSCTRL2_MEMCLK_MASK,
			AW88194_BIT_SYSCTRL2_MEMCLK_PLL);
	} else if (flag == AW88194_MEMCLK_OSC) {
		aw88194_reg_write_bits(aw88194, AW88194_REG_SYSCTRL2,
			AW88194_BIT_SYSCTRL2_MEMCLK_MASK,
			AW88194_BIT_SYSCTRL2_MEMCLK_OSC);
	} else {
		pr_err("%s: unknown memclk config, flag=0x%x\n",
			__func__, flag);
	}
}

static int aw88194_sysst_check(struct aw881xx *aw88194)
{
	int ret = -1;
	unsigned char i;
	uint16_t reg_val = 0;

	for (i = 0; i < AW88194_SYSST_CHECK_MAX; i++) {
		aw88194_reg_read(aw88194, AW88194_REG_SYSST, &reg_val);
		if ((reg_val & (~AW88194_BIT_SYSST_CHECK_MASK)) ==
			AW88194_BIT_SYSST_CHECK) {
			ret = 0;
			return ret;
		} else {
			pr_debug("%s: check fail, cnt=%d, reg_val=0x%04x\n",
				__func__, i, reg_val);
			msleep(2);
		}
	}
	pr_info("%s: check fail\n", __func__);

	return 0;//return ret; prize wyq 20200117 temp solution to fix mute issue
}

static int aw88194_get_sysint(struct aw881xx *aw88194, uint16_t *sysint)
{
	int ret = -1;
	uint16_t reg_val = 0;

	ret = aw88194_reg_read(aw88194, AW88194_REG_SYSINT, &reg_val);
	if (ret < 0)
		pr_info("%s: read sysint fail, ret=%d\n", __func__, ret);
	else
		*sysint = reg_val;

	return ret;
}

int aw88194_get_iis_status(void *aw881xx)
{
	struct aw881xx *aw88194 = (struct aw881xx *)aw881xx;
	int ret = -1;
	uint16_t reg_val = 0;

	pr_debug("%s: enter\n", __func__);

	aw88194_reg_read(aw88194, AW88194_REG_SYSST, &reg_val);
	if (reg_val & AW88194_BIT_SYSST_PLLS)
		ret = 0;

	return ret;
}

static int aw88194_get_dsp_status(struct aw881xx *aw88194)
{
	int ret = -1;
	uint16_t reg_val = 0;

	pr_debug("%s: enter\n", __func__);

	aw88194_reg_read(aw88194, AW88194_REG_WDT, &reg_val);
	if (reg_val)
		ret = 0;

	return ret;
}

int aw88194_get_dsp_config(void *aw881xx)
{
	struct aw881xx *aw88194 = (struct aw881xx *)aw881xx;
	int ret = -1;
	uint16_t reg_val = 0;

	pr_debug("%s: enter\n", __func__);

	aw88194_reg_read(aw88194, AW88194_REG_SYSCTRL, &reg_val);
	if (reg_val & AW88194_BIT_SYSCTRL_DSP_BYPASS)
		aw88194->dsp_cfg = AW88194_DSP_BYPASS;
	else
		aw88194->dsp_cfg = AW88194_DSP_WORK;

	return ret;
}

int aw88194_get_hmute(void *aw881xx)
{
	struct aw881xx *aw88194 = (struct aw881xx *)aw881xx;
	int ret = -1;
	uint16_t reg_val = 0;

	pr_debug("%s: enter\n", __func__);

	aw88194_reg_read(aw88194, AW88194_REG_PWMCTRL, &reg_val);
	if (reg_val & AW88194_BIT_PWMCTRL_HMUTE_ENABLE)
		ret = 1;
	else
		ret = 0;

	return ret;
}

static int aw88194_get_icalk(struct aw881xx *aw88194, int16_t *icalk)
{
	int ret = -1;
	uint16_t reg_val = 0;
	uint16_t reg_icalk = 0;

	ret = aw88194_reg_read(aw88194, AW88194_REG_EFRM, &reg_val);
	reg_icalk = (uint16_t) reg_val & AW88194_EF_ISENSE_GAINERR_SLP_MASK;

	if (reg_icalk & AW88194_EF_ISENSE_GAINERR_SLP_SIGN_MASK)
		reg_icalk = reg_icalk | AW88194_EF_ISENSE_GAINERR_SLP_NEG;

	*icalk = (int16_t) reg_icalk;

	return ret;
}

static int aw88194_get_vcalk(struct aw881xx *aw88194, int16_t *vcalk)
{
	int ret = -1;
	uint16_t reg_val = 0;
	uint16_t reg_vcalk = 0;

	ret = aw88194_reg_read(aw88194, AW88194_REG_EFRL, &reg_val);
	reg_val = reg_val >> AW88194_EF_VSENSE_GAIN_SHIFT;

	reg_vcalk = (uint16_t) reg_val & AW88194_EF_VSENSE_GAIN_MASK;

	if (reg_vcalk & AW88194_EF_VSENSE_GAIN_SIGN_MASK)
		reg_vcalk = reg_vcalk | AW88194_EF_VSENSE_GAIN_NEG;

	*vcalk = (int16_t) reg_vcalk;

	return ret;
}

static int aw88194_dsp_set_vcalb(struct aw881xx *aw88194)
{
	int ret = -1;
	uint16_t reg_val = 0;
	int vcalb;
	int icalk;
	int vcalk;
	int16_t icalk_val = 0;
	int16_t vcalk_val = 0;

	ret = aw88194_get_icalk(aw88194, &icalk_val);
	ret = aw88194_get_vcalk(aw88194, &vcalk_val);

	icalk = AW88194_CABL_BASE_VALUE + AW88194_ICABLK_FACTOR * icalk_val;
	vcalk = AW88194_CABL_BASE_VALUE + AW88194_VCABLK_FACTOR * vcalk_val;

	vcalb = (AW88194_VCAL_FACTOR * AW88194_VSCAL_FACTOR /
		AW88194_ISCAL_FACTOR) * vcalk / icalk;

	reg_val = (uint16_t)vcalb;
	pr_debug("%s: icalk=%d, vcalk=%d, vcalb=%d, reg_val=%d\n",
		__func__, icalk, vcalk, vcalb, reg_val);

	ret = aw88194_dsp_write(aw88194, AW88194_DSP_REG_VCALB, reg_val);

	return ret;
}

static int aw88194_set_intmask(struct aw881xx *aw88194, bool flag)
{
	int ret = -1;

	if (flag)
		ret = aw88194_reg_write(aw88194, AW88194_REG_SYSINTM,
			aw88194->intmask);
	else
		ret = aw88194_reg_write(aw88194, AW88194_REG_SYSINTM,
			AW88194_REG_SYSINTM_MASK);
	return ret;
}

static int aw88194_start(void *aw881xx)
{
	struct aw881xx *aw88194 = (struct aw881xx *)aw881xx;

	int ret = -1;

	pr_debug("%s: enter\n", __func__);

	aw88194_run_pwd(aw88194, false);
	ret = aw88194_sysst_check(aw88194);
	if (ret < 0) {
		aw88194_run_mute(aw88194, true);
	} else {
		aw88194_run_mute(aw88194, false);
		aw88194_set_intmask(aw88194, true);

		if ((aw88194->monitor_flag) &&
			((aw88194->scene_mode == AW881XX_SPK_MODE) ||
			(aw88194->scene_mode == AW881XX_VOICE_MODE) ||
			(aw88194->scene_mode == AW881XX_FM_MODE))) {
			aw881xx_monitor_start(aw88194);
		}
	}

	return ret;
}

static void aw88194_stop(void *aw881xx)
{
	struct aw881xx *aw88194 = (struct aw881xx *)aw881xx;

	pr_debug("%s: enter\n", __func__);

	aw88194_set_intmask(aw88194, false);
	aw88194_run_mute(aw88194, true);
	aw88194_run_pwd(aw88194, true);

	if (aw88194->monitor_flag)
		aw881xx_monitor_stop(aw88194);
}

/******************************************************
 *
 * aw88194 dsp
 *
 ******************************************************/
static int aw88194_dsp_check(struct aw881xx *aw88194)
{
	int ret = -1;
	uint16_t reg_val = 0;
	uint16_t iis_check_max = 5;
	uint16_t i = 0;

	pr_debug("%s: enter\n", __func__);

	aw88194_run_pwd(aw88194, false);
	aw88194_memclk_select(aw88194, AW88194_MEMCLK_PLL);
	for (i = 0; i < iis_check_max; i++) {
		ret = aw88194_get_iis_status(aw88194);
		if (ret < 0) {
			pr_err("%s: iis signal check error, reg=0x%x\n",
				__func__, reg_val);
			msleep(2);
		} else {
			if (aw88194->dsp_cfg == AW88194_DSP_WORK) {
				aw88194_dsp_enable(aw88194, false);
				aw88194_dsp_enable(aw88194, true);
				msleep(1);
				ret = aw88194_get_dsp_status(aw88194);
				if (ret < 0) {
					pr_err("%s: dsp wdt status error=%d\n",
						__func__, ret);
				} else {
					return 0;
				}
			} else if (aw88194->dsp_cfg == AW88194_DSP_BYPASS) {
				return 0;
			} else {
				pr_err("%s: unknown dsp cfg=%d\n",
					__func__, aw88194->dsp_cfg);
				return -EINVAL;
			}
		}
	}
	return -EINVAL;
}

static void aw88194_dsp_container_update(struct aw881xx *aw88194,
	struct aw88194_container *aw88194_cont, uint16_t base)
{
	int i = 0;
#ifdef AW88194_DSP_I2C_WRITES
	uint8_t tmp_val = 0;
	uint32_t tmp_len = 0;
#else
	uint16_t reg_val = 0;
#endif

	pr_debug("%s enter\n", __func__);

#ifdef AW88194_DSP_I2C_WRITES
	/* i2c writes */
	aw88194_reg_write(aw88194, AW88194_REG_DSPMADD, base);
	for (i = 0; i < aw88194_cont->len; i += 2) {
		tmp_val = aw88194_cont->data[i + 0];
		aw88194_cont->data[i + 0] = aw88194_cont->data[i + 1];
		aw88194_cont->data[i + 1] = tmp_val;
	}
	for (i = 0; i < aw88194_cont->len;
		i += AW88194_MAX_RAM_WRITE_BYTE_SIZE) {
		if ((aw88194_cont->len - i) < AW88194_MAX_RAM_WRITE_BYTE_SIZE)
			tmp_len = aw88194_cont->len - i;
		else
			tmp_len = AW88194_MAX_RAM_WRITE_BYTE_SIZE;
		aw88194_reg_writes(aw88194, AW88194_REG_DSPMDAT,
			&aw88194_cont->data[i], tmp_len);
	}
#else
	/* i2c write */
	aw88194_reg_write(aw88194, AW88194_REG_DSPMADD, base);
	for (i = 0; i < aw88194_cont->len; i += 2) {
		reg_val = (aw88194_cont->data[i + 1] << 8) +
			aw88194_cont->data[i + 0];
		aw88194_reg_write(aw88194, AW88194_REG_DSPMDAT, reg_val);
	}
#endif
	pr_debug("%s: exit\n", __func__);
}

static int aw88194_dsp_update_cali_re(struct aw881xx *aw88194)
{
	return aw881xx_dsp_update_cali_re(aw88194);
};

static void aw88194_dsp_cfg_loaded(const struct firmware *cont, void *context)
{
	struct aw881xx *aw88194 = context;
	struct aw88194_container *aw88194_cfg;
	int ret = -1;

	if (!cont) {
		pr_err("%s: failed to read %s\n",
			__func__, aw88194->cfg_name[aw88194->cfg_num]);
		release_firmware(cont);
		return;
	}

	pr_info("%s: loaded %s - size: %zu\n", __func__,
		aw88194->cfg_name[aw88194->cfg_num], cont ? cont->size : 0);

	aw88194_cfg = kzalloc(cont->size + sizeof(int), GFP_KERNEL);
	if (!aw88194_cfg) {
		release_firmware(cont);
		pr_err("%s: error allocating memory\n", __func__);
		return;
	}
	aw88194->dsp_cfg_len = cont->size;
	aw88194_cfg->len = cont->size;
	memcpy(aw88194_cfg->data, cont->data, cont->size);
	release_firmware(cont);

	if (aw88194->work_flag == false) {
		kfree(aw88194_cfg);
		pr_info("%s: mode_flag = %d\n", __func__, aw88194->work_flag);
		return;
	}

	mutex_lock(&aw88194->lock);
	aw88194_dsp_container_update(aw88194, aw88194_cfg,
		aw88194_base_addr[aw88194->cfg_num]);

	kfree(aw88194_cfg);

	aw88194_dsp_update_cali_re(aw88194);
	aw88194_dsp_set_vcalb(aw88194);

	ret = aw88194_dsp_check(aw88194);
	if (ret < 0) {
		aw88194->init = AW88194_INIT_NG;
		aw88194_run_mute(aw88194, true);
		pr_info("%s: fw/cfg update error\n", __func__);
	} else {
		pr_info("%s: fw/cfg update complete\n", __func__);

		ret = aw88194_start(aw88194);
		if (ret < 0) {
			pr_err("%s: start fail, ret=%d\n", __func__, ret);
		} else {
			pr_info("%s: start success\n", __func__);
			aw88194->init = AW88194_INIT_OK;
		}
	}
	mutex_unlock(&aw88194->lock);
}

static int aw88194_load_dsp_cfg(struct aw881xx *aw88194)
{
	pr_info("%s enter\n", __func__);

	aw88194->cfg_num++;

	return request_firmware_nowait(THIS_MODULE, FW_ACTION_HOTPLUG,
			aw88194->cfg_name[aw88194->cfg_num],
			aw88194->dev, GFP_KERNEL, aw88194,
			aw88194_dsp_cfg_loaded);
}

static void aw88194_dsp_fw_loaded(const struct firmware *cont, void *context)
{
	struct aw881xx *aw88194 = context;
	struct aw88194_container *aw88194_cfg;
	int ret = -1;

	if (!cont) {
		pr_err("%s: failed to read %s\n",
			__func__, aw88194->cfg_name[aw88194->cfg_num]);
		release_firmware(cont);
		return;
	}

	pr_info("%s: loaded %s - size: %zu\n",
		__func__, aw88194->cfg_name[aw88194->cfg_num],
		cont ? cont->size : 0);

	aw88194_cfg = kzalloc(cont->size + sizeof(int), GFP_KERNEL);
	if (!aw88194_cfg) {
		release_firmware(cont);
		pr_err("%s: error allocating memory\n", __func__);
		return;
	}
	aw88194->dsp_fw_len = cont->size;
	aw88194_cfg->len = cont->size;
	memcpy(aw88194_cfg->data, cont->data, cont->size);
	release_firmware(cont);

	if (aw88194->work_flag == true) {
		mutex_lock(&aw88194->lock);
		aw88194_dsp_container_update(aw88194, aw88194_cfg,
			aw88194_base_addr[aw88194->cfg_num]);
		mutex_unlock(&aw88194->lock);
	}
	kfree(aw88194_cfg);

	if (aw88194->work_flag == false) {
		pr_info("%s: mode_flag = %d\n",
			__func__, aw88194->work_flag);
		return;
	}

	ret = aw88194_load_dsp_cfg(aw88194);
	if (ret < 0) {
		pr_err("%s: cfg loading requested failed: %d\n",
			__func__, ret);
	}
}

static int aw88194_load_dsp_fw(struct aw881xx *aw88194)
{
	int ret = -1;

	pr_info("%s enter\n", __func__);

	aw88194->cfg_num++;

	if (aw88194->work_flag == true) {
		mutex_lock(&aw88194->lock);
		aw88194_run_mute(aw88194, true);
		aw88194_dsp_enable(aw88194, false);
		aw88194_memclk_select(aw88194, AW88194_MEMCLK_PLL);
		mutex_unlock(&aw88194->lock);
	} else {
		pr_info("%s: mode_flag = %d\n",
			__func__, aw88194->work_flag);
		return ret;
	}

	return request_firmware_nowait(THIS_MODULE, FW_ACTION_HOTPLUG,
			aw88194->cfg_name[aw88194->cfg_num],
			aw88194->dev, GFP_KERNEL, aw88194,
			aw88194_dsp_fw_loaded);
}

static void aw88194_update_dsp(void *aw881xx)
{
	struct aw881xx *aw88194 = (struct aw881xx *)aw881xx;
	uint16_t iis_check_max = 5;
	int i = 0;
	int ret = -1;

	pr_info("%s: enter\n", __func__);

	aw881xx_get_cfg_shift(aw88194);
	aw88194_get_dsp_config(aw88194);

	for (i = 0; i < iis_check_max; i++) {
		mutex_lock(&aw88194->lock);
		ret = aw88194_get_iis_status(aw88194);
		mutex_unlock(&aw88194->lock);
		if (ret < 0) {
			pr_err("%s: get no iis signal, ret=%d\n",
				__func__, ret);
			msleep(2);
		} else {
			ret = aw88194_load_dsp_fw(aw88194);
			if (ret < 0) {
				pr_err("%s: cfg loading requested failed: %d\n",
					__func__, ret);
			}
			break;
		}
	}
}

/******************************************************
 *
 * asoc interface
 *
 ******************************************************/
void aw88194_get_volume(void *aw881xx, unsigned int *vol)
{
	struct aw881xx *aw88194 = (struct aw881xx *)aw881xx;
	uint16_t reg_val = 0;
	uint16_t vol_val = 0;

	aw88194_reg_read(aw88194, AW88194_REG_HAGCCFG7, &reg_val);
	vol_val = (reg_val >> AW88194_VOL_REG_SHIFT);
	*vol = (vol_val >> 4) * AW88194_VOL_6DB_STEP +
		((vol_val & 0x0f) % AW88194_VOL_6DB_STEP);
}

void aw88194_set_volume(void *aw881xx, unsigned int vol)
{
	struct aw881xx *aw88194 = (struct aw881xx *)aw881xx;
	uint16_t reg_val = 0;
	uint16_t vol_val = 0;

	/* cal real value */
	vol_val = ((vol / AW88194_VOL_6DB_STEP) << 4) +
		(vol % AW88194_VOL_6DB_STEP);

	reg_val = (vol_val << AW88194_VOL_REG_SHIFT);

	/* write value */
	aw88194_reg_write_bits(aw88194, AW88194_REG_HAGCCFG7,
		(uint16_t)AW88194_BIT_HAGCCFG7_VOL_MASK, reg_val);
}


int aw88194_update_hw_params(void *aw881xx)
{
	struct aw881xx *aw88194 = (struct aw881xx *)aw881xx;
	uint16_t reg_val = 0;

	/* match rate */
	switch (aw88194->rate) {
	case 8000:
		reg_val = AW88194_BIT_I2SCTRL_SR_8K;
		break;
	case 16000:
		reg_val = AW88194_BIT_I2SCTRL_SR_16K;
		break;
	case 32000:
		reg_val = AW88194_BIT_I2SCTRL_SR_32K;
		break;
	case 44100:
		reg_val = AW88194_BIT_I2SCTRL_SR_44P1K;
		break;
	case 48000:
		reg_val = AW88194_BIT_I2SCTRL_SR_48K;
		break;
	case 96000:
		reg_val = AW88194_BIT_I2SCTRL_SR_96K;
		break;
	case 192000:
		reg_val = AW88194_BIT_I2SCTRL_SR_192K;
		break;
	default:
		reg_val = AW88194_BIT_I2SCTRL_SR_48K;
		pr_err("%s: rate can not support\n", __func__);
		break;
	}
	/* set chip rate */
	aw88194_reg_write_bits(aw88194, AW88194_REG_I2SCTRL,
		AW88194_BIT_I2SCTRL_SR_MASK, reg_val);

	/* match bit width */
	switch (aw88194->width) {
	case 16:
		reg_val = AW88194_BIT_I2SCTRL_FMS_16BIT;
		break;
	case 20:
		reg_val = AW88194_BIT_I2SCTRL_FMS_20BIT;
		break;
	case 24:
		reg_val = AW88194_BIT_I2SCTRL_FMS_24BIT;
		break;
	case 32:
		reg_val = AW88194_BIT_I2SCTRL_FMS_32BIT;
		break;
	default:
		reg_val = AW88194_BIT_I2SCTRL_FMS_16BIT;
		pr_err("%s: width can not support\n", __func__);
		break;
	}
	/* set width */
	aw88194_reg_write_bits(aw88194, AW88194_REG_I2SCTRL,
		AW88194_BIT_I2SCTRL_FMS_MASK, reg_val);

	return 0;
}

/******************************************************
 *
 * irq
 *
 ******************************************************/
void aw88194_interrupt_setup(void *aw881xx)
{
	struct aw881xx *aw88194 = (struct aw881xx *)aw881xx;
	uint16_t reg_val = 0;

	pr_info("%s: enter\n", __func__);

	aw88194_reg_read(aw88194, AW88194_REG_SYSINTM, &reg_val);
	reg_val &= (~AW88194_BIT_SYSINTM_PLLM);
	reg_val &= (~AW88194_BIT_SYSINTM_OTHM);
	reg_val &= (~AW88194_BIT_SYSINTM_OCDM);
	aw88194_reg_write(aw88194, AW88194_REG_SYSINTM, reg_val);
}

void aw88194_interrupt_handle(void *aw881xx)
{
	struct aw881xx *aw88194 = (struct aw881xx *)aw881xx;
	uint16_t reg_val = 0;

	pr_info("%s: enter\n", __func__);

	aw88194_reg_read(aw88194, AW88194_REG_SYSST, &reg_val);
	pr_info("%s: reg SYSST=0x%x\n", __func__, reg_val);

	aw88194_reg_read(aw88194, AW88194_REG_SYSINT, &reg_val);
	pr_info("%s: reg SYSINT=0x%x\n", __func__, reg_val);

	aw88194_reg_read(aw88194, AW88194_REG_SYSINTM, &reg_val);
	pr_info("%s: reg SYSINTM=0x%x\n", __func__, reg_val);
}

/*****************************************************
 *
 * monitor
 *
 *****************************************************/
/*
static int aw88194_monitor_get_gain_value(
	struct aw881xx *aw88194, uint16_t *reg_val)
{
	int ret = -1;

	if ((aw88194 == NULL) || (reg_val == NULL)) {
		pr_err("%s: aw88194 is %p, reg_val=%p\n",
			__func__, aw88194, reg_val);
		return ret;
	}

	ret = aw88194_reg_read(aw88194, AW88194_REG_HAGCCFG7, reg_val);
	*reg_val = *reg_val >> AW88194_HAGC_GAIN_SHIFT;

	return ret;
}
*/
static int aw88194_monitor_set_gain_value(struct aw881xx *aw88194,
	uint16_t gain_value)
{
	int ret = -1;
	uint16_t reg_val = 0;
	uint16_t read_reg_val;

	if (aw88194 == NULL) {
		pr_err("%s: aw88194 is %p\n", __func__, aw88194);
		return ret;
	}

	ret = aw88194_reg_read(aw88194, AW88194_REG_HAGCCFG7, &reg_val);
	if (ret)
		return ret;

	/* check gain */
	read_reg_val = reg_val;
	read_reg_val = read_reg_val >> AW88194_HAGC_GAIN_SHIFT;
	if (read_reg_val == gain_value) {
		pr_debug("%s: gain_value=0x%x, no change\n",
			__func__, gain_value);
		return ret;
	}

	reg_val &= AW88194_HAGC_GAIN_MASK;
	reg_val |= gain_value << AW88194_HAGC_GAIN_SHIFT;

	ret = aw88194_reg_write(aw88194, AW88194_REG_HAGCCFG7, reg_val);
	if (ret < 0)
		return ret;

	pr_debug("%s: set reg_val=0x%x, gain_value=0x%x\n",
		__func__, reg_val, gain_value);

	return ret;
}

/*
static int aw88194_monitor_get_bst_ipeak(
	struct aw881xx *aw88194, uint16_t *reg_val)
{
	int ret = -1;

	if ((aw88194 == NULL) || (reg_val == NULL)) {
		pr_err("%s: aw88194 is %p, reg_val=%p\n",
			__func__, aw88194, reg_val);
		return ret;
	}

	ret = aw88194_reg_read(aw88194, AW88194_REG_SYSCTRL2, reg_val);
	*reg_val = *reg_val & (AW88194_BIT_SYSCTRL2_BST_IPEAK_MASK);

	return ret;
}
*/
static int aw88194_monitor_set_bst_ipeak(struct aw881xx *aw88194,
	uint16_t bst_ipeak)
{
	int ret = -1;
	uint16_t reg_val = 0;
	uint16_t read_reg_val;

	if (aw88194 == NULL) {
		pr_err("%s: aw88194 is %p\n", __func__, aw88194);
		return ret;
	}

	ret = aw88194_reg_read(aw88194, AW88194_REG_SYSCTRL2, &reg_val);
	if (ret < 0)
		return ret;

	/* check ipeak */
	read_reg_val = reg_val;
	read_reg_val = read_reg_val & (~AW88194_BIT_SYSCTRL2_BST_IPEAK_MASK);
	if (read_reg_val == bst_ipeak) {
		pr_debug("%s: ipeak=0x%x, no change\n", __func__, bst_ipeak);
		return ret;
	}

	reg_val &= AW88194_BIT_SYSCTRL2_BST_IPEAK_MASK;
	reg_val |= bst_ipeak;

	ret = aw88194_reg_write(aw88194, AW88194_REG_SYSCTRL2, reg_val);
	if (ret < 0)
		return ret;

	pr_debug("%s: set reg_val=0x%x, bst_ipeak=0x%x\n",
		__func__, reg_val, bst_ipeak);

	return ret;
}

static int aw88194_monitor_vmax_check(struct aw881xx *aw88194)
{
	int ret = -1;

	ret = aw88194_get_iis_status(aw88194);
	if (ret < 0) {
		pr_err("%s: no iis signal\n", __func__);
		return ret;
	}

	ret = aw88194_get_dsp_status(aw88194);
	if (ret < 0) {
		pr_err("%s: dsp not work\n", __func__);
		return ret;
	}

	return 0;
}

static int aw88194_monitor_set_vmax(
	struct aw881xx *aw88194, uint16_t vmax)
{
	int ret = -1;
	uint16_t reg_val = 0;

	ret = aw88194_monitor_vmax_check(aw88194);
	if (ret) {
		pr_err("%s: vamx_check fail, ret=%d\n", __func__, ret);
		return ret;
	}

	ret = aw88194_dsp_read(aw88194, AW88194_DSP_REG_VMAX, &reg_val);
	if (ret)
		return ret;

	if (reg_val == vmax) {
		pr_info("%s: read vmax=0x%x\n", __func__, reg_val);
		return ret;
	}

	ret = aw88194_dsp_write(aw88194, AW88194_DSP_REG_VMAX, vmax);
	if (ret)
		return ret;

	return ret;
}

static int aw88194_monitor_get_voltage(struct aw881xx *aw88194,
	uint16_t *voltage)
{
	int ret = -1;

	if ((aw88194 == NULL) || (voltage == NULL)) {
		pr_err("%s aw88194 is %p, voltage=%p\n",
			__func__, aw88194, voltage);
		return ret;
	}

	ret = aw88194_reg_read(aw88194, AW88194_REG_VBAT, voltage);
	if (ret < 0)
		return ret;

	*voltage = (*voltage) * AW88194_VBAT_RANGE /
		AW88194_VBAT_COEFF_INT_10BIT;

	pr_debug("%s: chip voltage=%dmV\n", __func__, *voltage);

	return ret;
}

static int aw88194_monitor_get_temperature(
	struct aw881xx *aw88194, int16_t *temp)
{
	int ret = -1;
	uint16_t reg_val = 0;

	if ((aw88194 == NULL) || (temp == NULL)) {
		pr_err("%s aw88194= %p, temp=%p\n",
			__func__, aw88194, temp);
		return ret;
	}

	ret = aw88194_reg_read(aw88194, AW88194_REG_TEMP, &reg_val);
	if (ret < 0)
		return ret;

	if (reg_val & AW88194_TEMP_SIGN_MASK)
		reg_val |= AW88194_TEMP_NEG_MASK;

	*temp = (int)reg_val;

	pr_debug("%s: chip_temperature=%d\n", __func__, *temp);

	return ret;
}

static int aw88194_monitor_check_voltage(struct aw881xx *aw88194,
	uint16_t *bst_ipeak, uint16_t *gain_db)
{
	int ret = -1;
	uint16_t voltage = 0;

	if ((aw88194 == NULL) || (bst_ipeak == NULL) || (gain_db == NULL)) {
		pr_err("aw88194 is %p, bst_ipeak is %p, gain_db is %p\n",
			aw88194, bst_ipeak, gain_db);
		return ret;
	}

	ret = aw88194_monitor_get_voltage(aw88194, &voltage);

	if (voltage > AW88194_VOL_LIMIT_40) {
		*bst_ipeak = AW88194_IPEAK_350;
		*gain_db = AW88194_GAIN_00DB;
		aw88194->pre_vol_bst_ipeak = *bst_ipeak;
		aw88194->pre_vol_gain_db = *gain_db;
	} else if (voltage < AW88194_VOL_LIMIT_39 &&
		voltage > AW88194_VOL_LIMIT_38) {
		*bst_ipeak = AW88194_IPEAK_300;
		*gain_db = AW88194_GAIN_NEG_05DB;
		aw88194->pre_vol_bst_ipeak = *bst_ipeak;
		aw88194->pre_vol_gain_db = *gain_db;
	} else if (voltage < AW88194_VOL_LIMIT_37 &&
		voltage > AW88194_VOL_LIMIT_36) {
		*bst_ipeak = AW88194_IPEAK_275;
		*gain_db = AW88194_GAIN_NEG_10DB;
		aw88194->pre_vol_bst_ipeak = *bst_ipeak;
		aw88194->pre_vol_gain_db = *gain_db;
	} else if (voltage < AW88194_VOL_LIMIT_35) {
		*bst_ipeak = AW88194_IPEAK_250;
		*gain_db = AW88194_GAIN_NEG_15DB;
		aw88194->pre_vol_bst_ipeak = *bst_ipeak;
		aw88194->pre_vol_gain_db = *gain_db;
	} else {
		*bst_ipeak = aw88194->pre_vol_bst_ipeak;
		*gain_db = aw88194->pre_vol_gain_db;
	}
	pr_info("%s: bst_ipeak=0x%x, gain_db=0x%x\n",
		__func__, *bst_ipeak, *gain_db);

	return ret;
}

static void aw88194_monitor_check_temperature_deglitch(
	uint16_t *bst_ipeak, uint16_t *gain_db,
	uint16_t *vmax, int16_t temperature, int16_t pre_temp)
{
	if (temperature <= pre_temp) {
		if (temperature <= AW88194_TEMP_LIMIT_7 &&
			temperature >= AW88194_TEMP_LIMIT_5) {
			*bst_ipeak = AW88194_IPEAK_350;
			*gain_db = AW88194_GAIN_00DB;
			*vmax = AW88194_VMAX_80;
		} else if (temperature <= AW88194_TEMP_LIMIT_2 &&
			temperature >= AW88194_TEMP_LIMIT_0) {
			*bst_ipeak = AW88194_IPEAK_300;
			*gain_db = AW88194_GAIN_NEG_30DB;
			*vmax = AW88194_VMAX_70;
		} else if (temperature <= AW88194_TEMP_LIMIT_NEG_2 &&
			temperature >= AW88194_TEMP_LIMIT_NEG_5) {
			*bst_ipeak = AW88194_IPEAK_275;
			*gain_db = AW88194_GAIN_NEG_45DB;
			*vmax = AW88194_VMAX_60;
		}
	} else {
		if (temperature <= AW88194_TEMP_LIMIT_7 &&
			temperature >= AW88194_TEMP_LIMIT_5) {
			*bst_ipeak = AW88194_IPEAK_300;
			*gain_db = AW88194_GAIN_NEG_30DB;
			*vmax = AW88194_VMAX_70;
		} else if (temperature <= AW88194_TEMP_LIMIT_2 &&
			temperature >= AW88194_TEMP_LIMIT_0) {
			*bst_ipeak = AW88194_IPEAK_275;
			*gain_db = AW88194_GAIN_NEG_45DB;
			*vmax = AW88194_VMAX_60;
		} else if (temperature <= AW88194_TEMP_LIMIT_NEG_2 &&
			temperature >= AW88194_TEMP_LIMIT_NEG_5) {
			*bst_ipeak = AW88194_IPEAK_250;
			*gain_db = AW88194_GAIN_NEG_60DB;
			*vmax = AW88194_VMAX_50;
		}
	}
}

static int aw88194_monitor_check_temperature(
	struct aw881xx *aw88194, uint16_t *bst_ipeak,
	uint16_t *gain_db, uint16_t *vmax)
{
	int ret = -1;
	int16_t temperature = 0;
	int16_t pre_temp;

	if ((aw88194 == NULL) || (bst_ipeak == NULL) ||
		(gain_db == NULL) || (vmax == NULL)) {
		pr_err("%s: aw88194=%p, bst_ipeak=%p, gain_db=%p, vmax=%p\n",
			__func__, aw88194, bst_ipeak, gain_db, vmax);
		return ret;
	}
	pre_temp = aw88194->pre_temp;

	ret = aw88194_monitor_get_temperature(aw88194, &temperature);
	aw88194->pre_temp = temperature;
	if (temperature > AW88194_TEMP_LIMIT_7) {
		*bst_ipeak = AW88194_IPEAK_350;
		*gain_db = AW88194_GAIN_00DB;
		*vmax = AW88194_VMAX_80;
	} else if (temperature < AW88194_TEMP_LIMIT_5 &&
		temperature > AW88194_TEMP_LIMIT_2) {
		*bst_ipeak = AW88194_IPEAK_300;
		*gain_db = AW88194_GAIN_NEG_30DB;
		*vmax = AW88194_VMAX_70;
	} else if (temperature < AW88194_TEMP_LIMIT_0 &&
		temperature > AW88194_TEMP_LIMIT_NEG_2) {
		*bst_ipeak = AW88194_IPEAK_275;
		*gain_db = AW88194_GAIN_NEG_45DB;
		*vmax = AW88194_VMAX_60;
	} else if (temperature < AW88194_TEMP_LIMIT_NEG_5) {
		*bst_ipeak = AW88194_IPEAK_250;
		*gain_db = AW88194_GAIN_NEG_60DB;
		*vmax = AW88194_VMAX_50;
	} else {
		if (temperature == pre_temp) {
			*bst_ipeak = aw88194->pre_temp_bst_ipeak;
			*gain_db = aw88194->pre_temp_gain_db;
			*vmax = aw88194->pre_temp_vmax;
		} else {
		aw88194_monitor_check_temperature_deglitch(
			bst_ipeak, gain_db, vmax,
			temperature, pre_temp);
		}
	}

	aw88194->pre_temp_bst_ipeak = *bst_ipeak;
	aw88194->pre_temp_gain_db = *gain_db;
	aw88194->pre_temp_vmax = *vmax;

	pr_info("%s: bst_ipeak=0x%x, gain_db=0x%x, vmax=0x%0x\n",
		__func__, *bst_ipeak, *gain_db, *vmax);

	return ret;
}

static int aw88194_monitor_check_sysint(struct aw881xx *aw88194)
{
	int ret = -1;
	uint16_t sysint = 0;

	if (aw88194 == NULL) {
		pr_err("%s: aw88194 is NULL\n", __func__);
		return ret;
	}

	ret = aw88194_get_sysint(aw88194, &sysint);
	if (ret < 0)
		pr_err("%s: get_sysint fail, ret=%d\n", __func__, ret);
	else
		pr_info("%s: get_sysint=0x%04x\n", __func__, ret);

	return ret;
}

void aw88194_monitor_cal_ipeak(uint16_t *real_ipeak,
	uint16_t vol_ipeak, uint16_t temp_ipeak)
{
	if (real_ipeak == NULL) {
		pr_err("%s: real_ipeak=%p\n", __func__, real_ipeak);
		return;
	}

	if (vol_ipeak == AW88194_IPEAK_NONE &&
		temp_ipeak == AW88194_IPEAK_NONE)
		*real_ipeak = AW88194_IPEAK_NONE;
	else
		*real_ipeak = (vol_ipeak < temp_ipeak ? vol_ipeak : temp_ipeak);
}

void aw88194_monitor_cal_gain(uint16_t *real_gain,
	uint16_t vol_gain, uint16_t temp_gain)
{
	if (real_gain == NULL) {
		pr_err("%s: real_gain=%p\n", __func__, real_gain);
		return;
	}

	if (vol_gain == AW88194_GAIN_NONE ||
		temp_gain == AW88194_GAIN_NONE) {
		if (vol_gain == AW88194_GAIN_NONE &&
			temp_gain == AW88194_GAIN_NONE)
			*real_gain = AW88194_GAIN_NONE;
		else if (vol_gain == AW88194_GAIN_NONE)
			*real_gain = temp_gain;
		else
			*real_gain = vol_gain;
	} else {
		*real_gain = (vol_gain > temp_gain ? vol_gain : temp_gain);
	}
}

void aw88194_monitor(void *aw881xx)
{
	struct aw881xx *aw88194 = (struct aw881xx *)aw881xx;
	int ret;
	uint16_t vol_ipeak = 0;
	uint16_t vol_gain = 0;
	uint16_t temp_ipeak = 0;
	uint16_t temp_gain = 0;
	uint16_t vmax = 0;
	uint16_t real_ipeak;
	uint16_t real_gain;

	/* get ipeak and gain value by voltage and temperature */
	ret = aw88194_monitor_check_voltage(aw88194, &vol_ipeak, &vol_gain);
	if (ret < 0) {
		pr_err("%s: check voltage failed\n", __func__);
		return;
	}
	ret =
		aw88194_monitor_check_temperature(aw88194,
			&temp_ipeak, &temp_gain, &vmax);
	if (ret < 0) {
		pr_err("%s: check temperature failed\n", __func__);
		return;
	}
	/* get min Ipeak */
	if (vol_ipeak == AW88194_IPEAK_NONE &&
		temp_ipeak == AW88194_IPEAK_NONE)
		real_ipeak = AW88194_IPEAK_NONE;
	else
		real_ipeak = (vol_ipeak < temp_ipeak ? vol_ipeak : temp_ipeak);

	/* get min gain */
	if (vol_gain == AW88194_GAIN_NONE ||
		temp_gain == AW88194_GAIN_NONE) {
		if (vol_gain == AW88194_GAIN_NONE &&
			temp_gain == AW88194_GAIN_NONE)
			real_gain = AW88194_GAIN_NONE;
		else if (vol_gain == AW88194_GAIN_NONE)
			real_gain = temp_gain;
		else
			real_gain = vol_gain;
	} else {
		real_gain = (vol_gain > temp_gain ? vol_gain : temp_gain);
	}

	if (real_ipeak != AW88194_IPEAK_NONE)
		aw88194_monitor_set_bst_ipeak(aw88194, real_ipeak);

	if (real_gain != AW88194_GAIN_NONE)
		aw88194_monitor_set_gain_value(aw88194, real_gain);

	if (vmax != AW88194_VMAX_NONE)
		aw88194_monitor_set_vmax(aw88194, vmax);

	aw88194_monitor_check_sysint(aw88194);
}


unsigned char aw88194_get_reg_access(uint8_t reg_addr)
{
	return aw88194_reg_access[reg_addr];
}

int aw88194_get_dsp_mem_reg(void *aw881xx,
	uint8_t *mem_addr_reg, uint8_t *msm_data_reg)
{
	*mem_addr_reg = AW88194_REG_DSPMADD;
	*msm_data_reg = AW88194_REG_DSPMDAT;

	return 0;
}

struct device_ops aw88194_dev_ops = {
	aw88194_update_hw_params,
	aw88194_get_volume,
	aw88194_set_volume,

	aw88194_start,
	aw88194_stop,
	aw88194_update_dsp,
	aw88194_run_pwd,
	aw88194_monitor,
	aw88194_set_cali_re,

	aw88194_interrupt_setup,
	aw88194_interrupt_handle,

	aw88194_get_hmute,
	aw88194_get_dsp_config,
	aw88194_get_iis_status,
	aw88194_get_reg_access,
	aw88194_get_dsp_mem_reg,
};

void aw88194_ops(void *aw881xx)
{
	struct aw881xx *aw88194 = (struct aw881xx *)aw881xx;

	aw88194->dev_ops = aw88194_dev_ops;
}

