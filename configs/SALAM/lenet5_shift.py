import m5
from m5.objects import *
from m5.util import *
import ConfigParser
from HWAccConfig import *

class L1Cache(Cache):
	assoc = 2
	tag_latency = 2
	data_latency = 2
	response_latency = 2
	mshrs = 4
	tgts_per_mshr = 20
	
def __init__(self, size, options=None):
		self.size = size
		super(L1Cache, self).__init__()
		pass

def buildlenet5(options, system, clstr):

	local_low = 0x10020000
	local_high = 0x1005d67c
	local_range = AddrRange(local_low, local_high)
	external_range = [AddrRange(0x00000000, local_low-1), AddrRange(local_high+1, 0xFFFFFFFF)]
	system.iobus.master = clstr.local_bus.slave
	clstr._connect_caches(system, options, l2coherent=False)
	gic = system.realview.gic

	# Noncoherent DMA
	clstr.dma = NoncoherentDma(pio_addr=0x10020000, pio_size = 21, gic=gic, int_num=95)
	clstr.dma.cluster_dma = clstr.local_bus.slave
	clstr.dma.max_req_size = 4
	clstr.dma.buffer_size = 128
	clstr.dma.dma = clstr.coherency_bus.slave
	clstr.local_bus.master = clstr.dma.pio
	
	# Stream DMA
	clstr.streamdma = StreamDma(pio_addr=0x10020015, pio_size = 32, gic=gic, max_pending = 32)
	clstr.streamdma.stream_addr = 0x10020015 + 32
	clstr.streamdma.stream_size = 8
	clstr.streamdma.pio_delay = '1ns'
	clstr.streamdma.rd_int = 210
	clstr.streamdma.wr_int = 211
	clstr.streamdma.dma = clstr.coherency_bus.slave
	clstr.local_bus.master = clstr.streamdma.pio
	
	# top Definition
	acc = "top"
	config = "/home/he-man/gem5-SALAM/benchmarks/lenet5/shift/hw/top.ini"
	ir = "/home/he-man/gem5-SALAM/benchmarks/lenet5/shift/hw/top.ll"
	clstr.top = CommInterface(devicename=acc, gic=gic, pio_addr=0x1002003d, pio_size=9)
	AccConfig(clstr.top, config, ir)
	
	# conv0 Definition
	acc = "conv0"
	config = "/home/he-man/gem5-SALAM/benchmarks/lenet5/shift/hw/conv0.ini"
	ir = "/home/he-man/gem5-SALAM/benchmarks/lenet5/shift/hw/conv0.ll"
	clstr.conv0 = CommInterface(devicename=acc, gic=gic, pio_addr=0x10020046, pio_size=1)
	AccConfig(clstr.conv0, config, ir)
	
	# pool0 Definition
	acc = "pool0"
	config = "/home/he-man/gem5-SALAM/benchmarks/lenet5/shift/hw/pool0.ini"
	ir = "/home/he-man/gem5-SALAM/benchmarks/lenet5/shift/hw/pool0.ll"
	clstr.pool0 = CommInterface(devicename=acc, gic=gic, pio_addr=0x1002030b, pio_size=1)
	AccConfig(clstr.pool0, config, ir)
	
	# conv1 Definition
	acc = "conv1"
	config = "/home/he-man/gem5-SALAM/benchmarks/lenet5/shift/hw/conv1.ini"
	ir = "/home/he-man/gem5-SALAM/benchmarks/lenet5/shift/hw/conv1.ll"
	clstr.conv1 = CommInterface(devicename=acc, gic=gic, pio_addr=0x10020850, pio_size=1)
	AccConfig(clstr.conv1, config, ir)
	
	# pool1 Definition
	acc = "pool1"
	config = "/home/he-man/gem5-SALAM/benchmarks/lenet5/shift/hw/pool1.ini"
	ir = "/home/he-man/gem5-SALAM/benchmarks/lenet5/shift/hw/pool1.ll"
	clstr.pool1 = CommInterface(devicename=acc, gic=gic, pio_addr=0x10023469, pio_size=1)
	AccConfig(clstr.pool1, config, ir)
	
	# fc0 Definition
	acc = "fc0"
	config = "/home/he-man/gem5-SALAM/benchmarks/lenet5/shift/hw/fc0.ini"
	ir = "/home/he-man/gem5-SALAM/benchmarks/lenet5/shift/hw/fc0.ll"
	clstr.fc0 = CommInterface(devicename=acc, gic=gic, pio_addr=0x100240f2, pio_size=1)
	AccConfig(clstr.fc0, config, ir)
	
	# fc1 Definition
	acc = "fc1"
	config = "/home/he-man/gem5-SALAM/benchmarks/lenet5/shift/hw/fc1.ini"
	ir = "/home/he-man/gem5-SALAM/benchmarks/lenet5/shift/hw/fc1.ll"
	clstr.fc1 = CommInterface(devicename=acc, gic=gic, pio_addr=0x10053717, pio_size=1)
	AccConfig(clstr.fc1, config, ir)
	
	# top Config
	clstr.top.local = clstr.local_bus.slave
	clstr.top.pio = clstr.local_bus.master
	clstr.top.enable_debug_msgs = False
	
	# conv0 Config
	clstr.conv0.pio = clstr.local_bus.master
	clstr.conv0.stream = clstr.streamdma.stream_in
	clstr.conv0.enable_debug_msgs = False
	
	# Conv0Window (Variable)
	addr = 0x10020047
	spmRange = AddrRange(addr, addr + 0x64)
	clstr.conv0window = ScratchpadMemory(range = spmRange)
	clstr.conv0window.conf_table_reported = False
	clstr.conv0window.ready_mode = False
	clstr.conv0window.port = clstr.local_bus.master
	for i in range(25):
		clstr.conv0.spm = clstr.conv0window.spm_ports
	
	# Conv0Weights (Variable)
	addr = 0x100200ab
	spmRange = AddrRange(addr, addr + 0x258)
	clstr.conv0weights = ScratchpadMemory(range = spmRange)
	clstr.conv0weights.conf_table_reported = False
	clstr.conv0weights.ready_mode = False
	clstr.conv0weights.port = clstr.local_bus.master
	for i in range(25):
		clstr.conv0.spm = clstr.conv0weights.spm_ports
	
	# Conv0LUT (Variable)
	addr = 0x10020303
	spmRange = AddrRange(addr, addr + 0x4)
	clstr.conv0lut = ScratchpadMemory(range = spmRange)
	clstr.conv0lut.conf_table_reported = False
	clstr.conv0lut.ready_mode = False
	clstr.conv0lut.port = clstr.local_bus.master
	for i in range(1):
		clstr.conv0.spm = clstr.conv0lut.spm_ports
	
	# Conv0Out (Stream Variable)
	addr = 0x10020307
	clstr.conv0out = StreamBuffer(stream_address = addr, stream_size = 4, buffer_size = 16)
	clstr.conv0.stream = clstr.conv0out.stream_in
	clstr.pool0.stream = clstr.conv0out.stream_out
	
	
	# pool0 Config
	clstr.pool0.pio = clstr.local_bus.master
	clstr.pool0.enable_debug_msgs = False
	
	# Pool0Window (Variable)
	addr = 0x1002030c
	spmRange = AddrRange(addr, addr + 0x540)
	clstr.pool0window = ScratchpadMemory(range = spmRange)
	clstr.pool0window.conf_table_reported = False
	clstr.pool0window.ready_mode = False
	clstr.pool0window.port = clstr.local_bus.master
	for i in range(4):
		clstr.pool0.spm = clstr.pool0window.spm_ports
	
	# Pool0Out (Stream Variable)
	addr = 0x1002084c
	clstr.pool0out = StreamBuffer(stream_address = addr, stream_size = 4, buffer_size = 16)
	clstr.pool0.stream = clstr.pool0out.stream_in
	clstr.conv1.stream = clstr.pool0out.stream_out
	
	
	# conv1 Config
	clstr.conv1.pio = clstr.local_bus.master
	clstr.conv1.enable_debug_msgs = False
	
	# Conv1Window (Variable)
	addr = 0x10020851
	spmRange = AddrRange(addr, addr + 0x690)
	clstr.conv1window = ScratchpadMemory(range = spmRange)
	clstr.conv1window.conf_table_reported = False
	clstr.conv1window.ready_mode = False
	clstr.conv1window.port = clstr.local_bus.master
	for i in range(150):
		clstr.conv1.spm = clstr.conv1window.spm_ports
	
	# Conv1Weights (Variable)
	addr = 0x10020ee1
	spmRange = AddrRange(addr, addr + 0x2580)
	clstr.conv1weights = ScratchpadMemory(range = spmRange)
	clstr.conv1weights.conf_table_reported = False
	clstr.conv1weights.ready_mode = False
	clstr.conv1weights.port = clstr.local_bus.master
	for i in range(150):
		clstr.conv1.spm = clstr.conv1weights.spm_ports
	
	# Conv1LUT (Variable)
	addr = 0x10023461
	spmRange = AddrRange(addr, addr + 0x4)
	clstr.conv1lut = ScratchpadMemory(range = spmRange)
	clstr.conv1lut.conf_table_reported = False
	clstr.conv1lut.ready_mode = False
	clstr.conv1lut.port = clstr.local_bus.master
	for i in range(1):
		clstr.conv1.spm = clstr.conv1lut.spm_ports
	
	# Conv1Out (Stream Variable)
	addr = 0x10023465
	clstr.conv1out = StreamBuffer(stream_address = addr, stream_size = 4, buffer_size = 16)
	clstr.conv1.stream = clstr.conv1out.stream_in
	clstr.pool1.stream = clstr.conv1out.stream_out
	
	
	# pool1 Config
	clstr.pool1.pio = clstr.local_bus.master
	clstr.pool1.enable_debug_msgs = False
	
	# Pool1Window (Variable)
	addr = 0x1002346a
	spmRange = AddrRange(addr, addr + 0xc80)
	clstr.pool1window = ScratchpadMemory(range = spmRange)
	clstr.pool1window.conf_table_reported = False
	clstr.pool1window.ready_mode = False
	clstr.pool1window.port = clstr.local_bus.master
	for i in range(4):
		clstr.pool1.spm = clstr.pool1window.spm_ports
	
	# Pool1LUT (Variable)
	addr = 0x100240ea
	spmRange = AddrRange(addr, addr + 0x4)
	clstr.pool1lut = ScratchpadMemory(range = spmRange)
	clstr.pool1lut.conf_table_reported = False
	clstr.pool1lut.ready_mode = False
	clstr.pool1lut.port = clstr.local_bus.master
	for i in range(1):
		clstr.pool1.spm = clstr.pool1lut.spm_ports
	
	# Pool1Out (Stream Variable)
	addr = 0x100240ee
	clstr.pool1out = StreamBuffer(stream_address = addr, stream_size = 4, buffer_size = 16)
	clstr.pool1.stream = clstr.pool1out.stream_in
	clstr.fc0.stream = clstr.pool1out.stream_out
	
	
	# fc0 Config
	clstr.fc0.pio = clstr.local_bus.master
	clstr.fc0.enable_debug_msgs = False
	
	# fc0Window (Variable)
	addr = 0x100240f3
	spmRange = AddrRange(addr, addr + 0x640)
	clstr.fc0window = ScratchpadMemory(range = spmRange)
	clstr.fc0window.conf_table_reported = False
	clstr.fc0window.ready_mode = False
	clstr.fc0window.port = clstr.local_bus.master
	for i in range(80):
		clstr.fc0.spm = clstr.fc0window.spm_ports
	
	# fc0Weights (Variable)
	addr = 0x10024733
	spmRange = AddrRange(addr, addr + 0x2ee00)
	clstr.fc0weights = ScratchpadMemory(range = spmRange)
	clstr.fc0weights.conf_table_reported = False
	clstr.fc0weights.ready_mode = False
	clstr.fc0weights.port = clstr.local_bus.master
	for i in range(80):
		clstr.fc0.spm = clstr.fc0weights.spm_ports
	
	# fc0LUT (Variable)
	addr = 0x10053533
	spmRange = AddrRange(addr, addr + 0x4)
	clstr.fc0lut = ScratchpadMemory(range = spmRange)
	clstr.fc0lut.conf_table_reported = False
	clstr.fc0lut.ready_mode = False
	clstr.fc0lut.port = clstr.local_bus.master
	for i in range(1):
		clstr.fc0.spm = clstr.fc0lut.spm_ports
	
	# fc0Output (Variable)
	addr = 0x10053537
	spmRange = AddrRange(addr, addr + 0x1e0)
	clstr.fc0output = ScratchpadMemory(range = spmRange)
	clstr.fc0output.conf_table_reported = False
	clstr.fc0output.ready_mode = True
	clstr.fc0output.port = clstr.local_bus.master
	for i in range(80):
		clstr.fc0.spm = clstr.fc0output.spm_ports
		clstr.fc1.spm = clstr.fc0output.spm_ports
	
	# fc1 Config
	clstr.fc1.pio = clstr.local_bus.master
	clstr.fc1.stream = clstr.streamdma.stream_out
	clstr.fc1.enable_debug_msgs = True
	
	# fc1Window (Variable)
	addr = 0x10053718
	spmRange = AddrRange(addr, addr + 0x1e0)
	clstr.fc1window = ScratchpadMemory(range = spmRange)
	clstr.fc1window.conf_table_reported = False
	clstr.fc1window.ready_mode = False
	clstr.fc1window.port = clstr.local_bus.master
	for i in range(120):
		clstr.fc1.spm = clstr.fc1window.spm_ports
	
	# fc1Weights (Variable)
	addr = 0x100538f8
	spmRange = AddrRange(addr, addr + 0x9d80)
	clstr.fc1weights = ScratchpadMemory(range = spmRange)
	clstr.fc1weights.conf_table_reported = False
	clstr.fc1weights.ready_mode = False
	clstr.fc1weights.port = clstr.local_bus.master
	for i in range(120):
		clstr.fc1.spm = clstr.fc1weights.spm_ports
	
	# fc1LUT (Variable)
	addr = 0x1005d678
	spmRange = AddrRange(addr, addr + 0x4)
	clstr.fc1lut = ScratchpadMemory(range = spmRange)
	clstr.fc1lut.conf_table_reported = False
	clstr.fc1lut.ready_mode = False
	clstr.fc1lut.port = clstr.local_bus.master
	for i in range(1):
		clstr.fc1.spm = clstr.fc1lut.spm_ports
	
def makeHWAcc(options, system):

	system.lenet5 = AccCluster()
	buildlenet5(options, system, system.lenet5)

