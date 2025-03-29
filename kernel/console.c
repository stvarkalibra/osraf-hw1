// Console input and output.
// Input is from the keyboard or serial port.
// Output is written to the screen and serial port.

#include "types.h"
#include "defs.h"
#include "traps.h"
#include "spinlock.h"
#include "file.h"
#include "memlayout.h"
#include "proc.h"
#include "x86.h"
#include "printf.h"

#include <stdarg.h>

static void consputc(int);

static int panicked = 0;

static struct {
	struct spinlock lock;
	int locking;
} cons;



// Print to the console. only understands %d, %x, %p, %s.
void
cprintf(char *fmt, ...)
{
	int locking;
	va_list args;

	locking = cons.locking;
	if(locking)
		acquire(&cons.lock);

	if (fmt == 0)
		panic("null fmt");

	va_start(args, fmt);

	fnvprintf(consputc, fmt, args);

	va_end(args);

	if(locking)
		release(&cons.lock);
}

void
panic(char *s)
{
	int i;
	uint pcs[10];

	cli();
	cons.locking = 0;
	// use lapiccpunum so that we can call panic from mycpu()
	cprintf("lapicid %d: panic: ", lapicid());
	cprintf(s);
	cprintf("\n");
	getcallerpcs(&s, pcs);
	for(i=0; i<10; i++)
		cprintf(" %p", pcs[i]);
	panicked = 1; // freeze other CPU
	for(;;)
		;
}

#define BACKSPACE 0x100
#define CRTPORT 0x3d4
#define ALT_T_KEY 0x1C
static ushort *crt = (ushort*)P2V(0xb8000);  // CGA memory
static int table = 0;
static int cnt = 0;
static int color = 0x0700;
static int pamti[99];

void change_color(int cnt){

	if(cnt == 0){
		color = 0x0700;
	} else if(cnt == 1){
		color = 0xfd00;
	} else if(cnt == 2){
		color = 0xbc00;
	} else if (cnt == 3){
		color = 0xef00;
	}
	int bojac = 0;
	while (bojac < 2000){
		crt[bojac] &= 0x00ff;
		crt[bojac++] |= color;
	}


}

void zapamti(int pos){
	int tmp_pos=pos-79;
	int brojac=98;
	for(int i=0;i<9;i++){
		for(int j=0;j<11;j++){
			pamti[brojac--]=crt[tmp_pos+j];
		}
		tmp_pos-=80;
	}
}

void draw(int cnt, int pos){
	int br_redova = 9;
	int br_kolone = 11;
	int tmp = pos;
	if(table){
		if(tmp % 80 > 69){
			pos -= 12;
		}
		while(br_redova > 0){
			pos -= 79;
			if(br_redova % 2 == 1){
				for(int i = 0; i < br_kolone; i++){
					crt[pos++] = '-' | 0x8000;
				}
				pos -= 12;
			} else{
				if(br_redova == 8){
					crt[pos++]='|' | 0x8000;
					crt[pos++]=' ' | 0x8000;
					if(cnt == 3){
						crt[pos++]='W' | 0x2000;
						crt[pos++]='H' | 0x2000;
						crt[pos++]='T' | 0x2000;
						crt[pos++]=' ' | 0x2000;
						crt[pos++]='Y' | 0x2000;
						crt[pos++]='L' | 0x2000;
						crt[pos++]='W' | 0x2000;
					} else{
						crt[pos++]='W' | 0x8000;
						crt[pos++]='H' | 0x8000;
						crt[pos++]='T' | 0x8000;
						crt[pos++]=' ' | 0x8000;
						crt[pos++]='Y' | 0x8000;
						crt[pos++]='L' | 0x8000;
						crt[pos++]='W' | 0x8000;
					}

					crt[pos++]=' ' | 0x8000;
					crt[pos++]='|' | 0x8000;
					pos -= 12;
				}
				else if(br_redova == 6){
					crt[pos++]='|' | 0x8000;
					crt[pos++]=' ' | 0x8000;

					if(cnt == 2){
						crt[pos++]='r' | 0x2000;
						crt[pos++]='e' | 0x2000;
						crt[pos++]='d' | 0x2000;
						crt[pos++]=' ' | 0x2000;
						crt[pos++]='C' | 0x2000;
						crt[pos++]='Y' | 0x2000;
						crt[pos++]='N' | 0x2000;
					} else{
						crt[pos++]='r' | 0x8000;
						crt[pos++]='e' | 0x8000;
						crt[pos++]='d' | 0x8000;
						crt[pos++]=' ' | 0x8000;
						crt[pos++]='C' | 0x8000;
						crt[pos++]='Y' | 0x8000;
						crt[pos++]='N' | 0x8000;
					}

					crt[pos++]=' ' | 0x8000;
					crt[pos++]='|' | 0x8000;
					pos -= 12;
				} else if(br_redova == 4){
					crt[pos++]='|' | 0x8000;
					crt[pos++]=' ' | 0x8000;

					if(cnt == 1){
						crt[pos++]='m' | 0x2000;
						crt[pos++]='g' | 0x2000;
						crt[pos++]='t' | 0x2000;
						crt[pos++]=' ' | 0x2000;
						crt[pos++]='W' | 0x2000;
						crt[pos++]='H' | 0x2000;
						crt[pos++]='T' | 0x2000;
					} else{
						crt[pos++]='m' | 0x8000;
						crt[pos++]='g' | 0x8000;
						crt[pos++]='t' | 0x8000;
						crt[pos++]=' ' | 0x8000;
						crt[pos++]='W' | 0x8000;
						crt[pos++]='H' | 0x8000;
						crt[pos++]='T' | 0x8000;
					}

					crt[pos++]=' ' | 0x8000;
					crt[pos++]='|' | 0x8000;
					pos -= 12;
				} else if(br_redova == 2){
					crt[pos++]='|' | 0x8000;
					crt[pos++]=' ' | 0x8000;

					if(cnt == 0){
						crt[pos++]='l' | 0x2000;
						crt[pos++]='g' | 0x2000;
						crt[pos++]='r' | 0x2000;
						crt[pos++]=' ' | 0x2000;
						crt[pos++]='b' | 0x2000;
						crt[pos++]='l' | 0x2000;
						crt[pos++]='k' | 0x2000;
					} else{
						crt[pos++]='l' | 0x8000;
						crt[pos++]='g' | 0x8000;
						crt[pos++]='r' | 0x8000;
						crt[pos++]=' ' | 0x8000;
						crt[pos++]='b' | 0x8000;
						crt[pos++]='l' | 0x8000;
						crt[pos++]='k' | 0x8000;
					}

					crt[pos++]=' ' | 0x8000;
					crt[pos++]='|' | 0x8000;
					pos -= 12;
				}
			}
			br_redova--;
		}
		pos = tmp;
	} else if(!table){
		br_redova = 9;
		tmp = pos;
		if(tmp % 80 > 69){
			pos -= 12;
		}
		int brojac=98;
		while(br_redova > 0){

			pos -= 79;
			for(int i = 0; i < 11; i++){
				crt[pos++] = (pamti[brojac--] & 0xff) | color;
			}
			pos -= 12;
			br_redova--;
		}
		pos = tmp;
	}
}

static void
cgaputc(int c)
{
	int pos;

	// Cursor position: col + 80*row.
	outb(CRTPORT, 14);
	pos = inb(CRTPORT+1) << 8;
	outb(CRTPORT, 15);
	pos |= inb(CRTPORT+1);

	if(c == '\n'){
		if(!table){
			pos += 80 - pos%80;
		} else{
			cgaputc(ALT_T_KEY);
		}
	}
	else if(c == BACKSPACE){
		if(pos > 0) --pos;
	}else if(c == ALT_T_KEY){
		if(!table){
			table = 1;
			if(pos%80>69){
				zapamti(pos-12);
			}else{
				zapamti(pos);
			}

		} else if(table){
			table = 0;
		}
		draw(cnt, pos);
	} else{
		if(!table){
			crt[pos++] = ((c&0xff) & 0x00ff)| color;  // black on white
		} else{
			if(c == 'w'){
				cnt--;
			}
			if(c == 's'){
				cnt++;
			}
			cnt = (cnt + 4) % 4;
			change_color(cnt);
			draw(cnt, pos);

		}
	}

	if(pos < 0 || pos > 25*80)
		panic("pos under/overflow");

	if((pos/80) >= 24){  // Scroll up.
		memmove(crt, crt+80, sizeof(crt[0])*23*80);
		pos -= 80;
		memset(crt+pos, 0, sizeof(crt[0])*(24*80 - pos));
		change_color(cnt);
	}

	outb(CRTPORT, 14);
	outb(CRTPORT+1, pos>>8);
	outb(CRTPORT, 15);
	outb(CRTPORT+1, pos);
	crt[pos] = ' ' | color;
}

void
consputc(int c)
{
	if(panicked){
		cli();
		for(;;)
			;
	}

	if(c == BACKSPACE){
		uartputc('\b'); uartputc(' '); uartputc('\b');
		cgaputc(c);
	} else if(c == ALT_T_KEY){
		cgaputc(ALT_T_KEY);
	} else{
		uartputc(c);
		cgaputc(c);
	}
}

#define INPUT_BUF 128

struct {
	char buf[INPUT_BUF];
	uint r;  // Read index
	uint w;  // Write index
	uint e;  // Edit index
} input;

#define C(x)  ((x)-'@')  // Control-x

void
consoleintr(int (*getc)(void))
{
	int c, doprocdump = 0;

	acquire(&cons.lock);
	while((c = getc()) >= 0){
		switch(c){
			case C('P'):  // Process listing.
				// procdump() locks cons.lock indirectly; invoke later
				doprocdump = 1;
				break;
			case C('U'):  // Kill line.
				while(input.e != input.w &&
					input.buf[(input.e-1) % INPUT_BUF] != '\n'){
					input.e--;
				consputc(BACKSPACE);
					}
					break;
			case C('H'): case '\x7f':  // Backspace
				if(input.e != input.w){
					input.e--;
					consputc(BACKSPACE);
				}
				break;
			case '\n':
				if(table){
					consputc(ALT_T_KEY);
					break;
				}
				if(c != 0 && input.e-input.r < INPUT_BUF){
					c = (c == '\r') ? '\n' : c;
					input.buf[input.e++ % INPUT_BUF] = c;
					consputc(c);
					if(c == '\n' || c == C('D') || input.e == input.r+INPUT_BUF){
						input.w = input.e;
						wakeup(&input.r);
					}
				}
				break;
			case ALT_T_KEY:
				consputc(ALT_T_KEY);
				break;
			default:
				if(c != 0 && input.e-input.r < INPUT_BUF){
					c = (c == '\r') ? '\n' : c;
					input.buf[input.e++ % INPUT_BUF] = c;
					consputc(c);
					if(c == '\n' || c == C('D') || input.e == input.r+INPUT_BUF){
						input.w = input.e;
						wakeup(&input.r);
					}
				}
				break;
		}
	}
	release(&cons.lock);
	if(doprocdump) {
		procdump();  // now call procdump() wo. cons.lock held
	}
}

int
consoleread(struct inode *ip, char *dst, int n)
{
	uint target;
	int c;

	iunlock(ip);
	target = n;
	acquire(&cons.lock);
	while(n > 0){
		while(input.r == input.w){
			if(myproc()->killed){
				release(&cons.lock);
				ilock(ip);
				return -1;
			}
			sleep(&input.r, &cons.lock);
		}
		c = input.buf[input.r++ % INPUT_BUF];
		if(c == C('D')){  // EOF
			if(n < target){
				// Save ^D for next time, to make sure
				// caller gets a 0-byte result.
				input.r--;
			}
			break;
		}
		*dst++ = c;
		--n;
		if(c == '\n')
			break;
	}
	release(&cons.lock);
	ilock(ip);

	return target - n;
}

int
consolewrite(struct inode *ip, char *buf, int n)
{
	int i;

	iunlock(ip);
	acquire(&cons.lock);
	for(i = 0; i < n; i++)
		consputc(buf[i] & 0xff);
	release(&cons.lock);
	ilock(ip);

	return n;
}

void
consoleinit(void)
{
	initlock(&cons.lock, "console");

	devsw[CONSOLE].write = consolewrite;
	devsw[CONSOLE].read = consoleread;
	cons.locking = 1;

	ioapicenable(IRQ_KBD, 0);
}
