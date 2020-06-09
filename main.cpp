#include "mbed.h"
#include "mbed_rpc.h"
#include "fsl_port.h"
#include "fsl_gpio.h"
#define UINT14_MAX        16383
// FXOS8700CQ I2C address
#define FXOS8700CQ_SLAVE_ADDR0 (0x1E<<1) // with pins SA0=0, SA1=0
#define FXOS8700CQ_SLAVE_ADDR1 (0x1D<<1) // with pins SA0=1, SA1=0
#define FXOS8700CQ_SLAVE_ADDR2 (0x1C<<1) // with pins SA0=0, SA1=1
#define FXOS8700CQ_SLAVE_ADDR3 (0x1F<<1) // with pins SA0=1, SA1=1
// FXOS8700CQ internal register addresses
#define FXOS8700Q_STATUS 0x00
#define FXOS8700Q_OUT_X_MSB 0x01
#define FXOS8700Q_OUT_Y_MSB 0x03
#define FXOS8700Q_OUT_Z_MSB 0x05
#define FXOS8700Q_M_OUT_X_MSB 0x33
#define FXOS8700Q_M_OUT_Y_MSB 0x35
#define FXOS8700Q_M_OUT_Z_MSB 0x37
#define FXOS8700Q_WHOAMI 0x0D
#define FXOS8700Q_XYZ_DATA_CFG 0x0E
#define FXOS8700Q_CTRL_REG1 0x2A
#define FXOS8700Q_M_CTRL_REG1 0x5B
#define FXOS8700Q_M_CTRL_REG2 0x5C
#define FXOS8700Q_WHOAMI_VAL 0xC7
RawSerial pc(USBTX, USBRX);
RawSerial xbee(D12, D11);
I2C i2c(PTD9, PTD8);
int m_addr = FXOS8700CQ_SLAVE_ADDR1;

void FXOS8700CQ_readRegs(int addr, uint8_t * data, int len);
void FXOS8700CQ_writeRegs(uint8_t * data, int len);
void getAcc();
void getTimes(Arguments *in, Reply *out);
RPCFunction rpcAcc(&getTimes, "getTimes");

int collectTime = 0;	// how many time ata is collecte.
float Acc[15][3];
bool Tilt[15];
EventQueue queue(32 * EVENTS_EVENT_SIZE);
Thread t(osPriorityHigh);
EventQueue queue2(32 * EVENTS_EVENT_SIZE);
Thread t2;	// for accelerameter

void xbee_rx_interrupt(void);
void xbee_rx(void);
void reply_messange(char *xbee_reply, char *messange);
void check_addr(char *xbee_reply, char *messenger);

int main() {
	pc.baud(9600);
	char xbee_reply[4];
	// XBee setting
	xbee.baud(9600);
	xbee.printf("+++");
	xbee_reply[0] = xbee.getc();
	xbee_reply[1] = xbee.getc();
	if (xbee_reply[0] == 'O' && xbee_reply[1] == 'K') {
		pc.printf("enter AT mode.\r\n");
		xbee_reply[0] = '\0';
		xbee_reply[1] = '\0';
	}
	xbee.printf("ATMY 0x220\r\n");
	reply_messange(xbee_reply, "setting MY : 0x220");
	xbee.printf("ATDL 0x120\r\n");
	reply_messange(xbee_reply, "setting DL : 0x120");
	xbee.printf("ATID 0x1\r\n");
	reply_messange(xbee_reply, "setting PAN ID : 0x1");
	xbee.printf("ATWR\r\n");
	reply_messange(xbee_reply, "write config");
	xbee.printf("ATMY\r\n");
	check_addr(xbee_reply, "MY");
	xbee.printf("ATDL\r\n");
	check_addr(xbee_reply, "DL");
	xbee.printf("ATCN\r\n");
	reply_messange(xbee_reply, "exit AT mode");
	xbee.getc();
	// start
	pc.printf("start\r\n");
	t.start(callback(&queue, &EventQueue::dispatch_forever));

	uint8_t data[2];
	// Enable the FXOS8700Q
	FXOS8700CQ_readRegs(FXOS8700Q_CTRL_REG1, &data[1], 1);
	data[1] |= 0x01;
	data[0] = FXOS8700Q_CTRL_REG1;
	FXOS8700CQ_writeRegs(data, 2);

	t2.start(callback(&queue2, &EventQueue::dispatch_forever));
	queue2.call_every(100, getAcc);	// every 0.1 sec, get Acc
	// Setup a serial interrupt function of receiving data from xbee
	xbee.attach(xbee_rx_interrupt, Serial::RxIrq);
}

void xbee_rx_interrupt(void)
{
	xbee.attach(NULL, Serial::RxIrq); // detach interrupt
	queue.call(&xbee_rx);
}

void xbee_rx(void)
{
	char buf[100] = { 0 };
	char outbuf[100] = { 0 };
	while (xbee.readable()) {
		for (int i = 0; ; i++) {
			char recv = xbee.getc();
			if (recv == '\r') {
				break;
			}
			buf[i] = pc.putc(recv);
		}
		RPC::call(buf, outbuf);
		pc.printf("%s\r\n", outbuf);
		wait(0.1);
	}
	xbee.attach(xbee_rx_interrupt, Serial::RxIrq); // reattach interrupt
}

void reply_messange(char *xbee_reply, char *messange) {
	xbee_reply[0] = xbee.getc();
	xbee_reply[1] = xbee.getc();
	xbee_reply[2] = xbee.getc();
	if (xbee_reply[1] == 'O' && xbee_reply[2] == 'K') {
		pc.printf("%s\r\n", messange);
		xbee_reply[0] = '\0';
		xbee_reply[1] = '\0';
		xbee_reply[2] = '\0';
	}
}
void check_addr(char *xbee_reply, char *messenger) {
	xbee_reply[0] = xbee.getc();
	xbee_reply[1] = xbee.getc();
	xbee_reply[2] = xbee.getc();
	xbee_reply[3] = xbee.getc();
	pc.printf("%s = %c%c%c\r\n", messenger, xbee_reply[1], xbee_reply[2], xbee_reply[3]);
	xbee_reply[0] = '\0';
	xbee_reply[1] = '\0';
	xbee_reply[2] = '\0';
	xbee_reply[3] = '\0';
}
void getAcc() {
	int16_t acc16;
	float T[3];
	uint8_t res[6];
	FXOS8700CQ_readRegs(FXOS8700Q_OUT_X_MSB, res, 6);

	acc16 = (res[0] << 6) | (res[1] >> 2);
	if (acc16 > UINT14_MAX / 2)
		acc16 -= UINT14_MAX;
	T[0] = ((float)acc16) / 4096.0f;

	acc16 = (res[2] << 6) | (res[3] >> 2);
	if (acc16 > UINT14_MAX / 2)
		acc16 -= UINT14_MAX;
	T[1] = ((float)acc16) / 4096.0f;

	acc16 = (res[4] << 6) | (res[5] >> 2);
	if (acc16 > UINT14_MAX / 2)
		acc16 -= UINT14_MAX;
	T[2] = ((float)acc16) / 4096.0f;

	if (collectTime > 10)	collectTime = 0;
	Acc[collectTime][0] = T[0];
	Acc[collectTime][1] = T[1];
	Acc[collectTime][2] = T[2];
	if (T[0] > 0.5 || T[0] < -0.5 || T[1] > 0.5 || T[1] < -0.5) 
		Tilt[collectTime] = true;	// tilt
	else 
		Tilt[collectTime] = false;
	collectTime++;
}
void getTimes(Arguments *in, Reply *out) {
	pc.printf("\r\n%d\r\n", collectTime);
	xbee.printf("%d\r\n", collectTime);
	for (int i = 0; i < collectTime; i++) {
		pc.printf("%1.3f %1.3f %1.3f %d\r\n", \
		Acc[i][0], Acc[i][1], Acc[i][2], Tilt[i]);
		xbee.printf("%1.3f %1.3f %1.3f %d\r\n", \
		Acc[i][0], Acc[i][1], Acc[i][2], Tilt[i]);
	}
	collectTime = 0;
	for (int i = 0; i < 15; i++) {
		Acc[i][0] = 0;
		Acc[i][1] = 0;
		Acc[i][2] = 0;
		Tilt[i] = false;
	}
}
void FXOS8700CQ_readRegs(int addr, uint8_t * data, int len) {
	char t = addr;
	i2c.write(m_addr, &t, 1, true);
	i2c.read(m_addr, (char *)data, len);
}

void FXOS8700CQ_writeRegs(uint8_t * data, int len) {
	i2c.write(m_addr, (char *)data, len);
}