// evan kahn
// e155, final project

#include "easyPIO.h"
#include <string.h>
#include <unistd.h>
#include <stdint.h>



#define PI 3.14159265

#define LEDOUT 0
#define DEBUG 1

#define RPM 500

#define RGB(r, g, b) ((struct pixel) {r, g, b})

#define RED RGB(0xff, 0, 0)
#define GREEN RGB(0, 0xff, 0)
#define BLUE RGB(0, 0, 0xff)

#define WHITE RGB(0xff,0xff,0xff)
#define WHITE_64 RGB(0x80,0x80,0x80)

uint8_t pixels, revs, total_frames, speed;

struct pixel {
	unsigned char r, g, b;
};

//help with pixel_data structure from eric mueller; adapted from following code:
//   	https://gist.github.com/hmc-cs-emueller/c4ebbe9f3b4064fed16a

struct pixel *getPixel(struct pixel *frames, unsigned i, unsigned j, unsigned k){
	unsigned addr = i + j * pixels + k * pixels * revs;
// 	11 / 1
// 	51 / 0
// 	printf("start%d %d %u ", i,j,addr);
	return frames + addr;
}

void setLED (struct pixel *p){
	if (LEDOUT){
		spiSendReceive(0xff);
		spiSendReceive(p->b);
		spiSendReceive(p->g);
		spiSendReceive(p->r);
	}
}

void initFrame (){
	int i;
	if (LEDOUT) for (i=0;i<4;i++) spiSendReceive(0x00);
}

void endFrame (){
	if (LEDOUT){
		spiSendReceive(0xff);
		spiSendReceive(0xff);
		spiSendReceive(0xff);
		spiSendReceive(0xff);
	}
}


void lightUp(struct pixel *frames, int revnum, int framenum){
	int i;
	initFrame();
	struct pixel *p;
	if (DEBUG == 1) printf("\nbegin horiz %d\n", revnum);
	for (i=0;i<pixels;i++){	
		p = getPixel(frames, i, revnum, framenum);
		if (DEBUG){
			printf("%02d 0x%02x%02x%02x ",i,p->b,p->g,p->r);
		}
		setLED(p);
		if (DEBUG && ((i+1) % 8 == 0)) printf("\n");
	}
	endFrame();

}


int main() {

	if (LEDOUT){
		pioInit();
		pinMode(20, OUTPUT);
		spiInit(1000000);
		digitalWrite(20, 1);
		delayMicros(1000000);
	}
	
	int revnum = 0;

	//	frames per revolution: FRAMES
	// 	revolutions per second: RPM/60
	// 	seconds per revolution: 60/RPM
	// 	seconds per frame: 60/FRAMES/RPM
	// 	microseconds per frame: 1000000 * 60 / FRAMES / RPM

	unsigned revolutions = 0;

	struct pixel *frames;
	
	char fname[64];
	int done = 0;
	FILE *fp;
	do{
		printf("Enter filename: ");
		scanf("%s",fname);
		strcat(fname, ".led");

	if( access( fname, F_OK ) != -1 ) {	
		fp = fopen(fname, "r");
		fscanf(fp, "%c%c%c%c", &pixels, &revs, &speed, &total_frames);
		char junk;
		fscanf(fp, "%c%c%c%c", &junk, &junk, &junk, &junk);
		
		unsigned long size = pixels * revs * total_frames * sizeof(struct pixel);
		frames = malloc(size);
		if (!frames){
			printf("malloc failed");
			exit(1);
		}
		memset(frames, 0x00, size);
		fread(frames, sizeof(char), size, fp);
		done = 1;

	}else{
		printf("not found.\n");
	}
	}while (!done);
	
	int delay = 1000000 * 60 / (revs * RPM);
	int framenum = 0;

	while(revolutions < 2){

		lightUp(frames,revnum, framenum);
		
		if (LEDOUT) delayMicros(delay);
		
		revnum = ((revnum + 1) % revs);
		
		if (revnum == revs - 1){
			++revolutions;
			exit(1);
		}
		if (revolutions == speed){
			revolutions = 0;
			framenum = ((framenum + 1)%total_frames);
		}
	}
	
	printf("\n");
	return(0);
}

