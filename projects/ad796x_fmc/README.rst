Evaluating the AD796X
=======================


Contents
--------

.. contents:: Table of Contents
	:depth: 3

Supported Evaluation Boards
---------------------------

* `EVAL-AD7961 <https://www.analog.com/eval-ad7961.html>`_
* `EVAL-AD7960 <https://www.analog.com/eval-ad7960.html>`_

Overview
--------

The AD796x evaluation kit provides the hardware to evaluate the AD796x
ADC.

Refer to the AD796x IC data sheet for detailed information regarding
operation of the IC.

Applications
------------

* Digital imaging systems
* High speed data acquisition
* Spectroscopy
* Test equipment

No-OS Build Setup
-----------------

Please see no-os the wiki `here <https://wiki.analog.com/resources/no-os/build>`_

FPGA firmware (system_top.xsa) has to be copied to the project directory before running make.


No-OS Supported Examples
------------------------

Basic Example
^^^^^^^^^^^^^

In this example, samples are are printed out to the serial port.
the number of samples that are captured and the serial port configuration
can be modified in the project parameters.h file.

By default:

- 1000 samples are captured and printed on each run
- serial port settings: 115200,8n1.

**Build Command**

.. code-block:: bash

	# copy HDL to the project directory
	cp AD796x-PRJ/system_top.xsa project/ad796x/
	# to delete current build
	make reset
	# to build the project
	make
	# to flash the code
	make run


IIO example
^^^^^^^^^^^

Samples are sent to the iio stack, tools like iio_info and
iio_oscilloscope can be used

**Build Command**

.. code-block:: bash

	# copy HDL to the project directory
	cp AD796x-PRJ/system_top.xsa project/ad796x/
	# to delete current build
	make reset
	# to build the project
	make IIOD=y
	# to flash the code
	make run


Supported Platforms
-------------------------

* `EVAL-AD7960 <https://www.analog.com/eval-ad7960.html>`_ + `zedBoard <https://www.avnet.com/wps/portal/us/products/avnet-boards/avnet-board-families/zedboard/>`_
*  `EVAL-AD7961 <https://www.analog.com/eval-ad7961.html>`_ + `zedBoard <https://www.avnet.com/wps/portal/us/products/avnet-boards/avnet-board-families/zedboard/>`_

**Connections**:

* EVAL-AD796x is plugged to the zedboard FMC connector.
* micro-usb connected to the programming port of the zedboard
* micro-usb connected to the uart port of the zedboard


