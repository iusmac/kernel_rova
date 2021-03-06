/*
 *
 * FocalTech TouchScreen driver.
 *
 * Copyright (c) 2010-2017, Focaltech Ltd. All rights reserved.
 * Copyright (C) 2018 XiaoMi, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include "focaltech_core.h"
#ifdef CONFIG_TOUCHSCREEN_XIAOMI_DT2W
#include <linux/input/xiaomi_dt2w.h>
#endif
#if FTS_GESTURE_EN
#define KEY_GESTURE_U                           KEY_WAKEUP
#define KEY_GESTURE_UP                          KEY_UP
#define KEY_GESTURE_DOWN                        KEY_DOWN
#define KEY_GESTURE_LEFT                        KEY_LEFT
#define KEY_GESTURE_RIGHT                       KEY_RIGHT
#define KEY_GESTURE_O                           KEY_O
#define KEY_GESTURE_E                           KEY_E
#define KEY_GESTURE_M                           KEY_M
#define KEY_GESTURE_L                           KEY_L
#define KEY_GESTURE_W                           KEY_W
#define KEY_GESTURE_S                           KEY_S
#define KEY_GESTURE_V                           KEY_V
#define KEY_GESTURE_C                           KEY_C
#define KEY_GESTURE_Z                           KEY_Z

#define GESTURE_LEFT                            0x20
#define GESTURE_RIGHT                           0x21
#define GESTURE_UP                              0x22
#define GESTURE_DOWN                            0x23
#define GESTURE_DOUBLECLICK                     0x24
#define GESTURE_O                               0x30
#define GESTURE_W                               0x31
#define GESTURE_M                               0x32
#define GESTURE_E                               0x33
#define GESTURE_L                               0x44
#define GESTURE_S                               0x46
#define GESTURE_V                               0x54
#define GESTURE_Z                               0x41
#define GESTURE_C                               0x34

static struct fts_gesture_st fts_gesture_data;
static ssize_t fts_gesture_show(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t fts_gesture_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t fts_gesture_buf_show(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t fts_gesture_buf_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count);

static DEVICE_ATTR (fts_gesture_mode, S_IRUGO|S_IWUSR, fts_gesture_show, fts_gesture_store);
static DEVICE_ATTR (fts_gesture_buf, S_IRUGO|S_IWUSR, fts_gesture_buf_show, fts_gesture_buf_store);
static struct attribute *fts_gesture_mode_attrs[] = {

		&dev_attr_fts_gesture_mode.attr,
		&dev_attr_fts_gesture_buf.attr,
		NULL,
};

static struct attribute_group fts_gesture_group =
{
		.attrs = fts_gesture_mode_attrs,
};

static ssize_t fts_gesture_show(struct device *dev, struct device_attribute *attr, char *buf)
{
		int count;
		u8 val;
		struct i2c_client *client = container_of(dev, struct i2c_client, dev);

		mutex_lock(&ft5435_fts_input_dev->mutex);
		ft5435_ft5435_fts_i2c_read_reg(client, FTS_REG_GESTURE_EN, &val);
		count = sprintf(buf, "Gesture Mode: %s\n", fts_gesture_data.mode ? "On" : "Off");
		count += sprintf(buf + count, "Reg(0xD0) = %d\n", val);
		mutex_unlock(&ft5435_fts_input_dev->mutex);

		return count;
}

static ssize_t fts_gesture_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
		mutex_lock(&ft5435_fts_input_dev->mutex);

		if (FTS_SYSFS_ECHO_ON(buf)) {
			FTS_INFO("[GESTURE]enable gesture");
			fts_gesture_data.mode = ENABLE;
		} else if (FTS_SYSFS_ECHO_OFF(buf)) {
			FTS_INFO("[GESTURE]disable gesture");
			fts_gesture_data.mode = DISABLE;
		}

		mutex_unlock(&ft5435_fts_input_dev->mutex);

		return count;
}

static ssize_t fts_gesture_buf_show(struct device *dev, struct device_attribute *attr, char *buf)
{
		int count;
		int i = 0;

		mutex_lock(&ft5435_fts_input_dev->mutex);
		count = snprintf(buf, PAGE_SIZE, "Gesture ID: 0x%x\n", fts_gesture_data.header[0]);
		count += snprintf(buf + count, PAGE_SIZE, "Gesture PointNum: %d\n", fts_gesture_data.header[1]);
		count += snprintf(buf + count, PAGE_SIZE, "Gesture Point Buf:\n");
		for (i = 0; i < fts_gesture_data.header[1]; i++) {
			count += snprintf(buf + count, PAGE_SIZE, "%3d(%4d,%4d) ", i, fts_gesture_data.coordinate_x[i], fts_gesture_data.coordinate_y[i]);
		if ((i + 1)%4 == 0)
			count += snprintf(buf + count, PAGE_SIZE, "\n");
		}
		count += snprintf(buf + count, PAGE_SIZE, "\n");
		mutex_unlock(&ft5435_fts_input_dev->mutex);

		return count;
}

static ssize_t fts_gesture_buf_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
		return -EPERM;
}

int ft5435_fts_create_gesture_sysfs(struct i2c_client *client)
{
		int ret = 0;

#ifdef CONFIG_TOUCHSCREEN_XIAOMI_DT2W
		return 0;
#endif

		ret = sysfs_create_group(&client->dev.kobj, &fts_gesture_group);
		if ( ret != 0) {
			FTS_ERROR( "[GESTURE]fts_gesture_mode_group(sysfs) create failed!");
			sysfs_remove_group(&client->dev.kobj, &fts_gesture_group);
			return ret;
		}
		return 0;
}

static void fts_gesture_report(struct input_dev *input_dev, int gesture_id)
{
		int gesture;

		FTS_FUNC_ENTER();
		FTS_DEBUG("fts gesture_id==0x%x ", gesture_id);
		switch (gesture_id) {
		case GESTURE_LEFT:
			gesture = KEY_GESTURE_LEFT;
			break;
		case GESTURE_RIGHT:
			gesture = KEY_GESTURE_RIGHT;
			break;
		case GESTURE_UP:
			gesture = KEY_GESTURE_UP;
			break;
		case GESTURE_DOWN:
			gesture = KEY_GESTURE_DOWN;
			break;
		case GESTURE_DOUBLECLICK:
			gesture = KEY_GESTURE_U;
			break;
		case GESTURE_O:
			gesture = KEY_GESTURE_O;
			break;
		case GESTURE_W:
			gesture = KEY_GESTURE_W;
			break;
		case GESTURE_M:
			gesture = KEY_GESTURE_M;
			break;
		case GESTURE_E:
			gesture = KEY_GESTURE_E;
			break;
		case GESTURE_L:
			gesture = KEY_GESTURE_L;
			break;
		case GESTURE_S:
			gesture = KEY_GESTURE_S;
			break;
		case GESTURE_V:
			gesture = KEY_GESTURE_V;
			break;
		case GESTURE_Z:
			gesture = KEY_GESTURE_Z;
			break;
		case  GESTURE_C:
			gesture = KEY_GESTURE_C;
			break;
		default:
			gesture = -1;
			break;
		}
		if (gesture != -1) {
			FTS_DEBUG("Gesture Code=%d", gesture);
			input_report_key(input_dev, gesture, 1);
			input_sync(input_dev);
			input_report_key(input_dev, gesture, 0);
			input_sync(input_dev);
		}

		FTS_FUNC_EXIT();
}

static int fts_gesture_read_buffer(struct i2c_client *client, u8 *buf, int read_bytes)
{
		int remain_bytes;
		int ret;
		int i;

		if (read_bytes <= I2C_BUFFER_LENGTH_MAXINUM) {
			ret = ft5435_fts_i2c_read(client, buf, 1, buf, read_bytes);
		} else {
			ret = ft5435_fts_i2c_read(client, buf, 1, buf, I2C_BUFFER_LENGTH_MAXINUM);
			remain_bytes = read_bytes - I2C_BUFFER_LENGTH_MAXINUM;
			for (i = 1; remain_bytes > 0; i++) {
				if (remain_bytes <= I2C_BUFFER_LENGTH_MAXINUM)
					ret = ft5435_fts_i2c_read(client, buf, 0, buf + I2C_BUFFER_LENGTH_MAXINUM * i, remain_bytes);
			else
				ret = ft5435_fts_i2c_read(client, buf, 0, buf + I2C_BUFFER_LENGTH_MAXINUM * i, I2C_BUFFER_LENGTH_MAXINUM);
			remain_bytes -= I2C_BUFFER_LENGTH_MAXINUM;
			}
		}

		return ret;
}

static int fts_gesture_fw(void)
{
		int ret = 0;

		switch (ft5435_chip_types.chip_idh) {
		case 0x54:
		case 0x58:
		case 0x64:
		case 0x87:
		case 0x86:
		case 0x80:
		case 0xE7:
			ret = 1;
			break;
		default:
			ret = 0;
			break;
		}
		return ret;
}

int ft5435_fts_gesture_readdata(struct i2c_client *client)
{
		u8 buf[FTS_GESTRUE_POINTS * 4] = { 0 };
		int ret = -1;
		int i = 0;
		int gestrue_id = 0;
		int read_bytes = 0;
		u8 pointnum;

		FTS_FUNC_ENTER();
		memset(fts_gesture_data.header, 0, FTS_GESTRUE_POINTS_HEADER);
		memset(fts_gesture_data.coordinate_x, 0, FTS_GESTRUE_POINTS * sizeof(u16));
		memset(fts_gesture_data.coordinate_y, 0, FTS_GESTRUE_POINTS * sizeof(u16));

		buf[0] = FTS_REG_GESTURE_OUTPUT_ADDRESS;
		ret = ft5435_fts_i2c_read(client, buf, 1, buf, FTS_GESTRUE_POINTS_HEADER);
		if (ret < 0) {
			FTS_ERROR("[GESTURE]Read gesture header data failed!!");
			FTS_FUNC_EXIT();
			return ret;
		}

		if (fts_gesture_fw()) {
			memcpy(fts_gesture_data.header, buf, FTS_GESTRUE_POINTS_HEADER);
			gestrue_id = buf[0];
			pointnum = buf[1];
			read_bytes = ((int)pointnum) * 4 + 2;
			buf[0] = FTS_REG_GESTURE_OUTPUT_ADDRESS;
			FTS_DEBUG("[GESTURE]PointNum=%d", pointnum);
			ret = fts_gesture_read_buffer(client, buf, read_bytes);
		if (ret < 0) {
			FTS_ERROR("[GESTURE]Read gesture touch data failed!!");
			FTS_FUNC_EXIT();
			return ret;
		}

		fts_gesture_report(ft5435_fts_input_dev, gestrue_id);
		for (i = 0; i < pointnum; i++) {
			fts_gesture_data.coordinate_x[i] = (((s16) buf[0 + (4 * i + 2)]) & 0x0F) << 8
										      | (((s16) buf[1 + (4 * i + 2)]) & 0xFF);
			fts_gesture_data.coordinate_y[i] = (((s16) buf[2 + (4 * i + 2)]) & 0x0F) << 8
										      | (((s16) buf[3 + (4 * i + 2)]) & 0xFF);
		}
			FTS_FUNC_EXIT();
			return 0;
		} else {
			FTS_ERROR("[GESTURE]IC 0x%x need gesture lib to support gestures.", ft5435_chip_types.chip_idh);
			return 0;
		}
}

void ft5435_fts_gesture_recovery(struct i2c_client *client)
{
		if (fts_gesture_data.mode && fts_gesture_data.active) {
			ft5435_ft5435_fts_i2c_write_reg(client, 0xD1, 0xff);
			ft5435_ft5435_fts_i2c_write_reg(client, 0xD2, 0xff);
			ft5435_ft5435_fts_i2c_write_reg(client, 0xD5, 0xff);
			ft5435_ft5435_fts_i2c_write_reg(client, 0xD6, 0xff);
			ft5435_ft5435_fts_i2c_write_reg(client, 0xD7, 0xff);
			ft5435_ft5435_fts_i2c_write_reg(client, 0xD8, 0xff);
			ft5435_ft5435_fts_i2c_write_reg(client, FTS_REG_GESTURE_EN, ENABLE);
		}
}

int ft5435_fts_gesture_suspend(struct i2c_client *i2c_client)
{
		int i;
		u8 state;

		FTS_FUNC_ENTER();

#ifdef CONFIG_TOUCHSCREEN_XIAOMI_DT2W
		fts_gesture_data.mode = xiaomi_dt2w_enable;
#endif

		if (fts_gesture_data.mode == 0) {
			FTS_DEBUG("gesture is disabled");
			FTS_FUNC_EXIT();
			return -1;
		}

		for (i = 0; i < 5; i++) {
			ft5435_ft5435_fts_i2c_write_reg(i2c_client, 0xd1, 0xff);
			ft5435_ft5435_fts_i2c_write_reg(i2c_client, 0xd2, 0xff);
			ft5435_ft5435_fts_i2c_write_reg(i2c_client, 0xd5, 0xff);
			ft5435_ft5435_fts_i2c_write_reg(i2c_client, 0xd6, 0xff);
			ft5435_ft5435_fts_i2c_write_reg(i2c_client, 0xd7, 0xff);
			ft5435_ft5435_fts_i2c_write_reg(i2c_client, 0xd8, 0xff);
			ft5435_ft5435_fts_i2c_write_reg(i2c_client, FTS_REG_GESTURE_EN, 0x01);
			msleep(1);
			ft5435_ft5435_fts_i2c_read_reg(i2c_client, FTS_REG_GESTURE_EN, &state);
			if (state == 1)
				break;
		}

		if (i >= 5) {
			FTS_ERROR("[GESTURE]Enter into gesture(suspend) failed!\n");
			FTS_FUNC_EXIT();
			return -1;
		}

		fts_gesture_data.active = 1;
		FTS_DEBUG("[GESTURE]Enter into gesture(suspend) successfully!");
		FTS_FUNC_EXIT();
		return 0;
}

int ft5435_fts_gesture_resume(struct i2c_client *client)
{
		int i;
		u8 state;
		FTS_FUNC_ENTER();

		if (fts_gesture_data.mode == 0) {
			FTS_DEBUG("gesture is disabled");
			FTS_FUNC_EXIT();
			return -1;
		}

		if (fts_gesture_data.active == 0) {
			FTS_DEBUG("gesture is unactive");
			FTS_FUNC_EXIT();
			return -1;
		}

		fts_gesture_data.active = 0;
		for (i = 0; i < 5; i++) {
			ft5435_ft5435_fts_i2c_write_reg(client, FTS_REG_GESTURE_EN, 0x00);
			msleep(1);
				ft5435_ft5435_fts_i2c_read_reg(client, FTS_REG_GESTURE_EN, &state);
			if (state == 0)
				break;
		}

		if (i >= 5) {
			FTS_ERROR("[GESTURE]Clear gesture(resume) failed!\n");
		}

		FTS_FUNC_EXIT();
		return 0;
}

int ft5435_fts_gesture_init(struct input_dev *input_dev, struct i2c_client *client)
{
	struct fts_ts_data *data = i2c_get_clientdata(client);
	int ret = 0;
		FTS_FUNC_ENTER();
		input_set_capability(input_dev, EV_KEY, KEY_POWER);
		input_set_capability(input_dev, EV_KEY, KEY_GESTURE_U);
		input_set_capability(input_dev, EV_KEY, KEY_GESTURE_UP);
		input_set_capability(input_dev, EV_KEY, KEY_GESTURE_DOWN);
		input_set_capability(input_dev, EV_KEY, KEY_GESTURE_LEFT);
		input_set_capability(input_dev, EV_KEY, KEY_GESTURE_RIGHT);
		input_set_capability(input_dev, EV_KEY, KEY_GESTURE_O);
		input_set_capability(input_dev, EV_KEY, KEY_GESTURE_E);
		input_set_capability(input_dev, EV_KEY, KEY_GESTURE_M);
		input_set_capability(input_dev, EV_KEY, KEY_GESTURE_L);
		input_set_capability(input_dev, EV_KEY, KEY_GESTURE_W);
		input_set_capability(input_dev, EV_KEY, KEY_GESTURE_S);
		input_set_capability(input_dev, EV_KEY, KEY_GESTURE_V);
		input_set_capability(input_dev, EV_KEY, KEY_GESTURE_Z);
		input_set_capability(input_dev, EV_KEY, KEY_GESTURE_C);

		__set_bit(KEY_GESTURE_RIGHT, input_dev->keybit);
		__set_bit(KEY_GESTURE_LEFT, input_dev->keybit);
		__set_bit(KEY_GESTURE_UP, input_dev->keybit);
		__set_bit(KEY_GESTURE_DOWN, input_dev->keybit);
		__set_bit(KEY_GESTURE_U, input_dev->keybit);
		__set_bit(KEY_GESTURE_O, input_dev->keybit);
		__set_bit(KEY_GESTURE_E, input_dev->keybit);
		__set_bit(KEY_GESTURE_M, input_dev->keybit);
		__set_bit(KEY_GESTURE_W, input_dev->keybit);
		__set_bit(KEY_GESTURE_L, input_dev->keybit);
		__set_bit(KEY_GESTURE_S, input_dev->keybit);
		__set_bit(KEY_GESTURE_V, input_dev->keybit);
		__set_bit(KEY_GESTURE_C, input_dev->keybit);
		__set_bit(KEY_GESTURE_Z, input_dev->keybit);

		ft5435_fts_create_gesture_sysfs(client);
		fts_gesture_data.mode = 1;
		fts_gesture_data.active = 0;
	data->gesture_data = &fts_gesture_data;
		FTS_FUNC_EXIT();
		return 0;
}

int ft5435_fts_gesture_exit(struct i2c_client *client)
{
		FTS_FUNC_ENTER();
		sysfs_remove_group(&client->dev.kobj, &fts_gesture_group);
		FTS_FUNC_EXIT();
		return 0;
}

int fts_select_gesture_mode(struct input_dev *dev, unsigned int type, unsigned int code, int value)
{
	printk("[GESTURE]zhanghao enter the gesture func\n");
	if((type == EV_SYN )&& (code == SYN_CONFIG)) {
		mutex_lock(&ft5435_fts_input_dev->mutex);

		if (value == 5) {
			printk("[GESTURE]enable gesture\n");
			fts_gesture_data.mode = ENABLE;
		} else {
			printk("[GESTURE]disable gesture\n");
			fts_gesture_data.mode = DISABLE;
		}

		mutex_unlock(&ft5435_fts_input_dev->mutex);
	}

	return 0;
}

#endif
