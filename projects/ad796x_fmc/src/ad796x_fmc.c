/***************************************************************************//**
 *   @file   ad796x_fmc.c
 *   @brief  AD796X_FMC Application
 *   @author Axel Haslam (ahaslam@baylibre.com)
********************************************************************************
 * Copyright 2024(c) Analog Devices, Inc.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *  - Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  - Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  - Neither the name of Analog Devices, Inc. nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *  - The use of this software may or may not infringe the patent rights
 *    of one or more patent holders.  This license does not release you
 *    from the requirement that you obtain separate licenses from these
 *    patent holders to use this software.
 *  - Use of the software either in source or binary form, must be run
 *    on or directly connected to an Analog Devices Inc. component.
 *
 * THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, NON-INFRINGEMENT,
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ANALOG DEVICES BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, INTELLECTUAL PROPERTY RIGHTS, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/
#include "xil_cache.h"
#include "xparameters.h"
#include "axi_adc_core.h"

#include "no_os_pwm.h"
#include "axi_pwm_extra.h"

#include "axi_dmac.h"
#include "clk_axi_clkgen.h"
#include "ad796x.h"
#include "no_os_gpio.h"
#include "parameters.h"
#include "no_os_error.h"
#include "xilinx_gpio.h"

#include "no_os_print_log.h"

#ifdef IIO_SUPPORT
#include "iio_app.h"
#include "iio_axi_adc.h"
#include "xilinx_uart.h"
#endif

/******************************************************************************/
/********************** Macros and Constants Definitions **********************/
/******************************************************************************/
#define AD796X_SAMPLES			1000
#define AD796X_BYTES_PER_SAMPLE 	4
#define AD796X_XFER_SIZE		(AD796X_SAMPLES * AD796X_BYTES_PER_SAMPLE)
/***************************************************************************//**
* @brief main
*******************************************************************************/
int main(void)
{
	int ret;

	struct ad796x_dev *ad796x_device;

	/* GPIO */
	struct xil_gpio_init_param xil_gpio_init = {
		.device_id = GPIO_DEVICE_ID,
		.type = GPIO_PS
	};

	struct no_os_gpio_init_param gpio_adc_en3_fmc_param = {
		.number = GPIO_EN3_FMC,
		.platform_ops = &xil_gpio_ops,
		.extra = &xil_gpio_init
	};

	struct no_os_gpio_init_param gpio_adc_en2_fmc_param = {
		.number = GPIO_EN2_FMC,
		.platform_ops = &xil_gpio_ops,
		.extra = &xil_gpio_init
	};

	struct no_os_gpio_init_param gpio_adc_en1_fmc_param = {
		.number = GPIO_EN1_FMC,
		.platform_ops = &xil_gpio_ops,
		.extra = &xil_gpio_init
	};

	struct no_os_gpio_init_param gpio_adc_en0_fmc_param = {
		.number = GPIO_EN0_FMC,
		.platform_ops = &xil_gpio_ops,
		.extra = &xil_gpio_init
	};

	struct ad796x_init_param ad796x_init_param = {
		.gpio_adc_en3_fmc_param = &gpio_adc_en3_fmc_param,
		.gpio_adc_en2_fmc_param = &gpio_adc_en2_fmc_param,
		.gpio_adc_en1_fmc_param = &gpio_adc_en1_fmc_param,
		.gpio_adc_en0_fmc_param = &gpio_adc_en0_fmc_param,
		.mode = AD796X_MODE1_EXT_REF_5P0,
	};

	/* ADC Core */
	struct axi_adc_init ad796x_core_param = {
		.name = "ad796x_core",
		.num_channels = 1,
		.base = RX_CORE_BASEADDR
	};

	struct axi_adc *ad796x_core;

	/* AXI CLKGEN */
	struct axi_clkgen_init clkgen_init = {
		.name = "rx_clkgen",
		.base = RX_CLKGEN_BASEADDR,
		.parent_rate = 100000000,
	};

	struct axi_clkgen *ad796x_clk;

	/* AXI DMAC */
	struct axi_dmac_init ad796x_dmac_param = {
		.name = "ad796x_dmac",
		.base = RX_DMA_BASEADDR,
		.irq_option = IRQ_DISABLED
	};

	struct axi_dmac *ad796x_dmac;

	/* AXI PWM 0 */
	struct axi_pwm_init_param axi_pwm_0_extra = {
		.base_addr = AXI_PWMGEN_BASEADDR,
		.ref_clock_Hz = 125000000,
		.channel = 0
	};

	struct no_os_pwm_init_param axi_pwm_0_init = {
		.period_ns = 200,
		.duty_cycle_ns = 16,
		.phase_ns = 0,
		.platform_ops = &axi_pwm_ops,
		.extra = &axi_pwm_0_extra
	};

	struct no_os_pwm_desc *axi_pwm_0;

	/* AXI PWM 1 */
	struct axi_pwm_init_param axi_pwm_1_extra = {
		.base_addr = AXI_PWMGEN_BASEADDR,
		.ref_clock_Hz = 125000000,
		.channel = 1
	};

	struct no_os_pwm_init_param axi_pwm_1_init = {
		.period_ns = 200,
		.duty_cycle_ns = 144,
		.phase_ns = 0,
		.platform_ops = &axi_pwm_ops,
		.extra = &axi_pwm_1_extra
	};

	struct no_os_pwm_desc *axi_pwm_1;

	ret = ad796x_init(&ad796x_device, &ad796x_init_param);
	if (ret) {
		pr_err("error: ad796x_init error: %d\n", ret);
		return ret;
	}

	ret = axi_clkgen_init(&ad796x_clk, &clkgen_init);
	if (ret) {
		printf("error: axi_clkgen_init error: %d\n", ret);
		return ret;
	}

	ret = axi_clkgen_set_rate(ad796x_clk, 125000000);
	if (ret) {
		printf("error: axi_clkgen_set_rate error: %d\n", ret);
		return ret;
	}

	ret = no_os_pwm_init(&axi_pwm_0, &axi_pwm_0_init);
	if (ret) {
		printf("error: no_os_pwm_init error: %d\n", ret);
		return ret;
	}

	ret = no_os_pwm_init(&axi_pwm_1, &axi_pwm_1_init);
	if (ret) {
		printf("error: no_os_pwm_init error: %d\n", ret);
		return ret;
	}

	ret = axi_adc_init(&ad796x_core,  &ad796x_core_param);
	if (ret) {
		pr_err("axi_adc_init error: %d\n", ret);
		return ret;
	}

	ret = axi_dmac_init(&ad796x_dmac, &ad796x_dmac_param);
	if (ret) {
		pr_err("axi_dmac_init error: %d\n", ret);
		return ret;
	}

#ifdef IIO_SUPPORT
	struct iio_app_desc *app;
	struct iio_app_init_param app_init_param = { 0 };
	struct iio_axi_adc_desc *iio_axi_adc_desc;
	struct iio_device *dev_desc;

	struct xil_uart_init_param platform_uart_init_par = {
		.type = UART_PS,
		.irq_id = UART_IRQ_ID
	};

	struct no_os_uart_init_param iio_uart_ip = {
		.device_id = UART_DEVICE_ID,
		.irq_id = UART_IRQ_ID,
		.baud_rate = UART_BAUDRATE,
		.size = NO_OS_UART_CS_8,
		.parity = NO_OS_UART_PAR_NO,
		.stop = NO_OS_UART_STOP_1_BIT,
		.extra = &platform_uart_init_par,
		.platform_ops = &xil_uart_ops
	};

	struct iio_axi_adc_init_param iio_axi_adc_init_par;
	struct scan_type init_scan_type = {
		.sign = 's',
		.realbits = 32,
		.storagebits = 32,
		.shift = 0,
		.is_big_endian = false
	};

	struct iio_data_buffer read_buff = {
		.buff = (void *)ADC_DDR_BASEADDR,
		.size = 0xFFFFFFFF,
	};

	iio_axi_adc_init_par = (struct iio_axi_adc_init_param) {
		.rx_adc = ad796x_core,
		.rx_dmac = ad796x_dmac,
		.scan_type_common = &init_scan_type,
		.dcache_invalidate_range = (void (*)(uint32_t,
						     uint32_t))Xil_DCacheInvalidateRange,
	};

	ret = iio_axi_adc_init(&iio_axi_adc_desc, &iio_axi_adc_init_par);
	if (ret < 0)
		return ret;

	iio_axi_adc_get_dev_descriptor(iio_axi_adc_desc, &dev_desc);

	struct iio_app_device devices[] = {
		IIO_APP_DEVICE("ad796x_dev", iio_axi_adc_desc, dev_desc,
			       &read_buff, NULL, NULL),
	};

	app_init_param.devices = devices;
	app_init_param.nb_devices = NO_OS_ARRAY_SIZE(devices);
	app_init_param.uart_init_params = iio_uart_ip;

	ret = iio_app_init(&app, app_init_param);
	if (ret)
		return ret;

	return iio_app_run(app);

#else
	uint32_t *buf;
	int i;

	struct 	axi_dma_transfer read_transfer = {
		.size = AD796X_XFER_SIZE,
		.transfer_done = 0,
		.cyclic = NO,
		.src_addr = 0,
		.dest_addr = (uintptr_t)ADC_DDR_BASEADDR,
	};

	pr_info("Start Caputre\n");

	ret = axi_dmac_transfer_start(ad796x_dmac, &read_transfer);
	if (ret) {
		pr_err("axi_dmac_transfer_start() failed!\n");
		return ret;
	}

	ret = axi_dmac_transfer_wait_completion(ad796x_dmac, 3000);
	if (ret) {
		pr_err("axi_dmac_transfer_wait_completion() failed!\n");
		return ret;
	}

	buf = ADC_DDR_BASEADDR;
	for (i = 0; i < AD796X_SAMPLES; i++, buf++)
		pr_info("CH1: %ld\n", *buf);

	pr_info("\n Capture done.\n");

#endif
	return 0;
}
